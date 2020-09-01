/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2016  Benoit Blancard <benblan@users.sourceforge.net>*
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#include "common.h"
#include "utils/log.h"
#include "app.h"
#include "model/vehicle.h"
#include "core/gamesession.h"
#include "mission.h"

uint16 SFXObject::sfxIdCnt = 0;
const int Static::kStaticOrientation1 = 0;
const int Static::kStaticOrientation2 = 2;

MapObject::MapObject(uint16 anId, int m, ObjectNature aNature):
    size_x_(1), size_y_(1), size_z_(2),
    map_(m), frame_(0), elapsed_carry_(0),
    frames_per_sec_(8),
    dir_(0),
    time_show_anim_(-1), time_showing_anim_(-1),
    is_frame_drawn_(false),
    state_(0xFFFFFFFF)
{
    nature_ = aNature;
    id_ = anId;
}

const char* MapObject::natureName() {
    switch (nature_) {
    case kNaturePed:
        return "Ped";
    case kNatureWeapon:
        return "Weapon";
    case kNatureStatic:
        return "Static";
    case kNatureVehicle:
        return "Vehicle";
    default:
        return "Undefined";
    }
}

void MapObject::setOffX(int n)
{
    pos_.ox = n;
    while (pos_.ox < 0) {
        pos_.ox += 256;
        pos_.tx--;
    }
    while (pos_.ox > 255) {
        pos_.ox -= 256;
        pos_.tx++;
    }
}

void MapObject::setOffY(int n)
{
    pos_.oy = n;
    while (pos_.oy < 0) {
        pos_.oy += 256;
        pos_.ty--;
    }
    while (pos_.oy > 255) {
        pos_.oy -= 256;
        pos_.ty++;
    }
}

void MapObject::setOffZ(int n)
{
    pos_.oz = n;
    while (pos_.oz < 0) {
        pos_.oz += 128;
        pos_.tz--;
    }
    while (pos_.oz > 127) {
        pos_.oz -= 128;
        pos_.tz++;
    }
}

void MapObject::addOffs(int &x, int &y)
{
    x += ((pos_.ox - pos_.oy) * (TILE_WIDTH / 2)) / 256;
    y += ((pos_.ox + pos_.oy) * (TILE_HEIGHT / 3)) / 256;
    y -= (pos_.oz * (TILE_HEIGHT / 3)) / 128;
}

bool MapObject::animate(int elapsed)
{
    int frame_tics_ = 1000 / frames_per_sec_;
    int total_elapsed = elapsed + elapsed_carry_;
    elapsed_carry_ = total_elapsed % frame_tics_;
    int framewas = frame_;
    bool changed = true;
    frame_ += (total_elapsed / frame_tics_);
    if (framewas == frame_)
        changed = false;
    frame_ %= frames_per_sec_ << 3;
    return changed;
}

void MapObject::setDirection(int dir) {
    assert(dir >= 0);
    dir_ = dir;
}

/* NOTE posx = targetx - objx
 * posy = targety - objy
 * if dir == NULL, object callers dir_ will be set
 */
void MapObject::setDirection(int posx, int posy, int * dir) {

    int newDirection = -1;

    if (posx == 0) {
        if (posy < 0) {
            newDirection = 128;
        } else if (posy > 0) {
            newDirection = 0;
        }
    } else if (posy == 0) {
        if (posx > 0) {
            newDirection = 64;
        } else if (posx < 0) {
            newDirection = 192;
        }
    } else if (posx < 0) {
        if (posy > 0) {
            posx = -posx;
            newDirection = (int)((128.0
                * atan((double)posy / (double)posx)) / PI + 192.0);
        } else { // posy < 0
            int swapx = -posx;
            posx = -posy;
            posy = swapx;
            newDirection = (int)((128.0
                * atan((double)posy / (double)posx)) / PI + 128.0);
        }
    } else if (posx > 0 && posy < 0) {
        posy = -posy;
        newDirection = (int)((128.0
            * atan((double)posy / (double)posx)) / PI + 64.0);
    } else { // posx > 0 && posy > 0
        int swapx = posx;
        posx = posy;
        posy = swapx;
        newDirection = (int)((128.0
            * atan((double)posy / (double)posx)) / PI);
    }
    if (newDirection != -1) {
        if (dir == NULL)
            dir_ = newDirection;
        else
            *dir = newDirection;
    }
}

void MapObject::setDirectionTowardObject(const MapObject &object) {
    WorldPoint objectPos(object.position());

    this->setDirectionTowardPosition(objectPos);
}

void MapObject::setDirectionTowardPosition(const WorldPoint &pos) {
    WorldPoint thisPedPos(position());

    this->setDirection(pos.x - thisPedPos.x, pos.y - thisPedPos.y);
}

/*!
 * @returns direction for selected number of surfaces
 */
int MapObject::getDirection(int snum) {
    assert(snum > 0);

    int newDirection = 0;
    int sinc = 256 / snum;
    int sdec = sinc / 2;
    do {
        int s = newDirection * sinc;
        if (newDirection == 0) {
            if ((256 - sdec) <= dir_ || (s + sdec) > dir_)
                break;
        } else if ((s - sdec) <= dir_ && (s + sdec) > dir_)
            break;
        newDirection++;
    } while (newDirection < snum);
    assert(newDirection < snum);

    return newDirection;
}

/*
* NOTE: inc_xyz should point to array of three elements of type
* double for x,y,z
*/
bool MapObject::isBlocker(WorldPoint * pStartPt, WorldPoint * pEndPt,
               double * inc_xyz)
{
    // TODO: better set values of size for object, use to values
    // for single coordinate, rel_x_start(-vlaue) and rel_x_end(+value).
    // Vehicle and other objects with different directions will be handleDamage
    // correctly

    /* NOTE: algorithm, checks whether object is located within range
     * defined by "start" and "end", then we calculate distances from x, y, z
     * to their respective pStartPt, choose shortest, then longest between
     * them and recalculate position of entering of shot and exit point
     */

    // range_x check
    int range_x_h = pos_.tx * 256 + pos_.ox;
    int range_x_l = range_x_h - size_x_;
    range_x_h += size_x_;
    range_x_h--;
    int low_num = pStartPt->x;
    int high_num = pEndPt->x;
    bool flipped_x = false;
    if (pStartPt->x > pEndPt->x) {
        high_num = pStartPt->x;
        low_num = pEndPt->x;
        flipped_x = true;
    }
    if (range_x_l > high_num || range_x_h < low_num)
        return false;

    // range_y check
    int range_y_h = pos_.ty * 256 + pos_.oy;
    int range_y_l = range_y_h - size_y_;
    range_y_h += size_y_;
    range_y_h--;
    bool flipped_y = false;
    if (pStartPt->y > pEndPt->y) {
        high_num = pStartPt->y;
        low_num = pEndPt->y;
        flipped_y = true;
    } else {
        low_num = pStartPt->y;
        high_num = pEndPt->y;
    }
    if (range_y_l > high_num || range_y_h < low_num)
        return false;

    // range_z check
    int range_z_l = pos_.tz * 128 + pos_.oz;
    int range_z_h = range_z_l + size_z_;
    range_z_h--;
    bool flipped_z = false;
    if (pStartPt->z > pEndPt->z) {
        high_num = pStartPt->z;
        low_num = pEndPt->z;
        flipped_z = true;
    } else {
        low_num = pStartPt->z;
        high_num = pEndPt->z;
    }
    if (range_z_l > high_num || range_z_h < low_num)
        return false;

    double d_l[3];
    double d_h[3];
    if (inc_xyz[0] != 0) {
        d_l[0] = ((double)(range_x_l - pStartPt->x)) / inc_xyz[0];
        d_h[0] = ((double)(range_x_h - pStartPt->x)) / inc_xyz[0];
    } else {
        d_l[0] = 0.0;
        d_h[0] = 0.0;
    }
    if (inc_xyz[1] != 0) {
        d_l[1] = ((double)(range_y_l - pStartPt->y)) / inc_xyz[1];
        d_h[1] = ((double)(range_y_h - pStartPt->y)) / inc_xyz[1];
    } else {
        d_l[1] = 0.0;
        d_h[1] = 0.0;
    }
    if (inc_xyz[0] != 0) {
        d_l[2] = ((double)(range_z_l - pStartPt->z)) / inc_xyz[2];
        d_h[2] = ((double)(range_z_h - pStartPt->z)) / inc_xyz[2];
    } else {
        d_l[2] = 0.0;
        d_h[2] = 0.0;
    }

    // shortest distances to starting point
    double d_s[3];
    if (d_l[0] > d_h[0])
        d_s[0] = d_h[0];
    else
        d_s[0] = d_l[0];

    if (d_l[1] > d_h[1])
        d_s[1] = d_h[1];
    else
        d_s[1] = d_l[1];

    if (d_l[2] > d_h[2])
        d_s[2] = d_h[2];
    else
        d_s[2] = d_l[2];

    // TODO: another look at this function later
    uint8 indx = 0;
    // longest non-zero distance to start
    if (d_s[0] != 0.0) {
        if (d_s[1] != 0) {
            if (d_s[0] > d_s[1]) {
                if (d_s[2] != 0.0 && d_s[0] < d_s[2])
                    indx = 2;
            } else {
                indx = 1;
                if (d_s[2] != 0.0 && d_s[1] < d_s[2])
                    indx = 2;
            }
        }
    } else if (d_s[1] != 0) {
        indx = 1;
        if (d_s[2] != 0.0) {
            if (d_s[1] < d_s[2])
                indx = 2;
        }
    } else
        indx = 2;

    int range_g_l = (int)(d_l[indx] * inc_xyz[0] + pStartPt->x);
    int range_g_h = (int)(d_h[indx] * inc_xyz[0] + pStartPt->x);
    if (range_g_h < range_g_l) {
        low_num = range_g_h;
        high_num = range_g_l;
    } else {
        low_num = range_g_l;
        high_num = range_g_h;
    }
    if (low_num > range_x_h || high_num < range_x_l)
        return false;
    if (range_x_l < low_num)
        range_x_l = low_num;
    if (range_x_h > high_num)
        range_x_h = high_num;

    range_g_l = (int)(d_l[indx] * inc_xyz[1] + pStartPt->y);
    range_g_h = (int)(d_h[indx] * inc_xyz[1] + pStartPt->y);
    if (range_g_h < range_g_l) {
        low_num = range_g_h;
        high_num = range_g_l;
    } else {
        low_num = range_g_l;
        high_num = range_g_h;
    }
    if (low_num > range_y_h || high_num < range_y_l)
        return false;
    if (range_y_l < low_num)
        range_y_l = low_num;
    if (range_y_h > high_num)
        range_y_h = high_num;

    range_g_l = (int)(d_l[indx] * inc_xyz[2] + pStartPt->z);
    range_g_h = (int)(d_h[indx] * inc_xyz[2] + pStartPt->z);
    if (range_g_h < range_g_l) {
        low_num = range_g_h;
        high_num = range_g_l;
    } else {
        low_num = range_g_l;
        high_num = range_g_h;
    }
    if (low_num > range_z_h || high_num < range_z_l)
        return false;
    if (range_z_l < low_num)
        range_z_l = low_num;
    if (range_z_h > high_num)
        range_z_h = high_num;

    // restoring coordinates to their respective low/high values
    if (flipped_x) {
        pStartPt->x = range_x_h;
        pEndPt->x = range_x_l;
    } else {
        pStartPt->x = range_x_l;
        pEndPt->x = range_x_h;
    }

    if (flipped_y) {
        pStartPt->y = range_y_h;
        pEndPt->y = range_y_l;
    } else {
        pStartPt->y = range_y_l;
        pEndPt->y = range_y_h;
    }

    if (flipped_z) {
        pStartPt->z = range_z_h;
        pEndPt->z = range_z_l;
    } else {
        pStartPt->z = range_z_l;
        pEndPt->z = range_z_h;
    }

    return true;
}

void MapObject::offzOnStairs(uint8 twd) {
    switch (twd) {
        case 0x01:
            pos_.oz = 127 - (pos_.oy >> 1);
            break;
        case 0x02:
            pos_.oz = pos_.oy >> 1;
            break;
        case 0x03:
            pos_.oz = pos_.ox >> 1;
            break;
        case 0x04:
            pos_.oz = 127 - (pos_.ox >> 1);
            break;
        default:
            pos_.oz = 0;
            break;
    }
}

/*!
 * Constructor of the class.
 * \param m Map id
 * \param type Type of SfxObject (see SFXObject::SfxTypeEnum)
 * \param t_show
 * \param managed True means object is destroyed by another object than Mission
 */
SFXObject::SFXObject(int m, SfxTypeEnum type, int t_show, bool managed) : MapObject(sfxIdCnt++, m, kNatureUndefined) {
    type_ = type;
    managed_ = managed;
    draw_all_frames_ = true;
    loopAnimation_ = false;
    setTimeShowAnim(0);
    reset();
    switch(type) {
        case SFXObject::sfxt_Unknown:
            FSERR(Log::k_FLG_UI, "SFXObject", "SFXObject", ("Sfx object of type Unknown created"));
            sfx_life_over_ = true;
            break;
        case SFXObject::sfxt_BulletHit:
            anim_ = 382;
            break;
        case SFXObject::sfxt_FlamerFire:
            anim_ = 383;
            setFramesPerSec(12);
            break;
        case SFXObject::sfxt_Smoke:
            anim_ = 244;
            break;
        case SFXObject::sfxt_Fire_LongSmoke:
            // point of impact for laser
            anim_ = 389;
            break;
        case SFXObject::sfxt_ExplosionFire:
            anim_ = 390;
            setFramesPerSec(6);
            break;
        case SFXObject::sfxt_ExplosionBall:
            anim_ = 391;
            setFramesPerSec(6);
            break;
        case SFXObject::sfxt_LargeFire:
            anim_ = 243;
            setTimeShowAnim(3000 + t_show);
            break;
        case SFXObject::sfxt_SelArrow:
            anim_ = 601;
            time_show_anim_ = -1;
            setFramesPerSec(6);
            break;
        case SFXObject::sfxt_AgentFirst:
            anim_ = 1951;
            time_show_anim_ = -1;
            setFramesPerSec(4);
            break;
        case SFXObject::sfxt_AgentSecond:
            anim_ = 1952;
            time_show_anim_ = -1;
            setFramesPerSec(4);
            break;
        case SFXObject::sfxt_AgentThird:
            anim_ = 1953;
            time_show_anim_ = -1;
            setFramesPerSec(4);
            break;
        case SFXObject::sfxt_AgentFourth:
            anim_ = 1954;
            time_show_anim_ = -1;
            setFramesPerSec(4);
            break;
    }
}

void SFXObject::draw(int x, int y) {
    addOffs(x, y);
    g_App.gameSprites().drawFrame(anim_, frame_, x, y);
}

bool SFXObject::animate(int elapsed) {

    if (is_frame_drawn_) {
        bool changed = draw_all_frames_ ? MapObject::animate(elapsed) : false;
        if (type_ == SFXObject::sfxt_ExplosionBall) {
            int z = pos_.tz * 128 + pos_.oz;
            // 250 per sec
            z += ((elapsed + elapsed_left_) >> 2);
            elapsed_left_ = elapsed &3;
            if (z > (g_Session.getMission()->mmax_z_ - 1) * 128)
                z = (g_Session.getMission()->mmax_z_ - 1) * 128;
            pos_.tz = z / 128;
            pos_.oz = z % 128;
        }
        if (frame_ > g_App.gameSprites().lastFrame(anim_)
            && !leftTimeShowAnim(elapsed))
        {
            if (loopAnimation_) {
                reset();
            } else {
                sfx_life_over_ = true;
            }
        }
        return changed;
    }
    is_frame_drawn_ = true;
    return false;
}

void SFXObject::correctZ() {
    if (type_ == SFXObject::sfxt_ExplosionBall) {
        int z = pos_.tz * 128 + pos_.oz;
        z += 512;
        if (z > (g_Session.getMission()->mmax_z_ - 1) * 128)
            z = (g_Session.getMission()->mmax_z_ - 1) * 128;
        pos_.tz = z / 128;
        pos_.oz = z % 128;
    }
}

void SFXObject::reset() {
    sfx_life_over_ = false;
    frame_ = 0;
    elapsed_left_ = 0;
}

ShootableMapObject::ShootableMapObject(uint16 anId, int m, ObjectNature aNature):
    MapObject(anId, m, aNature)
{}

ShootableMovableMapObject::ShootableMovableMapObject(uint16 anId, int m, ObjectNature aNature):
        ShootableMapObject(anId, m, aNature) {
    speed_ = 0;
    base_speed_ = 0;
    dist_to_pos_ = 0;
}

/*!
 * This method adds the given offsets to the object's offX and offY
 * and moves it to a new tile if necessary.
 * \param nOffX amount to add to offX
 * \param nOffY amount to add to offY
 */
bool ShootableMovableMapObject::addOffsetToPosition(int nOffX, int nOffY)
{

    pos_.ox += nOffX;
    pos_.oy += nOffY;
    bool changed = false;

    while (pos_.ox < 0) {
        pos_.ox += 256;
        pos_.tx--;
        changed = true;
    }
    while (pos_.ox > 255) {
        pos_.ox -= 256;
        pos_.tx++;
        changed = true;
    }
    while (pos_.oy < 0) {
        pos_.oy += 256;
        pos_.ty--;
        changed = true;
    }
    while (pos_.oy > 255) {
        pos_.oy -= 256;
        pos_.ty++;
        changed = true;
    }

    return changed;
}

Static *Static::loadInstance(uint8 * data, uint16 id, int m)
{
    LevelData::Statics * gamdata =
        (LevelData::Statics *) data;
    Static *s = NULL;

    // TODO: verify whether object description is correct
    // subtype for doors, use instead orientation?

    // NOTE: objects states are usually mixed with type,
    // no separation between object type and its state
    uint16 curanim = READ_LE_UINT16(gamdata->index_current_anim);
    uint16 baseanim = READ_LE_UINT16(gamdata->index_base_anim);
    uint16 curframe = READ_LE_UINT16(gamdata->index_current_frame);
    switch(gamdata->sub_type) {
        case 0x01:
            // phone booth
            s = new EtcObj(id, m, curanim, curanim, curanim);
            s->setSizeX(128);
            s->setSizeY(128);
            s->setSizeZ(128);
            break;
        case 0x05:// 1040-1043, 1044 - damaged
            // crossroad things
            s = new Semaphore(id, m, 1040, 1044);
            s->setSizeX(48);
            s->setSizeY(48);
            s->setSizeZ(48);
            s->state_ = sttsem_Stt0;
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x06:
            // crossroad things
            s = new Semaphore(id, m, 1040, 1044);
            s->setSizeX(48);
            s->setSizeY(48);
            s->setSizeZ(48);
            s->state_ = sttsem_Stt1;
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x07:
            // crossroad things
            s = new Semaphore(id, m, 1040, 1044);
            s->setSizeX(48);
            s->setSizeY(48);
            s->setSizeZ(48);
            s->state_ = sttsem_Stt2;
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x08:
            // crossroad things
            s = new Semaphore(id, m, 1040, 1044);
            s->setSizeX(48);
            s->setSizeY(48);
            s->setSizeZ(48);
            s->state_ = sttsem_Stt3;
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x0B:
            // 0x0270 animation, is this object present in original game?
            //s = new EtcObj(m, curanim, curanim, curanim);
            //printf("0x0B anim %X\n", curanim);
            break;
        case 0x0A:
            s = new NeonSign(id, m, curanim);
            s->setFrame(g_App.gameSprites().getFrameFromFrameIndx(curframe));
            s->setExcludedFromBlockers(true);
            s->setSizeX(32);
            s->setSizeY(1);
            s->setSizeZ(48);
            break;
        case 0x0C: // closed door
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, m, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(256);
                s->setSizeY(1);
                s->setSizeZ(196);
            } else {
                baseanim++;
                s = new Door(id, m, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(1);
                s->setSizeY(256);
                s->setSizeZ(196);
            }
            break;
        case 0x0D: // closed door
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, m, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(256);
                s->setSizeY(1);
                s->setSizeZ(196);
            } else {
                baseanim++;
                s = new Door(id, m, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(1);
                s->setSizeY(256);
                s->setSizeZ(196);
            }
            break;
        case 0x0E: // opening doors, not open
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, m, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(256);
                s->setSizeY(1);
                s->setSizeZ(196);
            } else {
                baseanim++;
                s = new Door(id, m, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(1);
                s->setSizeY(256);
                s->setSizeZ(196);
            }
            s->state_ = sttdoor_Opening;
            break;
        case 0x0F: // opening doors, not open
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, m, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(256);
                s->setSizeY(1);
                s->setSizeZ(196);
            } else {
                baseanim++;
                s = new Door(id, m, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(1);
                s->setSizeY(256);
                s->setSizeZ(196);
            }
            s->state_ = sttdoor_Opening;
            break;
        case 0x11:
            // ???? what is this
            //s = new EtcObj(m, bas, curanim, curanim);
            //printf("0x11 anim %X\n", curanim);
            break;
        case 0x12:
            // open window
            s = new WindowObj(id, m, curanim - 2, curanim, curanim + 2, curanim + 4);
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80) {
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(96);
                s->setSizeY(4);
                s->setSizeZ(96);
            } else {
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(4);
                s->setSizeY(96);
                s->setSizeZ(96);
            }
            s->setHealth(1);
            s->setStartHealth(1);
            s->state_ = Static::sttwnd_Open;
            break;
        case 0x13:
            // closed window
            s = new WindowObj(id, m, curanim, curanim + 2, curanim + 4, curanim + 6);
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80) {
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(96);
                s->setSizeY(4);
                s->setSizeZ(96);
            } else {
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(4);
                s->setSizeY(96);
                s->setSizeZ(96);
            }
            s->setHealth(1);
            s->setStartHealth(1);
            s->state_ = Static::sttwnd_Closed;
            break;
        case 0x15:
            // damaged window
            s = new WindowObj(id, m, curanim - 6, curanim - 4, curanim - 2, curanim);
            s->setExcludedFromBlockers(true);
            s->setHealth(0);
            s->setStartHealth(1);
            s->state_ = Static::sttwnd_Damaged;
            break;
        case 0x16:
            // TODO: set state if damaged trees exist
            s = new Tree(id, m, curanim, curanim + 1, curanim + 2);
            s->setSizeX(64);
            s->setSizeY(64);
            s->setSizeZ(256);
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x19:
            // trash bin
            s = new EtcObj(id, m, curanim, curanim, curanim);
            s->setSizeX(64);
            s->setSizeY(64);
            s->setSizeZ(96);
            break;
        case 0x1A:
            // mail box
            s = new EtcObj(id, m, curanim, curanim, curanim);
            s->setSizeX(64);
            s->setSizeY(64);
            s->setSizeZ(96);
            break;
        case 0x1C:
            // ???? what is this?
            //s = new EtcObj(m, curanim, curanim, curanim);
            //printf("0x1C anim %X\n", curanim);
            break;
        case 0x1F:
            // advertisement on wall
            s = new EtcObj(id, m, curanim, curanim, curanim, smt_Advertisement);
            s->setExcludedFromBlockers(true);
            break;

        case 0x20:
            // window without light
            s = new AnimWindow(id, m, curanim);
            s->setStateMasks(sttawnd_LightOff);
            s->setTimeShowAnim(30000 + (rand() % 30000));
            break;
        case 0x21:
            // window light turns on
            s = new AnimWindow(id, m, curanim - 2);
            s->setTimeShowAnim(1000 + (rand() % 1000));
            s->setStateMasks(sttawnd_LightSwitching);

            // NOTE : 0x22 should have existed but it doesn't appear anywhere

        case 0x23:
            // window with person's shadow non animated,
            // even though on 1 map person appears I will ignore it
            s = new AnimWindow(id, m, 1959 + ((gamdata->orientation & 0x40) >> 5));
            s->setStateMasks(sttawnd_ShowPed);
            s->setTimeShowAnim(15000 + (rand() % 5000));
            break;
        case 0x24:
            // window with person's shadow, hides, actually animation
            // is of ped standing, but I will ignore it
            s = new AnimWindow(id, m, 1959 + 8 + ((gamdata->orientation & 0x40) >> 5));
            s->setStateMasks(sttawnd_PedDisappears);
            break;
        case 0x25:
            s = new AnimWindow(id, m, curanim);

            // NOTE : orientation, I assume, plays role of hidding object,
            // orientation 0x40, 0x80 are drawn (gamdata->desc always 7)
            // window without light
            s->setTimeShowAnim(30000 + (rand() % 30000));
            if (gamdata->orientation == 0x40 || gamdata->orientation == 0x80)
                s->setStateMasks(sttawnd_LightOff);
            else
                s->setStateMasks(sttawnd_LightOn);

            break;

        case 0x26:
            // 0x00,0x80 south - north = 0
            // 0x40,0xC0 weast - east = 2
            s = new LargeDoor(id, m, curanim, curanim + 1, curanim + 2);
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80) {
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(384);
                s->setSizeY(64);
                s->setSizeZ(192);
            } else {
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(64);
                s->setSizeY(384);
                s->setSizeZ(192);
            }
            break;
#ifdef _DEBUG
        default:
            printf("uknown static object type %02X , %02X, %X\n",
                gamdata->sub_type, gamdata->orientation,
                READ_LE_UINT16(gamdata->index_current_frame));
            printf("x = %i, xoff = %i, ", gamdata->mapposx[1],
                gamdata->mapposx[0]);
            printf("y = %i, yoff = %i, ", gamdata->mapposy[1],
                gamdata->mapposy[0]);
            printf("z = %i, zoff = %i\n", gamdata->mapposz[1],
                gamdata->mapposz[0]);
            break;
#endif
    }

    if (s) {
        int z = READ_LE_UINT16(gamdata->mapposz) >> 7;
        int oz = gamdata->mapposz[0] & 0x7F;
        // trick to draw
        if (s->type() == Static::smt_Advertisement)
            z += 1;

        s->setPosition(gamdata->mapposx[1], gamdata->mapposy[1],
            z, gamdata->mapposx[0], gamdata->mapposy[0], oz);

        //s->setMainType(gamdata->sub_type);
#if 0
        if (s->tileX() == 66 && s->tileY() == 49)
            oz = 2;
#endif
        s->setDirection(gamdata->orientation);
    }

    return s;
}

Door::Door(uint16 anId, int m, int anim, int closingAnim, int openAnim, int openingAnim) :
    Static(anId, m, Static::smt_Door), anim_(anim), closing_anim_(closingAnim),
        open_anim_(openAnim), opening_anim_(openingAnim) {
    state_ = Static::sttdoor_Closed;
}

void Door::draw(int x, int y)
{
    addOffs(x, y);
    g_App.gameSprites().drawFrame(anim_ + (state_ << 1), frame_, x, y);
}

bool Door::animate(int elapsed, Mission *obj)
{
    PedInstance *p = NULL;
    int x = tileX();
    int y = tileY();
    int z = tileZ();
    MapObject::ObjectNature aNature;
    int si;
    char inc_rel = 0, rel_inc = 0;
    char *i = 0, *j = 0;
    bool found = false;

    bool changed = MapObject::animate(elapsed);
    switch(state_) {
        case Static::sttdoor_Open:
            if (orientation_ == kStaticOrientation1) {
                i = &rel_inc;
                j = &inc_rel;
            } else if (orientation_ == kStaticOrientation2) {
                i = &inc_rel;
                j = &rel_inc;
            }
            assert(i != 0 && j != 0);
            for(*i = 0; *i < 2; *i += 1) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    p = (PedInstance *)(obj->findObjectWithNatureAtPos(x + inc_rel,
                        y + rel_inc, z, &aNature, &si, true));
                    if (!p && state_ == Static::sttdoor_Open && (!found)) {
                        state_ = Static::sttdoor_Closing;
                        setExcludedFromBlockers(false);
                        frame_ = 0;
                    } else if (p && p->isAlive()){
                        state_ = Static::sttdoor_Open;
                        setExcludedFromBlockers(true);
                        found = true;
                        p->hold_on_.wayFree = 0;
                    }
                } while (p);
            }
            break;
        case Static::sttdoor_Closed:
            if (orientation_ == kStaticOrientation1) {
                i = &rel_inc;
                j = &inc_rel;
            } else if (orientation_ == kStaticOrientation2) {
                i = &inc_rel;
                j = &rel_inc;
            }
            assert(i != 0 && j != 0);
            *i = 1;
            aNature = MapObject::kNaturePed; si = 0;
            do {
                p = (PedInstance *)(obj->findObjectWithNatureAtPos(x + inc_rel,
                    y + rel_inc, z, &aNature, &si, true));
                if (p && p->isAlive()) {
                    if (!found) {
                        state_ = Static::sttdoor_Opening;
                        setExcludedFromBlockers(false);
                        found = true;
                        frame_ = 0;
                    }
                    p->hold_on_.wayFree = 1;
                    p->hold_on_.tilex = x;
                    p->hold_on_.tiley = y;
                    p->hold_on_.tilez = z;
                    p->hold_on_.xadj = 0;
                    p->hold_on_.yadj = 0;
                    p->hold_on_.pathBlocker = this;
                }
            } while (p);
            *i = 0;
            aNature = MapObject::kNaturePed; si = 0;
            do {
                p = (PedInstance *)(obj->findObjectWithNatureAtPos(x + inc_rel,
                    y + rel_inc, z, &aNature, &si, true));
                if (p && p->isAlive()) {
                    if (!found) {
                        state_ = Static::sttdoor_Opening;
                        setExcludedFromBlockers(false);
                        found = true;
                        frame_ = 0;
                    }
                    p->hold_on_.wayFree = 1;
                    p->hold_on_.tilex = x;
                    p->hold_on_.tiley = y;
                    p->hold_on_.tilez = z;
                    p->hold_on_.xadj = 0;
                    p->hold_on_.yadj = 0;
                    p->hold_on_.pathBlocker = this;
                }
            } while (p);
            break;
        case Static::sttdoor_Closing:
            if (frame_ >= g_App.gameSprites().lastFrame(closing_anim_)) {
                state_ = Static::sttdoor_Closed;
                setExcludedFromBlockers(false);
                frame_ = 0;
            }
            break;
        case Static::sttdoor_Opening:
            if (frame_ >= g_App.gameSprites().lastFrame(opening_anim_)) {
                state_ = Static::sttdoor_Open;
                setExcludedFromBlockers(true);
                frame_ = 0;
            }
            break;
    }
    return changed;
}

bool Door::isPathBlocker()
{
    return state_ != Static::sttdoor_Open;
}


LargeDoor::LargeDoor(uint16 anId, int m, int anim, int closingAnim, int openingAnim):
        Static(anId, m, Static::smt_LargeDoor), anim_(anim),
        closing_anim_(closingAnim), opening_anim_(openingAnim) {
    state_ = Static::sttdoor_Closed;
}

void LargeDoor::draw(int x, int y)
{
    addOffs(x, y);
    switch(state_) {
        case Static::sttdoor_Open:
            break;
        case Static::sttdoor_Closing:
            g_App.gameSprites().drawFrame(closing_anim_, frame_, x, y);
            break;
        case Static::sttdoor_Closed:
            g_App.gameSprites().drawFrame(anim_, frame_, x, y);
            break;
        case Static::sttdoor_Opening:
            g_App.gameSprites().drawFrame(opening_anim_, frame_, x, y);
            break;
    }
}

bool LargeDoor::animate(int elapsed, Mission *obj)
{
    // TODO: there must be somewhere locked door
    GenericCar *v = NULL;
    PedInstance *p = NULL;
    int x = tileX();
    int y = tileY();
    int z = tileZ();
    MapObject::ObjectNature aNature;
    int si;
    char inc_rel = 0, rel_inc = 0;
    char *i = 0, *j = 0;
    bool found = false;
    std::vector<PedInstance *> found_peds;
    found_peds.reserve(256);
    std::vector<PedInstance *> found_peds_mid;
    found_peds_mid.reserve(256);
    char sign;
    int set_wayFree = 0;

    bool changed = MapObject::animate(elapsed);
    uint32 cur_state = state_;
    switch(state_) {
        case Static::sttdoor_Open:
            if (orientation_ == kStaticOrientation1) {
                i = &rel_inc;
                j = &inc_rel;
            } else if (orientation_ == kStaticOrientation2) {
                i = &inc_rel;
                j = &rel_inc;
            }
            assert(i != 0 && j != 0);
            *j = -1;
            for(*i = -2; *i < 3; (*i)++) {
                aNature = MapObject::kNatureVehicle; si = 0;
                v = (GenericCar *)(obj->findObjectWithNatureAtPos(x + inc_rel,
                    y + rel_inc,z, &aNature, &si, true));
                if (!v && !found) {
                    state_ = Static::sttdoor_Closing;
                    setExcludedFromBlockers(false);
                } else if (v){
                    state_ = Static::sttdoor_Open;
                    setExcludedFromBlockers(true);
                    found = true;
                    v->hold_on_.wayFree = 0;
                }
            }
            *j = 1;
            for(*i = -2; *i < 3; (*i)++) {
                aNature = MapObject::kNatureVehicle; si = 0;
                v = (GenericCar *)(obj->findObjectWithNatureAtPos(x + inc_rel,
                    y + rel_inc,z,&aNature,&si,true));
                if (!v && !found) {
                    state_ = Static::sttdoor_Closing;
                    setExcludedFromBlockers(false);
                } else if (v){
                    state_ = Static::sttdoor_Open;
                    setExcludedFromBlockers(true);
                    found = true;
                    v->hold_on_.wayFree = 0;
                }
            }
            *j = -1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    p = (PedInstance *)(obj->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (p) {
                        found_peds.push_back(p);
                        if (!found && p->hasAccessCard()) {
                            state_ = Static::sttdoor_Open;
                            setExcludedFromBlockers(true);
                            found = true;
                        }
                    }
                } while (p);
            }
            *j = 1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    p = (PedInstance *)(obj->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (p) {
                        found_peds.push_back(p);
                        if (!found && p->hasAccessCard()) {
                            state_ = Static::sttdoor_Open;
                            setExcludedFromBlockers(true);
                            found = true;
                        }
                    }
                } while (p);
            }
            *j = 0;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    p = (PedInstance *)(obj->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (p) {
                        found_peds_mid.push_back(p);
                        if (!found && p->hasAccessCard()) {
                            state_ = Static::sttdoor_Open;
                            setExcludedFromBlockers(true);
                            found = true;
                        }
                    }
                } while (p);
            }
            if (state_ == Static::sttdoor_Open) {
                for (std::vector<PedInstance *>::iterator it = found_peds.begin();
                    it != found_peds.end(); ++it )
                {
                    (*it)->hold_on_.wayFree = 0;
                }
                for (std::vector<PedInstance *>::iterator it = found_peds_mid.begin();
                    it != found_peds_mid.end(); ++it )
                {
                    (*it)->hold_on_.wayFree = 0;
                }
            } else {
                for (std::vector<PedInstance *>::iterator it = found_peds.begin();
                    it != found_peds.end(); ++it )
                {
                    p = *it;
                    p->hold_on_.wayFree = 2;
                    p->hold_on_.tilex = x;
                    p->hold_on_.tiley = y;
                    if (orientation_ == kStaticOrientation1) {
                        p->hold_on_.xadj = 1;
                        p->hold_on_.yadj = 0;
                    } else if (orientation_ == kStaticOrientation2) {
                        p->hold_on_.xadj = 0;
                        p->hold_on_.yadj = 1;
                    }
                    p->hold_on_.tilez = z;
                    p->hold_on_.pathBlocker = this;
                }
                for (std::vector<PedInstance *>::iterator it = found_peds_mid.begin();
                    it != found_peds_mid.end(); ++it )
                {
                    p = *it;
                    fs_dmg::DamageToInflict d;
                    d.dtype = fs_dmg::kDmgTypeCollision;
                    d.d_owner = NULL;
                    d.dvalue = 1024;
                    d.ddir = -1;
                    p->handleHit(d);
                }
            }
            break;
        case Static::sttdoor_Closed:
            if (orientation_ == kStaticOrientation1) {
                i = &rel_inc;
                j = &inc_rel;
                sign = 1;
            } else if (orientation_ == kStaticOrientation2) {
                i = &inc_rel;
                j = &rel_inc;
                sign = -1;
            }
            assert(i != 0 && j != 0);
            *j = -1 * sign;
            *i = -2;
            aNature = MapObject::kNatureVehicle; si = 0;
            v = (GenericCar *)(obj->findObjectWithNatureAtPos(x + inc_rel,
                y + rel_inc,z, &aNature, &si,true));
            if (v) {
                if (!found) {
                    state_ = Static::sttdoor_Opening;
                    setExcludedFromBlockers(false);
                    found = true;
                }
                v->hold_on_.wayFree = 1;
                v->hold_on_.pathBlocker = this;
            }
            *j = 1 * sign;
            *i = 2;
            aNature = MapObject::kNatureVehicle; si = 0;
            v = (GenericCar *)(obj->findObjectWithNatureAtPos(x + inc_rel,
                y + rel_inc,z, &aNature, &si,true));
            if (v) {
                if (!found) {
                    state_ = Static::sttdoor_Opening;
                    setExcludedFromBlockers(false);
                    found = true;
                }
                v->hold_on_.wayFree = 1;
                v->hold_on_.pathBlocker = this;
            }
            *j = -1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    p = (PedInstance *)(obj->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (p) {
                        found_peds.push_back(p);
                        if (!found && p->hasAccessCard()) {
                            state_ = Static::sttdoor_Opening;
                            setExcludedFromBlockers(false);
                            found = true;
                        }
                    }
                } while (p);
            }
            *j = 1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    p = (PedInstance *)(obj->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (p) {
                        found_peds.push_back(p);
                        if (!found && p->hasAccessCard()) {
                            state_ = Static::sttdoor_Opening;
                            setExcludedFromBlockers(false);
                            found = true;
                        }
                    }
                } while (p);
            }
            set_wayFree = state_ == Static::sttdoor_Opening ? 1 : 2;
            for (std::vector<PedInstance *>::iterator it = found_peds.begin();
                it != found_peds.end(); ++it )
            {
                p = *it;
                p->hold_on_.wayFree = set_wayFree;
                p->hold_on_.tilex = x;
                p->hold_on_.tiley = y;
                if (orientation_ == kStaticOrientation1) {
                    p->hold_on_.xadj = 1;
                    p->hold_on_.yadj = 0;
                } else if (orientation_ == kStaticOrientation2) {
                    p->hold_on_.xadj = 0;
                    p->hold_on_.yadj = 1;
                }
                p->hold_on_.tilez = z;
                p->hold_on_.pathBlocker = this;
            }
            break;
        case Static::sttdoor_Closing:
            if (frame_ >= g_App.gameSprites().lastFrame(closing_anim_)) {
                state_ = Static::sttdoor_Closed;
                setExcludedFromBlockers(false);
            }
        case Static::sttdoor_Opening:
            if (state_ == Static::sttdoor_Opening
                && frame_ >= g_App.gameSprites().lastFrame(opening_anim_))
            {
                state_ = Static::sttdoor_Open;
                setExcludedFromBlockers(true);
            }
            if (orientation_ == kStaticOrientation1) {
                i = &rel_inc;
                j = &inc_rel;
                sign = 1;
            } else if (orientation_ == kStaticOrientation2) {
                i = &inc_rel;
                j = &rel_inc;
                sign = -1;
            }
            assert(i != 0 && j != 0);
            *j = -1 * sign;
            *i = -2;
            set_wayFree = state_ == Static::sttdoor_Opening ? 1 : 2;
            aNature = MapObject::kNatureVehicle; si = 0;
            v = (GenericCar *)(obj->findObjectWithNatureAtPos(x + inc_rel,
                y + rel_inc,z, &aNature, &si,true));
            if (v) {
                v->hold_on_.wayFree = 1;
                v->hold_on_.pathBlocker = this;
            }
            *j = 1 * sign;
            *i = 2;
            aNature = MapObject::kNatureVehicle; si = 0;
            v = (GenericCar *)(obj->findObjectWithNatureAtPos(x + inc_rel,
                y + rel_inc,z, &aNature, &si,true));
            if (v) {
                v->hold_on_.wayFree = 1;
                v->hold_on_.pathBlocker = this;
            }
            *j = -1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    p = (PedInstance *)(obj->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (p) {
                        found_peds.push_back(p);
                    }
                } while (p);
            }
            *j = 1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    p = (PedInstance *)(obj->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (p) {
                        found_peds.push_back(p);
                    }
                } while (p);
            }
            for (std::vector<PedInstance *>::iterator it = found_peds.begin();
                it != found_peds.end(); ++it )
            {
                p = *it;
                p->hold_on_.wayFree = set_wayFree;
                p->hold_on_.tilex = x;
                p->hold_on_.tiley = y;
                if (orientation_ == kStaticOrientation1) {
                    p->hold_on_.xadj = 1;
                    p->hold_on_.yadj = 0;
                } else if (orientation_ == kStaticOrientation2) {
                    p->hold_on_.xadj = 0;
                    p->hold_on_.yadj = 1;
                }
                p->hold_on_.tilez = z;
                p->hold_on_.pathBlocker = this;
            }
            break;
    }
    if (cur_state != state_)
        frame_ = 0;
    return changed;
}

bool LargeDoor::isPathBlocker()
{
    return state_ != Static::sttdoor_Open;
}


Tree::Tree(uint16 anId, int m, int anim, int burningAnim, int damagedAnim) :
        Static(anId, m, Static::smt_Tree), anim_(anim), burning_anim_(burningAnim),
        damaged_anim_(damagedAnim) {
    state_ = Static::stttree_Healthy;
}

void Tree::draw(int x, int y)
{
    addOffs(x, y);
    switch (state_) {
        case Static::stttree_Healthy:
            g_App.gameSprites().drawFrame(anim_, frame_, x, y);
            break;
        case Static::stttree_Burning:
            g_App.gameSprites().drawFrame(burning_anim_, frame_, x, y);
            break;
        case Static::stttree_Damaged:
            g_App.gameSprites().drawFrame(damaged_anim_, frame_, x, y);
            break;
    }
}

bool Tree::animate(int elapsed, Mission *obj) {

    if (state_ == Static::stttree_Burning) {
        if (!(leftTimeShowAnim(elapsed))) {
            state_ = Static::stttree_Damaged;
            frame_ = 0;
            setFramesPerSec(2);
            return true;
        }
    }

    return MapObject::animate(elapsed);
}

/*!
 * Implementation for the Tree. Tree burns only when hit by laser of fire.
 * \param d Damage information
 */
void Tree::handleHit(fs_dmg::DamageToInflict &d) {
    if (isAlive() &&
        (d.dtype == fs_dmg::kDmgTypeLaser || d.dtype == fs_dmg::kDmgTypeBurn || d.dtype == fs_dmg::kDmgTypeExplosion)) {
        decreaseHealth(d.dvalue);
        if (isDead()) {
            state_ = Static::stttree_Burning;
            setTimeShowAnim(10000);
            setExcludedFromBlockers(true);
        }
    }
}

WindowObj::WindowObj(uint16 anId, int m, int anim, int openAnim, int breakingAnim,
                     int damagedAnim) :
        Static(anId, m, Static::smt_Window), anim_(anim), open_anim_(openAnim),
        breaking_anim_(breakingAnim), damaged_anim_(damagedAnim) {}

bool WindowObj::animate(int elapsed, Mission *obj) {
    bool updated = MapObject::animate(elapsed);

    if (state_ == Static::sttwnd_Breaking
        && frame_ >= g_App.gameSprites().lastFrame(breaking_anim_)
    ) {
        state_ = sttwnd_Damaged;
        updated = true;
    }
    return updated;
}

void WindowObj::draw(int x, int y)
{
    addOffs(x, y);
    g_App.gameSprites().drawFrame(anim_ + (state_ << 1), frame_, x, y);
}

/*!
 * Implementation for the Tree. Tree burns only when hit by laser of fire.
 * \param d Damage information
 */
void WindowObj::handleHit(fs_dmg::DamageToInflict &d) {
    if (isAlive() &&
        (d.dtype == fs_dmg::kDmgTypeBullet || d.dtype == fs_dmg::kDmgTypeExplosion)) {
        decreaseHealth(d.dvalue);
        if (isDead()) {
            state_ = Static::sttwnd_Breaking;
            setExcludedFromBlockers(true);
            frame_ = 0;
            setFramesPerSec(6);
        }
    }
}

EtcObj::EtcObj(uint16 anId, int m, int anim, int burningAnim, int damagedAnim, StaticType aType) :
        Static(anId, m, aType), anim_(anim), burning_anim_(burningAnim),
        damaged_anim_(damagedAnim) {}

void EtcObj::draw(int x, int y)
{
    addOffs(x, y);
    g_App.gameSprites().drawFrame(anim_, frame_, x, y);
}

NeonSign::NeonSign(uint16 anId, int m, int anim) : Static(anId, m, Static::smt_NeonSign) {
    anim_ = anim;
}

void NeonSign::draw(int x, int y)
{
    addOffs(x, y);
    g_App.gameSprites().drawFrame(anim_, frame_, x, y);
}

Semaphore::Semaphore(uint16 anId, int m, int anim, int damagedAnim) :
        Static(anId, m, Static::smt_Semaphore), anim_(anim),
        damaged_anim_(damagedAnim), elapsed_left_smaller_(0),
        elapsed_left_bigger_(0), up_down_(1)
{
    setFramesPerSec(2);
}

bool Semaphore::animate(int elapsed, Mission *obj) {
    if (state_ == Static::sttsem_Damaged) {
        if (elapsed_left_bigger_ == 0)
            return false;
        int chng = (elapsed + elapsed_left_smaller_) >> 1;
        elapsed_left_smaller_ = elapsed & 2;
        elapsed_left_bigger_ -= chng;
        if (elapsed_left_bigger_ < 0) {
            chng += elapsed_left_bigger_;
            elapsed_left_bigger_ = 0;
        }
        int z = pos_.tz * 128 + pos_.oz - chng;
        pos_.tz = z / 128;
        pos_.oz = z % 128;
        return true;
    }

    int chng = (elapsed + elapsed_left_smaller_) >> 2;
    elapsed_left_smaller_ = elapsed & 4;
    if (chng) {
        int oz = pos_.oz + chng * up_down_;
        if (oz > 127) {
            oz = 127 - (oz & 0x7F);
            up_down_ -= 2;
        } else if (oz < 64) {
            oz = 64 + (64 - oz);
            up_down_ += 2;
        }
        pos_.oz = oz;
    }

    chng = (elapsed + elapsed_left_bigger_) >> 6;
    elapsed_left_bigger_ = elapsed & 63;
    if (chng) {
        // Direction is used as storage for animation change, not my idea
        dir_ += chng;
        dir_ &= 0xFF;
        state_ = dir_ >> 6;
        state_++;
        if (state_ > Static::sttsem_Stt3)
            state_ = Static::sttsem_Stt0;
    }

    return MapObject::animate(elapsed);
}

/*!
 * Implementation for the Semaphore.
 * \param d Damage information
 */
void Semaphore::handleHit(fs_dmg::DamageToInflict &d) {
    if (isAlive() &&
        (d.dtype == fs_dmg::kDmgTypeLaser || d.dtype == fs_dmg::kDmgTypeExplosion)) {
        decreaseHealth(d.dvalue);
        if (isDead()) {
            state_ = Static::sttsem_Damaged;
            // To make this thing reach the ground need to get solid surface 0x0F
            Mission * m = g_Session.getMission();
            int z = pos_.tz;
            int indx = pos_.tx + pos_.ty * m->mmax_x_ + pos_.tz * m->mmax_m_xy;
            elapsed_left_bigger_ = 0;
            while (z != 0) {
                z--;
                indx -= m->mmax_m_xy;
                int twd = m->mtsurfaces_[indx];
                if (twd == 0x0F) {
                    elapsed_left_bigger_ = (pos_.tz - z) * 128 + pos_.oz;
                    break;
                }
            }
            setExcludedFromBlockers(true);
        }
    }
}

void Semaphore::draw(int x, int y)
{
    addOffs(x, y);
    g_App.gameSprites().drawFrame(anim_ +  state_, frame_, x, y);
}

AnimWindow::AnimWindow(uint16 anId, int m, int anim) : Static(anId, m, smt_AnimatedWindow) {
    setExcludedFromBlockers(true);
    setFramesPerSec(4);
    anim_ = anim;
}

void AnimWindow::draw(int x, int y)
{
    // When light is on, don't draw window
    // because lighted window is part of the map
    if (state_ == Static::sttawnd_LightOn)
        return;
    addOffs(x, y);
    g_App.gameSprites().drawFrame(anim_ + (state_ << 1), frame_, x, y);
}

bool AnimWindow::animate(int elapsed, Mission *obj)
{
    switch (state_) {
        case Static::sttawnd_LightOff:
            if (!leftTimeShowAnim(elapsed)) {
                // decide to start switching lights on
                // or continue being in dark
                if (rand() % 100 > 60) {
                    setTimeShowAnim(30000 + (rand() % 30000));
                } else {
                    state_ = Static::sttawnd_LightSwitching;
                    frame_ = 0;
                    setTimeShowAnim(1000 + (rand() % 1000));
                }
            }
            break;
        case Static::sttawnd_LightSwitching:
            if (!leftTimeShowAnim(elapsed)) {
                state_ = Static::sttawnd_LightOn;
                setTimeShowAnim(30000 + (rand() % 30000));
            }
            break;
        case Static::sttawnd_PedAppears:
            if (frame_ >= g_App.gameSprites().lastFrame(anim_
                + (Static::sttawnd_PedAppears << 1)))
            {
                state_ = Static::sttawnd_ShowPed;
                setTimeShowAnim(15000 + (rand() % 15000));
            }
            break;
        case Static::sttawnd_ShowPed:
            if (!leftTimeShowAnim(elapsed)) {
                // continue showing ped or hide it
                if (rand() % 100 > 50) {
                    setTimeShowAnim(15000 + (rand() % 5000));
                } else {
                    frame_ = 0;
                    state_ = Static::sttawnd_PedDisappears;
                }
            }
            break;
        case Static::sttawnd_PedDisappears:
            if (frame_ >= g_App.gameSprites().lastFrame(anim_
                + (Static::sttawnd_PedDisappears << 1)))
            {
                state_ = Static::sttawnd_LightOn;
                setTimeShowAnim(30000 + (rand() % 30000));
            }
            break;
        case Static::sttawnd_LightOn:
            if (!leftTimeShowAnim(elapsed)) {
                // we will continue showing lightson or switch
                // lights off or show ped
                int rnd_v = rand() % 100;
                if (rnd_v > 80) {
                    setTimeShowAnim(30000 + (rand() % 30000));
                } else if (rnd_v > 60) {
                    frame_ = 0;
                    state_ = Static::sttawnd_PedAppears;
                } else if (rnd_v > 10) {
                    state_ = Static::sttawnd_LightOff;
                    setTimeShowAnim(30000 + (rand() % 30000));
                } else {
                    frame_ = 0;
                    state_ = Static::sttawnd_LightSwitching;
                    setTimeShowAnim(1000 + (rand() % 1000));
                }
            }
            break;
    }

    return MapObject::animate(elapsed);
    return false;
}


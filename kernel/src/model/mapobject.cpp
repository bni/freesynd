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

#include "fs-kernel/model/mapobject.h"

#include "fs-engine/gfx/tile.h"
#include "fs-kernel/model/map.h"

MapObject::MapObject(uint16 anId, Map *pMap, ObjectNature aNature):
    size_x_(1), size_y_(1), size_z_(2),
    frame_(0), elapsed_carry_(0),
    frames_per_sec_(8),
    dir_(0),
    time_show_anim_(-1), time_showing_anim_(-1),
    is_frame_drawn_(false),
    state_(0xFFFFFFFF)
{
    nature_ = aNature;
    id_ = anId;
    pMap_ = pMap;
    isDrawable_ = true;
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

Point2D MapObject::addOffs(const Point2D &screenPos)
{
    Point2D posWithOffs;
    posWithOffs.x = (((pos_.ox - pos_.oy) * (TILE_WIDTH / 2)) / 256) + screenPos.x;
    posWithOffs.y = (((pos_.ox + pos_.oy) * (TILE_HEIGHT / 3)) / 256) + screenPos.y;
    posWithOffs.y -= (pos_.oz * (TILE_HEIGHT / 3)) / 128;

    return posWithOffs;
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

/** \brief
 *
 * \param anId uint16
 * \param m int
 * \param aNature ObjectNature
 *
 */
ShootableMapObject::ShootableMapObject(uint16 anId, Map *pMap, ObjectNature aNature):
    MapObject(anId, pMap, aNature)
{}

ShootableMovableMapObject::ShootableMovableMapObject(uint16 anId, Map *pMap, ObjectNature aNature):
        ShootableMapObject(anId, pMap, aNature) {
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

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2013  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "fs-kernel/model/sfxobject.h"

#include "fs-utils/log/log.h"
#include "fs-engine/gfx/spritemanager.h"

uint16 SFXObject::sfxIdCnt = 0;

/*!
 * Constructor of the class.
 * \param pMap a pointer to the map
 * \param type Type of SfxObject (see SFXObject::SfxTypeEnum)
 * \param drawable True means the object will be drawn by default
 * \param t_show
 * \param managed True means object is destroyed by another object than Mission
 */
SFXObject::SFXObject(Map *pMap, SfxTypeEnum type, bool drawable, int t_show, bool managed) : MapObject(sfxIdCnt++, pMap, kNatureUndefined) {
    type_ = type;
    managed_ = managed;
    draw_all_frames_ = true;
    loopAnimation_ = false;
    setTimeShowAnim(0);
    setDrawable(drawable);
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

void SFXObject::draw(const Point2D &screenPos) {
    g_SpriteMgr.drawFrame(anim_, frame_, addOffs(screenPos));
}

bool SFXObject::animate(int elapsed) {

    if (is_frame_drawn_) {
        bool changed = draw_all_frames_ ? MapObject::animate(elapsed) : false;
        if (type_ == SFXObject::sfxt_ExplosionBall) {
            int z = pos_.tz * 128 + pos_.oz;
            // 250 per sec
            z += ((elapsed + elapsed_left_) >> 2);
            elapsed_left_ = elapsed &3;
            if (z > (pMap_->maxZ() - 1) * 128)
                z = (pMap_->maxZ() - 1) * 128;
            pos_.tz = z / 128;
            pos_.oz = z % 128;
        }

        if (frame_ > g_SpriteMgr.lastFrame(anim_)
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


/** \brief
 *
 * \param mapMaxZ int
 * \return void
 *
 */
void SFXObject::correctZ(int mapMaxZ) {
    if (type_ == SFXObject::sfxt_ExplosionBall) {
        int z = pos_.tz * 128 + pos_.oz;
        z += 512;

        if (z > (mapMaxZ - 1) * 128)
            z = (mapMaxZ - 1) * 128;
        pos_.tz = z / 128;
        pos_.oz = z % 128;
    }
}

void SFXObject::reset() {
    sfx_life_over_ = false;
    frame_ = 0;
    elapsed_left_ = 0;
}

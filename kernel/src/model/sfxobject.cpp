/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net>
 *   Copyright (C) 2013, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
 *
 *   This program is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as 
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>. 
 * 
 */

#include "fs-kernel/model/sfxobject.h"

#include "fs-utils/log/log.h"
#include "fs-engine/gfx/animationmanager.h"

namespace fs_knl {
uint16_t SFXObject::sfxIdCnt = 0;

/*!
 * Constructor of the class.
 * \param pMap a pointer to the map
 * \param type Type of SfxObject (see SFXObject::SfxTypeEnum)
 * \param drawable True means the object will be drawn by default
 * \param t_show
 */
SFXObject::SFXObject(Map *pMap, SfxType type, bool drawable, int t_show) : MapObject(sfxIdCnt++, pMap, kNatureUndefined) {
    type_ = type;
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
            animationId_ = 382;
            break;
        case SFXObject::sfxt_FlamerFire:
            animationId_ = 383;
            setFramesPerSec(12);
            break;
        case SFXObject::sfxt_Smoke:
            animationId_ = 244;
            break;
        case SFXObject::sfxt_Fire_LongSmoke:
            // point of impact for laser
            animationId_ = 389;
            break;
        case SFXObject::sfxt_ExplosionFire:
            animationId_ = 390;
            setFramesPerSec(6);
            break;
        case SFXObject::sfxt_ExplosionBall:
            animationId_ = 391;
            setFramesPerSec(6);
            break;
        case SFXObject::sfxt_LargeFire:
            animationId_ = 243;
            setTimeShowAnim(3000 + t_show);
            break;
        case SFXObject::sfxt_SelArrow:
            animationId_ = 601;
            time_show_anim_ = -1;
            setFramesPerSec(6);
            break;
        case SFXObject::sfxt_AgentFirst:
            animationId_ = 1951;
            time_show_anim_ = -1;
            setFramesPerSec(4);
            break;
        case SFXObject::sfxt_AgentSecond:
            animationId_ = 1952;
            time_show_anim_ = -1;
            setFramesPerSec(4);
            break;
        case SFXObject::sfxt_AgentThird:
            animationId_ = 1953;
            time_show_anim_ = -1;
            setFramesPerSec(4);
            break;
        case SFXObject::sfxt_AgentFourth:
            animationId_ = 1954;
            time_show_anim_ = -1;
            setFramesPerSec(4);
            break;
    }
}

void SFXObject::draw(const Point2D &screenPos) {
    g_SpriteMgr.drawFrame(animationId_, frame_, addOffs(screenPos));
}

bool SFXObject::animate(uint32_t elapsed) {

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

        if (frame_ > g_SpriteMgr.lastFrame(animationId_)
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
}
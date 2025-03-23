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
 * \param addShow Used for fire animation to add randomized time to animation
 */
SFXObject::SFXObject(Map *pMap, SfxType type, bool drawable, uint32_t addShow) : 
        MapObject(sfxIdCnt++, pMap, kNatureUndefined), type_(type), sfx_life_over_(false), elapsed_left_(0) {
    setDrawable(drawable);
    
    switch(type) {
        case SFXObject::sfxt_Unknown:
            FSERR(Log::k_FLG_UI, "SFXObject", "SFXObject", ("Sfx object of type Unknown created"));
            sfx_life_over_ = true;
            break;
        case SFXObject::sfxt_BulletHit:
            animationId_ = animationPlayer_->addAnimation(382);
            break;
        case SFXObject::sfxt_FlamerFire:
            animationId_ = animationPlayer_->addAnimation(383, fs_eng::kAnimationModeLoop, 12);
            break;
        case SFXObject::sfxt_Smoke:
            animationId_ = animationPlayer_->addAnimation(244);
            break;
        case SFXObject::sfxt_Fire_LongSmoke:
            // point of impact for laser
            animationId_ = animationPlayer_->addAnimation(389);
            break;
        case SFXObject::sfxt_ExplosionFire:
            animationId_ = animationPlayer_->addAnimation(390, fs_eng::kAnimationModeSingle, 6);
            break;
        case SFXObject::sfxt_ExplosionBall:
            animationId_ = animationPlayer_->addAnimation(391, fs_eng::kAnimationModeSingle, 6);
            break;
        case SFXObject::sfxt_LargeFire:
            animationId_ = animationPlayer_->addAnimation(243, fs_eng::kAnimationModeLoop, 8, 3000 + addShow);
            break;
        case SFXObject::sfxt_SelArrow:
            animationId_ = animationPlayer_->addAnimation(601, fs_eng::kAnimationModeSingle, 6);
            break;
        case SFXObject::sfxt_AgentFirst:
            animationId_ = animationPlayer_->addAnimation(1951, fs_eng::kAnimationModeLoop, 4);
            break;
        case SFXObject::sfxt_AgentSecond:
            animationId_ = animationPlayer_->addAnimation(1952, fs_eng::kAnimationModeLoop, 4);
            break;
        case SFXObject::sfxt_AgentThird:
            animationId_ = animationPlayer_->addAnimation(1953, fs_eng::kAnimationModeLoop, 4);
            break;
        case SFXObject::sfxt_AgentFourth:
            animationId_ = animationPlayer_->addAnimation(1954, fs_eng::kAnimationModeLoop, 4);
            break;
    }
}

void SFXObject::draw(const Point2D &screenPos) {
    animationPlayer_->draw(addOffs(screenPos), 0);
}

void SFXObject::doUpdateState(uint32_t elapsed) {
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
}

void SFXObject::handleAnimationEnded() {
    sfx_life_over_ = true;
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

}
/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> 
 *   Copyright (C) 2013, 2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef KERNEL_SFXOBJECT_H
#define KERNEL_SFXOBJECT_H

#include "fs-kernel/model/mapobject.h"
#include "fs-kernel/model/map.h"

namespace fs_knl {

/*!
 * SFXObject are elements that are small animations with a small lifetime.
 * Once the lifetime is reached, the object can be destroyed.
 */
class SFXObject : public MapObject {
public:
    /*!
     * Type of SfxObject.
     */
    enum SfxType {
        sfxt_Unknown = 0,
        sfxt_BulletHit = 1,
        sfxt_FlamerFire = 2,
        sfxt_Smoke = 3,
        sfxt_Fire_LongSmoke = 4,
        sfxt_ExplosionFire = 5,
        sfxt_ExplosionBall = 6,
        sfxt_LargeFire = 7,
        sfxt_SelArrow = 8,
        sfxt_AgentFirst = 9,
        sfxt_AgentSecond = 10,
        sfxt_AgentThird = 11,
        sfxt_AgentFourth = 12
    };

    SFXObject(Map *pMap, SfxType type, bool drawable = true, uint32_t addShow = 0);
    virtual ~SFXObject() {}

    //! Return true if object can be destroyed
    bool sfxLifeOver() { return sfx_life_over_; }
    //! Used to force the end of object
    void forceEndofLife() { sfx_life_over_ = true; }

    void draw(const Point2D &screenPos) override;

    void playMainAnimation() {
        animationPlayer_->play(animationId_);
    }
    //!
    void correctZ(int mapMaxZ);

protected:
    //! @copydoc MapObject::doUpdateState()
    void doUpdateState(uint32_t elapsed) override;
    //! @copydoc MapObject::handleAnimationEnded()
    void handleAnimationEnded() override;

protected:
    static uint16_t sfxIdCnt;
    /*! The type of SfxObject.*/
    SfxType type_;
    //! Animation to use for this SfxObject
    uint16_t animationId_;
    //! When true, SFXObject life is over and object can be destroyed
    bool sfx_life_over_;

    uint32_t elapsed_left_;
};

}
#endif  //KERNEL_SFXOBJECT_H

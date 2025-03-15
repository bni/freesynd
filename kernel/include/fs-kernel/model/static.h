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

#ifndef MAPOBJECT_H
#define MAPOBJECT_H

#include <math.h>
#include <list>

#include "fs-kernel/model/mapobject.h"
#include "fs-utils/misc/timer.h"

namespace fs_knl {

/*!
 * Static map object class.
 */
class Static : public ShootableMapObject {
public:
    /*! Const for orientation 1 of Static.*/
    static const int kStaticOrientation1;
    /*! Const for orientation 2 of Static.*/
    static const int kStaticOrientation2;

    enum StaticType {
        // NOTE: should be the same name as Class
        smt_None = 0,
        smt_Advertisement,
        smt_Semaphore,
        smt_Door,
        smt_LargeDoor,
        smt_Tree,
        smt_Window,
        smt_AnimatedWindow,
        smt_NeonSign
    };

    enum StateDoors {
        sttdoor_Closed = 0,
        sttdoor_Closing,
        sttdoor_Open,
        sttdoor_Opening
    };

    enum StateWindows {
        sttwnd_Closed = 0,
        sttwnd_Open,
        sttwnd_Breaking,
        sttwnd_Damaged
    };

    enum StateAnimatedWindows {
        sttawnd_LightOff = 0,
        sttawnd_LightSwitching,
        sttawnd_PedAppears,
        sttawnd_ShowPed,
        sttawnd_PedDisappears,
        sttawnd_LightOn
    };
public:
    static Static *loadInstance(uint8_t *data, uint16_t id, Map *pMap);
    virtual ~Static() {}

    //! Return the type of statics
    StaticType type() { return type_; }
    //! Set the sub type of statics
    void setOrientation(int anOrientation) { orientation_ = anOrientation; }
    //! Return the type of statics
    int orientation() { return orientation_; }

    //! Return true if static should not be included in the search for blockers
    bool isExcludedFromBlockers() { return excludedFromBlockers_; }
    //! Set whether to include static in search for blockers
    void setExcludedFromBlockers(bool exclude) { excludedFromBlockers_ = exclude; }

protected:
    Static(uint16_t anId, Map *pMap, StaticType aType) :
            ShootableMapObject(anId, pMap, MapObject::kNatureStatic) {
        type_ = aType;
        orientation_ = kStaticOrientation1;
        excludedFromBlockers_ = false;
    }

protected:
    /*! Type of statics.*/
    StaticType type_;
    /*! Some statics can be displayed in 1 of 2 orientations : SW or SE.*/
    int orientation_;
    /*! This flag is used to exclude this object from the list of statics
     * that can block a shoot.
     */
     bool excludedFromBlockers_;
};

/*!
 * Door map object class.
 */
class Door : public Static {
public:
    Door(uint16_t id, Map *pMap, int anim, int closingAnim, int openAnim, int openingAnim);
    virtual ~Door() {}

    void draw(const Point2D &screenPos) override;
    bool animate(uint32_t elapsed) override;
    bool isPathBlocker() override;

protected:
    int anim_, closing_anim_, open_anim_, opening_anim_;
};

/*!
 * LargeDoor map object class.
 */
class LargeDoor : public Static {
public:
    LargeDoor(uint16_t id, Map *pMap, int anim, int closingAnim, int openingAnim);
    virtual ~LargeDoor() {}

    void draw(const Point2D &screenPos) override;
    bool animate(uint32_t elapsed) override;
    bool isPathBlocker() override;

protected:
    int anim_, closing_anim_, opening_anim_;
};
/*!
 * Tree map object class.
 */
class Tree : public Static {
public:
    Tree(uint16_t id, Map *pMap, uint16_t baseAnim);
    virtual ~Tree() {}

    void draw(const Point2D &screenPos) override;
    
    void handleHit(DamageToInflict &d)override;

protected:
    void handleAnimationEnded() override;

protected:
    uint16_t idleAnim_;
    uint16_t burningAnim_;
    uint16_t burntAnim_;
};

/*!
 * WindowObj represents simple windows that are open or close.
 * It can be broken with bullet or explosion.
 */
class WindowObj : public Static {
public:
    WindowObj(uint16_t id, Map *pMap, StateWindows state, uint16_t anim);
    virtual ~WindowObj() {}

    void draw(const Point2D &screenPos) override;
    void handleHit(DamageToInflict &d) override;

protected:
    void handleAnimationEnded() override;

protected:
    //! Animation for when window is breaking
    uint16_t breakingAnim_;
    //! After window wa broken, it is damaged
    uint16_t damagedAnim_;
};

/*!
 * EtcObj map object class.
 */
class EtcObj : public Static {
public:
    EtcObj(uint16_t id, Map *pMap, uint16_t baseAnim, StaticType type = smt_None);
    virtual ~EtcObj() {}

    void draw(const Point2D &screenPos) override;

protected:
    uint16_t idleAnim_;
};

/*!
 * NeonSign map object class.
 */
class NeonSign : public Static {
public:
    NeonSign(uint16_t id, Map *pMap, uint16_t anim);
    virtual ~NeonSign() {}

    void draw(const Point2D &screenPos) override;
};

/*!
 * Semaphore map object class.
 * That thing that bounces on crossroad.
 * For animation, there are 4 animations when the semaphore is up
 * and 1 animation when the semaphore has been damaged. We cycle through
 * the animation by incrementing animOffset_.
 */
class Semaphore : public Static {
public:
    Semaphore(uint16_t id, Map *pMap, uint16_t animationOffset);
    virtual ~Semaphore() {}

    void draw(const Point2D &screenPos) override;

    void handleHit(DamageToInflict &d) override;

protected:
    void doUpdateState(uint32_t elapsed) override;

protected:
    //! Number of animations when semaphore is on
    static const uint16_t kSemaphoreMaxColorAnim;
    //! Offset for the damaged animation relative to the base animation
    static const uint16_t kSemaphoreDamagedOffset;

    /*! used to make animation of movement up/down,
     * when damaged, stores time not consumed for movement down
     */
    int elapsed_left_smaller_;
    /*! animation color rotation,
     * when damaged, stores target Z distance to fall
     */
    int elapsed_left_bigger_;
    //! switch for moving up or down
    int up_down_;
    //! Timer to control the change of light for the semaphore
    fs_utl::Timer colorTimer_;
    //! Use to cycle through different animations including damaged.
    uint16_t animOffset_;
};

/*!
 * AnimatedWindow map object class.
 */
class AnimWindow : public Static {
public:
    AnimWindow(uint16_t id, Map *pMap, uint16_t anim);
    virtual ~AnimWindow() {}

    bool animate(uint32_t elapsed) override;
    void draw(const Point2D &screenPos) override;

protected:
    void handleAnimationEnded() override;

public:
    int anim_;
    uint16_t animLigthOff_;
    uint16_t animLigthSwitching_;
    uint16_t animLightOn_;
    uint16_t animPedAppears_;
    uint16_t animShowPed_;
    uint16_t animPedDisappears_;
};

}
#endif

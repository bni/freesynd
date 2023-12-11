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

#ifndef MAPOBJECT_H
#define MAPOBJECT_H

#include <math.h>
#include <list>

#include "fs-kernel/model/mapobject.h"

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

    enum stateDoors {
        sttdoor_Closed = 0,
        sttdoor_Closing,
        sttdoor_Open,
        sttdoor_Opening
    };

    enum stateTrees {
        stttree_Healthy = 0,
        stttree_Burning,
        stttree_Damaged
    };

    //semaphore, 4 animations + damaged
    enum stateSemaphores {
        sttsem_Stt0 = 0,
        sttsem_Stt1,
        sttsem_Stt2,
        sttsem_Stt3,
        sttsem_Damaged
    };

    enum stateWindows {
        sttwnd_Closed = 0,
        sttwnd_Open,
        sttwnd_Breaking,
        sttwnd_Damaged
    };

    enum stateAnimatedWindows {
        sttawnd_LightOff = 0,
        sttawnd_LightSwitching,
        sttawnd_PedAppears,
        sttawnd_ShowPed,
        sttawnd_PedDisappears,
        sttawnd_LightOn
    };
public:
    static Static *loadInstance(uint8 *data, uint16 id, Map *pMap);
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
    Static(uint16 anId, Map *pMap, StaticType aType) :
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
    Door(uint16 id, Map *pMap, int anim, int closingAnim, int openAnim, int openingAnim);
    virtual ~Door() {}

    void draw(const Point2D &screenPos) override;
    bool animate(int elapsed) override;
    bool isPathBlocker();

protected:
    int anim_, closing_anim_, open_anim_, opening_anim_;
};

/*!
 * LargeDoor map object class.
 */
class LargeDoor : public Static {
public:
    LargeDoor(uint16 id, Map *pMap, int anim, int closingAnim, int openingAnim);
    virtual ~LargeDoor() {}

    void draw(const Point2D &screenPos) override;
    bool animate(int elapsed) override;
    bool isPathBlocker();

protected:
    int anim_, closing_anim_, opening_anim_;
};
/*!
 * Tree map object class.
 */
class Tree : public Static {
public:
    Tree(uint16 id, Map *pMap, int anim, int burningAnim, int damagedAnim);
    virtual ~Tree() {}

    void draw(const Point2D &screenPos) override;
    bool animate(int elapsed) override;
    void handleHit(fs_dmg::DamageToInflict &d)override;

protected:
    int anim_, burning_anim_, damaged_anim_;
};

/*!
 * Window map object class.
 */
class WindowObj : public Static {
public:
    WindowObj(uint16 id, Map *pMap, int anim, int openAnim, int breakingAnim,
              int damagedAnim);
    virtual ~WindowObj() {}

    bool animate(int elapsed) override;
    void draw(const Point2D &screenPos) override;
    void handleHit(fs_dmg::DamageToInflict &d) override;

protected:
    int anim_, open_anim_, breaking_anim_, damaged_anim_;
};

/*!
 * EtcObj map object class.
 */
class EtcObj : public Static {
public:
    EtcObj(uint16 id, Map *pMap, int anim, int burningAnim, int damagedAnim, StaticType type = smt_None);
    virtual ~EtcObj() {}

    void draw(const Point2D &screenPos) override;

protected:
    int anim_, burning_anim_, damaged_anim_;
};

/*!
 * NeonSign map object class.
 */
class NeonSign : public Static {
public:
    NeonSign(uint16 id, Map *pMap, int anim);
    virtual ~NeonSign() {}

    void draw(const Point2D &screenPos) override;

protected:
    int anim_;
};

/*!
 * Semaphore map object class.
 * That thing that bounces on crossroad.
 */
class Semaphore : public Static {
public:
    Semaphore(uint16 id, Map *pMap, int anim, int damagedAnim);
    virtual ~Semaphore() {}

    bool animate(int elapsed) override;
    void draw(const Point2D &screenPos) override;

    void handleHit(fs_dmg::DamageToInflict &d) override;

protected:
    int anim_, damaged_anim_;
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
};

/*!
 * AnimatedWindow map object class.
 */
class AnimWindow : public Static {
public:
    AnimWindow(uint16 id, Map *pMap, int anim);
    virtual ~AnimWindow() {}

    bool animate(int elapsed) override;
    void draw(const Point2D &screenPos) override;

protected:
    int anim_;
};

#endif

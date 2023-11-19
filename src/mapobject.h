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
#include "model/damage.h"
#include "path.h"
#include "pathsurfaces.h"

class Mission;
class WeaponInstance;

/*!
 * SFXObject map object class.
 */
class SFXObject : public MapObject {
public:
    /*!
     * Type of SfxObject.
     */
    enum SfxTypeEnum {
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

    SFXObject(int m, SfxTypeEnum type, int t_show = 0, bool managed = false);
    virtual ~SFXObject() {}

    bool sfxLifeOver() { return sfx_life_over_; }
    //! Return true if object is managed by another object
    bool isManaged() { return managed_; }
    //! Set whether animation should loop or not
    void setLoopAnimation(bool flag) { loopAnimation_ = flag; }
    //! Reset animation
    void reset();

    void draw(const Point2D &screenPos, GameSpriteManager &spriteMgr) override;
    bool animate(int elapsed) override;
    void correctZ();
    void setDrawAllFrames(bool daf) {
        if (daf != draw_all_frames_) {
            draw_all_frames_ = daf;
            frame_ = 0;
        }
    }
protected:
    static uint16 sfxIdCnt;
    /*! The type of SfxObject.*/
    SfxTypeEnum type_;
    int anim_;
    bool sfx_life_over_;
    // to draw all frames or first frame only
    bool draw_all_frames_;
    //! Tells if the animation should restart automatically after ending
    bool loopAnimation_;
    int elapsed_left_;
    //! True means the life of the object is managed by something else than gameplaymenu
    bool managed_;
};


/*!
 * Shootable map object class.
 */
class ShootableMapObject : public MapObject {
public:
    ShootableMapObject(uint16 id, int m, ObjectNature nature);
    virtual ~ShootableMapObject() {}

    int health() { return health_; }

    // NOTE: objects that are not used or have negative health from start
    // shoud have -1; objects destroyed during gameplay 0
    void setHealth(int n) {
        if (n > 255)
            n = 255;

        if (n < -1)
            n = -1;

        health_ = n;
    }

    int startHealth() { return start_health_; }

    void setStartHealth(int n) {
        if (n > 255)
            n = 255;

        if (n < -1)
            n = -1;

        start_health_ = n;
    }

    /*!
     * Add a certain amount to health.
     * \return true if health reached max
     * \param amount how much to increase health
     */
    bool increaseHealth(int amount) {
        health_ += amount;
        if (health_ > start_health_) {
            health_ = start_health_;
        }

        return health_ == start_health_;
    }

    /*!
     * Remove a certain amount of health.
     */
    void decreaseHealth(int amount) {
        health_ -= amount;
        if (health_ <= 0) {
            health_ = 0;
        }
    }
    /*!
     * Reset current ped's health to starting health.
     */
    void resetHealth() {
        health_ = start_health_;
    }

    /*!
     * Method called when object is hit by a weapon shot.
     * By default do nothing. Subclasses must implement
     * to react to a shot.
     * \param d Damage description
     */
    virtual void handleHit(fs_dmg::DamageToInflict &d) {}

    bool isAlive() { return health_ > 0; }
    bool isDead() { return health_ <= 0; }

  protected:
    int health_, start_health_;
};

/*!
 * Shootable movable map object class.
 */
class ShootableMovableMapObject : public ShootableMapObject {
public:
    ShootableMovableMapObject(uint16 id, int m, ObjectNature nature);
    virtual ~ShootableMovableMapObject() {}

    void setSpeed(int new_speed) {
        speed_ = new_speed;
    }

    int speed() {
        return speed_;
    }
    void setBaseSpeed(int bs) {
        base_speed_ = bs;
    }
    /*!
     * Clear path to destination and sets speed to 0.
     */
    void clearDestination() {
        dest_path_.clear();
        speed_ = 0;
    }


    /**
     * Compute a path from the object current position to given destination using given speed.
     * Subclasses must implement this method.
     * \param m Mission* Mission data
     * \param locT Destination for the movement
     * \param newSpeed The speed at which the object will move. If speed is -1, use default speed.
     * \return true if there is a path towards given destination
     */
    virtual bool initMovementToDestination(Mission *m, const TilePoint &destinationPt, int newSpeed = -1) = 0;

    /**
     * Update position of the object using the current path and speed.
     * Subclasses must implement this method.
     * \param elapsed int
     * \param m Mission* Mission data
     * \return True if the position has been updated.
     *
     */
    virtual bool doMove(int elapsed, Mission *m) = 0;

    //! Return true if the ped is moving
    bool isMoving() { return speed_ != 0 || !dest_path_.empty();}
    //! Returns true if object currently has a destination point (ie it's arrived)
    bool hasDestination() { return !dest_path_.empty(); }

    FreeWay hold_on_;

protected:
    bool addOffsetToPosition(int nOffX, int nOffY);

protected:
    int speed_, base_speed_;
    //! on reaching this distance object should stop
    int dist_to_pos_;
    std::list<TilePoint> dest_path_;
};

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
    static Static *loadInstance(uint8 *data, uint16 id, int m);
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

    virtual bool animate(int elapsed, Mission *obj) {
        return MapObject::animate(elapsed);
    }

protected:
    Static(uint16 anId, int m, StaticType aType) :
            ShootableMapObject(anId, m, MapObject::kNatureStatic) {
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
    Door(uint16 id, int m, int anim, int closingAnim, int openAnim, int openingAnim);
    virtual ~Door() {}

    void draw(const Point2D &screenPos, GameSpriteManager &spriteMgr) override;
    bool animate(int elapsed, Mission *obj);
    bool isPathBlocker();

protected:
    int anim_, closing_anim_, open_anim_, opening_anim_;
};

/*!
 * LargeDoor map object class.
 */
class LargeDoor : public Static {
public:
    LargeDoor(uint16 id, int m, int anim, int closingAnim, int openingAnim);
    virtual ~LargeDoor() {}

    void draw(const Point2D &screenPos, GameSpriteManager &spriteMgr) override;
    bool animate(int elapsed, Mission *obj);
    bool isPathBlocker();

protected:
    int anim_, closing_anim_, opening_anim_;
};
/*!
 * Tree map object class.
 */
class Tree : public Static {
public:
    Tree(uint16 id, int m, int anim, int burningAnim, int damagedAnim);
    virtual ~Tree() {}

    void draw(const Point2D &screenPos, GameSpriteManager &spriteMgr) override;
    bool animate(int elapsed, Mission *obj);
    void handleHit(fs_dmg::DamageToInflict &d);

protected:
    int anim_, burning_anim_, damaged_anim_;
};

/*!
 * Window map object class.
 */
class WindowObj : public Static {
public:
    WindowObj(uint16 id, int m, int anim, int openAnim, int breakingAnim,
              int damagedAnim);
    virtual ~WindowObj() {}

    bool animate(int elapsed, Mission *obj);
    void draw(const Point2D &screenPos, GameSpriteManager &spriteMgr) override;
    void handleHit(fs_dmg::DamageToInflict &d);

protected:
    int anim_, open_anim_, breaking_anim_, damaged_anim_;
};

/*!
 * EtcObj map object class.
 */
class EtcObj : public Static {
public:
    EtcObj(uint16 id, int m, int anim, int burningAnim, int damagedAnim, StaticType type = smt_None);
    virtual ~EtcObj() {}

    void draw(const Point2D &screenPos, GameSpriteManager &spriteMgr) override;

protected:
    int anim_, burning_anim_, damaged_anim_;
};

/*!
 * NeonSign map object class.
 */
class NeonSign : public Static {
public:
    NeonSign(uint16 id, int m, int anim);
    virtual ~NeonSign() {}

    void draw(const Point2D &screenPos, GameSpriteManager &spriteMgr) override;

protected:
    int anim_;
};

/*!
 * Semaphore map object class.
 * That thing that bounces on crossroad.
 */
class Semaphore : public Static {
public:
    Semaphore(uint16 id, int m, int anim, int damagedAnim);
    virtual ~Semaphore() {}

    bool animate(int elapsed, Mission *obj);
    void draw(const Point2D &screenPos, GameSpriteManager &spriteMgr) override;

    void handleHit(fs_dmg::DamageToInflict &d);

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
    AnimWindow(uint16 id, int m, int anim);
    virtual ~AnimWindow() {}

    bool animate(int elapsed, Mission *obj);
    void draw(const Point2D &screenPos, GameSpriteManager &spriteMgr) override;

protected:
    int anim_;
};

#endif

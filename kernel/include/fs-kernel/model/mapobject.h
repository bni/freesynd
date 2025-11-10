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

#ifndef KERNEL_MAPOBJECT_H
#define KERNEL_MAPOBJECT_H

#include <math.h>
#include <list>
#include <memory>

#include "fs-utils/common.h"
#include "fs-engine/enginecommon.h"
#include "fs-engine/gfx/animationplayer.h"
#include "fs-kernel/model/position.h"
#include "fs-kernel/model/damage.h"

namespace fs_knl {

class Map;
class Mission;

/*!
 * Map object class.
 */
class MapObject {
public:
    /*!
     * Express the nature of a MapObject.
     */
    enum ObjectNature {
        kNatureUndefined = 0,
        kNaturePed = 1,
        kNatureWeapon = 2,
        kNatureStatic = 4,
        kNatureVehicle = 8
    };

public:
    MapObject(uint16_t id, Map *pMap, ObjectNature nature);
    virtual ~MapObject() {}

    /**
     * @name Inner attributes
     */
    ///@{
    //! Return the nature of the object
    ObjectNature nature() { return nature_; }
    //! Return true the object has the same nature as the given one
    bool is(ObjectNature aNature) const { return nature_ == aNature; }
    //! For debug purpose
    const char* natureName();

    //! Return the object's id
    uint16_t id() const { return id_; }

    //!Sets the current map
    void setMap(Map *pMap) {
        pMap_ = pMap;
    }
    ///@}

    /**
     * @name Position
     */
    ///@{
    const TilePoint & position() const { return pos_; }

    void setPosition(int tile_x, int tile_y, int tile_z, int off_x = 0,
            int off_y = 0, int off_z = 0) {
        pos_.tx = tile_x;
        pos_.ty = tile_y;
        pos_.tz = tile_z;
        pos_.ox = off_x;
        pos_.oy = off_y;
        pos_.oz = off_z;
    }

    void setPosition(const TilePoint &pos) {
        pos_.initFrom(pos);
    }

    /*!
     * Set the position of the object to be the given one.
     * \param pos New object position
     */
    void setPosition(const WorldPoint &pos) {
        setPosition(pos.x / 256, pos.y / 256, pos.z / 128, pos.x % 256,
                    pos.y % 256, pos.z % 128 );
    }

    void offzOnStairs(uint8_t twd);

    int tileX() const { return pos_.tx; }
    int tileY() const { return pos_.ty; }
    int tileZ() const { return pos_.tz; }

    void setTileX(int x) { pos_.tx = x; }
    void setTileY(int y) { pos_.ty = y; }
    void setTileZ(int z) { pos_.tz = z; }

    int offX() const { return pos_.ox; }
    int offY() const { return pos_.oy; }
    int offZ() const { return pos_.oz; }

    void setOffX(int n);
    void setOffY(int n);
    void setOffZ(int n);

    int sizeX() { return size_x_;}
    int sizeY() { return size_y_;}
    int sizeZ() { return size_z_;}

    void setSize(int sizeX, int sizeY, int sizeZ) { 
        size_x_ = sizeX;
        size_y_ = sizeY;
        size_z_ = sizeZ;
    }
    
    /*!
     *
     */
    bool samePosition(MapObject * other) {
        return pos_.equals(other->position());
    }

    /*!
     * checks whether current position is on the same tile as
     * posT.
     * \param posT point to compare.
     * \return true if tile is the same
     */
    bool sameTile(const TilePoint &posT) {
        return posT.tx == pos_.tx
        && posT.ty == pos_.ty
        && posT.tz == pos_.tz;
    }

    /*!
     * Return true if the distance between this object and the given object
     * is less than the given distance.
     * \param pObject The other object.
     * \param distance
     */
    bool isCloseTo(MapObject *pObject, int distance) {
        WorldPoint wp(pObject->position());
        return isCloseTo(wp, distance);
    }

    /*!
     * Return true if the distance between this object and the given location
     * is less than the given distance.
     * \param loc The location.
     * \param distance
     */
    bool isCloseTo(const WorldPoint &loc, int32_t distance) {
        int cx = pos_.tx * 256 + pos_.ox - (loc.x);
        int cy = pos_.ty * 256 + pos_.oy - (loc.y);
        int cz = pos_.tz * 128 + pos_.oz - (loc.z);

        return (cx * cx + cy * cy + cz * cz) < (distance * distance);
    }

    double distanceToPosition(const WorldPoint &pos) {
        int cx = pos_.tx * 256 + pos_.ox - (pos.x);
        int cy = pos_.ty * 256 + pos_.oy - (pos.y);
        int cz = pos_.tz * 128 + pos_.oz - (pos.z);

        return sqrt((double) (cx * cx + cy * cy + cz * cz));
    }

    double distanceToPosSz(const WorldPoint &pos) {
        int cx = pos_.tx * 256 + pos_.ox - (pos.x);
        int cy = pos_.ty * 256 + pos_.oy - (pos.y);
        int cz = pos_.tz * 128 + pos_.oz + (size_z_ >> 1) - (pos.z);

        return sqrt((double) (cx * cx + cy * cy + cz * cz));
    }


    /**
     * Returns true if given object is farther than this object.
     * Objects are considered on same tile. farther objects are drawn first.
     * \param pOther MapObject*
     * \return bool
     *
     */
    bool isBehindObjectOnSameTile(MapObject *pOther) {
        return pos_.ox < pOther->position().ox || pos_.oy < pOther->position().oy;
    }
    ///@}

    /**
     * @name Direction
     */
    ///@{
    void setDirection(int dir);
    void setDirection(int posx, int posy, int * dir = NULL);
    //! Set this ped's direction so that he looks at the given object.
    void setDirectionTowardObject(const MapObject &object);
    //! Set this ped's direction so that he looks at the given position.
    void setDirectionTowardPosition(const WorldPoint &pos);

    int direction() { return dir_;}
    //! Converts a direction into a discrete index. Default is set to 8 for possible directions for Peds.
    uint8_t getDiscreteDirection(int snum = 8);
    ///@}

    /**
     * @name Path management
     */
    ///@{
    struct FreeWay {
        // 0 - can go; 1 - wait; 2 - stop
        char wayFree;
        int tilex;
        int tiley;
        int tilez;
        //! create range by x, should be 0 if not used
        int xadj;
        //! create range by y, should be 0 if not used
        int yadj;
        MapObject * pathBlocker;
    };

    virtual bool isPathBlocker() {
        return false;
    }
    ///@}

    bool isBlocker(WorldPoint * pStartPt, WorldPoint * pEndPt,
        double * inc_xyz);

    /**
     * @name Animation
     */
    ///@{
    /** @brief Animates the object
     * SubClasses can implement this method.
     * @param elapsed int Time elapsed since last animation
     *
     */
    virtual void animate(uint32_t elapsed);

    /*!
     * Convenient method to play an animation using AnimationPlayer
     * @param mapObjectAnimationId The animation id to play
     * @param startFrame The starting frame
     * @param newMaxPlayTime Overrides maxTime
     * @return True if animation has been found
     */
    bool playAnimation(const uint16_t mapObjectAnimationId, const uint8_t startFrame = 0, uint32_t newMaxPlayTime = 0) { 
        return animationPlayer_->play(mapObjectAnimationId, startFrame, newMaxPlayTime); 
    }

    /*!
     * Convenient method to reset current animation
     */
    void resetAnimation() {
        animationPlayer_->resetAnimation();
    }

    ///@}

    /**
     * @name Draw object
     */
    ///@{
    //! Set if MapObject is visible on screen
    void setDrawable(bool drawable) {
        isDrawable_ = drawable;
    }

    //! Returns true is Object is visible on screen
    bool isDrawable() {
        return isDrawable_;
    }

    /*!
     * Draw this object at the given screen position.
     * Subclasses need to implement this method
     * @param screenPos Position on the screen
     */
    virtual void draw(const Point2D &screenPos) = 0;
    ///@}

protected:
    Point2D addOffs(const Point2D &screenPos);

    //! Subclasses reimplement this to update their internal state
    virtual void doUpdateState([[maybe_unused]] uint32_t elapsed) {}
    //! Subclasse reimplement this method to react to the end of an animation
    virtual void handleAnimationEnded() {}
    /*!
     * Creates a new instance of AnimationPlayer.
     * Subclasses can implement to set a different class
     */
    virtual std::unique_ptr<fs_eng::AnimationPlayer> createAnimationPlayer() {
        return std::make_unique<fs_eng::AnimationPlayer>();
    }

protected:
    //! Id of the object. Id is unique within a nature
    uint16_t id_;
    //! A pointer to the map that the object is on
    Map *pMap_;
    //! the nature of this object
    ObjectNature nature_;
    /*!
     * Tile based coordinates.
     */
    TilePoint pos_;
    //! these are not true sizes, but halfs of full size by respective coord
    int size_x_, size_y_, size_z_;
    //! objects direction
    int dir_;
    //! Animation player is used to store and play animation for this object
    std::unique_ptr<fs_eng::AnimationPlayer> animationPlayer_;

private:
    //! Object should be drawn only if visible
    bool isDrawable_;
};

/*!
 * A MapObject that has a health, starting at a given level and that can
 * take damage.
 */
class ShootableMapObject : public MapObject {
public:
    ShootableMapObject(uint16_t id, Map *pMap, ObjectNature nature);
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

    /*!
     * Set value of start health
     * @param n value to set
     * @param setHealth if true, health will be set with same value
     */
    void setStartHealth(int n, bool setHealth = false) {
        if (n > 255)
            n = 255;

        if (n < -1)
            n = -1;

        start_health_ = n;

        if (setHealth) {
            health_ = start_health_;
        }
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
    virtual void handleHit([[maybe_unused]] DamageToInflict &d) {}

    bool isAlive() { return health_ > 0; }
    bool isDead() { return health_ <= 0; }

  protected:
    int health_, start_health_;
};

/*!
 * @brief An object that can move and be shootable like Peds and vehicles.
 * Each object has a current speed that has a max. This max is fixe for vehicles.
 * You can set the destination using initMovementToDestination().
 */
class ShootableMovableMapObject : public ShootableMapObject {
public:
    ShootableMovableMapObject(uint16_t id, Map *pMap, ObjectNature nature, int maxSpeed = 0);
    virtual ~ShootableMovableMapObject() {}

    /*!
     * @brief Set speed for the object.
     * Given speed will be pass to applySpeedModifier() method before being set.
     * @param newSpeed Speed to set
     */
    void setSpeed(int newSpeed) {
        speed_ = applySpeedModifier(newSpeed);
    }

    /*!
     * @brief Set speed for the object to the max.
     */
    void setSpeedToMax() {
        speed_ = maxSpeed_;
    }

    //! Set speed to zero
    void stop() { speed_ = 0; }

    int speed() {
        return speed_;
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
    virtual bool doMove(uint32_t elapsed, Mission *m) = 0;

    //! Return true if the ped is moving
    bool isMoving() { return speed_ != 0; }
    //! Returns true if object currently has a destination point (ie it's arrived)
    bool hasDestination() { return !dest_path_.empty(); }

    FreeWay hold_on_;

protected:
    bool addOffsetToPosition(int nOffX, int nOffY);

    /*!
     * @brief Return a modified speed based on contextual parameters.
     * @return By default return unmodified speed
     */
    virtual int applySpeedModifier(int speed) { return speed; }

protected:
    //! on reaching this distance object should stop
    int dist_to_pos_;
    std::list<TilePoint> dest_path_;

private:
    //! Current speed of the objet
    int speed_;
    //! Maximum speed of the object. For vehicle, maximum speed is fixed
    int maxSpeed_;
};

}
#endif  //KERNEL_MAPOBJECT_H

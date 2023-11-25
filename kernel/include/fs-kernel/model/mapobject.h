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

#ifndef KERNEL_MAPOBJECT_H
#define KERNEL_MAPOBJECT_H

#include <math.h>
#include <list>

#include "fs-utils/common.h"
#include "fs-kernel/model/position.h"
#include "fs-kernel/model/damage.h"

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
    MapObject(uint16 id, Map *pMap, ObjectNature nature);
    virtual ~MapObject() {}

    //! Return the nature of the object
    ObjectNature nature() { return nature_; }
    //! Return true the object has the same nature as the given one
    bool is(ObjectNature aNature) const { return nature_ == aNature; }
    //! For debug purpose
    const char* natureName();

    //! Return the object's id
    uint16 id() const { return id_; }

    //! Set if MapObject is visible on screen
    void setDrawable(bool drawable) {
        isDrawable_ = drawable;
    }

    //! Returns true is Object is visible on screen
    bool isDrawable() {
        return isDrawable_;
    }

    //!Sets the current map
    void setMap(Map *pMap) {
        pMap_ = pMap;
    }

    virtual void draw(const Point2D &screenPos) = 0;

    const TilePoint & position()const { return pos_; }

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

    void setSizeX(int size_x) { size_x_ = size_x;}
    void setSizeY(int size_y) { size_y_ = size_y;}
    void setSizeZ(int size_z) { size_z_ = size_z;}

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
    bool isCloseTo(const WorldPoint &loc, int32 distance) {
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

    /** \brief Animates the object
     * SubClasses can implement this method.
     * \param elapsed int Time elapsed since last animation
     * \return virtual bool True mean the object has changed
     *
     */
    virtual bool animate(int elapsed);

    void setFramesPerSec(int framesPerSec)
    {
        frames_per_sec_ = framesPerSec;
    }
    int getFramesPerSec() { return frames_per_sec_; }

    typedef struct{
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
    }FreeWay;

    virtual bool isPathBlocker() {
        return false;
    }

    void setFrame(int frame) { frame_ = frame;}
    void setFrameFromObject(MapObject *m) {
        frame_ = m->frame_;
    }

    void setDirection(int dir);
    void setDirection(int posx, int posy, int * dir = NULL);
    //! Set this ped's direction so that he looks at the given object.
    void setDirectionTowardObject(const MapObject &object);
    //! Set this ped's direction so that he looks at the given position.
    void setDirectionTowardPosition(const WorldPoint &pos);

    int direction() { return dir_;}
    int getDirection(int snum = 8);

    void setTimeShowAnim(int t) {
        frame_ = 0;
        time_show_anim_ = t;
        time_showing_anim_ = 0;
    }
    bool leftTimeShowAnim(int t) {
        if (time_show_anim_ == -1)
            return true;
        time_showing_anim_ += t;
        return time_show_anim_ > time_showing_anim_;
    }

    bool isBlocker(WorldPoint * pStartPt, WorldPoint * pEndPt,
               double * inc_xyz);

    void setStateMasks(unsigned int state) {
        state_ = state;
    }
    unsigned int stateMasks() { return state_; }

    void offzOnStairs(uint8 twd);

protected:
    Point2D addOffs(const Point2D &screenPos);

protected:
    //! the nature of this object
    ObjectNature nature_;
    //! Id of the object. Id is unique within a nature
    uint16 id_;
    /*!
     * Tile based coordinates.
     */
    TilePoint pos_;
    //! these are not true sizes, but halfs of full size by respective coord
    int size_x_, size_y_, size_z_;
    //! A pointer to the map that the object is on
    Map *pMap_;
    //! animation frame changing
    int frame_;
    /*!
     * Time left, if frame needs to be drawn every 'n' milliseconds
     * elapsed time % 'n' = time left
     */
    int elapsed_carry_;
    //! how often this frame should be drawn per seccond
    int frames_per_sec_;
    //! objects direction
    int dir_;
    //! looped animations, time to show them is set here, if = -1 show forever
    int time_show_anim_;
    //! looped animations, playing time
    int time_showing_anim_;
    /*! animation to draw can, change this varible should provide check
     * whether first frame was drawn successfully, only if successful
     * MapObject::animate should be called
     */
    bool is_frame_drawn_;
    uint32 state_;

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
    ShootableMapObject(uint16 id, Map *pMap, ObjectNature nature);
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
    ShootableMovableMapObject(uint16 id, Map *pMap, ObjectNature nature);
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

#endif  //KERNEL_MAPOBJECT_H

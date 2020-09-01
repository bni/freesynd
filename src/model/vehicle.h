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

#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>

#include "mapobject.h"
#include "ped.h"
#include "map.h"

/*!
 * This class holds informations about the animation of a vehicle.
 */
class VehicleAnimation {
public:
    /*!
     * This enumeration lists the type of animations.
     */
    enum AnimationType {
        //! Vehicule under normal condition
        kNormalAnim,
        //! Burning vehicle
        kOnFireAnim,
        //! Burnt vehicle
        kBurntAnim,
    } ;

    VehicleAnimation();
    virtual ~VehicleAnimation() {}

    //! Draw the vehicle
    void draw(int x, int y, int dir, int frame);

    void set_base_anims(int anims);
    //void setAnimsBurning(int anims) { anims_burning_ = anims; }
    //void setAnimsBurnt(int anims) { anims_burnt_ = anims; }
    //! Sets the current animation for the vehicle
    void set_animation_type(AnimationType anim) {
        vehicle_anim_ = anim;
    }
    //! Returns the current vehicle animation
    AnimationType animation_type() {
        return vehicle_anim_;
    }
protected:
    int anims_, anims_burning_, anims_burnt_;
    //! Current type of animation
    AnimationType vehicle_anim_;
};

/*!
 * Generic class for all transports.
 * Transport can be driven or not.
 */
class Vehicle : public ShootableMovableMapObject{
public:
    static const uint8 kVehicleTypeLargeArmored;
    static const uint8 kVehicleTypeLargeArmoredDamaged;
    static const uint8 kVehicleTypeTrainHead;
    static const uint8 kVehicleTypeTrainBody;
    static const uint8 kVehicleTypeRegularCar;
    static const uint8 kVehicleTypeFireFighter;
    static const uint8 kVehicleTypeSmallArmored;
    static const uint8 kVehicleTypePolice;
    static const uint8 kVehicleTypeMedics;

    Vehicle(uint16 anId, uint8 aType, int m, VehicleAnimation *pAnimation) : ShootableMovableMapObject(anId, m, MapObject::kNatureVehicle) {
        type_ = aType;
        animation_ = pAnimation;
    }

    virtual ~Vehicle() {
        delete animation_;
    }

    bool animate(int elapsed);
    void draw(int x, int y);

    //! Return type of vehicle
    uint8 getType() { return type_; }
    //! Return true if vehicle is a car
    bool isCar() { return type_ != kVehicleTypeTrainHead && type_ != kVehicleTypeTrainBody; }

    //! Adds the given ped to the list of passengers
    virtual void addPassenger(PedInstance *p);
    //! Removes the passenger from the vehicle
    virtual void dropPassenger(PedInstance *p);

    //! Returns true if given ped is in the vehicle
    bool containsPed(PedInstance *p) {
        for (std::list<PedInstance *>::iterator it = passengers_.begin();
        it != passengers_.end(); it++)
        {
            if ((*it)->id() == p->id()) {
                return true;
            }
        }
        return false;
    }
    //! Returns true if the vehicle contains one of our agent
    bool containsOurAgents();
    //! Returns true if the vehicle contains peds considered hostile by the given ped
    bool containsHostilesForPed(PedInstance *p, unsigned int hostile_desc_alt);

protected:
    /*! The passengers of the vehicle.*/
    std::list <PedInstance *> passengers_;
    /*! Animation for vehicle.*/
    VehicleAnimation *animation_;

private:
    /*! Type of vehicle.*/
    uint8 type_;
};

/*!
 * This class represents a playable car on a map.
 */
class GenericCar : public Vehicle
{
public:
    GenericCar(VehicleAnimation *pAnimation, uint16 id, uint8 aType, int m);
    virtual ~GenericCar() {}

    //! See ShootableMovableMapObject::initMovementToDestination()
    bool initMovementToDestination(Mission *m, const TilePoint &destinationPt, int newSpeed = -1);

    void addDestinationV(int x, int y, int z, int ox = 128, int oy = 128,
            int new_speed = 160) {
        dest_path_.push_back(TilePoint(x, y, z, ox, oy));
        speed_ = new_speed;
    }

    //! See ShootableMovableMapObject::doMove()
    bool doMove(int elapsed, Mission *m);

    //! Adds the given ped to the list of passengers
    void addPassenger(PedInstance *p);
    //! Removes the passenger from the vehicle
    void dropPassenger(PedInstance *p);

    PedInstance *getDriver(void) {
        return pDriver_;
    }
    //! Set this ped as the new driver
    void setDriver(PedInstance *pPed, bool forceDriver = true);

    /*!
     * Return true given ped is the driver of this vehicle.
     * \param pPed a Ped.
     */
    bool isDriver(PedInstance *pPed) {
        return (pPed != NULL && pDriver_ == pPed);
    }

    void handleHit(fs_dmg::DamageToInflict &d);

protected:
    bool findPathToNearestWalkableTile(Map *pMap, const TilePoint &startPt, int *basex, int *basey, std::vector < TilePoint > *path2add);
    uint16 tileDir(int x, int y, int z);
    bool dirWalkable(TilePoint *p, int x, int y, int z);

protected:
    //! Vehicle driver
    PedInstance *pDriver_;
};

#endif

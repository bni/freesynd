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

#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>

#include "fs-kernel/model/mapobject.h"
#include "fs-kernel/model/map.h"
#include "ped.h"

namespace fs_knl {

/*!
 * Generic class for all transports.
 * Transport can be driven or not.
 */
class Vehicle : public ShootableMovableMapObject{
public:
    static const uint8_t kVehicleTypeLargeArmored;
    static const uint8_t kVehicleTypeLargeArmoredDamaged;
    static const uint8_t kVehicleTypeTrainHead;
    static const uint8_t kVehicleTypeTrainBody;
    static const uint8_t kVehicleTypeRegularCar;
    static const uint8_t kVehicleTypeFireFighter;
    static const uint8_t kVehicleTypeSmallArmored;
    static const uint8_t kVehicleTypePolice;
    static const uint8_t kVehicleTypeMedics;

    Vehicle(uint16_t anId, uint8_t aType, Map *pMap, int maxSpeed) :
        ShootableMovableMapObject(anId, pMap, MapObject::kNatureVehicle, maxSpeed) {
        type_ = aType;
    }

    virtual ~Vehicle() {}

    //! Animation used when the vehicle is ok
    uint16_t regularAnimation() { return regularAnimation_; }
    //! Animation used when the vehicle is burnt
    uint16_t burntAnimation() { return burntAnimation_; }

    void draw(const Point2D &screenPos) override;

    //! Return type of vehicle
    uint8_t getType() { return type_; }
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

    /*!
     * @brief Build vehicle animations using the base animation.
     */
    void setAnimations(uint16_t baseSpriteAnimationId);

protected:
    void doUpdateState([[maybe_unused]] uint32_t elapsed) override;
    void handleAnimationEnded() override;

protected:
    /*! The passengers of the vehicle.*/
    std::list <PedInstance *> passengers_;
    //! Animation used when the vehicle is ok
    uint16_t regularAnimation_;
    //! Animation used when the vehicle is burning
    uint16_t burningAnimation_;
    //! Animation used after the vehicle has finished burning
    uint16_t burntAnimation_;

private:
    /*! Type of vehicle.*/
    uint8_t type_;
};

/*!
 * This class represents a playable car on a map.
 */
class GenericCar : public Vehicle
{
public:
    GenericCar(uint16_t id, uint8_t aType, Map *pMap, int maxSpeed);
    virtual ~GenericCar() {}

    //! See ShootableMovableMapObject::initMovementToDestination()
    bool initMovementToDestination(Mission *m, const TilePoint &destinationPt, int newSpeed = -1) override;

    //! @copydoc ShootableMovableMapObject::doMove()
    bool doMove(uint32_t elapsed, Mission *m) override;

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

    void handleHit(DamageToInflict &d);

protected:
    bool findPathToNearestWalkableTile(const TilePoint &startPt, int *basex, int *basey, std::vector < TilePoint > *path2add);
    uint16_t tileDir(int x, int y, int z);
    bool dirWalkable(TilePoint *p, int x, int y, int z);

protected:
    //! Vehicle driver
    PedInstance *pDriver_;
};

}
#endif

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
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

#ifndef MODEL_OBJECTIVEDESC_H
#define MODEL_OBJECTIVEDESC_H

#include <vector>

#include "common.h"
#include "mapobject.h"
#include "ped.h"
#include "core/gameevent.h"

/*!
 * Defines all possible status for the current objective.
 */
enum EObjectiveStatus {
    //! Objective is not started
    kNotStarted,
    //! Objective is started
    kStarted,
    //! Objective has failed
    kFailed,
    //! Objective is completed
    kCompleted
};

class Mission;

/*!
 * An ObjectiveDesc class holds the elements defining an objective.
 */
class ObjectiveDesc {
public:
    ObjectiveDesc() {
        indx_grpid.targetindx = 0;
        status = kNotStarted;
        subobjindx = 0;
        nxtobjindx = 0;
    }

    virtual ~ObjectiveDesc() {};

    union {
        // index within vector of data
        uint16 targetindx;
        uint16 grpid;
    } indx_grpid;

    /*! Status of the objective.*/
    EObjectiveStatus status;

    // indx for sub objective
    uint16 subobjindx;

    //! This message should be set during objective definition
    std::string msg;
    uint16 nxtobjindx;

    /*!
     * Return true if objective is cleared (succeeded or failed)
     */
    bool isTerminated() {
        return status == kCompleted || status == kFailed;
    }

    /*!
     * This method declares the objective as 'started'.
     * Then calls handleStart() to give the class the ability
     * to customize the start phase.
     */
    void start(Mission *p_mission) {
        status = kStarted;
        handleStart(p_mission);
    }

    /*!
     * This method is call to evaluate the status of the objective in the
     * current mission.
     * Subclasses must implements this method.
     */
    virtual void evaluate(Mission *pMission) = 0;

protected:
    /*!
     * Subclasses should implements this method to do specific tasks on starting.
     */
    virtual void handleStart(Mission *p_mission) {}

    /*!
     * A common method to end objective.
     * \param succeeded True means objective is completed with success.
     */
    void endObjective(bool succeeded);
};

/*!
 * A ObjEliminate defines an objective where player has to kill every
 * ped of a given type.
 */
class ObjEliminate : public ObjectiveDesc {
public:
    ObjEliminate(PedInstance::objGroupDefMasks subtype);

    void evaluate(Mission *pMission);

protected:
    /*! The group to eliminate.*/
    uint32 groupDefMask_;
};

/*!
 * A TargetObjective defines an objective with a MapObject as target.
 */
class TargetObjective : public ObjectiveDesc {
public:
    TargetObjective(MapObject * pMapObject) : ObjectiveDesc() {
        p_target_ = pMapObject;
    }

    MapObject * target() { return p_target_; }
protected:
    /*!
     * All targeted objectives sends the same event to indicate
     * the target to the user (signal on the map).
     */
    void handleStart(Mission *p_mission);

protected:
    MapObject *p_target_;
};

/*!
 * This objective is for persuading civilians.
 */
class ObjPersuade : public TargetObjective {
public:
    ObjPersuade(MapObject * pMapObject);

    void evaluate(Mission *pMission);
};

/*!
 * A ObjAssassinate defines an objective where a Ped has to be killed.
 */
class ObjAssassinate : public TargetObjective {
public:
    ObjAssassinate(MapObject * pMapObject);

    void evaluate(Mission *pMission);
};

/*!
 * A ObjProtect defines an objective where player must prevent a Ped from being killed.
 */
class ObjProtect : public TargetObjective {
public:
    ObjProtect(MapObject * pMapObject);

    void evaluate(Mission *pMission);
};

/*!
 * This objective is for destroying a vehicle.
 */
class ObjDestroyVehicle : public TargetObjective {
public:
    ObjDestroyVehicle(MapObject * pVehicle);

    void evaluate(Mission *pMission);
};

/*!
 * This objective is for taking control of a vehicle.
 */
class ObjUseVehicle : public TargetObjective {
public:
    ObjUseVehicle(MapObject * pVehicle);

    void evaluate(Mission *pMission);
};

/*!
 * This objective is for taking a weapon.
 */
class ObjTakeWeapon : public TargetObjective {
public:
    ObjTakeWeapon(MapObject * pWeapon);

    void evaluate(Mission *pMission);
};

/*!
 * A LocationObjective defines an objective with a Location as target.
 */
class LocationObjective : public ObjectiveDesc {
public:
    LocationObjective(int x, int y, int z) : ObjectiveDesc() {
        objectiveLocw_.x = x;
        objectiveLocw_.y = y;
        objectiveLocw_.z = z;
    }

    const WorldPoint & location() { return objectiveLocw_; }
protected:
    WorldPoint objectiveLocw_;
};

/*!
 * This objective is for evacuating a list of peds to a location.
 * Objective is completed if agents and peds are in the red zone.
 */
class ObjEvacuate : public LocationObjective {
public:
    ObjEvacuate(int x, int y, int z, std::vector <PedInstance *> &lstOfPeds);

    void handleStart(Mission *p_mission);

    void evaluate(Mission *pMission);

protected:
    std::vector <PedInstance *> pedsToEvacuate;
};

#endif // MODEL_OBJECTIVEDESC_H

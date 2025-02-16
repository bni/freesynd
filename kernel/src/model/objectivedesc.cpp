/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
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

#include "fs-kernel/model/objectivedesc.h"

#include "fs-engine/appcontext.h"
#include "fs-engine/events/event.h"
#include "fs-kernel/model/ped.h"
#include "fs-kernel/model/vehicle.h"
#include "fs-kernel/model/squad.h"
#include "fs-kernel/model/mission.h"

namespace fs_knl {

/*!
 * Evaluate the objective. By default, only started objectives can
 * be evaluated. Moreover, this method checks that not all squad is dead.
 * If so, the objective is failed.
 * @param pMission 
 */
void ObjectiveDesc::evaluate(Mission *pMission) {
    if ( status == kStarted) {
        if (pMission->getSquad()->isAllDead()) {
            endObjective(false);
        } else {
            doEvaluate(pMission);
        }
    }
}

/*!
 * A common method to end targeted objective.
 * \param succeeded True means objective is completed with success.
 */
void ObjectiveDesc::endObjective(bool succeeded) {
    if ( status == kStarted) {
        status = succeeded ? kCompleted : kFailed;

        EventManager::fire<ObjectiveEndedEvent>(succeeded);
    }
}

/*!
 * All targeted objectives sets the current target to the objective target.
 */
void TargetObjective::handleStart() {
    EventManager::fire<TargetObjectiveStartedEvent>(p_target_);
}

ObjPersuade::ObjPersuade(MapObject * pMapObject) : TargetObjective(pMapObject) {
    msg = g_Ctx.getMessage("GOAL_PERSUADE");
}

/*!
 * Evaluate the objective.
 * \param evt
 * \param pMission
 */
void ObjPersuade::doEvaluate([[maybe_unused]] Mission *pMission) {
    PedInstance *p = static_cast<PedInstance *>(p_target_);
    if (p->isDead())
    {
        // Target is dead -> mission is failed
        endObjective(false);
    } else if (p->isPersuaded()) {
        endObjective(true);
    }
}

ObjAssassinate::ObjAssassinate(MapObject * pMapObject) : TargetObjective(pMapObject) {
    msg = g_Ctx.getMessage("GOAL_ASSASSINATE");
}

/*!
 * Evaluate the objective.
 * \param evt
 * \param pMission
 */
void ObjAssassinate::doEvaluate([[maybe_unused]] Mission *pMission) {
    PedInstance *p = static_cast<PedInstance *>(p_target_);
    if (p->isDead()) {
        // Target is dead -> objective is completed
        endObjective(true);
    } else if (p->hasEscaped()) {
        endObjective(false);
    }
}

ObjProtect::ObjProtect(MapObject * pMapObject) : TargetObjective(pMapObject) {
    msg = g_Ctx.getMessage("GOAL_PROTECT");
}

/*!
 * Evaluate the objective.
 * \param evt
 * \param pMission
 */
void ObjProtect::doEvaluate([[maybe_unused]] Mission *pMission) {
    PedInstance *p = static_cast<PedInstance *>(p_target_);
    if (p->isDead()) {
        // Target is dead -> objective is failed
        endObjective(false);
    } else if(p->currentAction() == NULL) {
        // Ped has finished all his actions
        endObjective(true);
    }
}

/*!
 * Constructeur.
 * \param
 */
ObjDestroyVehicle::ObjDestroyVehicle(MapObject * pVehicle) : TargetObjective(pVehicle) {
    msg = g_Ctx.getMessage("GOAL_DESTROY_VEHICLE");
}

/*!
 * Evaluate the objective.
 * \param evt
 * \param pMission
 */
void ObjDestroyVehicle::doEvaluate([[maybe_unused]] Mission *pMission) {
    Vehicle *pVehicle = static_cast<Vehicle *>(p_target_);

    if (pVehicle->isDead()) {
        endObjective(true);
    }
}

/*!
 * Constructor.
 * \param
 */
ObjUseVehicle::ObjUseVehicle(MapObject * pVehicle) : TargetObjective(pVehicle) {
    msg = g_Ctx.getMessage("GOAL_USE_VEHICLE");
}

/*!
 * Evaluate the objective.
 * \param evt
 * \param pMission
 */
void ObjUseVehicle::doEvaluate([[maybe_unused]] Mission *pMission) {
    GenericCar *pVehicle = static_cast<GenericCar *>(p_target_);

    if (pVehicle->isDead()) {
        endObjective(false);
        return;
    }

    PedInstance *p = pVehicle->getDriver();
    if (p && p->isOurAgent()) {
        endObjective(true);
    }
}

/*!
 * Constructeur.
 * \param
 */
ObjTakeWeapon::ObjTakeWeapon(MapObject * pWeapon) : TargetObjective(pWeapon) {
    msg = g_Ctx.getMessage("GOAL_TAKE_WEAPON");
}

/*!
 * Evaluate the objective.
 * \param evt
 * \param pMission
 */
void ObjTakeWeapon::doEvaluate([[maybe_unused]] Mission *pMission) {
    WeaponInstance *pWeapon = static_cast<WeaponInstance *>(p_target_);

    if (pWeapon->isDead()) {
        endObjective(false);
    } else {
        PedInstance *owner = pWeapon->owner();
        if (owner && owner->isOurAgent())
        {
            endObjective(true);
        }
    }
}

ObjEliminate::ObjEliminate(PedInstance::objGroupDefMasks subtype) :
        ObjectiveDesc() {
    if (subtype == PedInstance::og_dmAgent) {
        msg = g_Ctx.getMessage("GOAL_ELIMINATE_AGENTS");
        groupDefMask_ = subtype;
        indx_grpid.grpid = 2;
    } else if (subtype == PedInstance::og_dmPolice) {
        msg = g_Ctx.getMessage("GOAL_ELIMINATE_POLICE");
        groupDefMask_ = subtype;
        indx_grpid.grpid = 4;
    } else {
        groupDefMask_ = PedInstance::og_dmUndefined;
        indx_grpid.grpid = 0;
    }
}

/*!
 * Evaluate the objective.
 * \param pMission
 */
void ObjEliminate::doEvaluate(Mission *pMission) {
    for (size_t i = pMission->getSquad()->size(); i< pMission->numPeds(); i++) {
        PedInstance *pPed = pMission->ped(i);

        if(pPed->objGroupDef() == groupDefMask_
            // we can persuade them, will be
            // counted as eliminating for now
            && pPed->objGroupID() == indx_grpid.grpid
            && pPed->isAlive())
        {
            return;
        }
    }
    status = kCompleted;
}

ObjEvacuate::ObjEvacuate(int x, int y, int z, std::vector <PedInstance *> &lstOfPeds) :
        LocationObjective(x, y, z) {
    msg = g_Ctx.getMessage("GOAL_EVACUATE");
    // Copy all peds in the local list
    for (std::vector<PedInstance *>::iterator it_p = lstOfPeds.begin();
            it_p != lstOfPeds.end(); it_p++)
    {
        pedsToEvacuate.push_back(*it_p);
    }
}

void ObjEvacuate::handleStart() {
    EventManager::fire<EvacuateObjectiveStartedEvent>(objectiveLocw_);
}


/*!
 * Evaluate the objective.
 * \param pMission
 */
void ObjEvacuate::doEvaluate(Mission *pMission) {
    // evacuating people
    for (const auto &ped : pedsToEvacuate) {
        if (ped->isDead()) {
            // One of the peds is dead, objective is failed
            endObjective(false);
            return;
        }

        if (!ped->isCloseTo(objectiveLocw_, 512)) {
            // one of the peds is not yet in the evacuation perimeter
            return;
        }
    }

    Squad *pSquad = pMission->getSquad();
    for (auto it = pSquad->alive_begin(); it != pSquad->alive_end(); ++it) {
        PedInstance *pAgent = *it;
        if (!pAgent->isCloseTo(objectiveLocw_, 512)) {
            // one of the peds is not yet in the evacuation perimeter
            return;
        }
    }

    // If we're here, then all peds are in the circle -> objective is completed
    endObjective(true);
}

}
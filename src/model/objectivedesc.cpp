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

#include "model/objectivedesc.h"
#include "appcontext.h"
#include "ped.h"
#include "model/vehicle.h"
#include "model/squad.h"
#include "mission.h"
#include "core/gamecontroller.h"

/*!
 * A common method to end targeted objective.
 * \param succeeded True means objective is completed with success.
 */
void ObjectiveDesc::endObjective(bool succeeded) {
    status = succeeded ? kCompleted : kFailed;

    GameEvent evt;
    evt.stream = GameEvent::kMission;
    evt.type = succeeded ? GameEvent::kObjCompleted : GameEvent::kObjFailed;
    evt.pCtxt = NULL;
    g_gameCtrl.fireGameEvent(evt);
}

/*!
 * All targeted objectives sets the current target to the objective target.
 */
void TargetObjective::handleStart(Mission *p_mission) {
    GameEvent evt;
    evt.stream = GameEvent::kMission;
    evt.type = GameEvent::kObjTargetSet;
    evt.pCtxt = p_target_;
    g_gameCtrl.fireGameEvent(evt);
}

ObjPersuade::ObjPersuade(MapObject * pMapObject) : TargetObjective(pMapObject) {
    msg = g_Ctx.getMessage("GOAL_PERSUADE");
}

/*!
 * Evaluate the objective.
 * \param evt
 * \param pMission
 */
void ObjPersuade::evaluate(Mission *pMission) {
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
void ObjAssassinate::evaluate(Mission *pMission) {
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
void ObjProtect::evaluate(Mission *pMission) {
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
void ObjDestroyVehicle::evaluate(Mission *pMission) {
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
void ObjUseVehicle::evaluate(Mission *pMission) {
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
void ObjTakeWeapon::evaluate(Mission *pMission) {
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
void ObjEliminate::evaluate(Mission *pMission) {
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

void ObjEvacuate::handleStart(Mission *p_mission) {
    GameEvent evt;
    evt.stream = GameEvent::kMission;
    evt.type = GameEvent::kObjEvacuate;
    evt.pCtxt = &objectiveLocw_;
    g_gameCtrl.fireGameEvent(evt);
}


/*!
 * Evaluate the objective.
 * \param pMission
 */
void ObjEvacuate::evaluate(Mission *pMission) {
    // evacuating people
    for (std::vector<PedInstance *>::iterator it_p
        = pedsToEvacuate.begin();
        it_p != pedsToEvacuate.end(); it_p++)
    {
        if ((*it_p)->health() < 0) {
            // One of the peds is dead, objective is failed
            endObjective(false);
        }

        if (!(*it_p)->isCloseTo(objectiveLocw_, 512)) {
            // one of the peds is not yet in the evacuation perimeter
            return;
        }
    }

    for (size_t indx = AgentManager::kSlot1; indx < AgentManager::kMaxSlot; indx++) {
        PedInstance *pAgent = pMission->getSquad()->member(indx);
        if (pAgent && pAgent->isAlive()) {
            if (!pAgent->isCloseTo(objectiveLocw_, 512)) {
                // one of the peds is not yet in the evacuation perimeter
                return;
            }
        }
    }

    // If we're here, then all peds are in the circle -> objective is completed
    endObjective(true);
}

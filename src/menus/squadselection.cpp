/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2012  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "menus/squadselection.h"
#include "ped.h"
#include "model/vehicle.h"
#include "core/gameevent.h"
#include "mission.h"


/*!
 * Default constructor.
 */
SquadSelection::SquadSelection() {
    pSquad_ = NULL;
    selected_agents_ = 0;
    leader_ = 0;
}

/*!
 * Sets the squad and selects the first selectable agent.
 * Pre existing selection has been cleared.
 * \param pSquad The current squad.
 */
void SquadSelection::setSquad(Squad *pSquad) {
    clear();
    pSquad_ = pSquad;

    for (size_t i = AgentManager::kSlot1; i < AgentManager::kMaxSlot; i++) {
        // try to select agent. if true means agent has been selected
        if (selectAgent(i, false)) {
            break;
        }
    }
}

/*!
 * Clear current selection.
 */
void SquadSelection::clear() {
    selected_agents_ = 0;
}

/*!
 * Returns the number of selected agents
 * \return 0 if no agent is selected.
 */
size_t SquadSelection::size() {
    int agents = 0;

    for (size_t i = AgentManager::kSlot1; i < AgentManager::kMaxSlot; i++) {
        if (isAgentSelected(i)) {
            agents++;
        }
    }

    return agents;
}

/*!
 * Selects or deselects the agent on the given slot.
 * \param agentNo
 * \param addToGroup True if agent is added to the selection.
 * \return True if agent has been selected or deselected.
 */
bool SquadSelection::selectAgent(size_t agentNo, bool addToGroup) {
    if (isAgentSelectable(agentNo)) {
        // saves the current selection to check if it changes
        size_t oldSelection = selected_agents_;
        if (addToGroup) {
            // Add/remove to the group
            selected_agents_ ^= 1 << agentNo;
            // check if leader was deselected
            checkLeader(agentNo);
        } else {
            selected_agents_ = 1 << agentNo;
            leader_ = agentNo;
        }

        return (oldSelection != selected_agents_);
    }

    return false;
}

/*!
 * Deselects an agent. Called when an agent died.
 * \param p_ped The agent to deselect.
 */
void SquadSelection::deselectAgent(PedInstance *p_ped) {
    for (size_t i = AgentManager::kSlot1; i < AgentManager::kMaxSlot; i++) {
        if (pSquad_->member(i) == p_ped) {
            selected_agents_ &= ~(1 << i);
            // check if leader was deselected
            checkLeader(i);
            return;
        }
    }
}

/*!
 * Selects all agents.
 * \param b_selectAll True to select, false deselect
 */
void SquadSelection::selectAllAgents(bool b_selectAll) {
    if (b_selectAll) {
        // Select all agents
        for (size_t i = AgentManager::kSlot1; i < AgentManager::kMaxSlot; i++) {
            if (isAgentSelectable(i)) {
                selected_agents_ |= (1 << i);
            }
        }
    } else {
        // Deselect all except leader
        selected_agents_ = (1 << leader_);
    }
}

/*!
 * Returns true if agent is active and alive.
 */
bool SquadSelection::isAgentSelectable(size_t agentNo) {
    PedInstance *pPed = pSquad_->member(agentNo);
    return  pPed && pPed->isAlive();
}

/*!
 * checks if leader has been deselected then finds
 * a new leader.
 */
void SquadSelection::checkLeader(size_t agentNo) {
    if (agentNo == leader_) {
        // The leader has been deselected
        // find another leader among the remaining selection
        for (size_t i = AgentManager::kSlot1; i < AgentManager::kMaxSlot; i++) {
            if (isAgentSelectable(i) && leader_ != i) {
                leader_ = i;
                break;
            }
        }
    }
}

/*!
 * Deselects the leader selected weapon and for the other agents, it depends
 * on the leader's weapon.
 */
void SquadSelection::deselectWeaponOfSameCategory(Weapon *pWeaponFromLeader) {
    bool categoryShooting = pWeaponFromLeader->canShoot();
    for (SquadSelection::Iterator it = begin(); it != end(); ++it) {
        PedInstance *pAgent = *it;
        WeaponInstance *pWeaponToDeselect = pAgent->selectedWeapon();
        if (pWeaponToDeselect != NULL) {
            if (pAgent == leader() ||
                (categoryShooting && pWeaponToDeselect->getClass()->canShoot()) ||
                (pWeaponToDeselect->isInstanceOf(pWeaponFromLeader->getType()))) {
                pAgent->stopUsingWeapon();
                pAgent->deselectWeapon();
            }
        }
    }
}

/*!
 * Selects the given weapon for the leader and updates weapon selection
 * for other selected agents.
 * \param weapon_idx The index in the leader inventory of the weapon to select.
 * \param apply_to_all In case of Medikit, all selected agents must use one.
 */
void SquadSelection::selectWeaponFromLeader(int weapon_idx, bool applySelectionToAll) {
    PedInstance *pLeader = leader();
    WeaponInstance *pLeaderWeapon = pLeader->weapon(weapon_idx);

    for (SquadSelection::Iterator it = begin(); it != end(); ++it)
    {
        PedInstance *ped = *it;
        ped->stopUsingWeapon();
        if (pLeader == ped) {
            // Forces selection of the weapon for the leader
            pLeader->selectWeapon(weapon_idx);
        } else if (pLeaderWeapon->canShoot()) {
            ped->selectShootingWeaponWithSameTypeFirst(pLeaderWeapon);
        } else if (applySelectionToAll) {
            ped->selectMedikitOrShield(pLeaderWeapon->getClass()->getType());
        }
    } // end for
}

/*!
 * Choose the first agent who has free space in his inventory and send
 * him pickup weapon.
 * \param pWeapon The weapon to pickup
 * \param addAction True to add the action at the end of the list of action,
 * false to set as the only action.
 */
void SquadSelection::pickupWeapon(WeaponInstance *pWeapon, bool addAction) {
    for (SquadSelection::Iterator it = begin(); it != end(); ++it)
    {
        PedInstance *pAgent = *it;
        // Agent has space in inventory
        if (pAgent->numWeapons() < WeaponHolder::kMaxHoldedWeapons) {
            MovementAction * pActions = pAgent->createActionPickup(pWeapon);
            pAgent->addMovementAction(pActions, addAction);

            break;
        }
    }
}

/*!
 * All selected agents that are not in a vehicle, follows the given pedestrian.
 * \param pPed The ped to follow
 * \param addAction True to add the action at the end of the list of action,
 * false to set as the only action.
 */
void SquadSelection::followPed(PedInstance *pPed, bool addAction) {
    for (SquadSelection::Iterator it = begin(); it != end(); ++it)
    {
        PedInstance *pAgent = *it;
        if (!pAgent->inVehicle()) { // Agent must not be in a vehicle
            pAgent->addActionFollowPed(pPed);
        }
    }
}

/*!
 * Selected agents enter or leave the given vehicle.
 * - if he is in the vehicle, the vehicle stops and he gets out. It only works
 *    with cars : Trains automatically drop their passengers.
 * - else he gets in the given vehicle.
 * \param pVehicle The vehicle
 * \param addAction True to add the action at the end of the list of action,
 * false to set as the only action.
 */
void SquadSelection::enterOrLeaveVehicle(Vehicle *pVehicle, bool addAction) {
    for (SquadSelection::Iterator it = begin(); it != end(); ++it)
    {
        PedInstance *pAgent = *it;

        if (pVehicle->isCar() && pAgent->inVehicle() == pVehicle) {
            // First stop the car
            if (pVehicle->speed() != 0) {
                pVehicle->clearDestination();
                GenericCar *pCar = dynamic_cast<GenericCar *>(pVehicle);
                pCar->getDriver()->destroyAllActions();
            }

            // then drop passenger
            pVehicle->dropPassenger(pAgent);
            pAgent->dropPersuadedFromCar(pVehicle);
        } else {
            // Agent is out
            MovementAction *pAction =
                pAgent->createActionEnterVehicle(pVehicle);
            pAgent->addMovementAction(pAction, addAction);
            pAgent->informPersuadedToEnterVehicle(pVehicle);
        }
    }
}

/*!
 * Tells every agents in the selection to go to the given point.
 * Agents in a drivable vehicle will use vehicle and other will walk.
 * \param mapPt The destination point
 * \param addAction True to add the action at the end of the list of action,
 * false to set as the only action.
 */
void SquadSelection::moveTo(TilePoint &mapPt, bool addAction) {
    int i=0;
    for (SquadSelection::Iterator it = begin(); it != end(); ++it, i++)
    {
        PedInstance *pAgent = *it;
        Vehicle *pVehicle = pAgent->inVehicle();
        if (pVehicle) {
            if (pVehicle->isCar()) {
                // Agent is in drivable vehicle
                GenericCar *pCar = dynamic_cast<GenericCar *>(pVehicle);
                if (pCar->isDriver(pAgent))
                {
                    int stx = mapPt.tx;
                    int sty = mapPt.ty;
                    //int sox = ox;
                    //int soy = oy;
                    stx = mapPt.tx * 256 + mapPt.ox + 128 * (pVehicle->tileZ() - 1);
                    //sox = stx % 256;
                    stx = stx / 256;
                    sty = mapPt.ty * 256 + mapPt.oy + 128 * (pVehicle->tileZ() - 1);
                    //soy = sty % 256;
                    sty = sty / 256;
                    TilePoint posT = TilePoint(stx, sty, 0, 128, 128);
                    pAgent->addActionDriveVehicle(pCar, posT, addAction);
                }
            }
        } else {
            TilePoint tmpPosT = mapPt;

            if (size() > 1) {
                pSquad_->getPositionInSquadFormation(i, &tmpPosT);
            }

            pAgent->addActionWalk(tmpPosT, addAction);
        }
    } // end of for
}

/*!
 * Each selected agent will use his weapon and shoot at the given target.
 * Agent will shoot only if he's armed and ready to shoot.
 * \param aimedLocW Where the agent must shoot
 */
void SquadSelection::shootAt(const WorldPoint &aimedLocW) {
    for (SquadSelection::Iterator it = begin(); it != end(); ++it) {
        PedInstance *pAgent = *it;
        pAgent->addActionShootAt(aimedLocW);
    }
}


/**
 * Return true if the given object is in the line of fire of at least
 * one selected agent.
 * \param pMission Mission*
 * \param pTarget ShootableMapObject*
 * \return bool
 *
 */
bool SquadSelection::isTargetInRange(Mission *pMission, ShootableMapObject *pTarget) {
    for (SquadSelection::Iterator it = begin(); it != end(); ++it) {
        if ((*it)->isArmed()) {
            WorldPoint shooterPosW((*it)->position());
            WeaponInstance *pWeapon = (*it)->selectedWeapon();

            uint8 blockRes = pMission->checkIfBlockersInShootingLine(
                shooterPosW, &pTarget, NULL, false, false, pWeapon->range(), NULL, (*it));

            if (blockRes == 1) {
                return true;
            }
        }
    }

    return false;
}

/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2012, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "menus/squadselection.h"

#include "fs-kernel/model/ped.h"
#include "fs-kernel/model/vehicle.h"
#include "fs-kernel/model/mission.h"


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
void SquadSelection::setSquad(fs_knl::Squad *pSquad) {
    clear();
    pSquad_ = pSquad;

    for (size_t i = fs_knl::Squad::kSlot1; i < fs_knl::Squad::kMaxSlot; i++) {
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
    size_t agents = 0;

    for (size_t i = fs_knl::Squad::kSlot1; i < fs_knl::Squad::kMaxSlot; i++) {
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
void SquadSelection::deselectAgent(fs_knl::PedInstance *p_ped) {
    for (size_t i = fs_knl::Squad::kSlot1; i < fs_knl::Squad::kMaxSlot; i++) {
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
        for (size_t i = fs_knl::Squad::kSlot1; i < fs_knl::Squad::kMaxSlot; i++) {
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
    fs_knl::PedInstance *pPed = pSquad_->member(agentNo);
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
        for (size_t i = fs_knl::Squad::kSlot1; i < fs_knl::Squad::kMaxSlot; i++) {
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
void SquadSelection::deselectWeaponOfSameCategory(fs_knl::Weapon *pWeaponFromLeader) {
    bool categoryShooting = pWeaponFromLeader->canShoot();
    for (SquadSelection::Iterator it = begin(); it != end(); ++it) {
        fs_knl::PedInstance *pAgent = *it;
        fs_knl::WeaponInstance *pWeaponToDeselect = pAgent->selectedWeapon();
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
void SquadSelection::selectWeaponFromLeader(uint8_t weapon_idx, bool applySelectionToAll) {
    fs_knl::PedInstance *pLeader = leader();
    fs_knl::WeaponInstance *pLeaderWeapon = pLeader->weapon(weapon_idx);

    for (SquadSelection::Iterator it = begin(); it != end(); ++it)
    {
        fs_knl::PedInstance *ped = *it;
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
void SquadSelection::pickupWeapon(fs_knl::WeaponInstance *pWeapon, bool addAction) {
    for (SquadSelection::Iterator it = begin(); it != end(); ++it)
    {
        fs_knl::PedInstance *pAgent = *it;
        // Agent has space in inventory
        if (pAgent->numWeapons() < fs_knl::WeaponHolder::kMaxHoldedWeapons) {
            fs_knl::MovementAction * pActions = pAgent->createActionPickup(pWeapon);
            pAgent->addMovementAction(pActions, addAction);

            break;
        }
    }
}

/*!
 * All selected agents that are not in a vehicle, follows the given pedestrian.
 * This action replaces all actions in the agents list.
 * \param pPed The ped to follow
 */
void SquadSelection::followPed(fs_knl::PedInstance *pPed) {
    for (SquadSelection::Iterator it = begin(); it != end(); ++it)
    {
        fs_knl::PedInstance *pAgent = *it;
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
void SquadSelection::enterOrLeaveVehicle(fs_knl::Vehicle *pVehicle, bool addAction) {
    for (SquadSelection::Iterator it = begin(); it != end(); ++it)
    {
        fs_knl::PedInstance *pAgent = *it;

        if (pVehicle->isCar() && pAgent->inVehicle() == pVehicle) {
            // First stop the car
            if (pVehicle->speed() != 0) {
                pVehicle->clearDestination();
                fs_knl::GenericCar *pCar = dynamic_cast<fs_knl::GenericCar *>(pVehicle);
                pCar->getDriver()->destroyAllActions();
            }

            // then drop passenger
            pVehicle->dropPassenger(pAgent);
            pAgent->dropPersuadedFromCar(pVehicle);
        } else {
            // Agent is out
            fs_knl::MovementAction *pAction =
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
void SquadSelection::moveTo(fs_knl::TilePoint &mapPt, bool addAction) {
    size_t i=0;
    for (SquadSelection::Iterator it = begin(); it != end(); ++it, i++)
    {
        fs_knl::PedInstance *pAgent = *it;
        fs_knl::Vehicle *pVehicle = pAgent->inVehicle();
        if (pVehicle) {
            if (pVehicle->isCar()) {
                // Agent is in drivable vehicle
                fs_knl::GenericCar *pCar = dynamic_cast<fs_knl::GenericCar *>(pVehicle);
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
                    fs_knl::TilePoint posT = fs_knl::TilePoint(stx, sty, 0, 128, 128);
                    pAgent->addActionDriveVehicle(pCar, posT, addAction);
                }
            }
        } else {
            fs_knl::TilePoint tmpPosT = mapPt;

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
void SquadSelection::shootAt(const fs_knl::WorldPoint &aimedLocW) {
    for (SquadSelection::Iterator it = begin(); it != end(); ++it) {
        fs_knl::PedInstance *pAgent = *it;
        pAgent->addActionShootAt(aimedLocW);
    }
}

/*!
 * Aggressive move: agents walk to destination while shooting at target.
 * Agents start shooting and then walk — shooting continues during movement.
 * \param mapPt Destination to walk to
 * \param aimedLocW Where to shoot
 */
void SquadSelection::moveAndShoot(fs_knl::TilePoint &mapPt,
                                   const fs_knl::WorldPoint &aimedLocW) {
    size_t i = 0;
    for (SquadSelection::Iterator it = begin(); it != end(); ++it, i++) {
        fs_knl::PedInstance *pAgent = *it;

        // Start shooting first (weapon action runs in parallel with movement)
        pAgent->addActionShootAt(aimedLocW);

        // Then move to destination (movement doesn't cancel shooting)
        fs_knl::Vehicle *pVehicle = pAgent->inVehicle();
        if (pVehicle) {
            if (pVehicle->isCar()) {
                fs_knl::GenericCar *pCar = dynamic_cast<fs_knl::GenericCar *>(pVehicle);
                if (pCar->isDriver(pAgent)) {
                    int stx = mapPt.tx * 256 + mapPt.ox + 128 * (pVehicle->tileZ() - 1);
                    stx = stx / 256;
                    int sty = mapPt.ty * 256 + mapPt.oy + 128 * (pVehicle->tileZ() - 1);
                    sty = sty / 256;
                    fs_knl::TilePoint posT = fs_knl::TilePoint(stx, sty, 0, 128, 128);
                    pAgent->addActionDriveVehicle(pCar, posT, false);
                }
            }
        } else {
            fs_knl::TilePoint tmpPosT = mapPt;
            if (size() > 1) {
                pSquad_->getPositionInSquadFormation(i, &tmpPosT);
            }
            pAgent->addActionWalk(tmpPosT, false);
        }
    }
}


/**
 * Return true if the given object is in the line of fire of at least
 * one selected agent.
 * \param pMission Mission*
 * \param pTarget ShootableMapObject*
 * \return bool
 */
bool SquadSelection::isTargetInRange(fs_knl::Mission *pMission, fs_knl::ShootableMapObject *pTarget) {
    for (SquadSelection::Iterator it = begin(); it != end(); ++it) {
        if ((*it)->isArmed()) {
            fs_knl::WorldPoint shooterPosW((*it)->position());
            fs_knl::WeaponInstance *pWeapon = (*it)->selectedWeapon();

            uint8 blockRes = pMission->checkIfBlockersInShootingLine(
                shooterPosW, &pTarget, NULL, false, false, pWeapon->range(), NULL, (*it));

            if (blockRes == 1) {
                return true;
            }
        }
    }

    return false;
}

/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net>
 *   Copyright (C) 2014, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-kernel/ia/behaviour.h"

#include "fs-kernel/model/ped.h"
#include "fs-kernel/model/squad.h"
#include "fs-kernel/mgr/missionmanager.h"
#include "fs-kernel/mgr/weaponmanager.h"

namespace fs_knl {
//*************************************
// Constant definition
//*************************************
const int CommonAgentBehaviourComponent::kRegeratesHealthStep = 1;
const int PanicComponent::kScoutDistance = 1500;
const int PanicComponent::kDistanceToRun = 500;
const double PersuadedBehaviourComponent::kMaxRangeForSearchingWeapon = 500.0;
const int PoliceBehaviourComponent::kPoliceScoutDistance = 1500;
const int PoliceBehaviourComponent::kPolicePendingTime = 1500;
const int PlayerHostileBehaviourComponent::kEnemyScoutDistance = 1500;

Behaviour::~Behaviour() {
    destroyComponents();
}

void Behaviour::destroyComponents() {
    while(compLst_.size() != 0) {
        BehaviourComponent *pComp = compLst_.front();
        compLst_.pop_front();
        delete pComp;
    }
}

void Behaviour::handleBehaviourEvent(BehaviourEventType evtType, void *pCtxt) {
    for (std::list < BehaviourComponent * >::iterator it = compLst_.begin();
            it != compLst_.end(); it++) {
        BehaviourEvent event {pThisPed_, evtType, pCtxt};
        (*it)->handleBehaviourEvent(event);
    }
}

void Behaviour::addComponent(BehaviourComponent *pComp) {
    compLst_.push_back(pComp);
}

/*!
 * Destroy existing components and set given one as new one
 */
void Behaviour::replaceAllcomponentsBy(BehaviourComponent *pComp) {
    destroyComponents();
    addComponent(pComp);
}

/*!
 * Run the execute method  of each component listed in the behaviour.
 * Component must be enabled.
 * \param elapsed Time elapsed since last frame
 * \param pMission Mission data
 */
void Behaviour::execute(uint32_t elapsed, Mission *pMission) {
    if (pThisPed_->isDead()) {
        return;
    }

    for (std::list < BehaviourComponent * >::iterator it = compLst_.begin();
            it != compLst_.end(); it++) {
        BehaviourComponent *pComp = *it;
        if (pComp->isEnabled()) {
            Behaviour::BehaviourParam param {elapsed, pMission, pThisPed_};
            pComp->execute(param);
        }
    }
}

CommonAgentBehaviourComponent::CommonAgentBehaviourComponent(PedInstance *pPed):
        BehaviourComponent(), healthTimer_(pPed->getHealthRegenerationPeriod()) {
    doRegenerates_ = false;
}

/*!
 *
 * \param elapsed Time elapsed since last frame
 * \param pMission Mission data
 * \param pPed The owner of the behaviour
 */
void CommonAgentBehaviourComponent::execute(const Behaviour::BehaviourParam &param) {
    // If Agent is equiped with right chest, his health periodically updates
    if (doRegenerates_ && healthTimer_.update(param.elapsed)) {
        if (param.pPed->increaseHealth(kRegeratesHealthStep)) {
            doRegenerates_ = false;
        }
    }
}

void CommonAgentBehaviourComponent::handleBehaviourEvent(const Behaviour::BehaviourEvent &event) {
    switch(event.evtType) {
    case Behaviour::kBehvEvtHit:
        if (event.pPed->hasMinimumVersionOfMod(Mod::MOD_CHEST, Mod::MOD_V2)) {
            doRegenerates_ = true;
        }
        break;
    default:
        break;
    }
}

PersuaderBehaviourComponent::PersuaderBehaviourComponent():
        BehaviourComponent() {
    doUsePersuadotron_ = false;
    persuadotronRange_ = g_weaponMgr.getWeapon(Weapon::Persuadatron)->range();
}

void PersuaderBehaviourComponent::execute(const Behaviour::BehaviourParam &param) {
    // Check if Agent has selected his Persuadotron
    if (doUsePersuadotron_) {
        // iterate through all peds except our agents
        for (size_t i = param.pMission->getSquad()->size(); i < param.pMission->numPeds(); i++) {
            PedInstance *pOtherPed = param.pMission->ped(i);
            if (param.pPed->canPersuade(pOtherPed, persuadotronRange_)) {
                DamageToInflict dmg;
                dmg.dtype = kDmgTypePersuasion;
                dmg.d_owner = param.pPed;
                pOtherPed->insertHitAction(dmg);
            }
        }
    }
}

void PersuaderBehaviourComponent::handleBehaviourEvent(const Behaviour::BehaviourEvent &event) {
    switch(event.evtType) {
    case Behaviour::kBehvEvtPersuadotronActivated:
        doUsePersuadotron_ = true;
        break;
    case Behaviour::kBehvEvtPersuadotronDeactivated:
        doUsePersuadotron_ = false;
        break;
    default:
        break;
    }
}

PersuadedBehaviourComponent::PersuadedBehaviourComponent():
        BehaviourComponent(), checkWeaponTimer_(1000) {
    status_ = kPersuadStatusWaitForHitAction;
}

void PersuadedBehaviourComponent::execute(const Behaviour::BehaviourParam &param) {
    if (status_ == kPersuadStatusInitializing) {
        param.pPed->destroyAllActions(true);
        // set follow owner as new default action
        FollowAction *pAction = new FollowAction(param.pPed->owner());
        param.pPed->addToDefaultActions(pAction);
        param.pPed->addToDefaultActions(new ResetScriptedAction(Action::kActionDefault));
        param.pPed->addMovementAction(pAction, false);
        status_ = kPersuadStatusFollow;
    } else if (status_ == kPersuadStatusLookForWeapon) {
        if (checkWeaponTimer_.update(param.elapsed)) {
            WeaponInstance *pWeapon = findWeaponWithAmmo(param.pMission, param.pPed);
            if (pWeapon) {
                // a weapon is found
                // initiate alternative actions : go to weapon and take it
                status_ = kPersuadStatusTakeWeapon;
                if (param.pPed->altAction() == NULL) {
                    MovementAction * pActions = param.pPed->createActionPickup(pWeapon);
                    // set a warning after picking up weapon so we know we can select it
                    pActions->next()->setWarnBehaviour(true);
                    // add a reset action to automatically go back to follow owner after picking up weapon
                    pActions->next()->link(
                        new ResetScriptedAction(Action::kActionDefault));
                    param.pPed->addToAltActions(pActions);
                } else {
                    // just update weapon
                    changeTargetWeaponInAltActions(pWeapon, param.pPed);
                }
                // execute alternative actions
                param.pPed->setCurrentActionWithSource(Action::kActionAlt);
            }
        }
    }
}

void PersuadedBehaviourComponent::handleBehaviourEvent(const Behaviour::BehaviourEvent &event) {
    if (event.evtType == Behaviour::kBehvEvtWeaponOut) {
        PedInstance *pPedSource = static_cast<PedInstance *> (event.pCtxt);
        if (pPedSource == event.pPed->owner()) {
            // the ped who is armed is our owner so select weapon or look for one
            if (event.pPed->numWeapons() > 0) {
                event.pPed->selectWeapon((size_t) 0);
            } else {
                // ped has no weapon -> start looking for some
                status_ = kPersuadStatusLookForWeapon;
            }
        }
    } else if (event.evtType == Behaviour::kBehvEvtWeaponCleared) {
        PedInstance *pPedSource = static_cast<PedInstance *> (event.pCtxt);
        if (pPedSource == event.pPed->owner()) {
            // the ped who cleared his weapon is our owner so deselect weapon or
            // stop searching for one
            if (event.pPed->deselectWeapon() == NULL && status_ == kPersuadStatusLookForWeapon) {
                status_ = kPersuadStatusFollow;
            }
        }
    } else if (event.evtType == Behaviour::kBehvEvtActionEnded) {
        if (status_ == kPersuadStatusWaitForHitAction) {
            status_ = kPersuadStatusInitializing;
        } else {
            Action::ActionType *pType = static_cast<Action::ActionType *> (event.pCtxt);
            if (*pType == Action::kActTypePickUp) {
                if (event.pPed->owner()->isArmed()) {
                    event.pPed->selectWeapon((size_t) 0);
                }
                // weapon found so back to normal
                status_ = kPersuadStatusFollow;
            } else if (*pType == Action::kActTypeDrop) {
                // weapon dropped so look for another
                status_ = kPersuadStatusLookForWeapon;
            }
        }
    } else if (event.evtType == Behaviour::kBehvEvtEnterVehicle) {
        Vehicle *pVehicle = static_cast<Vehicle *> (event.pCtxt);
        MovementAction *pAction =
                event.pPed->createActionEnterVehicle(pVehicle);
        event.pPed->addMovementAction(pAction, false);
        status_ = kPersuadStatusFollow;
    }
}

/*!
 * Look for weapon on the ground with ammo.
 * The closest weapon within the given range will be return.
 * \param pMission Mission data
 * \param pPed The ped searching for the weapon
 * \return NULL if no weapon is found.
 */
WeaponInstance * PersuadedBehaviourComponent::findWeaponWithAmmo(Mission *pMission, PedInstance *pPed) {
   WeaponInstance *pWeaponFound = nullptr;
   double currentDistance = kMaxRangeForSearchingWeapon;

    size_t numweapons = pMission->numWeaponsOnGround();
    for (size_t i = 0; i < numweapons; ++i) {
        WeaponInstance *w = pMission->weaponOnGround(i);
        if (!w->hasOwner() && w->canShoot() && w->ammoRemaining() > 0) {
            double length = 0;
            if (pMission->getPathLengthBetween(pPed, w, kMaxRangeForSearchingWeapon, &length) == 0) {
                if (currentDistance > length) {
                    pWeaponFound = w;
                    currentDistance = length;
                }
            }
        }
    }
    return pWeaponFound;
}

void PersuadedBehaviourComponent::changeTargetWeaponInAltActions(WeaponInstance *pWeapon, PedInstance *pPed) {
    MovementAction *pAction = pPed->altAction();
    while (pAction != NULL) {
        if (pAction->type() == Action::kActTypeWalk) {
            WalkAction *pWalk = static_cast<WalkAction *> (pAction);
            pWalk->setDestination(pWeapon);
        } else if (pAction->type() == Action::kActTypePickUp) {
            PickupWeaponAction *pWalk = static_cast<PickupWeaponAction *> (pAction);
            pWalk->setWeapon(pWeapon);
        }
        pAction = pAction->next();
    }
}

PanicComponent::PanicComponent():
        BehaviourComponent(), scoutTimer_(500) {
    backFromPanic_ = false;
    status_ = kPanicStatusCalm;
    // this component will be activated by event to
    // lower CPU consumption
    setEnabled(false);
}

void PanicComponent::execute(const Behaviour::BehaviourParam &param) {
    if (status_ == kPanicStatusCalm && scoutTimer_.update(param.elapsed)) {
        pArmedPed_ = findNearbyArmedPed(param.pMission, param.pPed);
        if (pArmedPed_) {
            runAway(param.pPed);
            status_ = kPanicStatusInPanic;
        } else if (backFromPanic_) {
            backFromPanic_ = false;
            param.pPed->setCurrentActionWithSource(Action::kActionDefault);
            status_ = kPanicStatusCalm;
        }
    }
}

void PanicComponent::handleBehaviourEvent(const Behaviour::BehaviourEvent &event) {
    if (event.pPed->isImmunedToPanic()) {
        return;
    }

    switch(event.evtType) {
    case Behaviour::kBehvEvtWeaponOut:
        // Someone has shown a gun, so activate panic behaviour for peds
        if (!isEnabled()) {
            setEnabled(true);
            status_ = kPanicStatusCalm;
        }
        break;
    case Behaviour::kBehvEvtWeaponCleared:
        if (g_missionCtrl.mission()->numArmedPeds() == 0) {
            setEnabled(false);
            if (!event.pPed->isCurrentActionFromSource(Action::kActionDefault)) {
                event.pPed->setCurrentActionWithSource(Action::kActionDefault);
                status_ = kPanicStatusCalm;
            }
        }
        break;
    case Behaviour::kBehvEvtActionEnded:
        if (!event.pPed->isCloseTo(pArmedPed_, kScoutDistance)) {
            // Ped is far from armed guy,
            pArmedPed_ = NULL;
            // so next time check if there another enemy around
            status_ = kPanicStatusCalm;
            scoutTimer_.setToMax();
            backFromPanic_ = true;
        }
        break;
    default:
        break;
    }
}

/*!
 * Return the first armed ped that is close to the given ped.
 * \param pMission
 * \param pPed
 * \return NULL if no ped is found
 */
PedInstance * PanicComponent::findNearbyArmedPed(Mission *pMission, PedInstance *pPed) {
    for (size_t i = 0; i < pMission->numArmedPeds(); i++) {
        PedInstance *pOtherPed = pMission->armedPedAtIndex(i);
        if (pPed->isCloseTo(pOtherPed, kScoutDistance)) {
            return pOtherPed;
        }
    }
    return NULL;
}

/*!
 * Makes the ped runs in the opposite way of the armed ped.
 * \param pPed The panicking ped
 */
void  PanicComponent::runAway(PedInstance *pPed) {
    // setting opposite direction for movement
    WorldPoint thisPedLocW(pPed->position());
    WorldPoint otherLocW(pArmedPed_->position());

    pPed->setDirection(otherLocW.x - thisPedLocW.x,
        otherLocW.y - thisPedLocW.y);
    if (pPed->altAction() == NULL) {
        // Adds the action of running away
        WalkToDirectionAction *pAction =
            new WalkToDirectionAction();
        // walk for a certain distance
        pAction->setMaxDistanceToWalk(kDistanceToRun);
        pAction->setWarnBehaviour(true);
        pPed->addToAltActions(pAction);
        pPed->addToAltActions(new ResetScriptedAction(Action::kActionAlt));
    }

    pPed->setCurrentActionWithSource(Action::kActionAlt);
}

PoliceBehaviourComponent::PoliceBehaviourComponent():
        BehaviourComponent(), scoutTimer_(200) {
    status_ = kPoliceStatusDefault;
    pTarget_ = NULL;
}

void PoliceBehaviourComponent::execute(const Behaviour::BehaviourParam &param) {
    if (status_ == kPoliceStatusAlert && scoutTimer_.update(param.elapsed)) {
        findAndEngageNewTarget(param.pMission, param.pPed);
    } else if (status_ == kPoliceStatusCheckReengageOrDefault) {
        // check if there is a nearby enemy
        bool foundNewTarget = findAndEngageNewTarget(param.pMission, param.pPed);
        if ( !foundNewTarget && !param.pPed->isCurrentActionFromSource(Action::kActionDefault)) {
            // there is no one around so go back to patrol if it's not already the case
            param.pPed->deselectWeapon();
            param.pPed->setCurrentActionWithSource(Action::kActionDefault);
            if (param.pMission->numArmedPeds() != 0) {
                // There are still some armed peds so keep on alert
                status_ = kPoliceStatusAlert;
            } else {
                status_ = kPoliceStatusDefault;
            }
        }
    }
}

void PoliceBehaviourComponent::handleBehaviourEvent(const Behaviour::BehaviourEvent &event) {
    switch(event.evtType) {
    case Behaviour::kBehvEvtEjectedFromVehicle:
        handleEjectionFromVehicle(event.pPed, event.pCtxt);
        break;
    case Behaviour::kBehvEvtWeaponOut:
        if (status_ == kPoliceStatusDefault && !event.pPed->inVehicle()) {
            // When someone get his weapon out, police is on alert
            status_ = kPoliceStatusAlert;
        }
        break;
    case Behaviour::kBehvEvtWeaponCleared:
        // our target has dropped his weapon
        if (status_ == kPoliceStatusFollowAndShootTarget && pTarget_ == event.pCtxt) {
            status_ = kPoliceStatusPendingEndFollow;
            event.pPed->stopShooting();

            // just wait a few time before engaging another target or simply
            // continue with default behavior
            WaitAction *pWait = new WaitAction(WaitAction::kWaitWeapon, kPolicePendingTime);
            pWait->setWarnBehaviour(true);
            event.pPed->addMovementAction(pWait, false);
        } else if (status_ == kPoliceStatusAlert) {
            status_ = kPoliceStatusCheckReengageOrDefault;
        }
        break;
    case Behaviour::kBehvEvtActionEnded:
        {
            Action::ActionType *pType = static_cast<Action::ActionType *> (event.pCtxt);
            if (*pType == Action::kActTypeWait) {
                // We are at the end of waiting period so check if we need to engage right now
                // of if we can go back on patrol
                status_ = kPoliceStatusCheckReengageOrDefault;
            } else {
                status_ = kPoliceStatusAlert;
            }
        }

        break;
    default:
        break;
    }
}

void PoliceBehaviourComponent::handleEjectionFromVehicle(PedInstance *pPed, void *pCtxt) {
    pPed->destroyAllActions(true);
    pPed->addToDefaultActions(new WalkToDirectionAction());
    PedInstance *pShooter = static_cast<PedInstance *>(pCtxt);
    pPed->setDirectionTowardObject(*pShooter);
    // Add a walk action just to get away from the vehicle so that the ped can shoot
    // without being blocked by the vehicle
    WalkToDirectionAction *outOfCarAction = new WalkToDirectionAction();
    outOfCarAction->setMaxDistanceToWalk(20);
    outOfCarAction->setWarnBehaviour(true);
    pPed->addMovementAction(outOfCarAction, true);
    status_ = kPoliceStatusOutOfVehicle;
}

bool PoliceBehaviourComponent::findAndEngageNewTarget(Mission *pMission, PedInstance *pPed) {
    PedInstance *pArmedGuy = findArmedPedNotPolice(pMission, pPed);
    if (pArmedGuy != NULL) {
        followAndShootTarget(pPed, pArmedGuy);
    }
    return pArmedGuy != NULL;
}

/*!
 * Return a ped that has his weapon out and is not a police man and is close to this policeman.
 */
PedInstance * PoliceBehaviourComponent::findArmedPedNotPolice(Mission *pMission, PedInstance *pPed) {
    for (size_t i = 0; i < pMission->numArmedPeds(); i++) {
        PedInstance *pOtherPed = pMission->armedPedAtIndex(i);
        if (pPed != pOtherPed && pOtherPed->type() != PedInstance::kPedTypePolice && pPed->isCloseTo(pOtherPed, kPoliceScoutDistance)) {
            return pOtherPed;
        }
    }
    return NULL;
}

void PoliceBehaviourComponent::followAndShootTarget(PedInstance *pPed, PedInstance *pArmedGuy) {
    pTarget_ = pArmedGuy;
    status_ = kPoliceStatusFollowAndShootTarget;

    // Set new actions
    if (pPed->altAction() == NULL) { // the first time
        WaitBeforeShootingAction *pWarnAction = new WaitBeforeShootingAction(pArmedGuy);
        pPed->addToAltActions(pWarnAction);
        FollowToShootAction* pFollowAction = new FollowToShootAction(pArmedGuy);
        pPed->addToAltActions(pFollowAction);
        pPed->addToAltActions(new FireWeaponAction(pArmedGuy));
        pPed->addToAltActions(new ResetScriptedAction(Action::kActionAlt));
    } else { // just update the target in the existing chain of actions
        MovementAction *pAction = pPed->altAction();
        while (pAction != NULL) {
            switch(pAction->type()) {
            case Action::kActTypeWaitShoot:
                {
                WaitBeforeShootingAction *pAct = dynamic_cast<WaitBeforeShootingAction *>(pAction);
                pAct->setTarget(pArmedGuy);
                }
                break;
            case Action::kActTypeFollowToShoot:
                {
                FollowToShootAction *pAct = dynamic_cast<FollowToShootAction *>(pAction);
                pAct->setTarget(pArmedGuy);
                }
                break;
            case Action::kActTypeFire:
                {
                FireWeaponAction *pAct = dynamic_cast<FireWeaponAction *>(pAction);
                pAct->setTarget(pArmedGuy);
                }
                break;
            default:
                break;
            }
            pAction = pAction->next();
        }
    }
    pPed->setCurrentActionWithSource(Action::kActionAlt);
}

PlayerHostileBehaviourComponent::PlayerHostileBehaviourComponent():
        BehaviourComponent() {
    status_ = kHostileStatusDefault;
}

void PlayerHostileBehaviourComponent::execute(const Behaviour::BehaviourParam &param) {
    if (status_ == kHostileStatusDefault) {
        // In this mode, ped is looking for an enemy
        PedInstance *pArmedGuy = findPlayerAgent(param.pMission, param.pPed);
        if (pArmedGuy != NULL) {
            status_ = kHostileStatusFollowAndShoot;
            followAndShootTarget(param.pPed, pArmedGuy);
        }
    } else if (status_ == kHostileStatusFollowAndShoot && pTarget_->isDead()) {
        status_ = kHostileStatusPendingEndFollow;
        pTarget_ = NULL;
        param.pPed->stopShooting();
        // just wait a few time before engaging another target or simply
        // continue with default behavior
        WaitAction *pWait = new WaitAction(WaitAction::kWaitWeapon);
        pWait->setWarnBehaviour(true);
        param.pPed->addMovementAction(pWait, false);
    } else if (status_ == kHostileStatusCheckForDefault) {
        // check if there is a nearby enemy
        PedInstance *pArmedGuy = findPlayerAgent(param.pMission, param.pPed);
        if (pArmedGuy != NULL) {
            status_ = kHostileStatusFollowAndShoot;
            followAndShootTarget(param.pPed, pArmedGuy);
        } else {
            param.pPed->deselectWeapon();
            param.pPed->setCurrentActionWithSource(Action::kActionDefault);
            status_ = kHostileStatusDefault;
        }
    }
}

void PlayerHostileBehaviourComponent::handleBehaviourEvent(const Behaviour::BehaviourEvent &event) {
    if (event.evtType == Behaviour::kBehvEvtActionEnded) {
        // We are at the end of waiting period so check if we need to engage right now
        // of if we can go back to default
        status_ = kHostileStatusCheckForDefault;
    }
}

PedInstance * PlayerHostileBehaviourComponent::findPlayerAgent(Mission *pMission, PedInstance *pPed) {
    for (size_t i = 0; i < pMission->getSquad()->size(); i++) {
        PedInstance *pAgent = pMission->getSquad()->member(i);
        if (pAgent && pAgent->isAlive() && pPed->isCloseTo(pAgent, kEnemyScoutDistance)) {
            return pAgent;
        }
    }
    return NULL;
}

void PlayerHostileBehaviourComponent::followAndShootTarget(PedInstance *pPed, PedInstance *pArmedGuy) {
    pTarget_ = pArmedGuy;

    // Set new actions
    if (pPed->altAction() == NULL) { // the first time
        WaitBeforeShootingAction *pWarnAction = new WaitBeforeShootingAction(pArmedGuy);
        pPed->addToAltActions(pWarnAction);
        FollowToShootAction* pFollowAction = new FollowToShootAction(pArmedGuy);
        pPed->addToAltActions(pFollowAction);
        pPed->addToAltActions(new FireWeaponAction(pArmedGuy));
        pPed->addToAltActions(new ResetScriptedAction(Action::kActionAlt));
    } else { // just update the target in the existing chain of actions
        MovementAction *pAction = pPed->altAction();
        while (pAction != NULL) {
            switch(pAction->type()) {
            case Action::kActTypeWaitShoot:
                {
                WaitBeforeShootingAction *pAct = dynamic_cast<WaitBeforeShootingAction *>(pAction);
                pAct->setTarget(pArmedGuy);
                }
                break;
            case Action::kActTypeFollowToShoot:
                {
                FollowToShootAction *pAct = dynamic_cast<FollowToShootAction *>(pAction);
                pAct->setTarget(pArmedGuy);
                }
                break;
            case Action::kActTypeFire:
                {
                FireWeaponAction *pAct = dynamic_cast<FireWeaponAction *>(pAction);
                pAct->setTarget(pArmedGuy);
                }
                break;
            default:
                break;
            }
            pAction = pAction->next();
        }
    }
    pPed->setCurrentActionWithSource(Action::kActionAlt);
}

}

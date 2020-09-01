/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2012  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#include "ped.h"
#include "core/gamecontroller.h"
#include "utils/log.h"

/*!
 * Add the given action to the list of actions.
 * If appendAction is true, the action is added after all existing actions.
 * Else existing actions are destroyed and new action becomes the only one.
 * When current action is replaced, it is first suspended.
 * \param pActionToAdd The action to add
 * \param appendAction If true action is append after all existing actions.
 */
void PedInstance::addMovementAction(MovementAction *pActionToAdd, bool appendAction) {
    if (pActionToAdd == NULL) {
        return;
    }

    if (currentAction_ == NULL) {
        currentAction_ = pActionToAdd;
    } else if (appendAction) {
        // append new action at the end of action chain
        MovementAction *pAct = currentAction_;
        while(pAct->next()) {
            pAct = pAct->next();
        }
        pAct->link(pActionToAdd);
    } else {
        // replace current action with new one
        // first : suspend current action
        if (currentAction_->suspend(this)) {
            // action has been suspended so destroy chain and set new current
            destroyAllActions(false);
            currentAction_ = pActionToAdd;
        } else {
            // action cannot be suspended so delay new action using ReplaceCurrentAction
            ReplaceCurrentAction *pReplace = new ReplaceCurrentAction(pActionToAdd);
            currentAction_->insertNext(pReplace);
        }

    }
}

/*!
 * Adds the given action to the list of actions which source is default.
 * \param pToAdd MovementAction* Action to add
 * \return void
 *
 */
void PedInstance::addToDefaultActions(MovementAction *pToAdd) {
    if (defaultAction_ == NULL) {
        defaultAction_ = pToAdd;
    } else {
        MovementAction *pAction = defaultAction_;
        // get last action of the list
        while(pAction->next() != NULL) {
            pAction = pAction->next();
        }

        pAction->link(pToAdd);
    }

    // set all new actions repeatable because scripted action are generally repeated
    MovementAction *pAction = pToAdd;
    while (pAction != NULL) {
        pAction->setSource(Action::kActionDefault);
        pAction = pAction->next();
    }
}

/*!
 * Adds the given action to the list of actions which source is Alt.
 * \param pToAdd MovementAction* Action to add
 * \return void
 *
 */
void PedInstance::addToAltActions(MovementAction *pToAdd) {
    if (altAction_ == NULL) {
        altAction_ = pToAdd;
    } else {
        MovementAction *pAction = altAction_;
        // get last action of the list
        while(pAction->next() != NULL) {
            pAction = pAction->next();
        }

        pAction->link(pToAdd);
    }

    // set all new actions repeatable because scripted action are generally repeated
    MovementAction *pAction = pToAdd;
    while (pAction != NULL) {
        pAction->setSource(Action::kActionAlt);
        pAction = pAction->next();
    }
}

/*!
 * Destroy current action and following that are not default or alternative actions.
 * \param includeScripted Also destroy default and alt actions.
 * \return void
 *
 */
void PedInstance::destroyAllActions(bool includeScripted) {
    while (currentAction_ != NULL) {
        MovementAction *pNext = currentAction_->next();

        if (currentAction_->source() == Action::kActionNotScripted) {
            // for scripted actions, they will be destroyed in the next while
            currentAction_->removeAndJoinChain();
            delete currentAction_;
        }

        currentAction_ = pNext;
    }

    if (includeScripted) {
        while (defaultAction_ != NULL) {
            MovementAction *pNext = defaultAction_->next();
            defaultAction_->unlinkNext();
            delete defaultAction_;

            defaultAction_ = pNext;
        }

        while (altAction_ != NULL) {
            MovementAction *pNext = altAction_->next();
            altAction_->unlinkNext();
            delete altAction_;

            altAction_ = pNext;
        }
    }
}

/*!
 * Removes the current use weapon action.
 */
void PedInstance::destroyUseWeaponAction() {
    if (pUseWeaponAction_ != NULL) {
        delete pUseWeaponAction_;
        pUseWeaponAction_ = NULL;
    }
}

/*!
 * Restart the actions of given source and set as current action.
 * \param source
 */
void PedInstance::resetActions(Action::ActionSource source) {
    currentAction_ = (source == Action::kActionDefault) ?
                                            defaultAction_ : altAction_;

    if (currentAction_) {
        // reset all scripted actions
        MovementAction *pActionToReset = currentAction_;
        while (pActionToReset != NULL) {
            pActionToReset->reset();
            pActionToReset = pActionToReset->next();
        }
    }
}

/*!
 * Change the current action to the list of actions identified by given source.
 * \param source The new source of actions
 */
void PedInstance::setCurrentActionWithSource(Action::ActionSource source) {
    if (currentAction_ == NULL) {
        // no action currently defined so immediately assign new chain
        resetActions(source);
    } else {
        MovementAction *pAction = new ResetScriptedAction(source);
        if (currentAction_->suspend(this)) {
            // action has been suspended
			// so we can change source immediately
            currentAction_->insertPrevious(pAction);
            currentAction_ = pAction;
        } else {
			// action cannot be suspended
			// so let the action finish before changing source
            currentAction_->insertNext(pAction);
        }
    }
}

/*!
 * Adds the action to walk.
 * \param destPosT Destination point
 * \param origin Origin of action
 * \param appendAction If true action is append after all existing actions.
 */
void PedInstance::addActionWalk(const TilePoint &destPosT, bool appendAction) {
    WalkAction *action = new WalkAction(destPosT);
    addMovementAction(action, appendAction);
}

void PedInstance::addActionFollowPed(PedInstance *pPed) {
    FollowAction *pAction = new FollowAction(pPed);
    addMovementAction(pAction, false);
}

/*!
 * Change the owner of this ped.
 * Called when the former owner dies.
 * \param pNewOwner The new owner
 */
void PedInstance::setNewOwner(PedInstance *pNewOwner) {
    pNewOwner->addPersuaded(this);
    owner_ = pNewOwner;
    // follow new owner
    if (defaultAction_ != NULL && defaultAction_->type() == Action::kActTypeFollow) {
        FollowAction *pAction = static_cast<FollowAction *> (defaultAction_);
        pAction->setTarget(pNewOwner);
    }
}

/*!
 * Adds the action to drop weapon.
 * \param weaponIndex The index of the weapon to drop in the agent inventory.
 * \param appendAction If true action is append after all existing actions.
 */
void PedInstance::addActionPutdown(uint8 weaponIndex, bool appendAction) {
    PutdownWeaponAction *action = new PutdownWeaponAction(weaponIndex);
    addMovementAction(action, appendAction);
}

/*!
 * Create 2 actions : one to go to the given weapon's location and one
 * to pick up weapon.
 * \param pWeapon the weapon to pick up.
 */
MovementAction * PedInstance::createActionPickup(WeaponInstance *pWeapon) {
    // First go to weapon
    WalkAction *pWalk = new WalkAction(pWeapon);
    // Then pick it up
    PickupWeaponAction *pPick = new PickupWeaponAction(pWeapon);
    pWalk->link(pPick);

    return pWalk;
}

/*!
 * Creates two actions that tells the ped to go to the given car and then enter it.
 * \param pVehicle
 * \return The first action is returned.
 */
MovementAction * PedInstance::createActionEnterVehicle(Vehicle *pVehicle) {
    // First go to vehicle
    WalkAction *action = new WalkAction((ShootableMapObject *) pVehicle);
    // Then get in
    EnterVehicleAction *enterAct = new EnterVehicleAction(pVehicle);
    action->link(enterAct);

    return action;
}

//! Adds action to drive vehicle to destination
void PedInstance::addActionDriveVehicle(
        GenericCar *pVehicle, const TilePoint &destination, bool appendAction) {
    DriveVehicleAction *pAction = new DriveVehicleAction(pVehicle, destination);
    addMovementAction(pAction, appendAction);
}

/*!
 * Insert an action of type HitAction before any action in list and suspend
 * currently executing action.
 */
void PedInstance::insertHitAction(fs_dmg::DamageToInflict &d) {
    HitAction *pHitAct = NULL;
    if (d.d_owner == this) { // it's a suicide
        if (d.dtype == fs_dmg::kDmgTypeBullet) {
            // Ped's instantly dead
            pHitAct = new FallDeadHitAction(d);
        } else {
            // it's an explosion : walk and burn
            pHitAct = new WalkBurnHitAction(d);
        }
    } else if (d.dtype == fs_dmg::kDmgTypeExplosion) {
        if (hasMinimumVersionOfMod(Mod::MOD_CHEST, Mod::MOD_V2)) {
            pHitAct = new RecoilHitAction(d);
        } else {
            pHitAct = new WalkBurnHitAction(d);
        }
    } else if (d.dtype & (fs_dmg::kDmgTypeBullet | fs_dmg::kDmgTypeCollision)) {
        // When hit by a bullet or collision, ped is ejected
        pHitAct = new RecoilHitAction(d);
    } else if (d.dtype == fs_dmg::kDmgTypeBurn) {
        pHitAct = new WalkBurnHitAction(d);
    } else if (d.dtype == fs_dmg::kDmgTypeLaser) {
        // When hit by a laser, ped is vaporized
        pHitAct = new LaserHitAction(d);
    } else if (d.dtype == fs_dmg::kDmgTypePersuasion) {
        pHitAct = new PersuadedHitAction(d);
    }

    if (pHitAct != NULL) {
        //! if there was an action executing, suspends it
        if (currentAction_ != NULL) {
            currentAction_->suspend(this);
            currentAction_->insertPrevious(pHitAct);
        }
        // hit becomes new current action
        currentAction_ = pHitAct;
    }
}

/*!
 * Adds an action of shooting at something/somewhere.
 * Also adds a shooting action for persuaded if this is an agent.
 * \param aimedPt Where the ped must shoot
 * \return kShootActionNotAdded if no action (see canAddUseWeaponAction()),
 *
 */
uint8 PedInstance::addActionShootAt(const WorldPoint &aimedLocW) {
    if (canAddUseWeaponAction()) {
        uint8 res;
        // adds precision to the shoot
        WorldPoint adjAimedLocW = aimedLocW;
        WeaponInstance *pWeapon = selectedWeapon();
        adjustAimedPtWithRangeAndAccuracy(pWeapon->getClass(), &adjAimedLocW);

        if (pWeapon->getClass()->isAutomatic()) {
            pUseWeaponAction_ = new AutomaticShootAction(adjAimedLocW, pWeapon);
            res = ShootAction::kShootActionAutomaticShoot;
        } else {
            pUseWeaponAction_ = new ShootAction(adjAimedLocW, pWeapon);
            res = ShootAction::kShootActionSingleShoot;
        }

        // notify persuadeds to shoot
        for (std::set <PedInstance *>::iterator it =  persuadedSet_.begin();
                it != persuadedSet_.end(); it++) {
                    (*it)->addActionShootAt(aimedLocW);
        }

        return res;
    }

    return ShootAction::kShootActionNotAdded;
}

/*!
 * Adds an action to use the medikit on the owner.
 */
void PedInstance::addActionUseMedikit(WeaponInstance *pMedikit) {
    pUseWeaponAction_ = new UseMedikitAction(pMedikit);
}

/*!
 * Adds an action to use the energy shield on the owner.
 */
void PedInstance::addActionUseEnergyShield(WeaponInstance *pEnergyShield) {
    pUseWeaponAction_ = new UseEnergyShieldAction(pEnergyShield);
}

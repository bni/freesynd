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

#include "ia/actions.h"
#include "app.h"
#include "ped.h"
#include "model/weapon.h"
#include "model/vehicle.h"
#include "model/train.h"
#include "model/squad.h"
#include "mission.h"
#include "agentmanager.h"


//*************************************
// Constant definition
//*************************************
const int FollowAction::kFollowDistance = 192;
const int WalkBurnHitAction::kTimeToWalkBurning = 1000;
const uint8 ShootAction::kShootActionNotAdded = 0;
const uint8 ShootAction::kShootActionAutomaticShoot = 1;
const uint8 ShootAction::kShootActionSingleShoot = 2;

/*!
 * Default constructor.
 * \param aType What type of action.
 * \param origin Who has created this action.
 */
Action::Action(ActionType aType) {
    status_ = kActStatusNotStarted;
    type_ = aType;
    source_ = kActionNotScripted;
}

/*!
 * Reset the action.
 */
void Action::reset() {
    status_ = kActStatusNotStarted;
}

/*!
 * Default constructor.
 * \param aType What type of action.
 * \param exclusive Does action allow shooting
 * \param canExecVehicle Is action is allowed while ped is in vehicle
 */
MovementAction::MovementAction(ActionType aType, bool exclusive, bool canExecVehicle) :
Action(aType) {
    pNext_ = NULL;
    pPrevious_ = NULL;
    isExclusive_ = exclusive;
    canExecInVehicle_ = canExecVehicle;
    targetState_ = PedInstance::pa_smNone;
    warnBehaviour_ = false;
}

/*!
 * Action execution.
 * If action is not started :
 *   - if ped is in a vehicle and action cannot be executed inside, fail
 *   - else call doStart()
 * If action has failed, exit.
 * Else change Ped's state to targetState.
 * Calls doExecute() method.
 * If action is finished (failed or succeeded), quits state.
 * \return True to update screen.
 */
bool MovementAction::execute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (status_ == kActStatusNotStarted) {
        // Action is not started so start it
        status_ = kActStatusRunning;
        if (pPed->inVehicle() && !canExecInVehicle()) {
            // Ped is in a vehicle and action cannot be executed in a vehicle
            setFailed();
        } else {
            doStart(pMission, pPed);
        }
        if (status_ == kActStatusFailed) {
            // action failed so nothing changed
            return false;
        } else {
            // action has started correctly -> change state
            pPed->goToState(targetState_);
        }
    }

    bool update = false;
    if (!isFinished()) {
        update = doExecute(elapsed, pMission, pPed);
    }

    if (isFinished()) {
        pPed->leaveState(targetState_);
    }

    return update;
}

/*! \brief
 * Suspend the action if possible. Subclasses must call this method.
 * By default actions are suspendable but they are not suspended if they
 * are not started.
 * \param pPed PedInstance*
 * \return bool True if action has been suspended or if it was not started.
 *
 */
bool MovementAction::suspend(PedInstance *pPed) {
    if (status_ != kActStatusNotStarted) {
        pPed->leaveState(targetState_);
        savedStatus_ = status_;
        status_ = kActStatusSuspended;
    }

    return true;
}

void MovementAction::resume(Mission *pMission, PedInstance *pPed) {
    status_ = savedStatus_;
    pPed->goToState(targetState_);
}

/*!
 * Insert the action between this action and his current next
 * \param pAction MovementAction*
 * \return void
 *
 */
void MovementAction::insertNext(MovementAction *pAction) {
    if (pNext_ != NULL) {
        pAction->link(pNext_);
    }

    this->link(pAction);
}

/*!
 * Insert the action between this action and his current previous.
 * \param pAction MovementAction*
 * \return void
 *
 */
void MovementAction::insertPrevious(MovementAction *pAction) {
    if (pAction != NULL) {
        if (pPrevious_ != NULL) {
            pPrevious_->link(pAction);
        }

        pAction->link(this);
    }
}

void MovementAction::unlinkNext() {
    if (pNext_ != NULL) {
        pNext_->setPrevious(NULL);
        pNext_ = NULL;
    }
}

/*!
 * Remove this action from the chain of actions.
 * Link the previous to the next.
 * \return void
 *
 */
void MovementAction::removeAndJoinChain() {
    MovementAction *pPrev = previous();
    MovementAction *pNext = next();

    // remove link to next
    unlinkNext();
    // remove link to previous
    if (pPrev != NULL) {
        pPrev->unlinkNext();
        // link previous and next
        pPrev->link(pNext);
    }

    pPrevious_ = NULL;
    pNext_ = NULL;
}

WalkAction::WalkAction(const TilePoint &locT, int speed) :
    MovementAction(kActTypeWalk) {
    newSpeed_ = speed;
    destLocT_ = locT;
    targetState_ = PedInstance::pa_smWalking;
}

/*! \brief
 *
 * \param smo ShootableMapObject*
 * \param speed int
 *
 */
WalkAction::WalkAction(ShootableMapObject *smo, int speed) :
MovementAction(kActTypeWalk) {
    newSpeed_ = speed;
    targetState_ = PedInstance::pa_smWalking;
    setDestination(smo);
}

void WalkAction::setDestination(ShootableMapObject *smo) {
    // Set destination point
    destLocT_ = smo->position();
}

bool WalkAction::suspend(PedInstance *pPed) {
    pPed->setSpeed(0);
    return MovementAction::suspend(pPed);
}

void WalkAction::doStart(Mission *pMission, PedInstance *pPed) {
    // Go to given location at given speed
    if (!pPed->initMovementToDestination(pMission, destLocT_, newSpeed_)) {
        setFailed();
        return;
    }
}

/*!
 * This method first updates movement for the ped.
 * Then if the ped has no more destination point, it means that
 * he has arrived.
 * Else action continue.
 * \param elapsed Time elapsed since last frame
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
bool WalkAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    bool updated = pPed->doMove(elapsed, pMission);
    if (!pPed->hasDestination()) {
        // Ped has arrived at destination
        setSucceeded();
    }
    return updated;
}

WalkToDirectionAction::WalkToDirectionAction(const WorldPoint &destLocW) :
MovementAction(kActTypeWalk) {
    maxDistanceToWalk_ = 0;
    destLocW_ = destLocW;
    targetState_ = PedInstance::pa_smWalking;
    newSpeed_ = -1;
}

WalkToDirectionAction::WalkToDirectionAction(int speed) :
MovementAction(kActTypeWalk) {
    maxDistanceToWalk_ = 0;
    destLocW_.x = -1;
    destLocW_.y = -1;
    destLocW_.z = -1;
    targetState_ = PedInstance::pa_smWalking;
    newSpeed_ = speed;
}

bool WalkToDirectionAction::suspend(PedInstance *pPed) {
    pPed->setSpeed(0);
    return MovementAction::suspend(pPed);
}

void WalkToDirectionAction::doStart(Mission *pMission, PedInstance *pPed) {
    moveDirdesc_.clear();
    moveDirdesc_.bounce = true;
    pPed->setSpeed(newSpeed_ != -1? newSpeed_ : pPed->getDefaultSpeed());
    distWalked_ = 0;
}

/*!
 * Ped moves until he reaches destination.
 * \param elapsed Time elapsed since last frame
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
bool WalkToDirectionAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    bool endAction = false;
    int distanceToWalk = 0;
    if (destLocW_.x != -1) {
        int diffx = destLocW_.x - pPed->tileX() * 256 - pPed->offX();
        int diffy = destLocW_.y - pPed->tileY() * 256 - pPed->offY();

        // In this case, distance is the distance between ped and destination location
        distanceToWalk = (int)sqrt((double)(diffx * diffx + diffy * diffy));

        if (distanceToWalk > 0) {
            pPed->moveToDir(pMission,
                            elapsed,
                            moveDirdesc_,
                            -1,
                            destLocW_.x, destLocW_.y,
                            &distanceToWalk);

            if (pPed->tileX() * 256 - pPed->offX() == destLocW_.x
                && pPed->tileY() * 256 - pPed->offY() == destLocW_.y)
                // TODO: add correct z or ignore it?
            {
                endAction = true;
            }
        } else {
            endAction = true;
        }
    } else if (maxDistanceToWalk_ != 0) {
        distanceToWalk = maxDistanceToWalk_ - distWalked_;
        pPed->moveToDir(pMission,
                        elapsed,
                        moveDirdesc_,
                        -1,
                        -1, -1,
                        &distanceToWalk, true);

        distWalked_ += distanceToWalk;
        endAction = distWalked_ >= maxDistanceToWalk_;
    } else {
        // just keep moving
        pPed->moveToDir(pMission,
                        elapsed,
                        moveDirdesc_);
    }

    if (endAction) {
        setSucceeded();
        pPed->clearDestination();
    }

    return true;
}

/*!
 * Constructor.
 * \param range Radius of the trigger zone
 * \param loc Center of the trigger zone
 */
TriggerAction::TriggerAction(int32 range, const WorldPoint &loc) :
        MovementAction(kActTypeUndefined, false, true) {
    range_ = range;
    centerLoc_ = loc;
}

/*!
 * Check that at least one agent enters the zone.
 * \param elapsed Time elapsed since last frame
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
bool TriggerAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    for (uint8 i = 0; i < AgentManager::kMaxSlot; ++i) {
        PedInstance *pAgent = pMission->getSquad()->member(i);
        if(pAgent && pAgent->isAlive() && pAgent->isCloseTo(centerLoc_, range_)) {
            setSucceeded();
            return true;
        }
    }
    return false;
}

/*!
 * This action only sets the ped's state to "Escaped".
 * \param elapsed Time elapsed since last frame
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
bool EscapeAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    setSucceeded();
    pPed->escape();
    return true;
}

/*!
 * Class constructor.
 * \param pTarget The ped to follow.
 */
FollowAction::FollowAction(PedInstance *pTarget) :
MovementAction(kActTypeFollow) {
    pTarget_ = pTarget;
    targetState_ = PedInstance::pa_smWalking;
}

/*!
 * Saves the target current position in the targetLastPos_ field.
 */
void FollowAction::updateLastTargetPos() {
    targetLastPos_ = pTarget_->position();
}

void FollowAction::doStart(Mission *pMission, PedInstance *pPed) {
    updateLastTargetPos();
    // If target is not too close, then initiate movement.
    if (!pPed->isCloseTo(pTarget_, kFollowDistance)) {
        if (!pPed->initMovementToDestination(pMission, targetLastPos_)) {
            setFailed();
        }
    } else {
        // Else, Ped must stay standing. But the method Action::execute() will do a goToState()
        // right after doStart() has been called. So we'll have a walking
        // animation without the ped moving. See doExecute() for more
        targetState_ = PedInstance::pa_smStanding;
    }
}

bool FollowAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    bool updated = false;

    if (pTarget_->isDead()) {
        // target is dead so stop moving and terminate action
        pPed->clearDestination();
        setFailed();
    } else {
        if (pPed->speed() != 0) {
            if (pPed->isCloseTo(pTarget_, kFollowDistance)) {
                // We reached the target so stop moving temporarily
                pPed->clearDestination();
                pPed->leaveState(targetState_);
            } else {
                updated = pPed->doMove(elapsed, pMission);
            }
        }

        // target has moved: we use checkCurrPosTileOnly() to give time to ped
        // to walk away else animation is buggy
        if (!pTarget_->sameTile(targetLastPos_)) {
            // resetting target position
            updateLastTargetPos();
            if (pPed->initMovementToDestination(pMission, targetLastPos_)) {
                targetState_ = PedInstance::pa_smWalking;
                pPed->goToState(targetState_);
            } else {
                setFailed();
            }
        }
    }
    return updated;
}

/*!
 * Class constructor.
 * \param pTarget The ped to follow.
 */
FollowToShootAction::FollowToShootAction(PedInstance *pTarget) :
MovementAction(kActTypeFollowToShoot) {
    pTarget_ = pTarget;
    targetState_ = PedInstance::pa_smWalking;
    followDistance_ = 0;
}

/*!
 * If the ped's has no weapon, don't follow.
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
void FollowToShootAction::doStart(Mission *pMission, PedInstance *pPed) {
    if (pPed->selectedWeapon() == NULL) {
        setFailed();
        return;
    } else {
        followDistance_ = (pPed->selectedWeapon()->range() / 3 ) *2;
    }

    targetLastPosW_.reset();
}

bool FollowToShootAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    bool updated = false;

    if (pTarget_->isDead()) {
        // target is dead so stop moving and terminate action
        pPed->clearDestination();
        setFailed();
    } else {
        // target has moved: we check if target is not too far to give time to ped
        // to walk away else animation is buggy
        if (!pTarget_->isCloseTo(targetLastPosW_, 128)) {
            // resetting target position
            targetLastPosW_.convertFromTilePoint(pTarget_->position());
            if (!pPed->initMovementToDestination(pMission, pTarget_->position())) {
                setFailed();
                return true;
            }
        }

        WorldPoint pedPosW(pPed->position());
        // Ped stops walking if the target is in range of fire (ie close enough and not
        // hiding behing something)
        if (pPed->isCloseTo(pTarget_, followDistance_) &&
            pMission->checkBlockedByTile(pedPosW, &targetLastPosW_, true, followDistance_) == 1) {
            // We reached the target so stop moving
            setSucceeded();
            pPed->clearDestination();
        } else {
            updated = pPed->doMove(elapsed, pMission);
        }

    }
    return updated;
}

PutdownWeaponAction::PutdownWeaponAction(uint8 weaponIdx) : MovementAction(kActTypeDrop, true) {
    weaponIdx_ = weaponIdx;
    targetState_ = PedInstance::pa_smPutDown;
}

void PutdownWeaponAction::doStart(Mission *pMission, PedInstance *pPed) {
    status_ = kActStatusWaitForAnim;
}

/*!
 * Action can be executed only when the animation of dropping a weapon is done.
 */
bool PutdownWeaponAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (status_ != kActStatusWaitForAnim) {
        WeaponInstance *pWeapon = pPed->dropWeapon(weaponIdx_);
        // Dropping a timebomb means activate it to explode
        if (pWeapon->isInstanceOf(Weapon::TimeBomb)) {
            pWeapon->activate();
        }

        setSucceeded();
    }

    return true;
}

PickupWeaponAction::PickupWeaponAction(WeaponInstance *pWeapon) :
    MovementAction(kActTypePickUp, true) {
    pWeapon_ = pWeapon;
    targetState_ = PedInstance::pa_smPickUp;
}

void PickupWeaponAction::doStart(Mission *pMission, PedInstance *pPed) {
    // recheck here in case weapon was pickup between the time the action
    // was added and now
    if (pWeapon_->hasOwner() || pWeapon_->isDead() ||
        pPed->numWeapons() == WeaponHolder::kMaxHoldedWeapons ||
        !pPed->samePosition(pWeapon_)) {
        setFailed();
    } else {
        // the animation must run first then the object will be picked up
        status_ = kActStatusWaitForAnim;
    }
}

bool PickupWeaponAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (status_ != kActStatusWaitForAnim) {
        pWeapon_->setOwner(pPed);
        pWeapon_->deactivate();
        pPed->addWeapon(pWeapon_);
        pMission->removeWeaponOnGround(pWeapon_);

        setSucceeded();
    }
    return true;
}

EnterVehicleAction::EnterVehicleAction(Vehicle *pVehicle) :
        MovementAction(kActTypeUndefined, true) {
    pVehicle_ = pVehicle;
}

void EnterVehicleAction::doStart(Mission *pMission, PedInstance *pPed) {
    if (pVehicle_->isDead()) {
        setFailed();
    }
}

bool EnterVehicleAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    // Ped needs to be near vehicle to get in
    if (pPed->samePosition(pVehicle_)) {
        // state of ped is set in addPassenger
        pVehicle_->addPassenger(pPed);
    }
    // Finish action anyway
    setSucceeded();
    return true;
}

DriveVehicleAction::DriveVehicleAction(GenericCar *pVehicle, const TilePoint &dest) :
    MovementAction(kActTypeUndefined, false, true) {
    pVehicle_ = pVehicle;
    dest_ = dest;
}

void DriveVehicleAction::doStart(Mission *pMission, PedInstance *pPed) {
    if (pVehicle_->isDead() || !pVehicle_->containsPed(pPed)) {
        setFailed();
    }

    if (!pVehicle_->initMovementToDestination(pMission, dest_, 1024)) {
        setFailed();
    }
}

bool DriveVehicleAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (pPed->sameTile(dest_)) {
        setSucceeded();
    }
    return true;
}

DriveTrainAction::DriveTrainAction(TrainHead *pTrain, const TilePoint &dest) :
    MovementAction(kActTypeUndefined, false, true) {
    pTrain_ = pTrain;
    dest_ = dest;
}

void DriveTrainAction::doStart(Mission *pMission, PedInstance *pPed) {
    if (!pTrain_->containsPed(pPed)) {
        setFailed();
    }

    if (!pTrain_->initMovementToDestination(pMission, dest_, 1024)) {
        setFailed();
    }
}

bool DriveTrainAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (!pTrain_->hasDestination()) {
        setSucceeded();

        dropAgents(*pMission);
    }

    return true;
}

void DriveTrainAction::dropAgents(const Mission &mission) {
    // find center of train
    // Train are always five elements long
    TrainBody *pBody = pTrain_->getNext()->getNext();
    TilePoint dropPos = pBody->position();

    // There should be passengers only in train body
    pBody = pTrain_->getNext();
    while (pBody->getType() != Vehicle::kVehicleTypeTrainHead) {
        pBody->dropAllPassengers(mission, dropPos);
        pBody = pBody->getNext();
    }
}

WaitAction::WaitAction(WaitEnum waitFor, uint32 duration) :
MovementAction(kActTypeWait, true, true), waitTimer_(duration) {
    waitType_ = waitFor;
}

WaitAction::WaitAction(WaitEnum waitFor) :
MovementAction(kActTypeWait, true, true), waitTimer_(0) {
    waitType_ = waitFor;
}

void WaitAction::doStart(Mission *pMission, PedInstance *pPed) {
    waitTimer_.reset();
}

bool WaitAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (waitType_ == kWaitTime) {
        if (waitTimer_.update(elapsed)) {
            setSucceeded();
            return true;
        }
    } else if (waitType_ == kWaitWeapon) {
        if (!pPed->isUsingWeapon()) {
            setSucceeded();
            return true;
        }
    }
    return false;
}

WaitBeforeShootingAction::WaitBeforeShootingAction(PedInstance *pPed) :
MovementAction(kActTypeWaitShoot, true), waitTimer_(2000) {
    pTarget_ = pPed;
}

void WaitBeforeShootingAction::doStart(Mission *pMission, PedInstance *pPed) {
    if (pTarget_->isDead()) {
        setFailed();
    } else {
        waitTimer_.reset();
        pPed->clearDestination();
        pPed->selectShootingWeaponWithAmmo();
    }
}

bool WaitBeforeShootingAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    // point toward target
    pPed->setDirectionTowardObject(*pTarget_);

    if (waitTimer_.update(elapsed)) {
        if (pPed->type() == PedInstance::kPedTypeAgent && pTarget_->isOurAgent()) {
            // Warn only for player agents
            GameEvent evt;
            evt.stream = GameEvent::kMission;
            evt.type = GameEvent::kEvtWarnAgent;
            g_gameCtrl.fireGameEvent(evt);
        }
        setSucceeded();
        return true;
    }

    return false;
}

FireWeaponAction::FireWeaponAction(PedInstance *pPed) :
MovementAction(kActTypeFire) {
    pTarget_ = pPed;
}

void FireWeaponAction::doStart(Mission *pMission, PedInstance *pPed) {
    if (pTarget_->isDead()) {
        setFailed();
    } else if (pPed->type() == PedInstance::kPedTypePolice && pTarget_->selectedWeapon() == NULL) {
        // Police man don't shoot on peds that don't have gun out
        setFailed();
    } else if (pTarget_->isEnergyShieldActivated()) {
        // Don't shoot if target has an energy shield
        setFailed();
    } else {
        WorldPoint targetLocW(pTarget_->position());

        shootType_ = pPed->addActionShootAt(targetLocW);
        if (shootType_ == ShootAction::kShootActionNotAdded) {
            // failed to shoot because weapon has no ammo
            setFailed();
        } else if (shootType_ == ShootAction::kShootActionAutomaticShoot) {
            // todo :set a timer to controle time shooting
        }
    }
}

bool FireWeaponAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (!pPed->isUsingWeapon()) {
        setSucceeded();
    }

    return true;
}

HitAction::HitAction(fs_dmg::DamageToInflict &d) :
MovementAction(kActTypeHit) {
    damage_.aimedLocW = d.aimedLocW;
    damage_.dtype = d.dtype;
    damage_.dvalue = d.dvalue;
    damage_.d_owner = d.d_owner;
    damage_.originLocW = d.originLocW;
    damage_.pWeapon = d.pWeapon;
}

FallDeadHitAction::FallDeadHitAction(fs_dmg::DamageToInflict &d) :
HitAction(d) {
    targetState_ = PedInstance::pa_smNone;
}

/*!
 *
 * \param elapsed Time elapsed since last frame
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
bool FallDeadHitAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (status_ == kActStatusRunning) {
        pPed->handleDeath(pMission, damage_);
        setSucceeded();
    }
    return true;
}

RecoilHitAction::RecoilHitAction(fs_dmg::DamageToInflict &d) :
HitAction(d) {
    targetState_ = PedInstance::pa_smHit;
}

/*!
 *
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
void RecoilHitAction::doStart(Mission *pMission, PedInstance *pPed) {
    // Change direction due to impact
    pPed->setDirectionTowardPosition(damage_.originLocW);
    status_ = kActStatusWaitForAnim;
}

/*!
 *
 * \param elapsed Time elapsed since last frame
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
bool RecoilHitAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (status_ == kActStatusRunning) {
        if (pPed->handleDeath(pMission, damage_)) {
            targetState_ = PedInstance::pa_smNone;
        }
        setSucceeded();
    }
    return true;
}

LaserHitAction::LaserHitAction(fs_dmg::DamageToInflict &d) :
HitAction(d) {
    targetState_ = PedInstance::pa_smHitByLaser;
}

/*!
 *
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
void LaserHitAction::doStart(Mission *pMission, PedInstance *pPed) {
    // Change direction due to impact
    pPed->setDirectionTowardPosition(damage_.originLocW);
    status_ = kActStatusWaitForAnim;
}

/*!
 *
 * \param elapsed Time elapsed since last frame
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
bool LaserHitAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (status_ == kActStatusRunning) {
        if (pPed->handleDeath(pMission, damage_)) {
            targetState_ = PedInstance::pa_smNone;
        }
        setSucceeded();
    }
    return true;
}

WalkBurnHitAction::WalkBurnHitAction(fs_dmg::DamageToInflict &d) :
HitAction(d),burnTimer_(kTimeToWalkBurning) {
    targetState_ = PedInstance::pa_smWalkingBurning;
}

/*!
 *
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
void WalkBurnHitAction::doStart(Mission *pMission, PedInstance *pPed) {
    moveDirdesc_.clear();
    walkedDist_ = 0;
    moveDirection_ = rand() % 256;
    pPed->setSpeed(pPed->getDefaultSpeed());
}

/*!
 *
 * \param elapsed Time elapsed since last frame
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
bool WalkBurnHitAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    int walkDistDiff = 0;
    pPed->moveToDir(pMission, elapsed, moveDirdesc_, moveDirection_, -1, -1, &walkDistDiff, true);
    walkedDist_ += walkDistDiff;

    if (burnTimer_.update(elapsed)) {
        setSucceeded();

        if (pPed->handleDeath(pMission, damage_)) {
            targetState_ = PedInstance::pa_smNone;
        }
    }

    return true;
}

PersuadedHitAction::PersuadedHitAction(fs_dmg::DamageToInflict &d) :
HitAction(d) {
    targetState_ = PedInstance::pa_smHitByPersuadotron;
    warnBehaviour_ = true;
}

/*!
 *
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
void PersuadedHitAction::doStart(Mission *pMission, PedInstance *pPed) {
    status_ = kActStatusWaitForAnim;
    g_App.gameSounds().play(snd::PERSUADE);
}

/*!
 *
 * \param elapsed Time elapsed since last frame
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 */
bool PersuadedHitAction::doExecute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (status_ == kActStatusRunning) {
        PedInstance *pAgent = static_cast<PedInstance *>(damage_.d_owner);
        pPed->handlePersuadedBy(pAgent);
        setSucceeded();
    }
    return true;
}

ShootAction::ShootAction(const WorldPoint &aimedAt, WeaponInstance *pWeapon) :
    UseWeaponAction(kActTypeShoot, pWeapon) {
        aimedAt_ = aimedAt;
}

void ShootAction::setAimedAt(const WorldPoint &aimedAt) {
    aimedAt_ = aimedAt;
}

/*!
 * Execute the shoot action.
 * When action starts, it sets the firing state (and animation), fires
 * the weapon and then waits for the end of animation.
 * When animation is over, wait for weapon
 * to reload before finishing.
 * \param elapsed Time since last frame.
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 * \return true to redraw
 */
bool ShootAction::execute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (status_ == kActStatusNotStarted) {
        // Turn to target
        pPed->setDirectionTowardPosition(aimedAt_);
        // Shoot
        fs_dmg::DamageToInflict dmg;
        fillDamageDesc(pMission, pPed, pWeapon_, dmg);
        pWeapon_->playSound();
        pWeapon_->fire(pMission, dmg, elapsed);
        // change state to firing
        pPed->goToState(PedInstance::pa_smFiring);
        // waiting for animation to complete
        status_ = kActStatusWaitForAnim;
    } else if (status_ == kActStatusRunning) {
        // Shooting animation is finished
        pPed->leaveState(PedInstance::pa_smFiring);

        // The action is complete only after a certain laps of time to
        // simulate the fact that the weapon needs to be reloaded
        // and the shooter's reactivity to that
        timeToWait_ = pPed->getTimeBetweenShoots(pWeapon_);
        setWaitingForTime();
    } else if (status_ == kActStatusWaitForTime) {
        timeToWait_ -= elapsed;
        if (timeToWait_ <= 0) {
            // time is reached so action can finish
            setSucceeded();
        }
    }

    return true;
}

/*!
 * Fills the ShotAttributes structure with needed information.
 * \param pMission Mission data
 * \param targetLoc Where to shoot
 * \param dmg Structure to fill
 */
void ShootAction::fillDamageDesc(Mission *pMission,
                                    PedInstance *pShooter,
                                    WeaponInstance *pWeapon,
                                    fs_dmg::DamageToInflict &dmg) {
    dmg.pWeapon = pWeapon;
    dmg.dtype = pWeapon->getClass()->dmgType();
    dmg.dvalue =  pWeapon->getClass()->damagePerShot();
    dmg.range = pWeapon->getClass()->range();
    dmg.d_owner = pShooter;
    dmg.aimedLocW = aimedAt_;
    dmg.originLocW.convertFromTilePoint(pShooter->position());

    if (pWeapon->isInstanceOf(Weapon::Flamer)) {
        // the weapon is located at half the size of the shooter
        dmg.originLocW.z += pShooter->sizeZ() >> 1;

        switch(pShooter->getDirection()) {
        case 0:
            dmg.originLocW.y += 200;
            break;
        case 1:
            dmg.originLocW.x += 100;
            dmg.originLocW.y += 100;
            break;
        case 2:
            dmg.originLocW.x += 200;
            break;
        case 3:
            dmg.originLocW.x += 130;
            dmg.originLocW.y -= 120;
            break;
        case 4:
            dmg.originLocW.y -= 200;
            break;
        case 5:
            dmg.originLocW.x -= 100;
            dmg.originLocW.y -= 100;
            break;
        case 6:
            dmg.originLocW.x -= 200;
            break;
        case 7:
            dmg.originLocW.x -= 130;
            dmg.originLocW.y += 120;
            break;
        }
    }
}

AutomaticShootAction::AutomaticShootAction(const WorldPoint &aimedAt, WeaponInstance *pWeapon) :
        ShootAction(aimedAt, pWeapon),
        fireRateTimer_(pWeapon->getClass()->fireRate())
{
}

bool AutomaticShootAction::execute(int elapsed, Mission *pMission, PedInstance *pPed) {
    bool firstTime = false;
    if (status_ == kActStatusNotStarted) {
        setRunning();
        // change state to firing
        pPed->goToState(PedInstance::pa_smFiring);
        pWeapon_->setDirection(0);
        firstTime = true;
    }

    if (status_ == kActStatusRunning) {
        if (pPed->isDead() || pWeapon_->ammoRemaining() == 0) {
            stop();
        } else if (firstTime || fireRateTimer_.update(elapsed)) {
            pPed->setDirectionTowardPosition(aimedAt_);
            fs_dmg::DamageToInflict dmg;
            fillDamageDesc(pMission, pPed, pWeapon_, dmg);
            pWeapon_->playSound();
            pWeapon_->fire(pMission, dmg, elapsed);
        }
    } else if (status_ == kActStatusWaitForTime) {
        timeToWait_ -= elapsed;
        if (timeToWait_ <= 0) {
            // time is reached so action can finish
            setSucceeded();
        }
    }

    return true;
}

void AutomaticShootAction::stop() {
    if (status_ == kActStatusRunning) {
        PedInstance *pPed = pWeapon_->owner();
        // Shooting animation is finished
        pPed->leaveState(PedInstance::pa_smFiring);
        // The action is complete only after a certain laps of time to
        // simulate the fact that the weapon needs to be reloaded
        // and the shooter's reactivity to that
        timeToWait_ = pPed->getTimeBetweenShoots(pWeapon_);
        setWaitingForTime();
    }
}

/*!
 * Execute the Use medikit action.
 * \param elapsed Time since last frame.
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 * \return true to redraw
 */
bool UseMedikitAction::execute(int elapsed, Mission *pMission, PedInstance *pPed) {
    bool update = false;
    if (status_ == kActStatusNotStarted) {
        // set time before completing action
        status_ = kActStatusWaitForTime;
        timeToWait_ = pPed->getTimeBetweenShoots(pWeapon_);

        if (!pWeapon_->isInstanceOf(Weapon::MediKit)) {
            setFailed();
        } else {
            pWeapon_->playSound();
            fs_dmg::DamageToInflict dmg;
            dmg.d_owner = pPed;
            pWeapon_->fire(pMission, dmg, elapsed);
            update = true;
        }
    } else if (status_ == kActStatusWaitForTime) {
        timeToWait_ -= elapsed;
        if (timeToWait_ <= 0) {
            // time is reached so action can finish
            setSucceeded();
            update = true;
        }
    }

    return update;
}

/*!
 * Execute the Use medikit action.
 * \param elapsed Time since last frame.
 * \param pMission Mission data
 * \param pPed The ped executing the action.
 * \return true to redraw
 */
bool UseEnergyShieldAction::execute(int elapsed, Mission *pMission, PedInstance *pPed) {
    if (status_ == kActStatusNotStarted) {
        status_ = kActStatusRunning;

        if (!pWeapon_->isInstanceOf(Weapon::EnergyShield)) {
            setFailed();
            return false;
        } else {
            //pWeapon_->playSound();
            fs_dmg::DamageToInflict dmg;
            pWeapon_->fire(pMission, dmg, elapsed);
        }
    } else if (status_ == kActStatusRunning) {
        if (pWeapon_->consumeAmmoForEnergyShield(elapsed)) {
            // no more ammo
            pPed->setEnergyActivated(false);
            setSucceeded();
        }
    }

    return true;
}

void UseEnergyShieldAction::stop() {
    if (status_ == kActStatusRunning) {
        PedInstance *pPed = pWeapon_->owner();
        pPed->setEnergyActivated(false);
        setSucceeded();
    }
}

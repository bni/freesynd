/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
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

#include "fs-utils/common.h"
#include "fs-utils/log/log.h"
#include "fs-engine/gfx/animationmanager.h"
#include "fs-engine/events/event.h"
#include "fs-engine/system/system.h"
#include "fs-kernel/model/vehicle.h"
#include "fs-kernel/model/squad.h"
#include "fs-kernel/model/shot.h"
#include "fs-kernel/mgr/agentmanager.h"
#include "fs-kernel/mgr/missionmanager.h"
#include "fs-kernel/ia/behaviour.h"

namespace fs_knl {

//*************************************
// Constant definition
//*************************************
const int PedInstance::kAgentMaxHealth = 16;
const int PedInstance::kDefaultShootReactionTime = 200;
const uint32_t PedInstance::kPlayerGroupId = 1;
const int PedInstance::kAgentMaxSpeedWithOverweight = 64;

PedInstance::PedInstance(uint16_t anId, Map *pMap, PedType pedType, bool isOur, int maxSpeed) :
    ShootableMovableMapObject(anId, pMap, MapObject::kNaturePed, maxSpeed),
    type_(pedType),
    desc_state_(PedInstance::pd_smUndefined),
    hostile_desc_(PedInstance::pd_smUndefined),
    obj_group_def_(PedInstance::og_dmUndefined),
    old_obj_group_def_(PedInstance::og_dmUndefined),
    obj_group_id_(0), old_obj_group_id_(0),
    sight_range_(0), in_vehicle_(NULL),
    owner_(NULL),
    adrenaline_(IPAStim::Adrenaline),
    perception_(IPAStim::Perception),
    intelligence_(IPAStim::Intelligence)
{
    hold_on_.wayFree = 0;
    state_ = PedInstance::pa_smNone;
    is_our_ = isOur;

    tm_before_check_ = 1000;
    base_mod_acc_ = 0.1;

    behaviour_.setOwner(this);
    currentAction_ = NULL;
    defaultAction_ = NULL;
    altAction_ = NULL;
    pUseWeaponAction_ = NULL;
    panicImmuned_ = false;
    totalPersuasionPoints_ = 0;
    pSelectedWeaponBeforeMedikit_ = NULL;
}

PedInstance::~PedInstance() {
    destroyAllActions();
    destroyUseWeaponAction();
}

/*!
 * A ped cannot panic only if:
 * - he's immuned because he's an mission objective
 * - he's in a car
 * - he's persuaded
 * @return True means he can't panic
 */
bool PedInstance::isImmunedToPanic() { 
    return panicImmuned_ || isInVehicle() || isPersuaded();
}

bool PedInstance::isInPanic() {
    // TODO : find a way to know if he's panicking
    return false;
}

bool PedInstance::switchActionStateTo(uint32_t as) {
    uint32_t prevState = state_;
    switch(as) {
        case pa_smNone:
            //printf("Ped has undefined state");
            break;
        case pa_smStanding:
            state_ &= (pa_smAll ^(pa_smFollowing
                | pa_smInCar));
            state_ |= pa_smStanding;
            break;
        case pa_smWalking:
            state_ &= (pa_smAll ^(pa_smFollowing
                | pa_smInCar));
            state_ |= pa_smWalking;
            break;
        case pa_smWalkingBurning:
            state_ = pa_smWalkingBurning;
            break;
        case pa_smHit:
            state_ = pa_smHit;
            break;
        case pa_smHitByLaser:
            state_ = pa_smHitByLaser;
            break;
        case pa_smHitByPersuadotron:
            state_ = pa_smHitByPersuadotron;
            break;
        case pa_smFiring:
            state_ |= pa_smFiring;
            break;
        case pa_smFollowing:
            state_ &= (pa_smAll ^ (pa_smStanding | pa_smWalking));
            state_ |= pa_smFollowing;
            break;
        case pa_smPickUp:
            state_ = pa_smPickUp;
            break;
        case pa_smPutDown:
            state_ = pa_smPutDown;
            break;
        case pa_smBurning:
            state_ = pa_smBurning;
            break;
        case pa_smInCar:
            state_ = pa_smStanding | pa_smInCar;
            break;
        case pa_smDead:
            state_ = pa_smDead;
            break;
        case pa_smDying:
            state_ = pa_smDying;
            break;
        case pa_smUnavailable:
            state_ = pa_smUnavailable;
            break;
    }

    return prevState != state_;
}

/*!
 * \return true if state has changed.
 */
bool PedInstance::switchActionStateFrom(uint32_t as) {
    uint32_t prevState = state_;
    switch(as) {
        case pa_smNone:
            //printf("Ped has undefined state");
            break;
        case pa_smStanding:
            //state_ &= pa_smAll ^ pa_smStanding;
            printf("switchActionStateFrom : Ped %d cannot leave standing state\n", id_);
            break;
        case pa_smWalking:
            state_ &= pa_smAll ^ pa_smWalking;
            state_ |= pa_smStanding;
            break;
        case pa_smHit:
        case pa_smHitByLaser:
        case pa_smHitByPersuadotron:
            state_ = pa_smStanding;
            break;
        case pa_smFiring:
            state_ &= pa_smAll ^ pa_smFiring;
            break;
        case pa_smFollowing:
            state_ &= pa_smAll ^ (pa_smFollowing | pa_smWalking);
            state_ |= pa_smStanding;
            break;
        case pa_smPickUp:
        case pa_smPutDown:
            state_ = pa_smStanding;
            break;
        case pa_smBurning:
            state_ &= pa_smAll ^ pa_smBurning;
            break;
        case pa_smWalkingBurning:
            state_ = pa_smStanding;
            break;
        case pa_smInCar:
            state_ &= pa_smAll ^ (pa_smStanding | pa_smInCar);
            break;
        case pa_smDead:
            state_ = pa_smDead;
#ifdef _DEBUG
            printf("It's alive!\n");
#endif
            break;
        case pa_smUnavailable:
            state_ = pa_smUnavailable;
            break;
        default:
            state_ = pa_smStanding;
    }

    return prevState != state_;
}

void PedInstance::synchDrawnAnimWithActionState(void) {
    // TODO: complete
    if ((state_ & pa_smUnavailable) != 0) {
    } else if ((state_ & (pa_smWalking | pa_smFollowing)) != 0) {
        if ((state_ & pa_smFiring) != 0) {
        } else {
            playStandOrWalkAnimation();
        }
    } else if ((state_ & pa_smStanding) != 0) {
        if ((state_ & pa_smFiring) != 0) {
            playStandAndShootAnimation();
        } else {
            playStandOrWalkAnimation();
        }
    } else if ((state_ & pa_smPickUp) != 0) {
        playPickupOrDropAnimation();
    } else if ((state_ & pa_smPutDown) != 0) {
        playPickupOrDropAnimation();
    }
#ifdef _DEBUG
    if (state_ ==  pa_smNone)
        printf("synchDrawnAnimWithActionState : undefined state_ %d for ped %d\n", state_, id_);
#endif
}

/*!
 * Set the given state as the new state.
 * Update corresponding animation.
 * \param as new state
 */
void PedInstance::goToState(uint32_t as) {
    if(switchActionStateTo(as)) {
        synchDrawnAnimWithActionState();
    }
}

/*!
 * Leaves the given state.
 * Update corresponding animation.
 * \param as new state
 */
void PedInstance::leaveState(uint32_t as) {
    if (switchActionStateFrom(as)) {
        synchDrawnAnimWithActionState();
    }
}

/*!
 * @brief 
 * @param aState 
 * @return 
 */
bool PedInstance::isState(uint32_t aState) {
    return (state_ & aState) != 0;
}

void PedInstance::setAnimations(uint16_t baseSpriteAnimationId) {
    animations_.standAnimations[Weapon::Unarmed_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId);
    
    if (baseSpriteAnimationId == 1) { // Animations for agents
        animations_.standAnimations[Weapon::EnergyShield_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 16, fs_eng::kAnimationModeLoop);
        animations_.standAnimations[Weapon::Pistol_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        animations_.standAnimations[Weapon::Uzi_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        animations_.standAnimations[Weapon::Shotgun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        animations_.standAnimations[Weapon::Gauss_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        animations_.standAnimations[Weapon::Minigun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 32);
        animations_.standAnimations[Weapon::Laser_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        animations_.standAnimations[Weapon::Flamer_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 40, fs_eng::kAnimationModeLoop);
        animations_.standAnimations[Weapon::LongRange_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 48);
        
        animations_.walkAnimations[Weapon::Unarmed_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 8, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::EnergyShield_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 56, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Pistol_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Uzi_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Shotgun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Gauss_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Minigun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 72, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Laser_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Flamer_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 80, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::LongRange_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 88, fs_eng::kAnimationModeLoop);

        // I think 97 was the original pistol shoot anim, but it wasn't sexy enough.
        animations_.standShootAnimations[Weapon::Pistol_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.standShootAnimations[Weapon::Uzi_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104, fs_eng::kAnimationModeLoop);
        animations_.standShootAnimations[Weapon::Shotgun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.standShootAnimations[Weapon::Gauss_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.standShootAnimations[Weapon::Minigun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 112, fs_eng::kAnimationModeLoop);
        animations_.standShootAnimations[Weapon::Laser_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 120);
        animations_.standShootAnimations[Weapon::Flamer_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 128, fs_eng::kAnimationModeLoop);
        animations_.standShootAnimations[Weapon::LongRange_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 136);
        
        animations_.walkShootAnimations[Weapon::Pistol_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 152);
        animations_.walkShootAnimations[Weapon::Uzi_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 152);
        animations_.walkShootAnimations[Weapon::Shotgun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.walkShootAnimations[Weapon::Gauss_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 152);
        animations_.walkShootAnimations[Weapon::Minigun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 160);
        animations_.walkShootAnimations[Weapon::Laser_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 168);
        animations_.walkShootAnimations[Weapon::Flamer_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 176);
        animations_.walkShootAnimations[Weapon::LongRange_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 184);
    } else {
        // NOTE: peds other then agents have pistol like animations for
        // all weapons
        animations_.standAnimations[Weapon::EnergyShield_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId, fs_eng::kAnimationModeLoop);
        animations_.standAnimations[Weapon::Pistol_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        animations_.standAnimations[Weapon::Uzi_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        animations_.standAnimations[Weapon::Shotgun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        animations_.standAnimations[Weapon::Gauss_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        animations_.standAnimations[Weapon::Minigun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        animations_.standAnimations[Weapon::Laser_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        animations_.standAnimations[Weapon::Flamer_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24, fs_eng::kAnimationModeLoop);
        animations_.standAnimations[Weapon::LongRange_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24);
        
        animations_.walkAnimations[Weapon::Unarmed_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 8, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::EnergyShield_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 24, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Pistol_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Uzi_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Shotgun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Gauss_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Minigun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Laser_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::Flamer_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);
        animations_.walkAnimations[Weapon::LongRange_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 64, fs_eng::kAnimationModeLoop);

        animations_.standShootAnimations[Weapon::Pistol_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.standShootAnimations[Weapon::Uzi_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104, fs_eng::kAnimationModeLoop);
        animations_.standShootAnimations[Weapon::Shotgun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.standShootAnimations[Weapon::Gauss_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.standShootAnimations[Weapon::Minigun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104, fs_eng::kAnimationModeLoop);
        animations_.standShootAnimations[Weapon::Laser_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.standShootAnimations[Weapon::Flamer_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104, fs_eng::kAnimationModeLoop);
        animations_.standShootAnimations[Weapon::LongRange_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        
        animations_.walkShootAnimations[Weapon::Pistol_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.walkShootAnimations[Weapon::Uzi_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.walkShootAnimations[Weapon::Shotgun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.walkShootAnimations[Weapon::Gauss_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.walkShootAnimations[Weapon::Minigun_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.walkShootAnimations[Weapon::Laser_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.walkShootAnimations[Weapon::Flamer_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
        animations_.walkShootAnimations[Weapon::LongRange_Anim]  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 104);
    }

    animations_.pickAnimation  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 192);
    animations_.hitAnimation  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 193,
                                            fs_eng::kAnimationModeSingle, 6);
    animations_.vaporizeAnimation  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 197,
                                            fs_eng::kAnimationModeSingle, 6);
    animations_.dyingAnimation  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 205);
    animations_.deadAgentAnimation  = 
            animationPlayer_->addAnimation(205, fs_eng::kAnimationModeSingle, 2);
    animations_.deadAnimation  = 
            animationPlayer_->addAnimation(baseSpriteAnimationId + 206,
                                            fs_eng::kAnimationModeSingle, 2);
    animations_.walkBurnAnimation  =  animationPlayer_->addAnimation(209, fs_eng::kAnimationModeLoop);
    animations_.dyingBurnAnimation  = 
            animationPlayer_->addAnimation(210, fs_eng::kAnimationModeSingle, 6);
    animations_.smokeBurnAnimation  = 
            animationPlayer_->addAnimation(211, fs_eng::kAnimationModeLoop, 2, 7000);
    animations_.deadBurnAnimation  = 
            animationPlayer_->addAnimation(828, fs_eng::kAnimationModeSingle, 2);

    animations_.persuadedAnimation = animationPlayer_->addAnimation(236);

    /*
    TODO : should we use these animations?
    pedanim->setSinkAnim(201 + baseAnim);
    // 203 - die, 204 - dead, agent only
    pedanim->setDieAgentAnim(203 + baseAnim);
    // when this burning should be used?
    pedanim->setStandBurnAnim(208);
    */
}

void PedInstance::doUpdateState(uint32_t elapsed) {
    Mission *mission = g_missionCtrl.mission();
    // Execute current behaviour
    behaviour_.execute(elapsed, mission);

    // Execute any active action
    executeAction(elapsed, mission);

    // cannot shoot if ped is doing something exlusive
    if (currentAction_ == NULL || !currentAction_->isExclusive()) {
        executeUseWeaponAction(elapsed, mission);
    }
}

void PedInstance::handleAnimationEnded() {
    // An action was waiting for the animation to finish
    if (currentAction_ && currentAction_->isWaitingForAnimation()) {
        // so continue action
        currentAction_->setRunning();
    }
    if (pUseWeaponAction_ && pUseWeaponAction_->isWaitingForAnimation()) {
        // so continue action
        pUseWeaponAction_->setRunning();
    }
}

void PedInstance::handleSelectedWeaponHasNoAmmo() {
    // when weapon is empty persuaded will drop weapon
    if (isPersuaded()) {
        // we should be able to suspend as by default it should be a follow action
        currentAction_->suspend(this);
        PutdownWeaponAction *pDrop = new PutdownWeaponAction(0);
        pDrop->setWarnBehaviour(true);
        //
        pDrop->link(currentAction_);
        currentAction_ = pDrop;
    } else {
        // first deselect empty weapon
        WeaponInstance *pDeselectedWeapon = deselectWeapon();
        // selection was a shooting weapon so replace with the same type
        // or something else
        if (pDeselectedWeapon->canShoot()) {
            selectShootingWeaponWithSameTypeFirst(pDeselectedWeapon);
        } else if (pDeselectedWeapon->isInstanceOf(Weapon::EnergyShield)) {
            // Use another energy shield
            selectMedikitOrShield(Weapon::EnergyShield);
        } else if (pDeselectedWeapon->isInstanceOf(Weapon::MediKit)) {
            if (pSelectedWeaponBeforeMedikit_ != NULL) {
                selectWeapon(pSelectedWeaponBeforeMedikit_);
                pSelectedWeaponBeforeMedikit_ = NULL;
            }
        }
    }
}

/*!
 * Return true if :
 * - is not doing something that prevents him from using weapon
 * - is not already using a weapon
 * - weapon is usable (ie a shooting weapon or a medikit)
 */
bool PedInstance::canAddUseWeaponAction(WeaponInstance *pWeapon) {
    if (currentAction_ != NULL && currentAction_->isExclusive()) {
        return false;
    }

    if (pUseWeaponAction_ != NULL) {
        return false;
    }

    WeaponInstance *pWi = pWeapon != NULL ? pWeapon : selectedWeapon();
    return pWi != NULL && pWi->usesAmmo() && pWi->ammoRemaining() > 0;
}

/*!
 * Terminate the current action of using weapon.
 */
void PedInstance::stopUsingWeapon() {
    if (isUsingWeapon()) {
        pUseWeaponAction_->stop();
    }
}

/*!
 * Terminate the current action of using weapon that shoots.
 */
void PedInstance::stopShooting() {
    if (isUsingWeapon()&& pUseWeaponAction_->type() == Action::kActTypeShoot) {
        // stop shooting in case of automatic shooting
        pUseWeaponAction_->stop();
    }
}

/*!
 * Update the ped's shooting target.
 * \param aimedPt New target position
 */
void PedInstance::updateShootingTarget(const WorldPoint &aimedPt) {
    if (pUseWeaponAction_->type() == Action::kActTypeShoot) {
        ShootAction *pShoot = dynamic_cast<ShootAction *>(pUseWeaponAction_);
        pShoot->setAimedAt(aimedPt);
    }
}

/*!
 * Returns the mean time between two shoots.
 * When a ped has shot, it takes time to shoot again : time to reload
 * the weapon + ped's reactivity time (influenced by IPA and Mods)
 * \param pWeapon The weapon used to shoot
 * \return Time to wait
 */
int PedInstance::getTimeBetweenShoots(WeaponInstance *pWeapon) {
    // TODO : Add IPA and mods influence
    return kDefaultShootReactionTime +
            pWeapon->getClass()->reloadTime();
}

/*!
 * Forces an agent to commit suicide.
 * If he's equiped with the good version of Mod Chest, he will
 * explode causing damage on nearby Peds and all his weapons will
 * be destroyed.
 * Else he dies alone leaving his weapons on the ground.
 */
void PedInstance::commitSuicide() {
    if (hasMinimumVersionOfMod(Mod::MOD_CHEST, Mod::MOD_V2)) {
        // Having a chest v2 makes agent explode
        Explosion::createExplosion(g_missionCtrl.mission(), this, 512.0, 16);
    } else {
        // else he just shoot himself
        DamageToInflict dit;
        dit.dtype = kDmgTypeBullet;
        dit.d_owner = this;
        // force damage value to agent health so he's killed at once
        dit.dvalue = PedInstance::kAgentMaxHealth;

        handleHit(dit);
    }
}

void PedInstance::setEnergyActivated(bool isActivated) {
    if (isActivated) {
        fs_utl::setBitsWithMask(&desc_state_, pd_smShieldProtected);
    } else {
        desc_state_ &= pd_smAll ^ pd_smShieldProtected;
    }
}

bool isOnScreen(int scrollX, int scrollY, int x, int y) {
    return x >= scrollX && y >= scrollY
            && x < scrollX + fs_eng::kScreenWidth - 129
            && y < scrollY + fs_eng::kScreenHeight;
}

bool getOnScreen(int scrollX, int scrollY, Point2D &scPt, const Point2D &tScPt) {
    bool off = false;

    // get x, y on screen
    while (!isOnScreen(scrollX, scrollY, scPt.x, scPt.y)) {
        if (abs(tScPt.x - scPt.x) != 0)
            scPt.x += (tScPt.x - scPt.x) / abs(tScPt.x - scPt.x);

        if (abs(tScPt.y - scPt.y) != 0)
            scPt.y += (tScPt.y - scPt.y) / abs(tScPt.y - scPt.y);

        off = true;
    }

    return off;
}

void PedInstance::showPath(int scrollX, int scrollY, fs_eng::FSColor color) {
    Point2D pedScPt;
    pMap_->tileToScreenPoint(pos_, &pedScPt);
    pedScPt.y = pedScPt.y - pos_.tz * fs_eng::Tile::kTileHeight/3 + fs_eng::Tile::kTileHeight/3;

    for (std::list<TilePoint>::iterator it = dest_path_.begin();
            it != dest_path_.end(); ++it) {
        TilePoint & d = *it;
        Point2D pathSp;
        pMap_->tileToScreenPoint(d, &pathSp);
        pathSp.y = pathSp.y - d.tz * fs_eng::Tile::kTileHeight/3 + fs_eng::Tile::kTileHeight/3;

        int ox = pathSp.x;
        int oy = pathSp.y;
        if (isOnScreen(scrollX, scrollY, pathSp.x, pathSp.y))
            getOnScreen(scrollX, scrollY, pedScPt, pathSp);
        else if (isOnScreen(scrollX, scrollY, pedScPt.x, pedScPt.y))
            getOnScreen(scrollX, scrollY, pathSp, pedScPt);
        else {
            pedScPt.x = pathSp.x;
            pedScPt.y = pathSp.y;
            continue;
        }

        g_System.drawLine(pedScPt.add(-scrollX + 129, - scrollY),
                            pathSp.add(- scrollX + 129, - scrollY), color);
        g_System.drawLine(pedScPt.add(-scrollX + 129 -1, - scrollY),
                            pathSp.add(- scrollX + 129 -1, - scrollY), color);
        /* g_System.drawLine(pedScPt.x - scrollX + 129 - 1, pedScPt.y - scrollY,
                pathSp.x - scrollX + 129 - 1, pathSp.y - scrollY, cl); */
        g_System.drawLine(pedScPt.add(-scrollX + 129, - scrollY- 1),
                            pathSp.add(- scrollX + 129, - scrollY -1), color);
        /* g_Sg_System.drawLine(pedScPt.x - scrollX + 129, pedScPt.y - scrollY - 1,
                pathSp.x - scrollX + 129, pathSp.y - scrollY - 1, cl); */
        g_System.drawLine(pedScPt.add(-scrollX + 129 - 1, - scrollY- 1),
                            pathSp.add(- scrollX + 129 - 1, - scrollY -1), color);
        /* g_System.drawLine(pedScPt.x - scrollX + 129 - 1, pedScPt.y - scrollY - 1,
                pathSp.x - scrollX + 129 - 1, pathSp.y - scrollY - 1, cl); */

        pedScPt.x = ox;
        pedScPt.y = oy;
    }
}

/*!
 * Play a stand or walk animation whether the ped is moving or not and depending
 * on the selected weapon. 
 */
void PedInstance::playStandOrWalkAnimation() {
    Weapon::WeaponAnimIndex weapon_idx =
        selectedWeapon() ? selectedWeapon()->index() : Weapon::Unarmed_Anim;
    if (isMoving()) {
        animationPlayer_->play(animations_.walkAnimations[weapon_idx]);
    } else {
        animationPlayer_->play(animations_.standAnimations[weapon_idx]);
    }
}

void PedInstance::playStandAndShootAnimation() {
    Weapon::WeaponAnimIndex weapon_idx =
        selectedWeapon() ? selectedWeapon()->index() : Weapon::Unarmed_Anim;
    animationPlayer_->play(animations_.standShootAnimations[weapon_idx]);
}

void PedInstance::playWalkAndShootAnimation() {
    Weapon::WeaponAnimIndex weapon_idx =
        selectedWeapon() ? selectedWeapon()->index() : Weapon::Unarmed_Anim;
    animationPlayer_->play(animations_.walkShootAnimations[weapon_idx]);
}

void PedInstance::playPickupOrDropAnimation() {
    animationPlayer_->play(animations_.pickAnimation);
}

void PedInstance::playHitAnimation() {
    animationPlayer_->play(animations_.hitAnimation);
}

void PedInstance::playDyingAnimation() {
    animationPlayer_->play(animations_.dyingAnimation);
}

void PedInstance::playDeadAnimation() {
    animationPlayer_->play(animations_.deadAnimation);
}

void PedInstance::playVaporizeAnimation() {
    animationPlayer_->play(animations_.vaporizeAnimation);
}

void PedInstance::playDeadAgentAnimation() {
    animationPlayer_->play(animations_.deadAgentAnimation);
}

void PedInstance::playWalkBurnAnimation() {
    animationPlayer_->play(animations_.walkBurnAnimation);
}

void PedInstance::playDyingBurnAnimation() {
    animationPlayer_->play(animations_.dyingBurnAnimation);
}

void PedInstance::playSmokeBurnAnimation() {
    animationPlayer_->play(animations_.smokeBurnAnimation);
}

void PedInstance::playDeadBurnAnimation() {
    animationPlayer_->play(animations_.deadBurnAnimation);
}

void PedInstance::playPersuadedAnimation() {
    animationPlayer_->play(animations_.persuadedAnimation);
}

void PedInstance::draw(const Point2D &screenPos) {

    // ensure on map
    if (screenPos.x < 110 || screenPos.y < 0 || !isDrawable())
        return;

    Weapon::WeaponAnimIndex weapon_idx =
        selectedWeapon() ? selectedWeapon()->index() : Weapon::Unarmed_Anim;
    Point2D posWithOffs = addOffs(screenPos);

    if (animationPlayer_->isCurrentAnimation(animations_.standAnimations[weapon_idx])) {
        animationPlayer_->draw(posWithOffs, getDiscreteDirection());
    } else if (animationPlayer_->isCurrentAnimation(animations_.walkAnimations[weapon_idx])) {
        animationPlayer_->draw(posWithOffs, getDiscreteDirection());
    } else if (animationPlayer_->isCurrentAnimation(animations_.standShootAnimations[weapon_idx])) {
        animationPlayer_->draw(posWithOffs, getDiscreteDirection());
    } else if (animationPlayer_->isCurrentAnimation(animations_.walkShootAnimations[weapon_idx])) {
        animationPlayer_->draw(posWithOffs, getDiscreteDirection());
    } else if (animationPlayer_->isCurrentAnimation(animations_.hitAnimation)) {
        // There are only 4 animations for hit
        animationPlayer_->draw(posWithOffs, getDiscreteDirection(4));
    } else if (animationPlayer_->isCurrentAnimation(animations_.vaporizeAnimation)) {
        // There are only 4 animations for vaporized
        animationPlayer_->draw(posWithOffs, getDiscreteDirection(4));
    } else {
        animationPlayer_->draw(posWithOffs, 0);
    }
}

void PedInstance::drawSelectorAnim(const Point2D &screenPos) {

    Weapon::WeaponAnimIndex weapon_idx =
        selectedWeapon() ? selectedWeapon()->index() : Weapon::Unarmed_Anim;

    if (animationPlayer_->isCurrentAnimation(animations_.standAnimations[weapon_idx])) {
        animationPlayer_->draw(screenPos, getDiscreteDirection());
    } else if (animationPlayer_->isCurrentAnimation(animations_.walkAnimations[weapon_idx])) {
        animationPlayer_->draw(screenPos, getDiscreteDirection());
    } else if (animationPlayer_->isCurrentAnimation(animations_.standShootAnimations[weapon_idx])) {
        animationPlayer_->draw(screenPos, getDiscreteDirection());
    } else if (animationPlayer_->isCurrentAnimation(animations_.walkShootAnimations[weapon_idx])) {
        animationPlayer_->draw(screenPos, getDiscreteDirection());
    } else if (animationPlayer_->isCurrentAnimation(animations_.hitAnimation)) {
        // There are only 4 animations for hit
        animationPlayer_->draw(screenPos, getDiscreteDirection(4));
    } else if (animationPlayer_->isCurrentAnimation(animations_.vaporizeAnimation)) {
        // There are only 4 animations for vaporized
        animationPlayer_->draw(screenPos, getDiscreteDirection(4));
    } else {
        animationPlayer_->draw(screenPos, 0);
    }
}

bool PedInstance::inSightRange(MapObject *t) {

    return this->isCloseTo(t, sight_range_);
}

/*!
 * Called before a weapon is selected to check if weapon can be selected.
 * \param wi The weapon to select
 */
bool PedInstance::canSelectWeapon(WeaponInstance *pNewWeapon) {
    if (pNewWeapon->isInstanceOf(Weapon::MediKit)) {
        // we cas use medikit only if ped is hurt
        return health() != startHealth() &&
            canAddUseWeaponAction(pNewWeapon);
    } else if (pNewWeapon->isInstanceOf(Weapon::EnergyShield)) {
        return canAddUseWeaponAction(pNewWeapon);
    }

    return WeaponHolder::canSelectWeapon(pNewWeapon);
}

/*!
 * Called when a weapon has been deselected.
 * \param wi The deselected weapon
 */
void PedInstance::handleWeaponDeselected(WeaponInstance * wi) {
    if (wi->isInstanceOf(Weapon::AccessCard)) {
        rmEmulatedGroupDef(4, og_dmPolice);
    }

    if (wi->isInstanceOf(Weapon::Persuadatron)) {
        behaviour_.handleBehaviourEvent(Behaviour::kBehvEvtPersuadotronDeactivated);
    } else if (wi->canShoot() && (type_ != kPedTypePolice || isPersuaded())) {
        // don't warn if ped is police to limit calls
        EventManager::fire<ShootingWeaponSelectedEvent>(this, false);
    }
    // Update animation
    playStandOrWalkAnimation();
}

/*!
 * Called when a weapon has been selected.
 * \param wi The selected weapon
 * \param previousWeapon The previous selected weapon (can be null if no weapon was selected)
 */
void PedInstance::handleWeaponSelected(WeaponInstance * wi, WeaponInstance * previousWeapon) {
    switch(wi->getClass()->getType()) {
    case Weapon::EnergyShield:
        addActionUseEnergyShield(wi);
        playStandOrWalkAnimation();
        break;
    case Weapon::AccessCard:
        addEmulatedGroupDef(4, og_dmPolice);
        break;
    case Weapon::MediKit:
        if (previousWeapon != NULL &&
            previousWeapon->getClass()->canShoot() &&
            previousWeapon->ammoRemaining() > 0) {
            pSelectedWeaponBeforeMedikit_ = previousWeapon;
        }
        addActionUseMedikit(wi);
        break;
    case Weapon::Persuadatron:
        behaviour_.handleBehaviourEvent(Behaviour::kBehvEvtPersuadotronActivated);
        break;
    default:
        if (wi->canShoot()) {
            playStandOrWalkAnimation();
        }
        break;
    }

    if (type_ != kPedTypePolice || isPersuaded()) {
        if ((previousWeapon == NULL || !previousWeapon->canShoot()) && selectedWeapon()->canShoot()) {
            // alert if the ped shows a shooting weapon and either was not holding anything or a non shooting weapon
            EventManager::fire<ShootingWeaponSelectedEvent>(this, true);
        } else if (previousWeapon != NULL && previousWeapon->canShoot() && !selectedWeapon()->canShoot()) {
            // or alert if ped go from a shooting weapon to a no shooting weapon like the persuadotron
            EventManager::fire<ShootingWeaponSelectedEvent>(this, false);
        }
    }
}

/*!
 * Drops the weapon at given index on the ground.
 * \param index Index of weapon in the agent inventory.
 * \return the instance of dropped weapon
 */
WeaponInstance * PedInstance::dropWeapon(uint8_t index) {
    if (isWeaponSelected(weapon(index))) {
        stopUsingWeapon();
    }
    WeaponInstance *pWeapon = removeWeaponAtIndex(index);

    if(pWeapon) {
        pWeapon->setDrawable(true);
        pWeapon->setPosition(pos_);
        g_missionCtrl.mission()->addWeaponToGround(pWeapon);
    }

    return pWeapon;
}

/*!
 * Drop all the ped's weapons on the ground around him.
 * \return void
 *
 */
void PedInstance::dropAllWeapons() {
    uint8_t twd = g_missionCtrl.mission()->mtsurfaces_[pos_.tx + g_missionCtrl.mission()->mmax_x_ * pos_.ty
        + g_missionCtrl.mission()->mmax_m_xy * pos_.tz];

    while (weapons_.size()) {
        WeaponInstance *w = dropWeapon(0);

        // randomizing location for drop
        int ox = rand() % 256;
        int oy = rand() % 256;
        w->setPosition(pos_.tx, pos_.ty, pos_.tz, ox, oy);
        w->offzOnStairs(twd);
    }
}

bool PedInstance::wePickupWeapon() {
    return (state_ & pa_smPickUp) != 0;
}

Vehicle *PedInstance::inVehicle() const {
    return in_vehicle_;
}

void PedInstance::putInVehicle(Vehicle * pVehicle)
{
    setDrawable(false);
    in_vehicle_ = pVehicle;
    switchActionStateTo(PedInstance::pa_smInCar);
}

void PedInstance::leaveVehicle() {
    setDrawable(true);
    setPosition(in_vehicle_->position());
    in_vehicle_ = NULL;
    switchActionStateFrom(state_ & PedInstance::pa_smInCar);
}

/*!
 * Return the damage after applying reduction of Mod protection.
 * @param damage Damage description
 * @return True if Ped has died due to damage received
 */
bool PedInstance::takeDamage(DamageToInflict &damage) {
    // TODO : reduce damage based on Mod protection
    decreaseHealth(damage.dvalue);
    return isDead();
}

/*!
 * Method called when object is hit by a weapon shot.
 * \param d Damage description
 */
void PedInstance::handleHit(DamageToInflict &d) {
    if (canTakeAction(fs_knl::Action::kActTypeHit)) {
        insertHitAction(d);

        // Alert behaviour
        behaviour_.handleBehaviourEvent(Behaviour::kBehvEvtHit);
    }
}


/*!
 * This method do all things when a ped dies
 * @param damage The damage that caused the death 
 */
void PedInstance::handleDeath(const DamageToInflict &damage) {
    clearDestination();
    switchActionStateTo(PedInstance::pa_smDead);

    switch (damage.dtype) {
        case kDmgTypeBullet:
            //setDrawnAnim(PedInstance::ad_DieAnim);
            dropAllWeapons();
            break;
        /*case kDmgTypeLaser:
            if (is_our_) {
                setDrawnAnim(PedInstance::ad_DeadAgentAnim);
            } else {
                setDrawnAnim(PedInstance::ad_NoAnimation);
            }
            break;*/
        case kDmgTypeExplosion:
        case kDmgTypeBurn:
            if (hasMinimumVersionOfMod(Mod::MOD_CHEST, Mod::MOD_V2) &&
                damage.d_owner != this) {
                //setDrawnAnim(PedInstance::ad_DieAnim);
                dropAllWeapons();
            } else {
                // was burning because not enough protected or suicide
                // so die burning
                //setDrawnAnim(PedInstance::ad_DieBurnAnim);
            }
            break;
        default:
            FSERR(Log::k_FLG_GAME, "PedInstance", "handleDeath", ("Unhandled damage type: %d\n", damage.dtype))
    }

    updatePersuadedRelations(g_missionCtrl.mission()->getSquad());

    // send an event to alert agent died
    if (isOurAgent()) {
        EventManager::fire<AgentDiedEvent>(this);
    }
}

void PedInstance::addEnemyGroupDef(uint32_t eg_id, uint32_t eg_def) {
    enemy_group_defs_.add(eg_id, eg_def);
}

void PedInstance::rmEnemyGroupDef(uint32_t eg_id, uint32_t eg_def) {
    enemy_group_defs_.rm(eg_id, eg_def);
}

bool PedInstance::isInEnemyGroupDef(uint32_t eg_id, uint32_t eg_def) {
    return enemy_group_defs_.isIn(eg_id, eg_def);
}

void PedInstance::addEmulatedGroupDef(uint32_t eg_id, uint32_t eg_def) {
    emulated_group_defs_.add(eg_id, eg_def);
}
void PedInstance::rmEmulatedGroupDef(uint32_t eg_id, uint32_t eg_def) {
    emulated_group_defs_.rm(eg_id, eg_def);
}

bool PedInstance::isInEmulatedGroupDef(uint32_t eg_id, uint32_t eg_def) {
    return emulated_group_defs_.isIn(eg_id, eg_def);
}

bool PedInstance::isInEmulatedGroupDef(PedInstance::Mmuu32_t &r_egd,
        bool id_only)
{
    if (id_only) {
        return emulated_group_defs_.isIn_KeyOnly(r_egd);
    }
    return emulated_group_defs_.isIn_All(r_egd);
}

/*!
 * Returns true if the given object is considered hostile by this Ped.
 * If object is a Vehicle, check if it contains hostiles inside.
 * If it is another Ped, check if he is a friend or in a opposite group
 * \param obj The object whom hostility is being evaluated
 * \param hostile_desc_alt
 * \return true if object is considered hostile
 */
bool PedInstance::isHostileTo(ShootableMapObject *obj,
    unsigned int hostile_desc_alt)
{
    bool isHostile = false;

    if (obj->nature() == MapObject::kNatureVehicle) {
        Vehicle *pVehicle = static_cast<Vehicle *>(obj);
        isHostile = pVehicle->containsHostilesForPed(
            this, hostile_desc_alt);
    } else if (obj->nature() == MapObject::kNaturePed) {
        PedInstance *pPed = static_cast<PedInstance *>(obj);
        if (!isFriendWith(pPed)) {
            // Ped is not a declared friend, check its group
            if ((pPed)->emulatedGroupDefsEmpty()) {
                isHostile =
                    isInEnemyGroupDef(pPed->objGroupID(), pPed->objGroupDef());
            } else {
                isHostile = pPed->isInEmulatedGroupDef(enemy_group_defs_);
            }
            if (!isHostile) {
                if (hostile_desc_alt == PedInstance::pd_smUndefined)
                    hostile_desc_alt = hostile_desc_;
                isHostile = (pPed->descStateMasks() & hostile_desc_alt) != 0;
            }
        }
    }

    return isHostile;
}

/*!
 * Friend can be neutral to be sure that object is hostile use
 * isHostileTo and check hostiles_found_(isInHostilesFound)
 * \param p
 * \return True if other ped is considered a friend.
 */
bool PedInstance::isFriendWith(PedInstance *p) {
    // Search ped in friends
    if (friends_found_.find(p) != friends_found_.end())
        return true;
    if (p->isInEmulatedGroupDef(obj_group_id_, obj_group_def_)) {
        if (obj_group_def_ == og_dmPolice
            && !isPersuaded())
        {
            friends_not_seen_.insert(p);
        }
        return true;
    }
    if (friend_group_defs_.find(p->objGroupDef()) != friend_group_defs_.end())
        return true;
    return (p->objGroupID() == obj_group_id_);
}

void PedInstance::verifyHostilesFound(Mission *m) {
    std::vector <ShootableMapObject *> rm_set;
    WorldPoint cur_xyz(pos_);
    int check_rng = sight_range_;

    WeaponInstance *wi = selectedWeapon();
    if (wi && wi->canShoot() && wi->range() > check_rng)
        check_rng = wi->range();

    // removing destroyed, friends, objects out of shot/sight range
    for (Msmod_t::iterator it = hostiles_found_.begin();
        it != hostiles_found_.end(); ++it)
    {
        ShootableMapObject *smo = it->first;
        double distTo = 0;
        if (smo->isDead() || (smo->nature() == MapObject::kNaturePed
            && isFriendWith((PedInstance *)(smo)))
            || (smo->nature() == MapObject::kNatureVehicle
            && ((Vehicle *)smo)->containsHostilesForPed(this, hostile_desc_))
            || (m->checkIfBlockersInShootingLine(cur_xyz, &smo, NULL, false, false,
            check_rng, &distTo) != 1))
        {
            rm_set.push_back(smo);
        }
    }
    while (!rm_set.empty()) {
        hostiles_found_.erase(hostiles_found_.find(rm_set.back()));
        rm_set.pop_back();
    }
}

/*!
 * Initiate the amount, depend and effect level for a given type of IPA.
 * Levels passed to this method should come from original values and will be transformed into pourcentage.
 * @param type The type of IPA to init for this ped
 * @param amount The amount level
 * @param depend The depend level
 * @param effect The effect level
 */
void PedInstance::initAllLevelsForIPAType(IPAStim::IPAType type, uint8_t amount, uint8_t depend, uint8_t effect) {
    switch (type)
    {
    case IPAStim::IPAType::Adrenaline:
        adrenaline_.setLevels256(amount, depend, effect);
        break;
    case IPAStim::IPAType::Perception:
        perception_.setLevels256(amount, depend, effect);
        break;
    case IPAStim::IPAType::Intelligence:
        intelligence_.setLevels256(amount, depend, effect);
        break;
    default:
        break;
    }
}


/*!
 * Set amount for given IPA.
 * @param ipaType The type of IPA
 * @param percentage The amount expressed as a percentage
 */
void PedInstance::setIPAAmount(IPAStim::IPAType ipaType, uint8_t percentage) {
    switch(ipaType) {
        case IPAStim::Adrenaline:
            adrenaline_.setAmount(percentage);
            break;
        case IPAStim::Perception:
            perception_.setAmount(percentage);
            break;
        case IPAStim::Intelligence:
            intelligence_.setAmount(percentage);
            break;
    }
}

/*!
 * Movement speed calculated from base speed, mods, weight of inventory,
 * ipa, etc.
 */
int PedInstance::applySpeedModifier(int speed) {
    if (isInPanic()) {
        return 256;
    }
    
    float speed_new = static_cast<float>(speed) * getSpeedMultiplier();

    int weight_max = getMaxWeight();
    int weight_inv = getInventoryWeight();

    if (weight_inv > weight_max) {
        if ((weight_inv / weight_max) > 1)
            speed_new = kAgentMaxSpeedWithOverweight;
        else
            speed_new /= 2;
    }

    if (obj_group_def_ == PedInstance::og_dmAgent)
    {
        // See the comments in the IPAStim class for details on the multiplier
        // algorithm for adrenaline
        speed_new *= adrenaline_.getMultiplier();
    }

    if (isPersuaded()) {
        speed_new *= owner_->getSpeedOwnerBoost();
        //speed_new >>= 1;
    }

    return static_cast<int>(speed_new);
}

// NOTE: returned value is *2, it should be should be corrected
// during calculations with /2
int PedInstance::getSpeedOwnerBoost()
{
    if (obj_group_def_ == PedInstance::og_dmAgent)
    {
        float ipa_adr = adrenaline_.getMultiplier();
        if (ipa_adr > 1.0)
            return 4;
        else if (ipa_adr < 1.0)
            return 1;
    }

    return 2;
}

/*!
 * Adds a little imprecision to the aimed point. Precision depends
 * on the weapon used, the ped's mods and IPA levels.
 * \param pWeaponClass The type of weapon used to shoot
 * \param aimedPt Where the player has clicked on the map. This point
 * will be updated to reflect the influence of precision.
 */
void PedInstance::adjustAimedPtWithRangeAndAccuracy(Weapon *pWeaponClass, WorldPoint *pAimedLocW) {
    // 1- Adjust Range
    WorldPoint originLocW(pos_);
    if (originLocW.z > (pMap_->maxZ() - 1) * 128)
        return;

    if (pAimedLocW->z > (pMap_->maxZ() - 1) * 128)
        return;

    double d = distanceToPosition(*pAimedLocW);

    if (d == 0)
        return;

    double maxr = (double) pWeaponClass->range();
    if (d >= maxr) {
        // weapon's range is less than the distance to aimed point
        // so compute new aimed point that is clipped by the range
        double dist_k = maxr / d;
        pAimedLocW->x = originLocW.x + (int)((pAimedLocW->x - originLocW.x) * dist_k);
        pAimedLocW->y = originLocW.y + (int)((pAimedLocW->y - originLocW.y) * dist_k);
        pAimedLocW->z = originLocW.z + (int)((pAimedLocW->z - originLocW.z) * dist_k);
    }

    // 2- Adjust Accuracy
    // TODO Add imprecision and accuracy
    //double accuracy = pWeaponClass->shotAcurracy();
}

void PedInstance::getAccuracy(double &base_acc)
{
    double base_mod = base_mod_acc_;

    if (obj_group_def_ == PedInstance::og_dmAgent)
    {
        Mod *pMod = slots_[Mod::MOD_EYES];
        if (pMod) {
            base_mod += 0.006 * (pMod->getVersion() + 1);
        }
        pMod = slots_[Mod::MOD_BRAIN];
       if (pMod) {
            base_mod += 0.006 * (pMod->getVersion() + 1);
        }
        pMod = slots_[Mod::MOD_ARMS];
        if (pMod) {
            base_mod += 0.006 * (pMod->getVersion() + 1);
        }
        pMod = slots_[Mod::MOD_HEART];
        if (pMod) {
            base_mod += 0.006 * (pMod->getVersion() + 1);
        }
        pMod = slots_[Mod::MOD_LEGS];
        if (pMod) {
            base_mod += 0.006 * (pMod->getVersion() + 1);
        }
        // 0.59 max from here

        base_mod -= 0.4 * (2.0 - perception_.getMultiplier());
        base_mod += 0.4 * (2.0 - adrenaline_.getMultiplier());
        // 0.99 max after adrenaline
    }

    // NOTE :(1.0 - base_acc) is randomized and not dependent on anything
    // should be added back
    // Ex. weapon accuracy 70%, random value 30%, then
    // 0.7 * (1.0 - base_mod(here 0.5) + (1.0 - 0.7) = 0.65
    // the shot will be randomized at 65% of weapons max angle
    base_acc = base_acc * (1.0 - base_mod) + (1.0 - base_acc);
}

bool PedInstance::hasAccessCard()
{
    WeaponInstance * wi = selectedWeapon();
    Mod *pMod = slots_[Mod::MOD_BRAIN];
    return wi && pMod && wi->isInstanceOf(Weapon::AccessCard)
        && pMod->getVersion() == Mod::MOD_V3 ? true : false;
}

/*!
 * Returns the number of points an agent must have to persuade
 * a ped of given type. Civilians or criminals are always persuaded.
 * \param aType The type of the ped to persuade.
 */
uint16_t PedInstance::getRequiredPointsToPersuade(PedType aType) {
    Mod *pMod = slots_[Mod::MOD_BRAIN];
    uint16_t points = 0;
    if (aType == kPedTypeGuard) {
        if (!pMod) {
            points = 4;
        } else if (pMod->getVersion() == Mod::MOD_V1) {
            points = 2;
        } else {
            points = 1;
        }
    } else if (aType == kPedTypePolice) {
        if (!pMod) {
            points = 8;
        } else if (pMod->getVersion() == Mod::MOD_V1) {
            points = 4;
        } else if (pMod->getVersion() == Mod::MOD_V2) {
            points = 3;
        } else if (pMod->getVersion() == Mod::MOD_V3) {
            points = 2;
        }
    } else if (aType == kPedTypeAgent) {
        if (!pMod) {
            points = 32;
        } else if (pMod->getVersion() == Mod::MOD_V1) {
            points = 16;
        } else if (pMod->getVersion() == Mod::MOD_V2) {
            points = 11;
        } else if (pMod->getVersion() == Mod::MOD_V3) {
            points = 8;
        }
    }

    return points;
}

/*!
 * Return true if this agent can persuade the given ped.
 * A ped can be persuaded if he's not already persuaded or
 * if its persuasion points are less or equal than the agent
 * total persuasion points.
 * \param pOtherPed Ped to persuade.
 * \param persuadotronRange Distance under which a ped can be persuaded
 */
bool PedInstance::canPersuade(PedInstance *pOtherPed, const int persuadotronRange) {
    Action *pAction = pOtherPed->currentAction();
    if (pAction != NULL && pAction->type() == Action::kActTypeHit) {
        // cannot persuade a ped if he's currently being hit
        return false;
    }

    if (!pOtherPed->isPersuaded() && pOtherPed->isAlive() &&
            isCloseTo(pOtherPed, persuadotronRange)) {
        uint16_t points = getRequiredPointsToPersuade(pOtherPed->type());
        return points <= totalPersuasionPoints_;
    }

    return false;
}

/*!
 * Called when an agent tries to persuad this ped.
 * \param pAgent Agent trying to persuad
 */
void PedInstance::handlePersuadedBy(PedInstance *pAgent) {
    pAgent->addPersuaded(this);
    fs_utl::setBitsWithMask(&desc_state_, pd_smControlled);
    setObjGroupID(pAgent->objGroupID());
    owner_ = pAgent;
    setPanicImmuned();

    behaviour_.replaceAllcomponentsBy(new PersuadedBehaviourComponent());

    /////////////////// Check if still useful ////////////
    pAgent->cpyEnemyDefs(enemy_group_defs_);
    friends_found_.clear();
    hostiles_found_.clear();
    hostile_desc_ = pAgent->hostileDesc();
    //////////////////////////////////////////////////////
}

/*!
 * Adds given ped to the list of persuaded peds by this agent.
 * Increments the persuasion points of this ped depending on the type
 * of persuaded ped.
 * \param p Persuaded ped
 */
void PedInstance::addPersuaded(PedInstance *p) {
    persuadedSet_.insert(p);
    switch(p->type()) {
    case kPedTypeGuard:
        totalPersuasionPoints_ +=  3;
        break;
    case kPedTypePolice:
        totalPersuasionPoints_ +=  4;
        break;
    case kPedTypeAgent:
        totalPersuasionPoints_ +=  32;
        break;
    default:
        totalPersuasionPoints_ +=  1;
    }
}

void PedInstance::rmvPersuaded(PedInstance *p) {
    std::set <PedInstance *>::iterator it =  persuadedSet_.find(p);
    if (it != persuadedSet_.end())
        persuadedSet_.erase(it);
}

/*!
 * After a ped died (agent or other), updates the relation between the owner
 * and the peds he has persuaded.
 * If dead ped is a persuaded, just removed him from the list of his owner.
 * If dead ped is our agent, transfer all his persuaded to another living agent.
 * \param pSquad List of available agents
 */
void PedInstance::updatePersuadedRelations(Squad *pSquad) {
    if (isPersuaded()) {
        owner_->rmvPersuaded(this);
        owner_ = NULL;
    } else if (isOurAgent()) {
        // our agent is dead, assign all persuaded to another living agent
        for (uint8_t i = 0; i < Squad::kMaxSlot; ++i) {
            PedInstance *pAgent = pSquad->member(i);
            if (pAgent && pAgent->isAlive()) {
                while(!persuadedSet_.empty()) {
                    std::set <PedInstance *>::iterator it = persuadedSet_.begin();
                    PedInstance *pPed = *it;
                    persuadedSet_.erase(it);
                    pPed->setNewOwner(pAgent);
                }
                break;
            }
        }
    }
}

void PedInstance::informPersuadedToEnterVehicle(Vehicle *pVehicle) {
    ;
    for (std::set <PedInstance *>::iterator it = persuadedSet_.begin();
        it != persuadedSet_.end(); it++) {
            (*it)->behaviour().handleBehaviourEvent(Behaviour::kBehvEvtEnterVehicle, pVehicle);
    }
}

void PedInstance::dropPersuadedFromCar(Vehicle *pCar) {
    for (std::set <PedInstance *>::iterator it = persuadedSet_.begin();
        it != persuadedSet_.end(); it++) {
            pCar->dropPassenger(*it);
            // default action is to follow owner
            (*it)->setCurrentActionWithSource(Action::kActionDefault);
    }
}

}
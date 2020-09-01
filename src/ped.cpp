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

#include "common.h"
#include "app.h"
#include "utils/log.h"
#include "model/vehicle.h"
#include "model/squad.h"
#include "model/shot.h"
#include "mission.h"
#include "core/gamesession.h"
#include "ia/behaviour.h"

//*************************************
// Constant definition
//*************************************
const int PedInstance::kAgentMaxHealth = 16;
const int PedInstance::kDefaultShootReactionTime = 200;
const uint32 PedInstance::kPlayerGroupId = 1;

Ped::Ped() {
    memset(stand_anims_, 0, sizeof(stand_anims_));
    memset(walk_anims_, 0, sizeof(walk_anims_));
    memset(stand_fire_anims_, 0, sizeof(stand_fire_anims_));
    memset(walk_fire_anims_, 0, sizeof(walk_fire_anims_));
}

bool Ped::drawStandFrame(int x, int y, int dir, int frame,
                         Weapon::WeaponAnimIndex weapon)
{
    assert(weapon < NUM_ANIMS);
    return g_App.gameSprites().drawFrame(
            stand_anims_[weapon] + dir, frame, x, y);
}

int Ped::lastStandFrame(int dir, Weapon::WeaponAnimIndex weapon) {
    assert(weapon != 0 && weapon < NUM_ANIMS);
    return g_App.gameSprites().lastFrame(stand_anims_[weapon] + dir);
}

bool Ped::drawWalkFrame(int x, int y, int dir, int frame,
                        Weapon::WeaponAnimIndex weapon)
{
    assert(weapon < NUM_ANIMS);
    return g_App.gameSprites().drawFrame(
            walk_anims_[weapon] + dir, frame, x, y);
}

int Ped::lastWalkFrame(int dir, Weapon::WeaponAnimIndex weapon) {
    assert(weapon != 0 && weapon < NUM_ANIMS);
    return g_App.gameSprites().lastFrame(walk_anims_[weapon] + dir);
}

bool Ped::drawStandFireFrame(int x, int y, int dir, int frame,
        Weapon::WeaponAnimIndex weapon) {
    assert(weapon != 0 && weapon < NUM_ANIMS);
    return g_App.gameSprites().drawFrame(
            stand_fire_anims_[weapon] + dir, frame, x, y);
}

int Ped::lastStandFireFrame(int dir, Weapon::WeaponAnimIndex weapon)
{
    assert(weapon != 0 && weapon < NUM_ANIMS);
    return g_App.gameSprites().lastFrame(
            stand_fire_anims_[weapon] + dir);
}

bool Ped::drawWalkFireFrame(int x, int y, int dir, int frame,
        Weapon::WeaponAnimIndex weapon) {
    assert(weapon != 0 && weapon < NUM_ANIMS);
    return g_App.gameSprites().drawFrame(
            walk_fire_anims_[weapon] + dir, frame, x, y);
}

int Ped::lastWalkFireFrame(int dir, Weapon::WeaponAnimIndex weapon)
{
    assert(weapon != 0 && weapon < NUM_ANIMS);
    return g_App.gameSprites().lastFrame(walk_fire_anims_[weapon] + dir);
}

bool Ped::drawDieFrame(int x, int y, int frame) {
    return g_App.gameSprites().drawFrame(die_anim_, frame, x, y);
}

int Ped::lastDieFrame() {
    return g_App.gameSprites().lastFrame(die_anim_);
}

void Ped::drawDeadFrame(int x, int y, int frame) {
    g_App.gameSprites().drawFrame(dead_anim_, frame, x, y);
}

void Ped::drawDeadAgentFrame(int x, int y, int frame) {
    g_App.gameSprites().drawFrame(dead_agent_anim_, frame, x, y);
}

void Ped::drawHitFrame(int x, int y, int dir, int frame) {
    g_App.gameSprites().drawFrame(hit_anim_ + dir / 2, frame, x, y);
}

int Ped::lastHitFrame(int dir) {
    return g_App.gameSprites().lastFrame(hit_anim_ + dir / 2);
}

void Ped::drawPickupFrame(int x, int y, int frame) {
    g_App.gameSprites().drawFrame(pickup_anim_, frame, x, y);
}

int Ped::lastPickupFrame() {
    // same for putdown weapon
    return g_App.gameSprites().lastFrame(pickup_anim_);
}

void Ped::drawVaporizeFrame(int x, int y, int dir, int frame) {
    g_App.gameSprites().drawFrame(vaporize_anim_ + dir / 2, frame, x, y);
}

int Ped::lastVaporizeFrame(int dir) {
    return g_App.gameSprites().lastFrame(vaporize_anim_ + dir / 2);
}

void Ped::drawSinkFrame(int x, int y, int frame) {
    g_App.gameSprites().drawFrame(sink_anim_, frame, x, y);
}

int Ped::lastSinkFrame() {
    return g_App.gameSprites().lastFrame(sink_anim_);
}

void Ped::drawStandBurnFrame(int x, int y, int frame) {
    g_App.gameSprites().drawFrame(stand_burn_anim_, frame, x, y);
}

void Ped::drawWalkBurnFrame(int x, int y, int frame) {
    g_App.gameSprites().drawFrame(walk_burn_anim_, frame, x, y);
}

void Ped::drawDieBurnFrame(int x, int y, int frame) {
    g_App.gameSprites().drawFrame(die_burn_anim_, frame, x, y);
}

int Ped::lastDieBurnFrame() {
    return g_App.gameSprites().lastFrame(die_burn_anim_);
}

void Ped::drawSmokeBurnFrame(int x, int y, int frame) {
    g_App.gameSprites().drawFrame(smoke_burn_anim_, frame, x, y);
}

void Ped::drawDeadBurnFrame(int x, int y, int frame) {
    g_App.gameSprites().drawFrame(dead_burn_anim_, frame, x, y);
}

void Ped::drawPersuadeFrame(int x, int y, int frame) {
    g_App.gameSprites().drawFrame(persuade_anim_, frame, x, y);
}

int Ped::lastPersuadeFrame() {
    return g_App.gameSprites().lastFrame(persuade_anim_);
}

void PedInstance::setTypeFromValue(uint8 value) {
    switch(value) {
    case 0x01:
        type_ = kPedTypeCivilian;
        break;
    case 0x02:
        type_ = kPedTypeAgent;
        break;
    case 0x04:
        type_ = kPedTypePolice;
        break;
    case 0x08:
        type_ = kPedTypeGuard;
        break;
    case 0x10:
        type_ = kPedTypeCriminal;
        break;
    }
}

bool PedInstance::switchActionStateTo(uint32 as) {
    uint32 prevState = state_;
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
        case pa_smUnavailable:
            state_ = pa_smUnavailable;
            break;
    }

    return prevState != state_;
}

/*!
 * \return true if state has changed.
 */
bool PedInstance::switchActionStateFrom(uint32 as) {
    uint32 prevState = state_;
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
        setDrawnAnim(PedInstance::ad_NoAnimation);
    } else if ((state_ & pa_smDead) != 0) {
        //setDrawnAnim(PedInstance::ad_DeadAnim);
    } else if ((state_ & (pa_smWalking | pa_smFollowing)) != 0) {
        if ((state_ & pa_smFiring) != 0)
            setDrawnAnim(PedInstance::ad_WalkFireAnim);
        else
            setDrawnAnim(PedInstance::ad_WalkAnim);
    } else if ((state_ & pa_smWalkingBurning) != 0) {
        setDrawnAnim(PedInstance::ad_WalkBurnAnim);
    } else if ((state_ & pa_smStanding) != 0) {
        if ((state_ & pa_smFiring) != 0)
            setDrawnAnim(PedInstance::ad_StandFireAnim);
        else
            setDrawnAnim(PedInstance::ad_StandAnim);
    } else if ((state_ & pa_smPickUp) != 0) {
        setDrawnAnim(PedInstance::ad_PickupAnim);
    } else if ((state_ & pa_smPutDown) != 0) {
        setDrawnAnim(PedInstance::ad_PutdownAnim);
    } else if ((state_ & pa_smInCar) != 0) {
        setDrawnAnim(PedInstance::ad_StandAnim);
    } else if ((state_ & pa_smHit) != 0) {
        setDrawnAnim(PedInstance::ad_HitAnim);
    } else if ((state_ & pa_smHitByLaser) != 0) {
        setDrawnAnim(PedInstance::ad_VaporizeAnim);
    } else if (fs_cmn::isBitsOnWithMask(state_, pa_smHitByPersuadotron)) {
        setDrawnAnim(PedInstance::ad_PersuadedAnim);
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
void PedInstance::goToState(uint32 as) {
    if(switchActionStateTo(as)) {
        synchDrawnAnimWithActionState();
    }
}

/*!
 * Leaves the given state.
 * Update corresponding animation.
 * \param as new state
 */
void PedInstance::leaveState(uint32 as) {
    if (switchActionStateFrom(as)) {
        synchDrawnAnimWithActionState();
    }
}

/*!
 * Update the current frame.
 * \param elapsed Time since the last frame
 * \return True if animation has ended.
 */
bool PedInstance::updateAnimation(int elapsed) {
    MapObject::animate(elapsed);

    return handleDrawnAnim(elapsed);
}

/*!
 * Animates the ped (ie executes all the ped's actions).
 * If Ped has currently no action, execute behaviour
 * to determine default actions. Then executes the actions.
 * Ped can shoot while doing an action only if that action is not exclusive
 * (like dropping a weapon or entering a car).
 * Finally, update the animation. If an action is waiting for an animation
 * and that animation is finished, unlocks the action.
 * \param elapsed Time since the last frame
 * \param mission Mission data
 * \return True if something has changed (so update rendering)
 */
bool PedInstance::animate(int elapsed, Mission *mission) {
    // Execute current behaviour
    behaviour_.execute(elapsed, mission);

    // Execute any active action
    bool update = executeAction(elapsed, mission);

    // cannot shoot if ped is doing something exlusive
    if (currentAction_ == NULL || !currentAction_->isExclusive()) {
        update |= executeUseWeaponAction(elapsed, mission);
    }

    if (updateAnimation(elapsed)) {
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

    return update;
}

/*!
 * Executes the maximum number of actions.
 * \param elapsed Time since the last frame
 * \param mission Mission data
 * \return True if something has changed (to update rendering)
 */
bool PedInstance::executeAction(int elapsed, Mission *pMission) {
    bool updated = false;

    while(currentAction_ != NULL) {
        // execute action
        updated |= currentAction_->execute(elapsed, pMission, this);
        if (currentAction_->isFinished()) {
            if (health_ == 0) {
                // Ped may have died during execution of a HitAction.
                destroyAllActions(true);
                destroyUseWeaponAction();
            } else {
                bool warnBehaviour = currentAction_->warnBehaviour();
                Action::ActionType actionType = currentAction_->type();
                // current action is finished : go to next one
                MovementAction *pNext = currentAction_->next();

                if (currentAction_->source() == Action::kActionNotScripted) {
                    currentAction_->removeAndJoinChain();
                    delete currentAction_;
                    currentAction_ = NULL;
                }

                if (warnBehaviour) {
                    behaviour_.handleBehaviourEvent(Behaviour::kBehvEvtActionEnded, &actionType);
                }

                // If next action was suspended, resume it
                if (pNext != NULL && pNext->isSuspended()) {
                    pNext->resume(pMission, this);
                }

                currentAction_ = pNext;
            }
        } else if (currentAction_->type() == Action::kActTypeReset) {
            ResetScriptedAction *pReset = static_cast<ResetScriptedAction *>(currentAction_);
            Action::ActionSource source = pReset->sourceToReset();

            if (pReset->source() == Action::kActionNotScripted) {
                pReset->removeAndJoinChain();
                delete pReset;
            }
            resetActions(source);
            break;
        } else if (currentAction_->type() == Action::kActTypeReplaceCurrent) {
            ReplaceCurrentAction *pReplace = static_cast<ReplaceCurrentAction *>(currentAction_);
            MovementAction *pTarget = pReplace->targetAction();
            // delete actions
            destroyAllActions(false);
            currentAction_ = pTarget;
            break;
        } else {
            // current action is still running, so stop iterate now
            // we will continue next time
            break;
        }
    }

    return updated;
}

/*!
 * Executes a shoot action.
 */
bool PedInstance::executeUseWeaponAction(int elapsed, Mission *pMission) {
    bool updated = false;
    if(pUseWeaponAction_ != NULL) {
        // execute action
        updated |= pUseWeaponAction_->execute(elapsed, pMission, this);
        if (pUseWeaponAction_->isFinished()) {
            // erase action
            delete pUseWeaponAction_;
            pUseWeaponAction_ = NULL;

            // then select another weapon maybe
            if (selectedWeapon() && selectedWeapon()->ammoRemaining() == 0) {
                handleSelectedWeaponHasNoAmmo();
            }
        }
    }

    return updated;
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
                selectWeapon(*pSelectedWeaponBeforeMedikit_);
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
        Explosion::createExplosion(g_Session.getMission(), this, 512.0, 16);
    } else {
        // else he just shoot himself
        fs_dmg::DamageToInflict dit;
        dit.dtype = fs_dmg::kDmgTypeBullet;
        dit.d_owner = this;
        // force damage value to agent health so he's killed at once
        dit.dvalue = PedInstance::kAgentMaxHealth;

        handleHit(dit);
    }
}

void PedInstance::setEnergyActivated(bool isActivated) {
    if (isActivated) {
        fs_cmn::setBitsWithMask(&desc_state_, pd_smShieldProtected);
    } else {
        desc_state_ &= pd_smAll ^ pd_smShieldProtected;
    }
}

bool isOnScreen(int scrollX, int scrollY, int x, int y) {
    return x >= scrollX && y >= scrollY
            && x < scrollX + GAME_SCREEN_WIDTH - 129
            && y < scrollY + GAME_SCREEN_HEIGHT;
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

void PedInstance::showPath(int scrollX, int scrollY) {
    Point2D pedScPt;
    g_App.maps().map(map())->tileToScreenPoint(pos_, &pedScPt);
    pedScPt.y = pedScPt.y - pos_.tz * TILE_HEIGHT/3 + TILE_HEIGHT/3;

    for (std::list<TilePoint>::iterator it = dest_path_.begin();
            it != dest_path_.end(); ++it) {
        TilePoint & d = *it;
        Point2D pathSp;
        g_App.maps().map(map())->tileToScreenPoint(d, &pathSp);
        pathSp.y = pathSp.y - d.tz * TILE_HEIGHT/3 + TILE_HEIGHT/3;

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

        int cl = 11;
        g_Screen.drawLine(pedScPt.x - scrollX + 129, pedScPt.y - scrollY,
                pathSp.x - scrollX + 129, pathSp.y - scrollY, cl);
        g_Screen.drawLine(pedScPt.x - scrollX + 129 - 1, pedScPt.y - scrollY,
                pathSp.x - scrollX + 129 - 1, pathSp.y - scrollY, cl);
        g_Screen.drawLine(pedScPt.x - scrollX + 129, pedScPt.y - scrollY - 1,
                pathSp.x - scrollX + 129, pathSp.y - scrollY - 1, cl);
        g_Screen.drawLine(pedScPt.x - scrollX + 129 - 1, pedScPt.y - scrollY - 1,
                pathSp.x - scrollX + 129 - 1, pathSp.y - scrollY - 1, cl);

        pedScPt.x = ox;
        pedScPt.y = oy;
    }
}

PedInstance::PedInstance(Ped *ped, uint16 anId, int m, bool isOur) :
    ShootableMovableMapObject(anId, m, MapObject::kNaturePed),
    ped_(ped),
    desc_state_(PedInstance::pd_smUndefined),
    hostile_desc_(PedInstance::pd_smUndefined),
    obj_group_def_(PedInstance::og_dmUndefined),
    old_obj_group_def_(PedInstance::og_dmUndefined),
    obj_group_id_(0), old_obj_group_id_(0),
    drawn_anim_(PedInstance::ad_StandAnim),
    sight_range_(0), in_vehicle_(NULL),
    owner_(NULL)
{
    hold_on_.wayFree = 0;
    state_ = PedInstance::pa_smNone;
    is_our_ = isOur;

    adrenaline_  = new IPAStim(IPAStim::Adrenaline);
    perception_  = new IPAStim(IPAStim::Perception);
    intelligence_ = new IPAStim(IPAStim::Intelligence);

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

PedInstance::~PedInstance()
{
    delete ped_;
    ped_ = NULL;

    delete adrenaline_;
    adrenaline_ = NULL;
    delete perception_;
    perception_ = NULL;
    delete intelligence_;
    intelligence_ = NULL;

    destroyAllActions();
    destroyUseWeaponAction();
}

void PedInstance::draw(int x, int y) {

    // ensure on map
    if (x < 110 || y < 0 || map_ == -1)
        return;

    Weapon::WeaponAnimIndex weapon_idx =
        selectedWeapon() ? selectedWeapon()->index() : Weapon::Unarmed_Anim;
    addOffs(x, y);


    switch(drawnAnim()){
        case PedInstance::ad_HitAnim:
            ped_->drawHitFrame(x, y, getDirection(), frame_);
            break;
        case PedInstance::ad_DieAnim:
            ped_->drawDieFrame(x, y, frame_);
            break;
        case PedInstance::ad_DeadAnim:
            ped_->drawDeadFrame(x, y, frame_);
            break;
        case PedInstance::ad_DeadAgentAnim:
            ped_->drawDeadAgentFrame(x, y, frame_);
            break;
        case PedInstance::ad_PickupAnim:
            ped_->drawPickupFrame(x, y, frame_);
            break;
        case PedInstance::ad_PutdownAnim:
            ped_->drawPickupFrame(x, y, frame_);
            break;
        case PedInstance::ad_WalkAnim:
            ped_->drawWalkFrame(x, y, getDirection(), frame_, weapon_idx);
            break;
        case PedInstance::ad_StandAnim:
            ped_->drawStandFrame(x, y, getDirection(), frame_, weapon_idx);
            break;
        case PedInstance::ad_WalkFireAnim:
            ped_->drawWalkFireFrame(x, y, getDirection(), frame_, weapon_idx);
            break;
        case PedInstance::ad_StandFireAnim:
            ped_->drawStandFireFrame(x, y, getDirection(), frame_, weapon_idx);
            break;
        case PedInstance::ad_VaporizeAnim:
            ped_->drawVaporizeFrame(x, y, getDirection(), frame_);
            break;
        case PedInstance::ad_SinkAnim:
            ped_->drawSinkFrame(x, y, frame_);
            break;
        case PedInstance::ad_StandBurnAnim:
            ped_->drawStandBurnFrame(x, y, frame_);
            break;
        case PedInstance::ad_WalkBurnAnim:
            ped_->drawWalkBurnFrame(x, y, frame_);
            break;
        case PedInstance::ad_DieBurnAnim:
            ped_->drawDieBurnFrame(x, y, frame_);
            break;
        case PedInstance::ad_SmokeBurnAnim:
            ped_->drawSmokeBurnFrame(x, y, frame_);
            break;
        case PedInstance::ad_DeadBurnAnim:
            ped_->drawDeadBurnFrame(x, y, frame_);
            break;
        case PedInstance::ad_PersuadedAnim:
            ped_->drawPersuadeFrame(x, y, frame_);
            break;
        case PedInstance::ad_NoAnimation:
            break;
    }
}

void PedInstance::drawSelectorAnim(int x, int y) {

    Weapon::WeaponAnimIndex weapon_idx =
        selectedWeapon() ? selectedWeapon()->index() : Weapon::Unarmed_Anim;

    switch(drawnAnim()) {
        case PedInstance::ad_HitAnim:
            ped_->drawHitFrame(x, y, getDirection(), frame_);
            break;
        case PedInstance::ad_DieAnim:
            ped_->drawDieFrame(x, y, frame_);
            break;
        case PedInstance::ad_DeadAnim:
            ped_->drawDeadFrame(x, y, frame_);
            break;
        case PedInstance::ad_DeadAgentAnim:
            ped_->drawDeadAgentFrame(x, y, frame_);
            break;
        case PedInstance::ad_PickupAnim:
            ped_->drawPickupFrame(x, y, frame_);
            break;
        case PedInstance::ad_PutdownAnim:
            ped_->drawPickupFrame(x, y, frame_);
            break;
        case PedInstance::ad_WalkAnim:
            ped_->drawWalkFrame(x, y, getDirection(), frame_, weapon_idx);
            break;
        case PedInstance::ad_StandAnim:
            ped_->drawStandFrame(x, y, getDirection(), frame_, weapon_idx);
            break;
        case PedInstance::ad_WalkFireAnim:
            ped_->drawWalkFireFrame(x, y, getDirection(), frame_, weapon_idx);
            break;
        case PedInstance::ad_StandFireAnim:
            ped_->drawStandFireFrame(x, y, getDirection(), frame_, weapon_idx);
            break;
        case PedInstance::ad_VaporizeAnim:
            ped_->drawVaporizeFrame(x, y, getDirection(), frame_);
            break;
        case PedInstance::ad_SinkAnim:
            ped_->drawSinkFrame(x, y, frame_);
            break;
        case PedInstance::ad_StandBurnAnim:
            ped_->drawStandBurnFrame(x, y, frame_);
            break;
        case PedInstance::ad_WalkBurnAnim:
            ped_->drawWalkBurnFrame(x, y, frame_);
            break;
        case PedInstance::ad_DieBurnAnim:
            ped_->drawDieBurnFrame(x, y, frame_);
            break;
        case PedInstance::ad_SmokeBurnAnim:
            ped_->drawSmokeBurnFrame(x, y, frame_);
            break;
        case PedInstance::ad_DeadBurnAnim:
            ped_->drawDeadBurnFrame(x, y, frame_);
            break;
        case PedInstance::ad_PersuadedAnim:
            ped_->drawPersuadeFrame(x, y, frame_);
            break;
        case PedInstance::ad_NoAnimation:
#ifdef _DEBUG
            printf("hmm ad_NoAnimation\n");
#endif
            break;
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

    return true;
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
        GameEvent::sendEvt(GameEvent::kMission, GameEvent::kEvtShootingWeaponDeselected, this);
    }
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
        break;
    }

    if (type_ != kPedTypePolice || isPersuaded()) {
        if (previousWeapon == NULL && selectedWeapon()->canShoot()) {
            // alert if it's the first time the ped shows a shooting weapon
            GameEvent::sendEvt(GameEvent::kMission, GameEvent::kEvtShootingWeaponSelected, this);
        } else if (previousWeapon != NULL && previousWeapon->canShoot() && !selectedWeapon()->canShoot()) {
            // or alert if ped go from a shooting weapon to a no shooting weapon like the persuadotron
            GameEvent::sendEvt(GameEvent::kMission, GameEvent::kEvtShootingWeaponDeselected, this);
        }
    }
}

/*!
 * Drops the weapon at given index on the ground.
 * \param index Index of weapon in the agent inventory.
 * \return the instance of dropped weapon
 */
WeaponInstance * PedInstance::dropWeapon(uint8 index) {
    if (selected_weapon_ == index) {
        stopUsingWeapon();
    }
    WeaponInstance *pWeapon = removeWeaponAtIndex(index);

    if(pWeapon) {
        pWeapon->setMap(map_);
        pWeapon->setDrawable(true);
        pWeapon->setPosition(pos_);
        g_Session.getMission()->addWeaponToGround(pWeapon);
    }

    return pWeapon;
}

/*!
 * Drop all the ped's weapons on the ground around him.
 * \return void
 *
 */
void PedInstance::dropAllWeapons() {
    Mission *m = g_Session.getMission();
    uint8 twd = m->mtsurfaces_[pos_.tx + m->mmax_x_ * pos_.ty
        + m->mmax_m_xy * pos_.tz];

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
    setMap(g_Session.getMission()->map());
    setPosition(in_vehicle_->position());
    in_vehicle_ = NULL;
    switchActionStateFrom(state_ & PedInstance::pa_smInCar);
}

PedInstance::AnimationDrawn PedInstance::drawnAnim(void) {
    return drawn_anim_;
}

void PedInstance::setDrawnAnim(PedInstance::AnimationDrawn drawn_anim) {
    if (drawn_anim_ == drawn_anim)
        return;

    drawn_anim_ = drawn_anim;
    frame_ = 0;
    is_frame_drawn_ = false;
    switch (drawn_anim_) {
        case PedInstance::ad_HitAnim:
            setFramesPerSec(6);
            break;
        case PedInstance::ad_DieAnim:
            setFramesPerSec(8);
            break;
        case PedInstance::ad_DeadAnim:
            setFramesPerSec(2);
            break;
        case PedInstance::ad_DeadAgentAnim:
            setFramesPerSec(2);
            break;
        case PedInstance::ad_PickupAnim:
            setFramesPerSec(8);
            break;
        case PedInstance::ad_PutdownAnim:
            setFramesPerSec(8);
            break;
        case PedInstance::ad_WalkAnim:
            setFramesPerSec(8);
            break;
        case PedInstance::ad_StandAnim:
            setFramesPerSec(8);
            break;
        case PedInstance::ad_WalkFireAnim:
            setFramesPerSec(8);
            break;
        case PedInstance::ad_StandFireAnim:
            setFramesPerSec(8);
            break;
        case PedInstance::ad_VaporizeAnim:
            setFramesPerSec(6);
            break;
        case PedInstance::ad_SinkAnim:
            setFramesPerSec(8);
            break;
        case PedInstance::ad_StandBurnAnim:
            setFramesPerSec(8);
            break;
        case PedInstance::ad_WalkBurnAnim:
            setFramesPerSec(8);
            break;
        case PedInstance::ad_DieBurnAnim:
            setFramesPerSec(6);
            break;
        case PedInstance::ad_SmokeBurnAnim:
            setFramesPerSec(2);
            break;
        case PedInstance::ad_DeadBurnAnim:
            setFramesPerSec(2);
            break;
        case PedInstance::ad_PersuadedAnim:
            setFramesPerSec(8);
            break;
        case PedInstance::ad_NoAnimation:
            break;
    }
}

bool PedInstance::handleDrawnAnim(int elapsed) {
    Weapon::WeaponAnimIndex weapon_idx =
        selectedWeapon() ? selectedWeapon()->index() : Weapon::Unarmed_Anim;

    PedInstance::AnimationDrawn curanim = drawnAnim();
    // TODO: resolve switch selected weapon and current drawing
    // FIXME: quick fix. to remove
    if (weapon_idx == Weapon::Unarmed_Anim
        && (curanim == PedInstance::ad_StandFireAnim
        || curanim == PedInstance::ad_WalkFireAnim))
    {
        return true;
    }
    bool answer = true;
    switch (curanim) {
        case PedInstance::ad_HitAnim:
            if (frame_ < ped_->lastHitFrame(getDirection()))
                answer = false;
            break;
        case PedInstance::ad_DieAnim:
            if (frame_ < ped_->lastDieFrame()) {
                answer = false;
                break;
            }
            setDrawnAnim(PedInstance::ad_DeadAnim);
            break;
        case PedInstance::ad_DeadAnim:
            break;
        case PedInstance::ad_DeadAgentAnim:
            break;
        case PedInstance::ad_PickupAnim:
        case PedInstance::ad_PutdownAnim:
            if (frame_ < ped_->lastPickupFrame())
                answer = false;
            break;
        case PedInstance::ad_WalkAnim:
        case PedInstance::ad_StandAnim:
            break;
        case PedInstance::ad_WalkFireAnim:
            if(frame_ < ped_->lastWalkFireFrame(getDirection(), weapon_idx))
                answer = false;
            break;
        case PedInstance::ad_StandFireAnim:
            if(frame_ < ped_->lastStandFireFrame(getDirection(), weapon_idx))
                answer = false;
            break;
        case PedInstance::ad_VaporizeAnim:
            if (frame_ < ped_->lastVaporizeFrame(getDirection())) {
                answer = false;
            }
            break;
        case PedInstance::ad_SinkAnim:
            // TODO: use this in future
            break;
        case PedInstance::ad_WalkBurnAnim:
        case PedInstance::ad_StandBurnAnim:
            if (leftTimeShowAnim(elapsed)) {
                answer = false;
                break;
            }
            setDrawnAnim(PedInstance::ad_DieBurnAnim);
            break;
        case PedInstance::ad_DieBurnAnim:
            if (frame_ >= ped_->lastDieBurnFrame()) {
                setDrawnAnim(PedInstance::ad_SmokeBurnAnim);
                setTimeShowAnim(7000);
            } else
                answer = false;
            break;
        case PedInstance::ad_SmokeBurnAnim:
            if (leftTimeShowAnim(elapsed)) {
                answer = false;
                break;
            }
            setDrawnAnim(PedInstance::ad_DeadBurnAnim);
            break;
        case PedInstance::ad_DeadBurnAnim:
            break;
        case PedInstance::ad_PersuadedAnim:
            if (frame_ < ped_->lastPersuadeFrame())
                answer = false;
            break;
        case PedInstance::ad_NoAnimation:
            break;
    }
    return answer;
}

/*!
 * Return the damage after applying reduction of Mod protection.
 * \param d Damage description
 */
int PedInstance::getRealDamage(fs_dmg::DamageToInflict &d) {
    // TODO : implement
    return d.dvalue;
}

/*!
 * Method called when object is hit by a weapon shot.
 * \param d Damage description
 */
void PedInstance::handleHit(fs_dmg::DamageToInflict &d) {
    if (health_ > 0) {
        decreaseHealth(getRealDamage(d));

        PedInstance *pShooter = dynamic_cast<PedInstance *>(d.d_owner);
        if (pShooter && pShooter->isOurAgent()) {
            g_Session.getMission()->stats()->incrHits();
        }

        // Only add a hit if ped is not currently being hit
        if (currentAction_ == NULL || currentAction_->type() != Action::kActTypeHit) {
            insertHitAction(d);
        }

        // Alert behaviour
        behaviour_.handleBehaviourEvent(Behaviour::kBehvEvtHit);
    }
}

/*!
 * Method called when object is hit by a weapon shot.
 * \param d Damage description
 * \return true if Ped has died
 */
bool PedInstance::handleDeath(Mission *pMission, fs_dmg::DamageToInflict &d) {
    if (health_ == 0) {
        clearDestination();
        switchActionStateTo(PedInstance::pa_smDead);

        switch (d.dtype) {
            case fs_dmg::kDmgTypeBullet:
                setDrawnAnim(PedInstance::ad_DieAnim);
                dropAllWeapons();
                break;
            case fs_dmg::kDmgTypeLaser:
                if (is_our_) {
                    setDrawnAnim(PedInstance::ad_DeadAgentAnim);
                } else {
                    setDrawnAnim(PedInstance::ad_NoAnimation);
                }
                break;
            case fs_dmg::kDmgTypeExplosion:
            case fs_dmg::kDmgTypeBurn:
                if (hasMinimumVersionOfMod(Mod::MOD_CHEST, Mod::MOD_V2) &&
                    d.d_owner != this) {
                    setDrawnAnim(PedInstance::ad_DieAnim);
                    dropAllWeapons();
                } else {
                    // was burning because not enough protected or suicide
                    // so die burning
                    setDrawnAnim(PedInstance::ad_DieBurnAnim);
                }
                break;
            default:
                FSERR(Log::k_FLG_GAME, "PedInstance", "handleDeath", ("Unhandled damage type: %d\n", d.dtype))
        }

        updatePersuadedRelations(pMission->getSquad());

        // send an event to alert agent died
        if (isOurAgent()) {
            GameEvent::sendEvt(GameEvent::kMission, GameEvent::kAgentDied, this);
        }
    }

    return health_ == 0;
}

void PedInstance::addEnemyGroupDef(uint32 eg_id, uint32 eg_def) {
    enemy_group_defs_.add(eg_id, eg_def);
}

void PedInstance::rmEnemyGroupDef(uint32 eg_id, uint32 eg_def) {
    enemy_group_defs_.rm(eg_id, eg_def);
}

bool PedInstance::isInEnemyGroupDef(uint32 eg_id, uint32 eg_def) {
    return enemy_group_defs_.isIn(eg_id, eg_def);
}

void PedInstance::addEmulatedGroupDef(uint32 eg_id, uint32 eg_def) {
    emulated_group_defs_.add(eg_id, eg_def);
}
void PedInstance::rmEmulatedGroupDef(uint32 eg_id, uint32 eg_def) {
    emulated_group_defs_.rm(eg_id, eg_def);
}

bool PedInstance::isInEmulatedGroupDef(uint32 eg_id, uint32 eg_def) {
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
 * Movement speed calculated from base speed, mods, weight of inventory,
 * ipa, etc.
 */
int PedInstance::getDefaultSpeed()
{
    int speed_new = base_speed_;

    int weight_max = 0;
    Mod *pMod = slots_[Mod::MOD_LEGS];
    if (pMod) {
        weight_max += 5 << (pMod->getVersion() + 1);
        speed_new *= (pMod->getVersion() + 5);
        speed_new >>= 2;
    } else
        weight_max = 5;
    pMod = slots_[Mod::MOD_ARMS];
    if (pMod)
        weight_max += 5 << (pMod->getVersion() + 1);
    else
        weight_max += 5;

    int weight_inv = 0;
    for (uint8 i = 0; i < weapons_.size(); ++i)
        weight_inv += weapons_[i]->getWeight();

    if (weight_inv > weight_max) {
        if ((weight_inv / weight_max) > 1)
            speed_new = 64;
        else
            speed_new /= 2;
    }

    if (obj_group_def_ == PedInstance::og_dmAgent)
    {
        // See the comments in the IPAStim class for details on the multiplier
        // algorithm for adrenaline
        speed_new = (int)((float)speed_new * adrenaline_->getMultiplier());
    }

    if (isPersuaded()) {
        speed_new *= owner_->getSpeedOwnerBoost();
        speed_new >>= 1;
    }

    return speed_new;
}

// NOTE: returned value is *2, it should be should be corrected
// during calculations with /2
int PedInstance::getSpeedOwnerBoost()
{
    if (obj_group_def_ == PedInstance::og_dmAgent)
    {
        float ipa_adr = adrenaline_->getMultiplier();
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
    if (originLocW.z > (g_App.maps().map(map_)->maxZ() - 1) * 128)
        return;

    if (pAimedLocW->z > (g_App.maps().map(map_)->maxZ() - 1) * 128)
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

        base_mod -= 0.4 * (2.0 - perception_->getMultiplier());
        base_mod += 0.4 * (2.0 - adrenaline_->getMultiplier());
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
uint16 PedInstance::getRequiredPointsToPersuade(PedType aType) {
    Mod *pMod = slots_[Mod::MOD_BRAIN];
    uint16 points = 0;
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
        uint16 points = getRequiredPointsToPersuade(pOtherPed->type());
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
    fs_cmn::setBitsWithMask(&desc_state_, pd_smControlled);
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
    case kPedTypePolice:
        totalPersuasionPoints_ +=  4;
    case kPedTypeAgent:
        totalPersuasionPoints_ +=  32;
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
        for (uint8 i = 0; i < AgentManager::kMaxSlot; ++i) {
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

/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net>
 *   Copyright (C) 2016, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-kernel/model/static.h"

#include "fs-utils/common.h"
#include "fs-utils/log/log.h"
#include "fs-engine/gfx/animationmanager.h"
#include "fs-kernel/mgr/missionmanager.h"
#include "fs-kernel/model/ped.h"

namespace fs_knl {

const int Static::kStaticOrientation1 = 0;
const int Static::kStaticOrientation2 = 2;
const uint16_t Semaphore::kSemaphoreMaxColorAnim = 4;
const uint16_t Semaphore::kSemaphoreDamagedOffset = 4;

Static *Static::loadInstance(uint8_t * data, uint16_t id, Map *pMap)
{
    LevelData::Statics * gamdata =
        (LevelData::Statics *) data;
    Static *s = NULL;

    // TODO: verify whether object description is correct
    // subtype for doors, use instead orientation?

    // NOTE: objects states are usually mixed with type,
    // no separation between object type and its state
    uint16_t curanim = fs_utl::READ_LE_UINT16(gamdata->index_current_anim);
    uint16_t baseanim = fs_utl::READ_LE_UINT16(gamdata->index_base_anim);
    uint16_t curframe = fs_utl::READ_LE_UINT16(gamdata->index_current_frame);
    switch(gamdata->sub_type) {
        case 0x01:
            // phone booth
            s = new EtcObj(id, pMap, curanim);
            s->setSize(128, 128, 128);
            break;
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
            // crossroad things
            s = new Semaphore(id, pMap, gamdata->sub_type - 0x05);
            s->setSize(48, 48, 48);
            //s->state_ = sttsem_Stt0 + (gamdata->sub_type - 0x05);
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x0B:
            // 0x0270 animation, is this object present in original game?
            //s = new EtcObj(m, curanim, curanim, curanim);
            //printf("0x0B anim %X\n", curanim);
            break;
        case 0x0A:
            s = new NeonSign(id, pMap, curanim, curframe);
            break;
        case 0x0C: // closed door
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, pMap, baseanim, Static::kStateDoorClosed);
                s->setOrientation(kStaticOrientation1);
                s->setSize(256, 1, 196);
            } else {
                baseanim++;
                s = new Door(id, pMap, baseanim, Static::kStateDoorClosed);
                s->setOrientation(kStaticOrientation2);
                s->setSize(1, 256, 196);
            }
            break;
        case 0x0D: // closed door
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, pMap, baseanim, Static::kStateDoorClosed);
                s->setOrientation(kStaticOrientation1);
                s->setSize(256, 1, 196);
            } else {
                baseanim++;
                s = new Door(id, pMap, baseanim, Static::kStateDoorClosed);
                s->setOrientation(kStaticOrientation2);
                s->setSize(1, 256, 196);
            }
            break;
        case 0x0E: // opening doors, not open
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, pMap, baseanim, kStateDoorOpening);
                s->setOrientation(kStaticOrientation1);
                s->setSize(256, 1, 196);
            } else {
                baseanim++;
                s = new Door(id, pMap, baseanim, kStateDoorOpening);
                s->setOrientation(kStaticOrientation2);
                s->setSize(1, 256, 196);
            }
            break;
        case 0x0F: // opening doors, not open
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, pMap, baseanim, kStateDoorOpening);
                s->setOrientation(kStaticOrientation1);
                s->setSize(256, 1, 196);
            } else {
                baseanim++;
                s = new Door(id, pMap, baseanim, kStateDoorOpening);
                s->setOrientation(kStaticOrientation2);
                s->setSize(1, 256, 196);
            }
            break;
        case 0x11:
            // ???? what is this
            //s = new EtcObj(m, bas, curanim, curanim);
            //printf("0x11 anim %X\n", curanim);
            break;
        case 0x12:  // open window
            s = new WindowObj(id, pMap, Static::kStateWindowOpen, curanim - 2);
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80) {
                s->setOrientation(kStaticOrientation1);
                s->setSize(96, 4, 96);
            } else {
                s->setOrientation(kStaticOrientation2);
                s->setSize(4, 96, 96);
            }
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x13: // closed window
            s = new WindowObj(id, pMap, Static::kStateWindowClosed, curanim);
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80) {
                s->setOrientation(kStaticOrientation1);
                s->setSize(96, 4, 96);
            } else {
                s->setOrientation(kStaticOrientation2);
                s->setSize(4, 96, 96);
            }
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x15: // damaged window
            s = new WindowObj(id, pMap, Static::kStateWindowDamaged, curanim - 6);
            s->setExcludedFromBlockers(true);
            s->setHealth(0);
            s->setStartHealth(1);
            break;
        case 0x16:
            // TODO: set state if damaged trees exist
            s = new Tree(id, pMap, curanim);
            s->setSize(64, 64, 256);
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x19: // trash bin
            s = new EtcObj(id, pMap, curanim);
            s->setSize(64, 64, 96);
            break;
        case 0x1A: // mail box
            s = new EtcObj(id, pMap, curanim);
            s->setSize(64, 64, 96);
            break;
        case 0x1C: // ???? what is this?
            //s = new EtcObj(m, curanim, curanim, curanim);
            //printf("0x1C anim %X\n", curanim);
            break;
        case 0x1F: // advertisement on wall
            s = new EtcObj(id, pMap, curanim, smt_Advertisement);
            s->setExcludedFromBlockers(true);
            break;

        case 0x20: // window without light
        {
            AnimWindow *pWin = new AnimWindow(id, pMap, curanim);
            pWin->playLightOffAnimation();
            s = pWin;
        }
            break;
        case 0x21: // window light turns on
        {
            AnimWindow *pWin = new AnimWindow(id, pMap, curanim - 2);
            pWin->playLightSwitchingAnimation();
            s = pWin;
        }
            break;
        // NOTE : 0x22 should have existed but it doesn't appear anywhere
        case 0x23:
        {
            // window with person's shadow non animated,
            // even though on 1 map person appears I will ignore it
            AnimWindow *pWin = new AnimWindow(id, pMap, 1959 + ((gamdata->orientation & 0x40) >> 6));
            pWin->playShowPedAnimation();
            s = pWin;
        }
            break;
        case 0x24:
        {
            // window with person's shadow, hides, actually animation
            // is of ped standing, but I will ignore it
            // TODO : find a mission with this type of window to check if animation id is ok
            AnimWindow *pWin = new AnimWindow(id, pMap, 1959 + 8 + ((gamdata->orientation & 0x40) >> 6));
            pWin->playedDisappearAnimation();
            s = pWin;
        }
            break;
        case 0x25:
        {
            AnimWindow *pWin = new AnimWindow(id, pMap, curanim);
            // NOTE : orientation, I assume, plays role of hidding object,
            // orientation 0x40, 0x80 are drawn (gamdata->desc always 7)
            // window without light
            if (gamdata->orientation == 0x40 || gamdata->orientation == 0x80) {
                pWin->playLightOffAnimation();
            } else {
                pWin->playLightOnAnimation();
            }
            s = pWin;
        }
            break;

        case 0x26:
            // 0x00,0x80 south - north = 0
            // 0x40,0xC0 weast - east = 2
            s = new LargeDoor(id, pMap, curanim);
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80) {
                s->setOrientation(kStaticOrientation1);
                s->setSize(384, 64, 192);
            } else {
                s->setOrientation(kStaticOrientation2);
                s->setSize(64, 384, 192);
            }
            break;
#ifdef _DEBUG
        default:
            printf("uknown static object type %02X , %02X, %X\n",
                gamdata->sub_type, gamdata->orientation,
                fs_utl::READ_LE_UINT16(gamdata->index_current_frame));
            printf("x = %i, xoff = %i, ", gamdata->mapposx[1],
                gamdata->mapposx[0]);
            printf("y = %i, yoff = %i, ", gamdata->mapposy[1],
                gamdata->mapposy[0]);
            printf("z = %i, zoff = %i\n", gamdata->mapposz[1],
                gamdata->mapposz[0]);
            break;
#endif
    }

    if (s) {
        int z = fs_utl::READ_LE_UINT16(gamdata->mapposz) >> 7;
        int oz = gamdata->mapposz[0] & 0x7F;
        // trick to draw
        if (s->type() == Static::smt_Advertisement)
            z += 1;

        s->setPosition(gamdata->mapposx[1], gamdata->mapposy[1],
            z, gamdata->mapposx[0], gamdata->mapposy[0], oz);

        //s->setMainType(gamdata->sub_type);
#if 0
        if (s->tileX() == 66 && s->tileY() == 49)
            oz = 2;
#endif
        s->setDirection(gamdata->orientation);
    }

    return s;
}

/*!
 * @brief 
 * @param anId id of the static
 * @param pMap 
 * @param baseAnim The first animation
 * @param initialState initial state of the Door object
 */
Door::Door(uint16_t anId, Map *pMap, uint16_t baseAnim, Static::StateDoors initialState) : 
        Static(anId, pMap, Static::smt_Door) {
    
    closedAnim_ = animationPlayer_->addAnimation(baseAnim);
    openingAnim_ = animationPlayer_->addAnimation(baseAnim + 2);
    openedAnim_ = animationPlayer_->addAnimation(baseAnim + 4);
    closingAnim_ = animationPlayer_->addAnimation(baseAnim + 6);

    state_ = initialState;
    if (state_ == Static::kStateDoorClosed) {
        playAnimation(closedAnim_);
    } else if (state_ == Static::kStateDoorOpening) {
        playAnimation(openingAnim_);
    }
}

void Door::draw(const Point2D &screenPos) {
    animationPlayer_->draw(addOffs(screenPos), 0);
}

void Door::doUpdateState([[maybe_unused]] uint32_t elapsed) {
    Mission *pMission = g_missionCtrl.mission();
    ShootableMovableMapObject *pPed = NULL;
    int x = tileX();
    int y = tileY();
    int z = tileZ();
    MapObject::ObjectNature aNature;
    int si;
    char inc_rel = 0, rel_inc = 0;
    char *i = 0, *j = 0;
    bool found = false;

    switch(state_) {
        case Static::kStateDoorOpen:
            if (orientation_ == kStaticOrientation1) {
                i = &rel_inc;
                j = &inc_rel;
            } else if (orientation_ == kStaticOrientation2) {
                i = &inc_rel;
                j = &rel_inc;
            }
            assert(i != 0 && j != 0);
            for(*i = 0; *i < 2; *i += 1) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    pPed = dynamic_cast<ShootableMovableMapObject *>(pMission->findObjectWithNatureAtPos(x + inc_rel,
                        y + rel_inc, z, &aNature, &si, true));
                    if (!pPed && state_ == Static::kStateDoorOpen && (!found)) {
                        state_ = Static::kStateDoorClosing;
                        setExcludedFromBlockers(false);
                        playAnimation(closingAnim_);
                    } else if (pPed && pPed->isAlive()){
                        state_ = Static::kStateDoorOpen;
                        playAnimation(openedAnim_);
                        setExcludedFromBlockers(true);
                        found = true;
                        pPed->hold_on_.wayFree = 0;
                    }
                } while (pPed);
            }
            break;
        case Static::kStateDoorClosed:
            if (orientation_ == kStaticOrientation1) {
                i = &rel_inc;
                j = &inc_rel;
            } else if (orientation_ == kStaticOrientation2) {
                i = &inc_rel;
                j = &rel_inc;
            }
            assert(i != 0 && j != 0);
            *i = 1;
            aNature = MapObject::kNaturePed; si = 0;
            do {
                pPed = dynamic_cast<ShootableMovableMapObject *>(pMission->findObjectWithNatureAtPos(x + inc_rel,
                    y + rel_inc, z, &aNature, &si, true));
                if (pPed && pPed->isAlive()) {
                    if (!found) {
                        state_ = Static::kStateDoorOpening;
                        setExcludedFromBlockers(false);
                        found = true;
                        playAnimation(openingAnim_);
                    }
                    pPed->hold_on_.wayFree = 1;
                    pPed->hold_on_.tilex = x;
                    pPed->hold_on_.tiley = y;
                    pPed->hold_on_.tilez = z;
                    pPed->hold_on_.xadj = 0;
                    pPed->hold_on_.yadj = 0;
                    pPed->hold_on_.pathBlocker = this;
                }
            } while (pPed);
            *i = 0;
            aNature = MapObject::kNaturePed; si = 0;
            do {
                pPed = dynamic_cast<ShootableMovableMapObject *>(pMission->findObjectWithNatureAtPos(x + inc_rel,
                    y + rel_inc, z, &aNature, &si, true));
                if (pPed && pPed->isAlive()) {
                    if (!found) {
                        state_ = Static::kStateDoorOpening;
                        setExcludedFromBlockers(false);
                        found = true;
                        playAnimation(openingAnim_);
                    }
                    pPed->hold_on_.wayFree = 1;
                    pPed->hold_on_.tilex = x;
                    pPed->hold_on_.tiley = y;
                    pPed->hold_on_.tilez = z;
                    pPed->hold_on_.xadj = 0;
                    pPed->hold_on_.yadj = 0;
                    pPed->hold_on_.pathBlocker = this;
                }
            } while (pPed);
            break;
    }
}

/*!
 * @copydoc MapObject::handleAnimationEnded()
 */
void Door::handleAnimationEnded() {
    if (animationPlayer_->isCurrentAnimation(closingAnim_)) {
        state_ = Static::kStateDoorClosed;
        setExcludedFromBlockers(false);
        playAnimation(closedAnim_);
    } else if (animationPlayer_->isCurrentAnimation(openingAnim_)) {
        state_ = Static::kStateDoorOpen;
        setExcludedFromBlockers(true);
        playAnimation(openedAnim_);
    }
}


/*!
 * Return true if door should be counted as a blocker
 * @return False if door is opened
 */
bool Door::isPathBlocker() {
    return state_ != Static::kStateDoorOpen;
}


/*!
 * @brief 
 * @param anId 
 * @param pMap 
 * @param baseAnim 
 */
LargeDoor::LargeDoor(uint16_t anId, Map *pMap, uint16_t baseAnim):
        Static(anId, pMap, Static::smt_LargeDoor) {
    
    closedAnim_ = animationPlayer_->addAnimation(baseAnim);
    closingAnim_ = animationPlayer_->addAnimation(baseAnim + 2);
    openingAnim_ = animationPlayer_->addAnimation(baseAnim + 4);

    state_ = Static::kStateDoorClosed;
    playAnimation(closedAnim_);
}

void LargeDoor::draw(const Point2D &screenPos) {
   // When a large door is opened, we just don't draw it
    if (state_ != Static::kStateDoorOpen) {
        animationPlayer_->draw(addOffs(screenPos), 0);
    }
}

void LargeDoor::doUpdateState(uint32_t elapsed) {
    // TODO: there must be somewhere locked door
    Mission *pMission = g_missionCtrl.mission();
    ShootableMovableMapObject *pVehicle = NULL;
    PedInstance *pPed = NULL;
    int x = tileX();
    int y = tileY();
    int z = tileZ();
    MapObject::ObjectNature aNature;
    int si;
    char inc_rel = 0, rel_inc = 0;
    char *i = 0, *j = 0;
    bool found = false;
    std::vector<PedInstance *> found_peds;
    found_peds.reserve(256);
    std::vector<PedInstance *> found_peds_mid;
    found_peds_mid.reserve(256);
    char sign;
    int set_wayFree = 0;
    uint32_t cur_state = state_;

    switch(state_) {
        case Static::kStateDoorOpen:
            if (orientation_ == kStaticOrientation1) {
                i = &rel_inc;
                j = &inc_rel;
            } else if (orientation_ == kStaticOrientation2) {
                i = &inc_rel;
                j = &rel_inc;
            }
            assert(i != 0 && j != 0);
            *j = -1;
            for(*i = -2; *i < 3; (*i)++) {
                aNature = MapObject::kNatureVehicle; si = 0;
                pVehicle = dynamic_cast<ShootableMovableMapObject *>
                                (pMission->findObjectWithNatureAtPos(x + inc_rel,
                                                                    y + rel_inc,z, &aNature, &si, true));
                if (!pVehicle && !found) {
                    state_ = Static::kStateDoorClosing;
                    playAnimation(closingAnim_);
                    setExcludedFromBlockers(false);
                } else if (pVehicle){
                    state_ = Static::kStateDoorOpen;
                    // I set the closedAnim just to have an animation set as no frame
                    // is drawn when the state is Open
                    playAnimation(closedAnim_);
                    setExcludedFromBlockers(true);
                    found = true;
                    pVehicle->hold_on_.wayFree = 0;
                }
            }
            *j = 1;
            for(*i = -2; *i < 3; (*i)++) {
                aNature = MapObject::kNatureVehicle; si = 0;
                pVehicle = dynamic_cast<ShootableMovableMapObject *>
                                (pMission->findObjectWithNatureAtPos(x + inc_rel,
                                                                    y + rel_inc,z,&aNature,&si,true));
                if (!pVehicle && !found) {
                    state_ = Static::kStateDoorClosing;
                    playAnimation(closingAnim_);
                    setExcludedFromBlockers(false);
                } else if (pVehicle){
                    state_ = Static::kStateDoorOpen;
                    // see comment above
                    playAnimation(closedAnim_);
                    setExcludedFromBlockers(true);
                    found = true;
                    pVehicle->hold_on_.wayFree = 0;
                }
            }
            *j = -1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    pPed = (PedInstance *)(pMission->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (pPed) {
                        found_peds.push_back(pPed);
                        if (!found && pPed->hasAccessCard()) {
                            state_ = Static::kStateDoorOpen;
                            // see comment above
                            playAnimation(closedAnim_);
                            setExcludedFromBlockers(true);
                            found = true;
                        }
                    }
                } while (pPed);
            }
            *j = 1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    pPed = (PedInstance *)(pMission->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (pPed) {
                        found_peds.push_back(pPed);
                        if (!found && pPed->hasAccessCard()) {
                            state_ = Static::kStateDoorOpen;
                            // see comment above
                            playAnimation(closedAnim_);
                            setExcludedFromBlockers(true);
                            found = true;
                        }
                    }
                } while (pPed);
            }
            *j = 0;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    pPed = (PedInstance *)(pMission->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (pPed) {
                        found_peds_mid.push_back(pPed);
                        if (!found && pPed->hasAccessCard()) {
                            state_ = Static::kStateDoorOpen;
                            // see comment above
                            playAnimation(closedAnim_);
                            setExcludedFromBlockers(true);
                            found = true;
                        }
                    }
                } while (pPed);
            }
            if (state_ == Static::kStateDoorOpen) {
                for (PedInstance *foundPed : found_peds) {
                    foundPed->hold_on_.wayFree = 0;
                }
                for (PedInstance *foundPedMid : found_peds_mid) {
                    foundPedMid->hold_on_.wayFree = 0;
                }
            } else {
                for (PedInstance *pPed : found_peds) {
                    pPed->hold_on_.wayFree = 2;
                    pPed->hold_on_.tilex = x;
                    pPed->hold_on_.tiley = y;
                    if (orientation_ == kStaticOrientation1) {
                        pPed->hold_on_.xadj = 1;
                        pPed->hold_on_.yadj = 0;
                    } else if (orientation_ == kStaticOrientation2) {
                        pPed->hold_on_.xadj = 0;
                        pPed->hold_on_.yadj = 1;
                    }
                    pPed->hold_on_.tilez = z;
                    pPed->hold_on_.pathBlocker = this;
                }
                for (PedInstance *pPed : found_peds_mid) {
                    DamageToInflict d;
                    d.dtype = kDmgTypeCollision;
                    d.d_owner = NULL;
                    d.dvalue = 1024;
                    d.ddir = -1;
                    pPed->handleHit(d);
                }
            }
            break;
        case Static::kStateDoorClosed:
            if (orientation_ == kStaticOrientation1) {
                i = &rel_inc;
                j = &inc_rel;
                sign = 1;
            } else if (orientation_ == kStaticOrientation2) {
                i = &inc_rel;
                j = &rel_inc;
                sign = -1;
            }
            assert(i != 0 && j != 0);
            *j = -1 * sign;
            *i = -2;
            aNature = MapObject::kNatureVehicle; si = 0;
            pVehicle = dynamic_cast<ShootableMovableMapObject *>
                            (pMission->findObjectWithNatureAtPos(x + inc_rel,
                                                                y + rel_inc,z, &aNature, &si,true));
            if (pVehicle) {
                if (!found) {
                    state_ = Static::kStateDoorOpening;
                    playAnimation(openingAnim_);
                    setExcludedFromBlockers(false);
                    found = true;
                }
                pVehicle->hold_on_.wayFree = 1;
                pVehicle->hold_on_.pathBlocker = this;
            }
            *j = 1 * sign;
            *i = 2;
            aNature = MapObject::kNatureVehicle; si = 0;
            pVehicle = dynamic_cast<ShootableMovableMapObject *>
                            (pMission->findObjectWithNatureAtPos(x + inc_rel,
                                                                y + rel_inc,z, &aNature, &si,true));
            if (pVehicle) {
                if (!found) {
                    state_ = Static::kStateDoorOpening;
                    playAnimation(openingAnim_);
                    setExcludedFromBlockers(false);
                    found = true;
                }
                pVehicle->hold_on_.wayFree = 1;
                pVehicle->hold_on_.pathBlocker = this;
            }
            *j = -1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    pPed = (PedInstance *)(pMission->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (pPed) {
                        found_peds.push_back(pPed);
                        if (!found && pPed->hasAccessCard()) {
                            state_ = Static::kStateDoorOpening;
                            playAnimation(openingAnim_);
                            setExcludedFromBlockers(false);
                            found = true;
                        }
                    }
                } while (pPed);
            }
            *j = 1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    pPed = (PedInstance *)(pMission->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (pPed) {
                        found_peds.push_back(pPed);
                        if (!found && pPed->hasAccessCard()) {
                            state_ = Static::kStateDoorOpening;
                            playAnimation(openingAnim_);
                            setExcludedFromBlockers(false);
                            found = true;
                        }
                    }
                } while (pPed);
            }
            set_wayFree = state_ == Static::kStateDoorOpening ? 1 : 2;
            for (PedInstance *pPed : found_peds) {
                pPed->hold_on_.wayFree = set_wayFree;
                pPed->hold_on_.tilex = x;
                pPed->hold_on_.tiley = y;
                if (orientation_ == kStaticOrientation1) {
                    pPed->hold_on_.xadj = 1;
                    pPed->hold_on_.yadj = 0;
                } else if (orientation_ == kStaticOrientation2) {
                    pPed->hold_on_.xadj = 0;
                    pPed->hold_on_.yadj = 1;
                }
                pPed->hold_on_.tilez = z;
                pPed->hold_on_.pathBlocker = this;
            }
            break;
        
        case Static::kStateDoorOpening:
            if (orientation_ == kStaticOrientation1) {
                i = &rel_inc;
                j = &inc_rel;
                sign = 1;
            } else if (orientation_ == kStaticOrientation2) {
                i = &inc_rel;
                j = &rel_inc;
                sign = -1;
            }
            assert(i != 0 && j != 0);
            *j = -1 * sign;
            *i = -2;
            set_wayFree = state_ == Static::kStateDoorOpening ? 1 : 2;
            aNature = MapObject::kNatureVehicle; si = 0;
            pVehicle = dynamic_cast<ShootableMovableMapObject *>
                    (pMission->findObjectWithNatureAtPos(x + inc_rel,
                y + rel_inc,z, &aNature, &si,true));
            if (pVehicle) {
                pVehicle->hold_on_.wayFree = 1;
                pVehicle->hold_on_.pathBlocker = this;
            }
            *j = 1 * sign;
            *i = 2;
            aNature = MapObject::kNatureVehicle; si = 0;
            pVehicle = dynamic_cast<ShootableMovableMapObject *>
                    (pMission->findObjectWithNatureAtPos(x + inc_rel,
                y + rel_inc,z, &aNature, &si,true));
            if (pVehicle) {
                pVehicle->hold_on_.wayFree = 1;
                pVehicle->hold_on_.pathBlocker = this;
            }
            *j = -1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    pPed = (PedInstance *)(pMission->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (pPed) {
                        found_peds.push_back(pPed);
                    }
                } while (pPed);
            }
            *j = 1;
            for (*i = -1; *i <= 1; (*i)++ ) {
                aNature = MapObject::kNaturePed; si = 0;
                do {
                    pPed = (PedInstance *)(pMission->findObjectWithNatureAtPos(x + rel_inc,
                        y + inc_rel, z, &aNature, &si, true));
                    if (pPed) {
                        found_peds.push_back(pPed);
                    }
                } while (pPed);
            }
            for (PedInstance *pPed : found_peds) {
                pPed->hold_on_.wayFree = set_wayFree;
                pPed->hold_on_.tilex = x;
                pPed->hold_on_.tiley = y;
                if (orientation_ == kStaticOrientation1) {
                    pPed->hold_on_.xadj = 1;
                    pPed->hold_on_.yadj = 0;
                } else if (orientation_ == kStaticOrientation2) {
                    pPed->hold_on_.xadj = 0;
                    pPed->hold_on_.yadj = 1;
                }
                pPed->hold_on_.tilez = z;
                pPed->hold_on_.pathBlocker = this;
            }
            break;
    }
}
    
void LargeDoor::handleAnimationEnded() {
    if (animationPlayer_->isCurrentAnimation(closingAnim_)) {
        state_ = Static::kStateDoorClosed;
        setExcludedFromBlockers(false);
    } else if (animationPlayer_->isCurrentAnimation(openingAnim_)) {
        state_ = Static::kStateDoorOpen;
        setExcludedFromBlockers(true);
    }
}


bool LargeDoor::isPathBlocker() {
    return state_ != Static::kStateDoorOpen;
}


/*!
 * @brief 
 * @param anId 
 * @param pMap 
 * @param baseAnim 
 */
Tree::Tree(uint16_t anId, Map *pMap, uint16_t baseAnim) :
        Static(anId, pMap, Static::smt_Tree) {
    idleAnim_ = animationPlayer_->addAnimation(baseAnim);
    burningAnim_ = animationPlayer_->addAnimation(baseAnim + 1);
    burntAnim_ = animationPlayer_->addAnimation(baseAnim + 2, fs_eng::kAnimationModeSingle, 2, 10000);
    animationPlayer_->play(idleAnim_);
}

void Tree::draw(const Point2D &screenPos) {
    Point2D posWithOffs = addOffs(screenPos);
    animationPlayer_->draw(posWithOffs, 0);
}

void Tree::handleAnimationEnded() {
    if (animationPlayer_->isCurrentAnimation(burningAnim_)) {
        animationPlayer_->play(burntAnim_);
    }
}

/*!
 * Implementation for the Tree. Tree burns only when hit by laser of fire.
 * \param d Damage information
 */
void Tree::handleHit(DamageToInflict &d) {
    if (isAlive() &&
        (d.dtype == kDmgTypeLaser || d.dtype == kDmgTypeBurn || d.dtype == kDmgTypeExplosion)) {
        decreaseHealth(d.dvalue);
        if (isDead()) {
            animationPlayer_->play(burningAnim_);
            setExcludedFromBlockers(true);
        }
    }
}

/*!
 * Constructor for the class
 * @param anId 
 * @param pMap 
 * @param state Initial state for the window
 * @param anim base animation. Other animations are always 2 after
 */
WindowObj::WindowObj(uint16_t anId, Map *pMap, StateWindows state, uint16_t anim):
        Static(anId, pMap, Static::smt_Window) {
    state_ = state;
    // We don't need to retain the id of the open/close anim as we
    // never come back to them after we move to breaking/damage anim
    if (state == Static::kStateWindowOpen) {
        playAnimation(animationPlayer_->addAnimation(anim + 2));
    } else if (state == Static::kStateWindowClosed) {
        playAnimation(animationPlayer_->addAnimation(anim));
    }
    breakingAnim_ = animationPlayer_->addAnimation(anim + 4, fs_eng::kAnimationModeSingle, 6);
    damagedAnim_ = animationPlayer_->addAnimation(anim + 6);

    if (state == Static::kStateWindowDamaged) {
        playAnimation(damagedAnim_);
    }
}

void WindowObj::handleAnimationEnded() {
    if (animationPlayer_->isCurrentAnimation(breakingAnim_)) {
        animationPlayer_->play(damagedAnim_);
        state_ = kStateWindowDamaged;
    }
}

void WindowObj::draw(const Point2D &screenPos) {
    animationPlayer_->draw(addOffs(screenPos), 0);
}

/*!
 * @brief 
 * @param d 
 */
void WindowObj::handleHit(DamageToInflict &d) {
    if (isAlive() &&
        (d.dtype == kDmgTypeBullet || d.dtype == kDmgTypeExplosion)) {
        decreaseHealth(d.dvalue);
        if (isDead()) {
            state_ = Static::kStateWindowBreaking;
            setExcludedFromBlockers(true);
            playAnimation(breakingAnim_);
        }
    }
}

/*!
 * Constructor
 * @param anId 
 * @param pMap 
 * @param baseAnim 
 * @param aType 
 */
EtcObj::EtcObj(uint16_t anId, Map *pMap, uint16_t baseAnim, StaticType aType) :
        Static(anId, pMap, aType) {
    fs_eng::AnimationMode mode = aType == Static::smt_Advertisement ?
            fs_eng::kAnimationModeLoop : fs_eng::kAnimationModeSingle;
    idleAnim_ = animationPlayer_->addAnimation(baseAnim, mode);
    animationPlayer_->play(idleAnim_);
}

void EtcObj::draw(const Point2D &screenPos) {
    animationPlayer_->draw(addOffs(screenPos), 0);
}

/*!
 * @brief Constructor of the class
 * @param anId 
 * @param pMap 
 * @param anim 
 */
NeonSign::NeonSign(uint16_t anId, Map *pMap, uint16_t anim, uint16_t currentFrame) : 
        Static(anId, pMap, Static::smt_NeonSign) {
    uint16_t animation = animationPlayer_->addAnimation(anim, fs_eng::kAnimationModeLoop);
    animationPlayer_->play(animation, g_SpriteMgr.getFrameFromFrameIndx(currentFrame));
    setExcludedFromBlockers(true);
    setSize(32, 1, 48);
}

void NeonSign::draw(const Point2D &screenPos) {
    animationPlayer_->draw(addOffs(screenPos), 0);
}

/*!
 * Constructor
 * @param anId 
 * @param pMap 
 * @param animationOffset offset to the base animation which is 1040. Should not be higher than 4
 */
Semaphore::Semaphore(uint16_t anId, Map *pMap, uint16_t animationOffset) :
        Static(anId, pMap, Static::smt_Semaphore),
        elapsed_left_smaller_(0), elapsed_left_bigger_(0), up_down_(1),
        colorTimer_(700), animOffset_(animationOffset) {
    // regular animation
    playAnimation(animationPlayer_->addAnimation(1040));
}

void Semaphore::doUpdateState(uint32_t elapsed) {
    if (isDead()) {
        if (elapsed_left_bigger_ == 0)
            return;
        int chng = (elapsed + elapsed_left_smaller_) >> 1;
        elapsed_left_smaller_ = elapsed & 2;
        elapsed_left_bigger_ -= chng;
        if (elapsed_left_bigger_ < 0) {
            chng += elapsed_left_bigger_;
            elapsed_left_bigger_ = 0;
        }
        int z = pos_.tz * 128 + pos_.oz - chng;
        pos_.tz = z / 128;
        pos_.oz = z % 128;
        return;
    }

    int chng = (elapsed + elapsed_left_smaller_) >> 2;
    elapsed_left_smaller_ = elapsed & 4;
    if (chng) {
        int oz = pos_.oz + chng * up_down_;
        if (oz > 127) {
            oz = 127 - (oz & 0x7F);
            up_down_ -= 2;
        } else if (oz < 64) {
            oz = 64 + (64 - oz);
            up_down_ += 2;
        }
        pos_.oz = oz;
    }

    if (colorTimer_.update(elapsed)) {
        animOffset_++;
        if (animOffset_ >= kSemaphoreMaxColorAnim)
            animOffset_ = 0;
    }
}

/*!
 * Implementation for the Semaphore.
 * \param d Damage information
 */
void Semaphore::handleHit(DamageToInflict &d) {
    if (isAlive() &&
        (d.dtype == kDmgTypeLaser || d.dtype == kDmgTypeExplosion)) {
        decreaseHealth(d.dvalue);
        if (isDead()) {
            animOffset_ = kSemaphoreDamagedOffset;
            // To make this thing reach the ground need to get solid surface 0x0F
            Mission * pMission = g_missionCtrl.mission();
            int z = pos_.tz;
            int indx = pos_.tx + pos_.ty * pMission->mmax_x_ + pos_.tz * pMission->mmax_m_xy;
            elapsed_left_bigger_ = 0;
            while (z != 0) {
                z--;
                indx -= pMission->mmax_m_xy;
                int twd = pMission->mtsurfaces_[indx];
                if (twd == 0x0F) {
                    elapsed_left_bigger_ = (pos_.tz - z) * 128 + pos_.oz;
                    break;
                }
            }
            setExcludedFromBlockers(true);
        }
    }
}

void Semaphore::draw(const Point2D &screenPos) {
    animationPlayer_->draw(addOffs(screenPos), animOffset_);
}

AnimWindow::AnimWindow(uint16_t anId, Map *pMap, uint16_t anim) : Static(anId, pMap, smt_AnimatedWindow) {
    setExcludedFromBlockers(true);
    //setFramesPerSec(4);
    animLigthOff_ = animationPlayer_->addAnimation(anim);
    animLigthSwitching_ = animationPlayer_->addAnimation(anim + 2);
    animPedAppears_ = animationPlayer_->addAnimation(anim + 4);
    animShowPed_ = animationPlayer_->addAnimation(anim + 6);
    animPedDisappears_ = animationPlayer_->addAnimation(anim + 8);
    animLightOn_ = animationPlayer_->addAnimation(anim);
}

void AnimWindow::draw(const Point2D &screenPos) {
    // When light is on, don't draw window
    // because lighted window is part of the map
    if (animationPlayer_->isCurrentAnimation(animLightOn_))
        return;

    animationPlayer_->draw(addOffs(screenPos), 0);
}

void AnimWindow::handleAnimationEnded() {
    if (animationPlayer_->isCurrentAnimation(animLigthOff_)) {
        // decide to start switching lights on
        // or continue being in dark
        if (rand() % 100 > 60) {
            playLightOffAnimation();
        } else {
            playLightSwitchingAnimation();
        }
    } else if (animationPlayer_->isCurrentAnimation(animLigthSwitching_)) {
        playLightOnAnimation();
    } else if (animationPlayer_->isCurrentAnimation(animPedAppears_)) {
        playShowPedAnimation();
    } else if (animationPlayer_->isCurrentAnimation(animShowPed_)) {
        // continue showing ped or hide it
        if (rand() % 100 > 50) {
            playShowPedAnimation();
        } else {
            animationPlayer_->play(animPedDisappears_);
        }
    } else if (animationPlayer_->isCurrentAnimation(animPedDisappears_)) {
        playLightOnAnimation();
    } else if (animationPlayer_->isCurrentAnimation(animLightOn_)) {
        // we will continue showing lightson or switch
        // lights off or show ped
        int rnd_v = rand() % 100;
        if (rnd_v > 80) {
            playLightOnAnimation();
        } else if (rnd_v > 60) {
            animationPlayer_->play(animPedAppears_);
        } else if (rnd_v > 10) {
            playLightOffAnimation();
        } else {
            playLightSwitchingAnimation();
        }
    }
}

}
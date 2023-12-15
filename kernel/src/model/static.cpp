/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2016  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "fs-kernel/model/static.h"

#include "fs-utils/common.h"
#include "fs-utils/log/log.h"
#include "fs-engine/gfx/spritemanager.h"
#include "fs-kernel/mgr/missionmanager.h"
#include "fs-kernel/model/ped.h"

const int Static::kStaticOrientation1 = 0;
const int Static::kStaticOrientation2 = 2;

Static *Static::loadInstance(uint8 * data, uint16 id, Map *pMap)
{
    LevelData::Statics * gamdata =
        (LevelData::Statics *) data;
    Static *s = NULL;

    // TODO: verify whether object description is correct
    // subtype for doors, use instead orientation?

    // NOTE: objects states are usually mixed with type,
    // no separation between object type and its state
    uint16 curanim = READ_LE_UINT16(gamdata->index_current_anim);
    uint16 baseanim = READ_LE_UINT16(gamdata->index_base_anim);
    uint16 curframe = READ_LE_UINT16(gamdata->index_current_frame);
    switch(gamdata->sub_type) {
        case 0x01:
            // phone booth
            s = new EtcObj(id, pMap, curanim, curanim, curanim);
            s->setSizeX(128);
            s->setSizeY(128);
            s->setSizeZ(128);
            break;
        case 0x05:// 1040-1043, 1044 - damaged
            // crossroad things
            s = new Semaphore(id, pMap, 1040, 1044);
            s->setSizeX(48);
            s->setSizeY(48);
            s->setSizeZ(48);
            s->state_ = sttsem_Stt0;
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x06:
            // crossroad things
            s = new Semaphore(id, pMap, 1040, 1044);
            s->setSizeX(48);
            s->setSizeY(48);
            s->setSizeZ(48);
            s->state_ = sttsem_Stt1;
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x07:
            // crossroad things
            s = new Semaphore(id, pMap, 1040, 1044);
            s->setSizeX(48);
            s->setSizeY(48);
            s->setSizeZ(48);
            s->state_ = sttsem_Stt2;
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x08:
            // crossroad things
            s = new Semaphore(id, pMap, 1040, 1044);
            s->setSizeX(48);
            s->setSizeY(48);
            s->setSizeZ(48);
            s->state_ = sttsem_Stt3;
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x0B:
            // 0x0270 animation, is this object present in original game?
            //s = new EtcObj(m, curanim, curanim, curanim);
            //printf("0x0B anim %X\n", curanim);
            break;
        case 0x0A:
            s = new NeonSign(id, pMap, curanim);
            s->setFrame(g_SpriteMgr.getFrameFromFrameIndx(curframe));
            s->setExcludedFromBlockers(true);
            s->setSizeX(32);
            s->setSizeY(1);
            s->setSizeZ(48);
            break;
        case 0x0C: // closed door
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, pMap, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(256);
                s->setSizeY(1);
                s->setSizeZ(196);
            } else {
                baseanim++;
                s = new Door(id, pMap, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(1);
                s->setSizeY(256);
                s->setSizeZ(196);
            }
            break;
        case 0x0D: // closed door
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, pMap, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(256);
                s->setSizeY(1);
                s->setSizeZ(196);
            } else {
                baseanim++;
                s = new Door(id, pMap, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(1);
                s->setSizeY(256);
                s->setSizeZ(196);
            }
            break;
        case 0x0E: // opening doors, not open
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, pMap, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(256);
                s->setSizeY(1);
                s->setSizeZ(196);
            } else {
                baseanim++;
                s = new Door(id, pMap, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(1);
                s->setSizeY(256);
                s->setSizeZ(196);
            }
            s->state_ = sttdoor_Opening;
            break;
        case 0x0F: // opening doors, not open
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80
                || gamdata->orientation == 0x7E || gamdata->orientation == 0xFE) {
                s = new Door(id, pMap, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(256);
                s->setSizeY(1);
                s->setSizeZ(196);
            } else {
                baseanim++;
                s = new Door(id, pMap, baseanim, baseanim + 2, baseanim + 4, baseanim + 6);
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(1);
                s->setSizeY(256);
                s->setSizeZ(196);
            }
            s->state_ = sttdoor_Opening;
            break;
        case 0x11:
            // ???? what is this
            //s = new EtcObj(m, bas, curanim, curanim);
            //printf("0x11 anim %X\n", curanim);
            break;
        case 0x12:
            // open window
            s = new WindowObj(id, pMap, curanim - 2, curanim, curanim + 2, curanim + 4);
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80) {
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(96);
                s->setSizeY(4);
                s->setSizeZ(96);
            } else {
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(4);
                s->setSizeY(96);
                s->setSizeZ(96);
            }
            s->setHealth(1);
            s->setStartHealth(1);
            s->state_ = Static::sttwnd_Open;
            break;
        case 0x13:
            // closed window
            s = new WindowObj(id, pMap, curanim, curanim + 2, curanim + 4, curanim + 6);
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80) {
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(96);
                s->setSizeY(4);
                s->setSizeZ(96);
            } else {
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(4);
                s->setSizeY(96);
                s->setSizeZ(96);
            }
            s->setHealth(1);
            s->setStartHealth(1);
            s->state_ = Static::sttwnd_Closed;
            break;
        case 0x15:
            // damaged window
            s = new WindowObj(id, pMap, curanim - 6, curanim - 4, curanim - 2, curanim);
            s->setExcludedFromBlockers(true);
            s->setHealth(0);
            s->setStartHealth(1);
            s->state_ = Static::sttwnd_Damaged;
            break;
        case 0x16:
            // TODO: set state if damaged trees exist
            s = new Tree(id, pMap, curanim, curanim + 1, curanim + 2);
            s->setSizeX(64);
            s->setSizeY(64);
            s->setSizeZ(256);
            s->setHealth(1);
            s->setStartHealth(1);
            break;
        case 0x19:
            // trash bin
            s = new EtcObj(id, pMap, curanim, curanim, curanim);
            s->setSizeX(64);
            s->setSizeY(64);
            s->setSizeZ(96);
            break;
        case 0x1A:
            // mail box
            s = new EtcObj(id, pMap, curanim, curanim, curanim);
            s->setSizeX(64);
            s->setSizeY(64);
            s->setSizeZ(96);
            break;
        case 0x1C:
            // ???? what is this?
            //s = new EtcObj(m, curanim, curanim, curanim);
            //printf("0x1C anim %X\n", curanim);
            break;
        case 0x1F:
            // advertisement on wall
            s = new EtcObj(id, pMap, curanim, curanim, curanim, smt_Advertisement);
            s->setExcludedFromBlockers(true);
            break;

        case 0x20:
            // window without light
            s = new AnimWindow(id, pMap, curanim);
            s->setStateMasks(sttawnd_LightOff);
            s->setTimeShowAnim(30000 + (rand() % 30000));
            break;
        case 0x21:
            // window light turns on
            s = new AnimWindow(id, pMap, curanim - 2);
            s->setTimeShowAnim(1000 + (rand() % 1000));
            s->setStateMasks(sttawnd_LightSwitching);

            // NOTE : 0x22 should have existed but it doesn't appear anywhere

        case 0x23:
            // window with person's shadow non animated,
            // even though on 1 map person appears I will ignore it
            s = new AnimWindow(id, pMap, 1959 + ((gamdata->orientation & 0x40) >> 5));
            s->setStateMasks(sttawnd_ShowPed);
            s->setTimeShowAnim(15000 + (rand() % 5000));
            break;
        case 0x24:
            // window with person's shadow, hides, actually animation
            // is of ped standing, but I will ignore it
            s = new AnimWindow(id, pMap, 1959 + 8 + ((gamdata->orientation & 0x40) >> 5));
            s->setStateMasks(sttawnd_PedDisappears);
            break;
        case 0x25:
            s = new AnimWindow(id, pMap, curanim);

            // NOTE : orientation, I assume, plays role of hidding object,
            // orientation 0x40, 0x80 are drawn (gamdata->desc always 7)
            // window without light
            s->setTimeShowAnim(30000 + (rand() % 30000));
            if (gamdata->orientation == 0x40 || gamdata->orientation == 0x80)
                s->setStateMasks(sttawnd_LightOff);
            else
                s->setStateMasks(sttawnd_LightOn);

            break;

        case 0x26:
            // 0x00,0x80 south - north = 0
            // 0x40,0xC0 weast - east = 2
            s = new LargeDoor(id, pMap, curanim, curanim + 1, curanim + 2);
            if (gamdata->orientation == 0x00 || gamdata->orientation == 0x80) {
                s->setOrientation(kStaticOrientation1);
                s->setSizeX(384);
                s->setSizeY(64);
                s->setSizeZ(192);
            } else {
                s->setOrientation(kStaticOrientation2);
                s->setSizeX(64);
                s->setSizeY(384);
                s->setSizeZ(192);
            }
            break;
#ifdef _DEBUG
        default:
            printf("uknown static object type %02X , %02X, %X\n",
                gamdata->sub_type, gamdata->orientation,
                READ_LE_UINT16(gamdata->index_current_frame));
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
        int z = READ_LE_UINT16(gamdata->mapposz) >> 7;
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

Door::Door(uint16 anId, Map *pMap, int anim, int closingAnim, int openAnim, int openingAnim) :
    Static(anId, pMap, Static::smt_Door), anim_(anim), closing_anim_(closingAnim),
        open_anim_(openAnim), opening_anim_(openingAnim) {
    state_ = Static::sttdoor_Closed;
}

void Door::draw(const Point2D &screenPos)
{
    g_SpriteMgr.drawFrame(anim_ + (state_ << 1), frame_, addOffs(screenPos));
}

bool Door::animate(int elapsed)
{
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

    bool changed = MapObject::animate(elapsed);
    switch(state_) {
        case Static::sttdoor_Open:
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
                    if (!pPed && state_ == Static::sttdoor_Open && (!found)) {
                        state_ = Static::sttdoor_Closing;
                        setExcludedFromBlockers(false);
                        frame_ = 0;
                    } else if (pPed && pPed->isAlive()){
                        state_ = Static::sttdoor_Open;
                        setExcludedFromBlockers(true);
                        found = true;
                        pPed->hold_on_.wayFree = 0;
                    }
                } while (pPed);
            }
            break;
        case Static::sttdoor_Closed:
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
                        state_ = Static::sttdoor_Opening;
                        setExcludedFromBlockers(false);
                        found = true;
                        frame_ = 0;
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
                        state_ = Static::sttdoor_Opening;
                        setExcludedFromBlockers(false);
                        found = true;
                        frame_ = 0;
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
        case Static::sttdoor_Closing:
            if (frame_ >= g_SpriteMgr.lastFrame(closing_anim_)) {
                state_ = Static::sttdoor_Closed;
                setExcludedFromBlockers(false);
                frame_ = 0;
            }
            break;
        case Static::sttdoor_Opening:
            if (frame_ >= g_SpriteMgr.lastFrame(opening_anim_)) {
                state_ = Static::sttdoor_Open;
                setExcludedFromBlockers(true);
                frame_ = 0;
            }
            break;
    }
    return changed;
}

bool Door::isPathBlocker()
{
    return state_ != Static::sttdoor_Open;
}


LargeDoor::LargeDoor(uint16 anId, Map *pMap, int anim, int closingAnim, int openingAnim):
        Static(anId, pMap, Static::smt_LargeDoor), anim_(anim),
        closing_anim_(closingAnim), opening_anim_(openingAnim) {
    state_ = Static::sttdoor_Closed;
}

void LargeDoor::draw(const Point2D &screenPos)
{
    Point2D posWithOffs = addOffs(screenPos);
    switch(state_) {
        case Static::sttdoor_Open:
            break;
        case Static::sttdoor_Closing:
            g_SpriteMgr.drawFrame(closing_anim_, frame_, posWithOffs);
            break;
        case Static::sttdoor_Closed:
            g_SpriteMgr.drawFrame(anim_, frame_, posWithOffs);
            break;
        case Static::sttdoor_Opening:
            g_SpriteMgr.drawFrame(opening_anim_, frame_, posWithOffs);
            break;
    }
}

bool LargeDoor::animate(int elapsed)
{
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

    bool changed = MapObject::animate(elapsed);
    uint32 cur_state = state_;
    switch(state_) {
        case Static::sttdoor_Open:
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
                    state_ = Static::sttdoor_Closing;
                    setExcludedFromBlockers(false);
                } else if (pVehicle){
                    state_ = Static::sttdoor_Open;
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
                    state_ = Static::sttdoor_Closing;
                    setExcludedFromBlockers(false);
                } else if (pVehicle){
                    state_ = Static::sttdoor_Open;
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
                            state_ = Static::sttdoor_Open;
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
                            state_ = Static::sttdoor_Open;
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
                            state_ = Static::sttdoor_Open;
                            setExcludedFromBlockers(true);
                            found = true;
                        }
                    }
                } while (pPed);
            }
            if (state_ == Static::sttdoor_Open) {
                for (std::vector<PedInstance *>::iterator it = found_peds.begin();
                    it != found_peds.end(); ++it )
                {
                    (*it)->hold_on_.wayFree = 0;
                }
                for (std::vector<PedInstance *>::iterator it = found_peds_mid.begin();
                    it != found_peds_mid.end(); ++it )
                {
                    (*it)->hold_on_.wayFree = 0;
                }
            } else {
                for (std::vector<PedInstance *>::iterator it = found_peds.begin();
                    it != found_peds.end(); ++it )
                {
                    pPed = *it;
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
                for (std::vector<PedInstance *>::iterator it = found_peds_mid.begin();
                    it != found_peds_mid.end(); ++it )
                {
                    pPed = *it;
                    fs_dmg::DamageToInflict d;
                    d.dtype = fs_dmg::kDmgTypeCollision;
                    d.d_owner = NULL;
                    d.dvalue = 1024;
                    d.ddir = -1;
                    pPed->handleHit(d);
                }
            }
            break;
        case Static::sttdoor_Closed:
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
                    state_ = Static::sttdoor_Opening;
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
                    state_ = Static::sttdoor_Opening;
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
                            state_ = Static::sttdoor_Opening;
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
                            state_ = Static::sttdoor_Opening;
                            setExcludedFromBlockers(false);
                            found = true;
                        }
                    }
                } while (pPed);
            }
            set_wayFree = state_ == Static::sttdoor_Opening ? 1 : 2;
            for (std::vector<PedInstance *>::iterator it = found_peds.begin();
                it != found_peds.end(); ++it )
            {
                pPed = *it;
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
        case Static::sttdoor_Closing:
            if (frame_ >= g_SpriteMgr.lastFrame(closing_anim_)) {
                state_ = Static::sttdoor_Closed;
                setExcludedFromBlockers(false);
            }
        case Static::sttdoor_Opening:
            if (state_ == Static::sttdoor_Opening
                && frame_ >= g_SpriteMgr.lastFrame(opening_anim_))
            {
                state_ = Static::sttdoor_Open;
                setExcludedFromBlockers(true);
            }
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
            set_wayFree = state_ == Static::sttdoor_Opening ? 1 : 2;
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
            for (std::vector<PedInstance *>::iterator it = found_peds.begin();
                it != found_peds.end(); ++it )
            {
                pPed = *it;
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
    if (cur_state != state_)
        frame_ = 0;
    return changed;
}

bool LargeDoor::isPathBlocker()
{
    return state_ != Static::sttdoor_Open;
}


Tree::Tree(uint16 anId, Map *pMap, int anim, int burningAnim, int damagedAnim) :
        Static(anId, pMap, Static::smt_Tree), anim_(anim), burning_anim_(burningAnim),
        damaged_anim_(damagedAnim) {
    state_ = Static::stttree_Healthy;
}

void Tree::draw(const Point2D &screenPos)
{
    Point2D posWithOffs = addOffs(screenPos);
    switch (state_) {
        case Static::stttree_Healthy:
            g_SpriteMgr.drawFrame(anim_, frame_, posWithOffs);
            break;
        case Static::stttree_Burning:
            g_SpriteMgr.drawFrame(burning_anim_, frame_, posWithOffs);
            break;
        case Static::stttree_Damaged:
            g_SpriteMgr.drawFrame(damaged_anim_, frame_, posWithOffs);
            break;
    }
}

bool Tree::animate(int elapsed) {

    if (state_ == Static::stttree_Burning) {
        if (!(leftTimeShowAnim(elapsed))) {
            state_ = Static::stttree_Damaged;
            frame_ = 0;
            setFramesPerSec(2);
            return true;
        }
    }

    return MapObject::animate(elapsed);
}

/*!
 * Implementation for the Tree. Tree burns only when hit by laser of fire.
 * \param d Damage information
 */
void Tree::handleHit(fs_dmg::DamageToInflict &d) {
    if (isAlive() &&
        (d.dtype == fs_dmg::kDmgTypeLaser || d.dtype == fs_dmg::kDmgTypeBurn || d.dtype == fs_dmg::kDmgTypeExplosion)) {
        decreaseHealth(d.dvalue);
        if (isDead()) {
            state_ = Static::stttree_Burning;
            setTimeShowAnim(10000);
            setExcludedFromBlockers(true);
        }
    }
}

WindowObj::WindowObj(uint16 anId, Map *pMap, int anim, int openAnim, int breakingAnim,
                     int damagedAnim) :
        Static(anId, pMap, Static::smt_Window), anim_(anim), open_anim_(openAnim),
        breaking_anim_(breakingAnim), damaged_anim_(damagedAnim) {}

bool WindowObj::animate(int elapsed) {
    bool updated = MapObject::animate(elapsed);

    if (state_ == Static::sttwnd_Breaking
        && frame_ >= g_SpriteMgr.lastFrame(breaking_anim_)
    ) {
        state_ = sttwnd_Damaged;
        updated = true;
    }
    return updated;
}

void WindowObj::draw(const Point2D &screenPos)
{
    g_SpriteMgr.drawFrame(anim_ + (state_ << 1), frame_, addOffs(screenPos));
}

/*!
 * Implementation for the Tree. Tree burns only when hit by laser of fire.
 * \param d Damage information
 */
void WindowObj::handleHit(fs_dmg::DamageToInflict &d) {
    if (isAlive() &&
        (d.dtype == fs_dmg::kDmgTypeBullet || d.dtype == fs_dmg::kDmgTypeExplosion)) {
        decreaseHealth(d.dvalue);
        if (isDead()) {
            state_ = Static::sttwnd_Breaking;
            setExcludedFromBlockers(true);
            frame_ = 0;
            setFramesPerSec(6);
        }
    }
}

EtcObj::EtcObj(uint16 anId, Map *pMap, int anim, int burningAnim, int damagedAnim, StaticType aType) :
        Static(anId, pMap, aType), anim_(anim), burning_anim_(burningAnim),
        damaged_anim_(damagedAnim) {}

void EtcObj::draw(const Point2D &screenPos)
{
    g_SpriteMgr.drawFrame(anim_, frame_, addOffs(screenPos));
}

NeonSign::NeonSign(uint16 anId, Map *pMap, int anim) : Static(anId, pMap, Static::smt_NeonSign) {
    anim_ = anim;
}

void NeonSign::draw(const Point2D &screenPos)
{
    g_SpriteMgr.drawFrame(anim_, frame_, addOffs(screenPos));
}

Semaphore::Semaphore(uint16 anId, Map *pMap, int anim, int damagedAnim) :
        Static(anId, pMap, Static::smt_Semaphore), anim_(anim),
        damaged_anim_(damagedAnim), elapsed_left_smaller_(0),
        elapsed_left_bigger_(0), up_down_(1)
{
    setFramesPerSec(2);
}

bool Semaphore::animate(int elapsed) {
    if (state_ == Static::sttsem_Damaged) {
        if (elapsed_left_bigger_ == 0)
            return false;
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
        return true;
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

    chng = (elapsed + elapsed_left_bigger_) >> 6;
    elapsed_left_bigger_ = elapsed & 63;
    if (chng) {
        // Direction is used as storage for animation change, not my idea
        dir_ += chng;
        dir_ &= 0xFF;
        state_ = dir_ >> 6;
        state_++;
        if (state_ > Static::sttsem_Stt3)
            state_ = Static::sttsem_Stt0;
    }

    return MapObject::animate(elapsed);
}

/*!
 * Implementation for the Semaphore.
 * \param d Damage information
 */
void Semaphore::handleHit(fs_dmg::DamageToInflict &d) {
    if (isAlive() &&
        (d.dtype == fs_dmg::kDmgTypeLaser || d.dtype == fs_dmg::kDmgTypeExplosion)) {
        decreaseHealth(d.dvalue);
        if (isDead()) {
            state_ = Static::sttsem_Damaged;
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

void Semaphore::draw(const Point2D &screenPos)
{
    g_SpriteMgr.drawFrame(anim_ +  state_, frame_, addOffs(screenPos));
}

AnimWindow::AnimWindow(uint16 anId, Map *pMap, int anim) : Static(anId, pMap, smt_AnimatedWindow) {
    setExcludedFromBlockers(true);
    setFramesPerSec(4);
    anim_ = anim;
}

void AnimWindow::draw(const Point2D &screenPos)
{
    // When light is on, don't draw window
    // because lighted window is part of the map
    if (state_ == Static::sttawnd_LightOn)
        return;
    g_SpriteMgr.drawFrame(anim_ + (state_ << 1), frame_, addOffs(screenPos));
}

bool AnimWindow::animate(int elapsed)
{
    switch (state_) {
        case Static::sttawnd_LightOff:
            if (!leftTimeShowAnim(elapsed)) {
                // decide to start switching lights on
                // or continue being in dark
                if (rand() % 100 > 60) {
                    setTimeShowAnim(30000 + (rand() % 30000));
                } else {
                    state_ = Static::sttawnd_LightSwitching;
                    frame_ = 0;
                    setTimeShowAnim(1000 + (rand() % 1000));
                }
            }
            break;
        case Static::sttawnd_LightSwitching:
            if (!leftTimeShowAnim(elapsed)) {
                state_ = Static::sttawnd_LightOn;
                setTimeShowAnim(30000 + (rand() % 30000));
            }
            break;
        case Static::sttawnd_PedAppears:
            if (frame_ >= g_SpriteMgr.lastFrame(anim_
                + (Static::sttawnd_PedAppears << 1)))
            {
                state_ = Static::sttawnd_ShowPed;
                setTimeShowAnim(15000 + (rand() % 15000));
            }
            break;
        case Static::sttawnd_ShowPed:
            if (!leftTimeShowAnim(elapsed)) {
                // continue showing ped or hide it
                if (rand() % 100 > 50) {
                    setTimeShowAnim(15000 + (rand() % 5000));
                } else {
                    frame_ = 0;
                    state_ = Static::sttawnd_PedDisappears;
                }
            }
            break;
        case Static::sttawnd_PedDisappears:
            if (frame_ >= g_SpriteMgr.lastFrame(anim_
                + (Static::sttawnd_PedDisappears << 1)))
            {
                state_ = Static::sttawnd_LightOn;
                setTimeShowAnim(30000 + (rand() % 30000));
            }
            break;
        case Static::sttawnd_LightOn:
            if (!leftTimeShowAnim(elapsed)) {
                // we will continue showing lightson or switch
                // lights off or show ped
                int rnd_v = rand() % 100;
                if (rnd_v > 80) {
                    setTimeShowAnim(30000 + (rand() % 30000));
                } else if (rnd_v > 60) {
                    frame_ = 0;
                    state_ = Static::sttawnd_PedAppears;
                } else if (rnd_v > 10) {
                    state_ = Static::sttawnd_LightOff;
                    setTimeShowAnim(30000 + (rand() % 30000));
                } else {
                    frame_ = 0;
                    state_ = Static::sttawnd_LightSwitching;
                    setTimeShowAnim(1000 + (rand() % 1000));
                }
            }
            break;
    }

    return MapObject::animate(elapsed);
    return false;
}


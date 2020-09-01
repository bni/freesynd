/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#include <stdio.h>
#include <assert.h>

#include "utils/log.h"
#include "pedmanager.h"
#include "core/gamecontroller.h"

PedManager::PedManager()
{
}

void PedManager::initAnimation(Ped *pedanim, unsigned short baseAnim)
{
    if (baseAnim == 1) {
        pedanim->setStandAnim(Weapon::Unarmed_Anim, 0 + baseAnim);
        pedanim->setWalkAnim(Weapon::Unarmed_Anim, 8 + baseAnim);
        pedanim->setStandAnim(Weapon::EnergyShield_Anim, 16 + baseAnim);
        pedanim->setStandAnim(Weapon::Pistol_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::Uzi_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::Shotgun_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::Gauss_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::Minigun_Anim, 32 + baseAnim);
        pedanim->setStandAnim(Weapon::Laser_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::Flamer_Anim, 40 + baseAnim);
        pedanim->setStandAnim(Weapon::LongRange_Anim, 48 + baseAnim);
        pedanim->setWalkAnim(Weapon::EnergyShield_Anim, 56 + baseAnim);
        pedanim->setWalkAnim(Weapon::Pistol_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::Uzi_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::Shotgun_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::Gauss_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::Minigun_Anim, 72 + baseAnim);
        pedanim->setWalkAnim(Weapon::Laser_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::Flamer_Anim, 80 + baseAnim);
        pedanim->setWalkAnim(Weapon::LongRange_Anim, 88 + baseAnim);
        // I think 97 was the original pistol shoot anim, but it wasn't sexy enough.
        //pedanim->setStandFireAnim(Weapon::Laser_Anim, 96 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Pistol_Anim, 104 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Uzi_Anim, 104 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Shotgun_Anim, 104 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Gauss_Anim, 104 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Minigun_Anim, 112 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Laser_Anim, 120 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Flamer_Anim, 128 + baseAnim);
        pedanim->setStandFireAnim(Weapon::LongRange_Anim, 136 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Pistol_Anim, 152 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Uzi_Anim, 152 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Shotgun_Anim, 152 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Gauss_Anim, 152 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Minigun_Anim, 160 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Laser_Anim, 168 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Flamer_Anim, 176 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::LongRange_Anim, 184 + baseAnim);
    } else {
        // NOTE: peds other then agents have pistol like animations for
        // all weapons
        pedanim->setStandAnim(Weapon::Unarmed_Anim, 0 + baseAnim);
        pedanim->setWalkAnim(Weapon::Unarmed_Anim, 8 + baseAnim);
        pedanim->setStandAnim(Weapon::EnergyShield_Anim, 0 + baseAnim);
        pedanim->setStandAnim(Weapon::Pistol_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::Uzi_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::Shotgun_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::Gauss_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::Minigun_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::Laser_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::Flamer_Anim, 24 + baseAnim);
        pedanim->setStandAnim(Weapon::LongRange_Anim, 24 + baseAnim);
        pedanim->setWalkAnim(Weapon::EnergyShield_Anim, 24 + baseAnim);
        pedanim->setWalkAnim(Weapon::Pistol_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::Uzi_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::Shotgun_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::Gauss_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::Minigun_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::Laser_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::Flamer_Anim, 64 + baseAnim);
        pedanim->setWalkAnim(Weapon::LongRange_Anim, 64 + baseAnim);
        // I think 97 was the original pistol shoot anim, but it wasn't sexy enough.
        //pedanim->setStandFireAnim(Weapon::Laser_Anim, 96 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Pistol_Anim, 104 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Uzi_Anim, 104 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Shotgun_Anim, 104 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Gauss_Anim, 104 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Minigun_Anim, 104 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Laser_Anim, 104 + baseAnim);
        pedanim->setStandFireAnim(Weapon::Flamer_Anim, 104 + baseAnim);
        pedanim->setStandFireAnim(Weapon::LongRange_Anim, 104 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Pistol_Anim, 104 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Uzi_Anim, 104 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Shotgun_Anim, 104 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Gauss_Anim, 104 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Minigun_Anim, 104 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Laser_Anim, 104 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::Flamer_Anim, 104 + baseAnim);
        pedanim->setWalkFireAnim(Weapon::LongRange_Anim, 104 + baseAnim);
    }
    pedanim->setPickupAnim(192 + baseAnim);
    pedanim->setHitAnim(193 + baseAnim);
    pedanim->setVaporizeAnim(197 + baseAnim);
    pedanim->setSinkAnim(201 + baseAnim);
    // 203 - die, 204 - dead, agent only
    pedanim->setDieAgentAnim(203 + baseAnim);
    pedanim->setDeadAgentAnim(205);
    pedanim->setDieAnim(205 + baseAnim);
    pedanim->setDeadAnim(206 + baseAnim);
    // when this burning should be used?
    pedanim->setStandBurnAnim(208);
    // this one used when hit with flamethrower
    pedanim->setWalkBurnAnim(209);
    pedanim->setDieBurnAnim(210);
    pedanim->setSmokeBurnAnim(211);
    pedanim->setDeadBurnAnim(828);
    pedanim->setPersuadeAnim(236);
}

/*!
 * Instanciate a PedInstance from the given data.
 * \param gamdata
 * \param ped_idx Index of the ped in the file.
 * \param map id of the map
 * \return NULL if the ped could not be created.
 */
PedInstance *PedManager::loadInstance(const LevelData::People & gamdata, uint16 ped_idx, int map, uint32 playerGroupId)
{
    if(gamdata.type == 0x0 ||
        gamdata.location == LevelData::kPeopleLocNotVisible ||
        gamdata.location == LevelData::kPeopleLocAboveWalkSurf)
        return NULL;

    bool isOurAgent = ped_idx < AgentManager::kMaxSlot;
    if (isOurAgent && !g_gameCtrl.agents().isSquadSlotActive(ped_idx)) {
        // Creates agent only if he's active
        return NULL;
    }if (ped_idx >= 4 && ped_idx < 8) {
        // Ped between index 4 and 7 are not used
        // In original game must be the place where persuaded agents were stored
        return NULL;
    }

    Ped *pedanim = new Ped();
    initAnimation(pedanim, READ_LE_UINT16(gamdata.index_base_anim));
    PedInstance *newped = new PedInstance(pedanim, ped_idx, map, isOurAgent);

    int hp = READ_LE_INT16(gamdata.health);
    if (isOurAgent) {
        // not in all missions our agents health is 16, this fixes it
        hp = PedInstance::kAgentMaxHealth;
    }else if (hp <= 0) {
        hp = 2;
    }

    newped->setStartHealth(hp);

    newped->setDirection(gamdata.orientation);
    if (gamdata.state == LevelData::kPeopleStateDead) {
        newped->setDrawnAnim(PedInstance::ad_DeadAnim);
        newped->setHealth(-1);
        newped->setStateMasks(PedInstance::pa_smDead);
    } else {
        newped->setHealth(hp);
        newped->setStateMasks(PedInstance::pa_smStanding);
    }
    // this is tile based Z we get, realword Z is in gamdata,
    // for correct calculations of viewpoint, target hit etc.
    // Zr = (Zt * 128) / 256
    int z = READ_LE_UINT16(gamdata.mapposz) >> 7;
    // some peds have z = 0 - map paraguay
    int oz = gamdata.mapposz[0] & 0x7F;
    newped->setSizeX(32);
    newped->setSizeY(32);
    newped->setSizeZ(256);
    //printf("x %i y %i z %i ox %i oy %i oz %i\n", gamdata.mapposx[1], gamdata.mapposy[1], z, gamdata.mapposx[0], gamdata.mapposy[0], oz);
    newped->setPosition(gamdata.mapposx[1], gamdata.mapposy[1],
                        z, gamdata.mapposx[0],
                        gamdata.mapposy[0], oz);
    newped->setTypeFromValue(gamdata.type_ped);

    newped->setAllAdrenaLevels(gamdata.adrena_amount,
        gamdata.adrena_dependency, gamdata.adrena_effect);
    newped->setAllInteliLevels(gamdata.inteli_amount,
        gamdata.inteli_dependency, gamdata.inteli_effect);
    newped->setAllPercepLevels(gamdata.percep_amount,
        gamdata.percep_dependency, gamdata.percep_effect);

    if (isOurAgent) {
        // We're loading one of our agents
        Agent *pAg = g_gameCtrl.agents().squadMember(ped_idx);
        initOurAgent(pAg, playerGroupId, newped);
    } else {
        unsigned int mt = newped->type();
        newped->setObjGroupDef(mt);
        if (mt == PedInstance::og_dmAgent) {
            initEnemyAgent(newped);
        } else if (mt == PedInstance::og_dmGuard) {
            initGuard(newped);
        } else if (mt == PedInstance::og_dmPolice) {
            initPolice(newped);
        } else if (mt == PedInstance::og_dmCivilian) {
            initCivilian(newped);
        } else if (mt == PedInstance::og_dmCriminal) {
            initCriminal(newped);
        }
        newped->setSightRange(7 * 256);
    }

    return newped;
}

/*!
 * Initialize the ped instance as one of our agent.
 * \param pAgent The agent reference
 * \param obj_group_id Id of the agent's group.
 * \param pPed The ped to initialize
 */
void PedManager::initOurAgent(Agent *pAgent, unsigned int obj_group_id, PedInstance *pPed) {
    LOG(Log::k_FLG_GAME, "PedManager","initOurAgent", ("Create player agent with id %d", pPed->id()))

    while (pAgent->numWeapons()) {
        WeaponInstance *wi = pAgent->removeWeaponAtIndex(0);
        pPed->addWeapon(wi);
        wi->setOwner(pPed);
    }
    *((ModOwner *)pPed) = *((ModOwner *)pAgent);

    pPed->setObjGroupID(obj_group_id);
    pPed->setObjGroupDef(PedInstance::og_dmAgent);
    pPed->addEnemyGroupDef(2);
    pPed->addEnemyGroupDef(3);
    pPed->setSightRange(7 * 256);
    pPed->setBaseSpeed(256);
    pPed->setTimeBeforeCheck(400);
    pPed->setBaseModAcc(0.5);

    // Set components of behaviour for our agent
    pPed->behaviour().addComponent(new CommonAgentBehaviourComponent(pPed));
    pPed->behaviour().addComponent(new PersuaderBehaviourComponent());
}

/*!
 * Initialize the ped instance as an enemy agent.
 * \param p_agent The agent reference
 * \param obj_group_id Id of the agent's group.
 * \param pPed The ped to initialize
 */
void PedManager::initEnemyAgent(PedInstance *pPed) {
    LOG(Log::k_FLG_GAME, "PedManager","initEnemyAgent", ("Create enemy agent with id %d", pPed->id()))

    pPed->setObjGroupID(2);
    pPed->addEnemyGroupDef(1);
    pPed->setBaseSpeed(256);
    // enemies get top version of mods
    pPed->addMod(g_gameCtrl.mods().getHighestVersion(Mod::MOD_LEGS));
    pPed->addMod(g_gameCtrl.mods().getHighestVersion(Mod::MOD_LEGS));
    pPed->addMod(g_gameCtrl.mods().getHighestVersion(Mod::MOD_ARMS));
    pPed->addMod(g_gameCtrl.mods().getHighestVersion(Mod::MOD_CHEST));
    pPed->addMod(g_gameCtrl.mods().getHighestVersion(Mod::MOD_HEART));
    pPed->addMod(g_gameCtrl.mods().getHighestVersion(Mod::MOD_EYES));
    pPed->addMod(g_gameCtrl.mods().getHighestVersion(Mod::MOD_BRAIN));
    pPed->setTimeBeforeCheck(400);
    pPed->setBaseModAcc(0.5);

    pPed->behaviour().addComponent(new PlayerHostileBehaviourComponent());
}

/*!
 * Initialize the ped instance as a guard.
 * \param p_agent The agent reference
 * \param obj_group_id Id of the agent's group.
 * \param pPed The ped to initialize
 */
void PedManager::initGuard(PedInstance *pPed) {
    LOG(Log::k_FLG_GAME, "PedManager","initGuard", ("Create guard with id %d", pPed->id()))

    pPed->setObjGroupID(3);
    pPed->addEnemyGroupDef(1);
    pPed->setBaseSpeed(192);
    pPed->setTimeBeforeCheck(300);
    pPed->setBaseModAcc(0.45);

    pPed->behaviour().addComponent(new PlayerHostileBehaviourComponent());
}

/*!
 * Initialize the ped instance as a police.
 * \param p_agent The agent reference
 * \param obj_group_id Id of the agent's group.
 * \param pPed The ped to initialize
 */
void PedManager::initPolice(PedInstance *pPed) {
    LOG(Log::k_FLG_GAME, "PedManager","initPolice", ("Create police with id %d", pPed->id()))

    pPed->setObjGroupID(4);
    pPed->setBaseSpeed(160);
    pPed->setTimeBeforeCheck(400);
    pPed->setBaseModAcc(0.4);

    pPed->behaviour().addComponent(new PoliceBehaviourComponent());
}

/*!
 * Initialize the ped instance as a civilian.
 * \param p_agent The agent reference
 * \param obj_group_id Id of the agent's group.
 * \param pPed The ped to initialize
 */
void PedManager::initCivilian(PedInstance *pPed) {
    LOG(Log::k_FLG_GAME, "PedManager","initCivilian", ("Create civilian with id %d", pPed->id()))

    pPed->setObjGroupID(5);
    pPed->addEnemyGroupDef(6);
    pPed->setBaseSpeed(128);
    pPed->setTimeBeforeCheck(600);
    pPed->setBaseModAcc(0.2);

    pPed->behaviour().addComponent(new PanicComponent());
}

/*!
 * Initialize the ped instance as a criminal.
 * \param p_agent The agent reference
 * \param obj_group_id Id of the agent's group.
 * \param pPed The ped to initialize
 */
void PedManager::initCriminal(PedInstance *pPed) {
    LOG(Log::k_FLG_GAME, "PedManager","initCriminal", ("Create criminal with id %d", pPed->id()))

    pPed->setObjGroupID(6);
    pPed->setBaseSpeed(128);
    pPed->setTimeBeforeCheck(500);
    pPed->setBaseModAcc(0.2);
}

/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net>
 *   Copyright (C) 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-kernel/mgr/pedmanager.h"

#include <stdio.h>
#include <assert.h>

#include "fs-utils/log/log.h"
#include "fs-kernel/mgr/agentmanager.h"
#include "fs-kernel/mgr/modmanager.h"

namespace fs_knl {

/*!
 * Instanciate a PedInstance from the given data.
 * \param gamdata
 * \param ped_idx Index of the ped in the file.
 * \param map id of the map
 * \return NULL if the ped could not be created.
 */
PedInstance *PedManager::loadInstance(const LevelData::People & gamdata, uint16_t ped_idx, Map *pMap, uint32_t playerGroupId)
{
    if(gamdata.type == 0x0 ||
        gamdata.location == LevelData::kPeopleLocNotVisible ||
        gamdata.location == LevelData::kPeopleLocAboveWalkSurf)
        return NULL;

    bool isOurAgent = ped_idx < Squad::kMaxSlot;
    if (isOurAgent && !g_agentMgr.isSquadSlotActive(ped_idx)) {
        // Creates agent only if he's active
        return NULL;
    }if (ped_idx >= 4 && ped_idx < 8) {
        // Ped between index 4 and 7 are not used
        // In original game must be the place where persuaded agents were stored
        return NULL;
    }

    PedInstance::PedType pedType = getTypeFromValue(gamdata.type_ped);
    PedInstance *newped = new PedInstance(ped_idx, pMap, pedType, isOurAgent, getDefaultSpeed(pedType));
    newped->setAnimations(fs_utl::READ_LE_UINT16(gamdata.index_base_anim));

    int hp = fs_utl::READ_LE_INT16(gamdata.health);
    if (isOurAgent) {
        // not in all missions our agents health is 16, this fixes it
        hp = PedInstance::kAgentMaxHealth;
    }else if (hp <= 0) {
        hp = 2;
    }

    newped->setStartHealth(hp);

    newped->setDirection(gamdata.orientation);
    if (gamdata.state == LevelData::kPeopleStateDead) {
        newped->playDeadAnimation();
        newped->setHealth(-1);
        newped->switchActionStateTo(PedInstance::pa_smDead);
    } else {
        newped->setHealth(hp);
        newped->goToState(PedInstance::pa_smStanding);
    }
    // this is tile based Z we get, realword Z is in gamdata,
    // for correct calculations of viewpoint, target hit etc.
    // Zr = (Zt * 128) / 256
    int z = fs_utl::READ_LE_UINT16(gamdata.mapposz) >> 7;
    // some peds have z = 0 - map paraguay
    int oz = gamdata.mapposz[0] & 0x7F;
    newped->setSize(32, 32, 256);
    //printf("x %i y %i z %i ox %i oy %i oz %i\n", gamdata.mapposx[1], gamdata.mapposy[1], z, gamdata.mapposx[0], gamdata.mapposy[0], oz);
    newped->setPosition(gamdata.mapposx[1], gamdata.mapposy[1],
                        z, gamdata.mapposx[0],
                        gamdata.mapposy[0], oz);

    newped->initAllLevelsForIPAType(IPAStim::Adrenaline,
        gamdata.adrena_amount, gamdata.adrena_dependency, gamdata.adrena_effect);
    newped->initAllLevelsForIPAType(IPAStim::Intelligence,
        gamdata.inteli_amount, gamdata.inteli_dependency, gamdata.inteli_effect);
    newped->initAllLevelsForIPAType(IPAStim::Perception,
        gamdata.percep_amount, gamdata.percep_dependency, gamdata.percep_effect);

    if (isOurAgent) {
        // We're loading one of our agents
        Agent *pAg = g_agentMgr.squadMember(ped_idx);
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
 * @brief 
 * @param value 
 */
PedInstance::PedType PedManager::getTypeFromValue(uint8_t value) {
    switch(value) {
    case 0x01:
        return PedInstance::kPedTypeCivilian;
    case 0x02:
        return PedInstance::kPedTypeAgent;
        break;
    case 0x04:
        return PedInstance::kPedTypePolice;
        break;
    case 0x08:
        return PedInstance::kPedTypeGuard;
        break;
    case 0x10:
        return PedInstance::kPedTypeCriminal;
        break;
    }

    return PedInstance::kPedTypeCivilian;
}

int PedManager::getDefaultSpeed(PedInstance::PedType type) {
    switch (type) {
        case PedInstance::kPedTypeCivilian :
            return 128;
        case PedInstance::kPedTypeAgent :
            return 256;
        case PedInstance::kPedTypePolice :
            return 160;
        case PedInstance::kPedTypeGuard :
            return 192;
        case PedInstance::kPedTypeCriminal:
            return 128;
        default:
            return 0;
    }
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
    // Scale sight range by Perception IPA (multiplier range 0.5–2.0)
    pPed->setSightRange(static_cast<int>(7 * 256 * pPed->perception().getMultiplier()));
    pPed->setTimeBeforeCheck(400);
    pPed->setBaseModAcc(0.5);

    // Set components of behaviour for our agent
    pPed->behaviour().addComponent(new CommonAgentBehaviourComponent(pPed));
    pPed->behaviour().addComponent(new PersuaderBehaviourComponent());
    pPed->behaviour().addComponent(new AgentDefenseBehaviourComponent());
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
    // enemies get top version of mods
    pPed->addMod(g_modMgr.getHighestVersion(Mod::MOD_LEGS));
    pPed->addMod(g_modMgr.getHighestVersion(Mod::MOD_LEGS));
    pPed->addMod(g_modMgr.getHighestVersion(Mod::MOD_ARMS));
    pPed->addMod(g_modMgr.getHighestVersion(Mod::MOD_CHEST));
    pPed->addMod(g_modMgr.getHighestVersion(Mod::MOD_HEART));
    pPed->addMod(g_modMgr.getHighestVersion(Mod::MOD_EYES));
    pPed->addMod(g_modMgr.getHighestVersion(Mod::MOD_BRAIN));
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
    pPed->setTimeBeforeCheck(500);
    pPed->setBaseModAcc(0.2);
}

}
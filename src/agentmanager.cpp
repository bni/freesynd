/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
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

#include "agentmanager.h"
#include "weaponmanager.h"
#include "modmanager.h"

const char * const g_AgentNames[] = {
    "AFSHAR",
    "ARNOLD",
    "BAIRD",
    "BALDWIN",
    "BLACK",
    "BOYD",
    "BOYESEN",
    "BRAZIER",
    "BROWN",
    "BUSH",
    "CARR",
    "CHRISMAS",
    "CLINTON",
    "COOPER",
    "CORPES",
    "COX",
    "DAWSON",
    "DONKIN",
    "DISKETT",
    "DUNNE",
    "EDGAR",
    "EVANS",
    "FAIRLEY",
    "FAWCETT",
    "FLINT",
    "FLOYD",
    "GRIFFITHS",
    "HARRIS",
    "HASTINGS",
    "HERBERT",
    "HICKMAN",
    "HICKS",
    "HILL",
    "JAMES",
    "JEFFERY",
    "JOESEPH",
    "JOHNSON",
    "JOHNSTON",
    "JONES",
    "LEWIS",
    "LINDSELL",
    "LOCKLEY",
    "MARTIN",
    "MCENTEE",
    "MCLAUGHIN",
    "MOLYNEUX",
    "MUNRO",
    "MORRIS",
    "MUMFORD",
    "NIXON",
    "PARKER",
    "PRATT",
    "REID",
    "RENNIE",
    "RICE",
    "RIPLEY",
    "ROBERTSON",
    "ROMANO",
    "SEAT",
    "SEN",
    "SHAW",
    "SIMMONS",
    "SNELLING",
    "TAYLOR",
    "TROWERS",
    "WEBLEY",
    "WELLESLEY",
    "WILD",
    "WILLIS",
    0
};
const int g_NumAgentNames = (sizeof(g_AgentNames) / sizeof(char *)) - 1; // minus one accounts for the NULL at the end of the names list

int const AgentManager::MAX_AGENT = 18;
const size_t AgentManager::kMaxSlot = 4;
const size_t AgentManager::kSlot1 = 0;
const size_t AgentManager::kSlot2 = 1;
const size_t AgentManager::kSlot3 = 2;
const size_t AgentManager::kSlot4 = 3;

AgentManager::AgentManager() {
    nextName_ = 0;
    for (int i = 0; i != MAX_AGENT; ++i)
        agents_.add(NULL);
}

void AgentManager::loadAgents() {
    // TODO : load names from file
}

void AgentManager::destroy() {
    for (int i = 0; i != MAX_AGENT; ++i) {
        if (agents_.get(i)) {
            delete agents_.get(i);
            agents_.setAt(i, NULL);
        }
    }

    clearSquad();
}

void AgentManager::reset(bool onlyWomen) {
    nextName_ = 0;
    // First delete existing agents
    destroy();

    // Then recreate the first 8 available agents
    for (size_t i = 0; i < 8; i++) {
        Agent * pAgent = new Agent(g_AgentNames[nextName_], onlyWomen ? true : ((i % 2) == 0));
        pAgent->addWeapon(WeaponInstance::createInstance(pWeaponManager_->getWeapon(Weapon::Pistol)));

        agents_.setAt(i, pAgent);
        // Adds the first 4 agents to the squad
        if (i < kMaxSlot) {
            setSquadMember(i, pAgent);
        }
        nextName_++;
    }
}

void AgentManager::destroyAgentSlot(size_t squadSlot) {
    Agent *p_agent = squadMember(squadSlot);
    setSquadMember(squadSlot, NULL);
    for (int inc = 0; inc < AgentManager::MAX_AGENT; inc++) {
        if (agent(inc) == p_agent) {
            delete agents_.get(inc);
            agents_.setAt(inc, NULL);
            return;
        }
    }
}

bool AgentManager::saveToFile(PortableFile &file) {
    file.write32(nextName_);
    for (int i=0; i<AgentManager::MAX_AGENT; i++) {
        Agent *pAgent = agents_.get(i);
        // This flag tells if there is an agent on this slot
        file.write8b(pAgent != 0);
        if (pAgent) {
            pAgent->saveToFile(file);
        }
    }

    // save current squad
    for (size_t i=0; i<kMaxSlot; i++) {
        Agent *pAgent = squadMember(i);
        int id = pAgent ? pAgent->getId() : 0;
        file.write32(id);
    }
    return true;
}

bool AgentManager::loadFromFile(PortableFile &infile, const FormatVersion& v) {
    nextName_ = infile.read32();
    for (int i=0; i<AgentManager::MAX_AGENT; i++) {
        bool isAgent = infile.read8b();
        if (isAgent) {
            Agent *pAgent = agents_.get(i);
            if (pAgent == NULL) {
                // Create an empty agent
                pAgent = new Agent("", true);
            }
            pAgent->loadFromFile(infile, v);

            // Mods
            int nb = infile.read32();
            for (int mIndex = 0; mIndex < nb; mIndex++) {
                int type = infile.read32();
                Mod::EModType mt = Mod::Unknown;
                Mod::EModVersion mv = Mod::MOD_V1;
                switch (type) {
                    case 0: mt = Mod::MOD_LEGS;break;
                    case 1: mt = Mod::MOD_ARMS;break;
                    case 2: mt = Mod::MOD_CHEST;break;
                    case 3: mt = Mod::MOD_HEART;break;
                    case 4: mt = Mod::MOD_EYES;break;
                    case 5: mt = Mod::MOD_BRAIN;break;
                    default: mt = Mod::Unknown;
                }

                int ver = infile.read32();
                switch (ver) {
                    case 0: mv = Mod::MOD_V1;break;
                    case 1: mv = Mod::MOD_V2;break;
                    case 2: mv = Mod::MOD_V3;break;
                    default: mv = Mod::MOD_V1;
                }
                // add the mod
                if (mt != Mod::Unknown) {
                    pAgent->addMod(pModManager_->getMod(mt, mv));
                }
            }
            // Weapons
            nb = infile.read32();
            for (int wIndex = 0; wIndex < nb; wIndex++) {
                int type = infile.read32();
                Weapon::WeaponType wt = Weapon::Unknown;
                switch (type) {
                    case Weapon::Persuadatron:
                        wt = Weapon::Persuadatron;
                        break;
                    case Weapon::Pistol:
                        wt = Weapon::Pistol;
                        break;
                    case Weapon::GaussGun:
                        wt = Weapon::GaussGun;
                        break;
                    case Weapon::Shotgun:
                        wt = Weapon::Shotgun;
                        break;
                    case Weapon::Uzi:
                        wt = Weapon::Uzi;
                        break;
                    case Weapon::Minigun:
                        wt = Weapon::Minigun;
                        break;
                    case Weapon::Laser:
                        wt = Weapon::Laser;
                        break;
                    case Weapon::Flamer:
                        wt = Weapon::Flamer;
                        break;
                    case Weapon::LongRange:
                        wt = Weapon::LongRange;
                        break;
                    case Weapon::Scanner:
                        wt = Weapon::Scanner;
                        break;
                    case Weapon::MediKit:
                        wt = Weapon::MediKit;
                        break;
                    case Weapon::TimeBomb:
                        wt = Weapon::TimeBomb;
                        break;
                    case Weapon::AccessCard:
                        wt = Weapon::AccessCard;
                        break;
                    case Weapon::EnergyShield:
                        wt = Weapon::EnergyShield;
                        break;
                    default:
                        wt = Weapon::Unknown;
                }
                if (wt != Weapon::Unknown) {
                    int ammo = infile.read32();
                    WeaponInstance *pInst = WeaponInstance::createInstance(pWeaponManager_->getWeapon(wt), ammo);
                    pAgent->addWeapon(pInst);
                } else
                    // if not read, we will corrupt loading
                   infile.read32();
            }
        } else if (agents_.get(i)) {
            destroyAgentSlot(i);
        }
    }

    // Read squad
    for (size_t squadInd=0; squadInd<kMaxSlot; squadInd++) {
        int id = infile.read32();
        if (id != 0) {
            for (int iAgnt=0; iAgnt<MAX_AGENT; iAgnt++) {
                Agent *pAgent = agent(iAgnt);
                if (pAgent && pAgent->getId() == id) {
                    setSquadMember(squadInd, pAgent);
                    break;
                }
            }
        } else {
            setSquadMember(squadInd, NULL);
        }
    }
    return true;
}

Agent* AgentManager::createAgent(bool onlyWomen)
{
    for (int i = 0; i < MAX_AGENT; i++) {
        if (agents_.get(i) == NULL) {
            Agent * pAgent = new Agent(g_AgentNames[rand() % g_NumAgentNames],
                onlyWomen ? true : ((rand() % 2) == 0));
            agents_.setAt(i, pAgent);
            return pAgent;
        }
    }
    return NULL;
}

void AgentManager::clearSquad() {
    for (size_t s=0; s<kMaxSlot; s++) {
        a_squad_[s] = NULL;
    }
}

//! Returns true if the slot holds an agent and if he's active
bool AgentManager::isSquadSlotActive(size_t slotId) {
    assert(slotId < kMaxSlot);
    return a_squad_[slotId] && a_squad_[slotId]->isActive();
}

//! Return the slot that holds the given agent or -1 if ni agent is found
int AgentManager::getSquadSlotForAgent(Agent *pAgent) {
    if (pAgent) {
        for (size_t i=0; i<kMaxSlot; i++) {
            if (pAgent == a_squad_[i]) {
                return i;
            }
        }
    }
    return -1;
}

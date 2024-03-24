/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "core/gamecontroller.h"

#include <list>

#include "fs-utils/log/log.h"
#include "fs-utils/io/file.h"
#include "fs-kernel/model/squad.h"
#include "fs-kernel/model/ped.h"

#include "core/gamesession.h"

GameController::GameController(MapManager *pMapManager) :
        missions_(pMapManager),
        session_(std::make_unique<GameSession>(&weaponMgr_, &mods_)) {
    agents_.setModManager(&mods_);
    agents_.setWeaponManager(&weaponMgr_);
}

GameController::~GameController() {}

void GameController::destroy() {
    LOG(Log::k_FLG_MEM, "GameController", "destroy", ("Destruction..."))
    agents_.destroy();
}

bool GameController::reset() {
    g_missionCtrl.destroyMission();
    // Reset default mods and weapons
    mods_.reset();
    weaponMgr_.reset();
    // TODO add reading cheatcode for onlywomen parameter
    agents_.reset();

    // Reset user session
    if (!g_Session.reset()) {
        return false;
    }

    return true;
}


/*!
 * Changes the user informations.
 */
void GameController::change_user_infos(const char *company_name, const char *player_name,
                            int new_logo, int new_color) {
    g_Session.setCompanyName(company_name);
    g_Session.setUserName(player_name);
    g_Session.setLogo(new_logo);

    if (g_Session.getLogoColour() != new_color) {
        // we must check in the map if the new player color
        // has not already been assigned to an enemy syndicate
        // => if true, invert colors.
        g_Session.exchange_color_wt_syndicate(new_color);
    }
}

void GameController::handle_mission_end(Mission *p_mission) {
    int elapsed = p_mission->stats()->missionDuration();
    g_Session.updateTime(elapsed);

    // synch ped agents with agent from cryo chamber
    transferAgentToCryoChamber(p_mission);

    if (p_mission->completed()) {
        g_Session.mark_selected_block_completed();

        // Make the next missions available
        Block &blk = g_Session.getSelectedBlock();
        if (blk.next != NULL) {
            char s[50];
            strcpy(s, blk.next);
            char *token = strtok(s, ":");
            while ( token != NULL ) {
                int id = atoi(token);
                Block &next_blk = g_Session.getBlock(id);
                if (next_blk.status == BLK_UNAVAIL) {
                    // Make available only if the mission is not already finished
                    next_blk.status = BLK_AVAIL;
                }
                token = strtok(NULL, ":");
            }
        }

        // simulate other syndicates activity
        simulate_enemy_moves();
    }
}

void GameController::transferAgentToCryoChamber(Mission *pMission) {
    // Update for squad
    for (size_t i = AgentManager::kSlot1; i < AgentManager::kMaxSlot; i++) {
        PedInstance *pPedAgent = pMission->getSquad()->member(i);
        if (pPedAgent) {
            Agent *pAg = agents().squadMember(i);
            if (pPedAgent->isDead()) {
                // an agent died -> remove him from cryo
                agents().destroyAgentSlot(i);
            } else {
                // synch only weapons
                pPedAgent->transferWeapons(*pAg);
            }
        }
    }

    // Add persuaded agents only in case of mission success
    if (pMission->stats()->agentCaptured() > 0 && pMission->completed()) {
        for (size_t i = pMission->getSquad()->size(); i < pMission->numPeds(); i++) {
            PedInstance *pPed = pMission->ped(i);
            if (pPed->objGroupDef() == PedInstance::og_dmAgent) {
                Agent *pAg = agents().createAgent(false);
                if (pAg) {
                    pPed->transferWeapons(*pAg);
                    pPed->transferMods(*pAg);
                }
            }
        }
    }
}

/*!
 * This method simulates enemy syndicates activtiy
 * by changing ownership of a random number of countries (depending on
 * the number of remaining syndicates).
 */
void GameController::simulate_enemy_moves() {
    // Total number of active syndicates ie syndicates
    // that own at least one country
    int nb_active_synds = 0;
    // Each list of the array contains the id of countries
    // owned by a given syndicate.
    std::list<int> blocks_per_synd[7];
    for (int i=0; i<GameSession::NB_MISSION; i++) {
        Block & blk = g_Session.getBlock(i);
        if (blk.status == BLK_AVAIL || blk.status == BLK_UNAVAIL) {
            if (blocks_per_synd[blk.syndicate_owner].empty()) {
                // We found a country not owned by player
                // so we can count its owner as an active syndicate
                nb_active_synds++;
            }
            blocks_per_synd[blk.syndicate_owner].push_back(i);
        }
    }

    // Computes how many movements (ie change country ownership) we will simulate
    int nb_mvt = get_nb_mvt_for_active_synds(nb_active_synds);

    // Simulate movements

    // this list stores id of missions that shifted
    // to prevent movint them multiple times
    std::list<int> used_block;
    while (nb_mvt > 0) {
        nb_mvt--;
        bool move_done = false;
        do {
            // randomly chose a syndicate
            // that will lose a country
            int synd_from_id = rand() % 7;
            if (!blocks_per_synd[synd_from_id].empty()) {
                // randomly chose one of its countries
                int m_pos = rand() % blocks_per_synd[synd_from_id].size();
                int i = 0;
                for (std::list < int >::iterator it = blocks_per_synd[synd_from_id].begin();
                        it != blocks_per_synd[synd_from_id].end(); it++) {
                    if (i == m_pos) {
                        // First check to see if mission hasn't already been shited before
                        bool is_in_list = false;
                        for (std::list < int >::iterator used_it = used_block.begin();
                                used_it != used_block.end(); used_it++) {
                            if (*it == *used_it) {
                                is_in_list = true;
                                break;
                            }
                        }
                        if (!is_in_list) {
                            int synd_to_id;
                            do {
                                // find another syndicate that owns countries.
                                // synds that do not have countries are out
                                synd_to_id = rand() % 7;
                            } while (synd_from_id == synd_to_id || blocks_per_synd[synd_to_id].empty());

                            // Gives the country to the other syndicate
                            g_Session.getBlock(*it).syndicate_owner = synd_to_id;
                            used_block.push_back(*it); // remember we used this mission
                            move_done = true;
                        }
                        // out anyway
                        break;
                    }

                    i++;
                } // end for
            } // end if
        } while (!move_done);
    }
}

/*!
 * Returns a random number of shifts given the number of
 * syndicates in activity.
 */
int GameController::get_nb_mvt_for_active_synds(int nb_active_synds) {
    switch(nb_active_synds) {
    case 7:
        return rand() % 9 + 2; // between 2 and 10 moves
    case 6:
        return rand() % 7 + 2; // between 2 and 8 moves
    case 5:
        return rand() % 6 + 2; // between 2 and 7 moves
    case 4:
        return rand() % 5 + 1; // between 1 and 5 moves
    case 3:
        return rand() % 4 + 1; // between 1 and 4 moves
    case 2:
        return rand() % 2 + 1; // between 1 and 2 moves
    default:
        return 0;
    }
}

bool GameController::saveGameToFile(int fileSlot, std::string name) {
    LOG(Log::k_FLG_IO, "GameController", "saveGameToFile", ("Saving %s in slot %d", name.c_str(), fileSlot))

    PortableFile outfile;
    std::string path;

    File::getFullPathForSaveSlot(fileSlot, path);
    LOG(Log::k_FLG_IO, "GameController", "saveGameToFile", ("Saving to file %s", path.c_str()))

    outfile.open_to_overwrite(path.c_str());

    if (outfile) {
        // write file format version
        outfile.write8(1); // major
        outfile.write8(2); // minor

        // Slot name is 31 characters long, nul-padded
        outfile.write_string(name, 31);

        // Session
        g_Session.saveToFile(outfile);

        // Weapons
        weaponManager().saveToFile(outfile);

        // Mods
        mods().saveToFile(outfile);

        // Agents
        // TODO move in sesion saveToFile
        agents().saveToFile(outfile);

        // save researches
        // TODO move in sesion saveToFile
        g_Session.researchManager().saveToFile(outfile);

        return true;
    }

    return false;
}

bool GameController::loadGameFromFile(int fileSlot) {

    std::string path;
    PortableFile infile;

    File::getFullPathForSaveSlot(fileSlot, path);

    infile.open_to_read(path.c_str());

    if (infile) {
        // FIXME: detect original game saves

        // Read version
        unsigned char vMaj = infile.read8();
        unsigned char vMin = infile.read8();
        FormatVersion v(vMaj, vMin);

        // validate that this is a supported version.
        if (v.gt(1,2)) {
            FSERR(Log::k_FLG_IO, "GameController", "loadGameFromFile", ("Cannot load file, unsupported version %d.%d", vMaj, vMin))
            return false;
        }

        if (v == 0x0100) {
            // the 1.0 format is in native byte order instead of big-endian.
            infile.set_system_endian();
        }

        reset();

        // Read slot name
        std::string slotName;
        // Original game: 20 chars on screen, 20 written, 19 read.
        // v1.0: 25 characters.
        // v1.1: 31 characters.
        slotName = infile.read_string((v == 0x0100) ? 25 : 31, true);

        g_Session.loadFromFile(infile, v);

        // Weapons
        weaponManager().loadFromFile(infile, v);

        // Mods
        mods().loadFromFile(infile, v);

        // Agents
        agents().loadFromFile(infile, v);

        // Research
        g_Session.researchManager().loadFromFile(infile, v);

        return true;
    }

    return false;
}

void GameController::cheatFunds() {
    g_Session.setMoney(100000000);
}
/*!
 * Activate cheat mode in which all completed missions can be replayed.
 */
void GameController::cheatRepeatOrCompleteMission() {
    g_Session.cheatReplayMission();
}

void GameController::cheatWeaponsAndMods() {
    weaponManager().cheatEnableAllWeapons();
    mods().cheatEnableAllMods();
}

void GameController::cheatEquipAllMods() {
    for (int agent = 0; agent < AgentManager::MAX_AGENT; agent++) {
        Agent *pAgent = agents().agent(agent);
        if (pAgent) {
            pAgent->clearSlots();

            pAgent->addMod(mods().getMod(Mod::MOD_LEGS, Mod::MOD_V3));
            pAgent->addMod(mods().getMod(Mod::MOD_ARMS, Mod::MOD_V3));
            pAgent->addMod(mods().getMod(Mod::MOD_EYES, Mod::MOD_V3));
            pAgent->addMod(mods().getMod(Mod::MOD_BRAIN, Mod::MOD_V3));
            pAgent->addMod(mods().getMod(Mod::MOD_CHEST, Mod::MOD_V3));
            pAgent->addMod(mods().getMod(Mod::MOD_HEART, Mod::MOD_V3));
        }
    }
}

/*!
 * Activate cheat mode in which all missions are playable.
 */
void GameController::cheatAnyMission() {
    g_Session.cheatEnableAllMission();
}

void GameController::cheatResurrectAgents() {
    // TODO: Implement cheatResurrectAgents()
}

void GameController::cheatOwnAllCountries() {
    // TODO: Implement cheatOwnAllCountries()
}

void GameController::cheatAccelerateTime() {
    g_Session.cheatAccelerateTime();
}

void GameController::cheatFemaleRecruits() {
    agents().reset(true);

    for (size_t i = 0; i < AgentManager::kMaxSlot; i++)
        agents().setSquadMember(i, agents().agent(i));
}

void GameController::cheatEquipFancyWeapons() {
    for (int i = 0; i < AgentManager::MAX_AGENT; i++) {
        if (agents().agent(i)) {
        agents().agent(i)->destroyAllWeapons();
#ifdef _DEBUG
        agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::Minigun)));
        agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::TimeBomb)));
        agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::GaussGun)));
        agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::Flamer)));
        agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::Uzi)));
        agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::Persuadatron)));
        agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::EnergyShield)));
        agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::Shotgun)));
#else
        agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::Minigun)));
        agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::Minigun)));
        agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::Persuadatron)));
        agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::TimeBomb)));
        agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::EnergyShield)));
        agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::EnergyShield)));
        agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::Laser)));
        agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(weaponManager().getWeapon(Weapon::Laser)));
#endif
        }
    }
}


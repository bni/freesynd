/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net>
 *   Copyright (C) 2010, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "core/gamecontroller.h"

#include <list>

#include "fs-utils/log/log.h"
#include "fs-utils/io/file.h"
#include "fs-kernel/model/squad.h"
#include "fs-kernel/model/ped.h"

#include "core/gamesession.h"

//! The maximum number of syndicate in the game
constexpr uint8_t kMaxSyndicate { 8 };

GameController::GameController() :
        missionMgr_(&tileMgr_),
        session_(std::make_unique<GameSession>(&weaponMgr_, &mods_)) {
    agents_.setModManager(&mods_);
    agents_.setWeaponManager(&weaponMgr_);
}

GameController::~GameController() {}

bool GameController::initialize() {

    LOG(Log::k_FLG_INFO, "GameController", "initialize", ("loading game tileset..."))
    if (!tileMgr_.loadTiles()) {
        return false;
    }

    LOG(Log::k_FLG_INFO, "GameController", "initialize", ("Loading game data..."))
    return reset();
}

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
 * Activate a cheat code with the given name. Possible
 * cheat codes are :
 * - DO IT AGAIN : Possibility to replay a completed mission
 * - NUK THEM : Enable all missions and resurrect dead agents
 * - OWN THEM : All countries belong to the user
 * - ROB A BANK : Puts $100 000 000 in funds
 * - TO THE TOP : Puts $100 000 000 in funds and activates all missions
 * - COOPER TEAM : $100 000 000 in funds, select any mission, all weapons
 *   and mods
 * - WATCH THE CLOCK : Accelerates time
 *
 * \param name The name of a cheat code.
 */
void GameController::setCheatCode(const char *name) {

    // Repeat mission with previously obtained items, press 'C' or 'Ctrl-C'
    // to instantly complete a mission
    if (!strcmp(name, "DO IT AGAIN"))
        cheatRepeatOrCompleteMission();
    else if (!strcmp(name, "NUK THEM")) {
        // Select any mission, resurrect dead agents
        cheatAnyMission();
        cheatResurrectAgents();
    }
    else if (!strcmp(name, "OWN THEM")) {
        // Own all countries
        cheatOwnAllCountries();
    }
    else if (!strcmp(name, "ROB A BANK")) {
        // $100 000 000 in funds
        cheatFunds();
    }
    else if (!strcmp(name, "TO THE TOP")) {
        // $100 000 000 in funds, select any mission
        cheatFunds();
        cheatAnyMission();
    }
    else if (!strcmp(name, "COOPER TEAM")) {
        // $100 000 000 in funds, select any mission, all weapons and mods
        cheatFemaleRecruits();
        cheatFunds();
        cheatAnyMission();
        cheatWeaponsAndMods();
        cheatEquipAllMods();
        cheatEquipFancyWeapons();
    }
    else if (!strcmp(name, "WATCH THE CLOCK")) {
        // Accelerate time for faster research completion
        cheatAccelerateTime();
    }
}

/*!
 * Changes the user informations.
 */
void GameController::change_user_infos(const char *company_name, const char *player_name,
                            int new_logo, int new_color) {
    g_Session.setCompanyName(company_name);
    g_Session.setUserName(player_name);
    g_Session.setLogo(new_logo);
    g_Session.setLogoColour(new_color);
}

/*!
 * Updates the game time based on the given elapsed millisecond since
 * the last update.
 * For every day passed, it calls the updateCountries() method and update
 * the user's amount of money.
 * \param elapsed The number of millisecond since the last update.
 * \return True if time has changed.
 */
bool GameController::updateTime(uint32_t elapsed) {
    fs_knl::GameTime previousTime = session_->currentTime();
    uint32_t hours = session_->addElapsedTime(elapsed);

    // first update countries to collect money
    uint32_t daysElapsed = session_->currentTime().diffInDays(previousTime);
    for (uint32_t i=0; i < daysElapsed; i++) {
        session_->increaseMoney(session_->updateCountries());
    }

    // then update ongoing research
    session_->decreaseMoney(session_->researchManager().process(hours, session_->getMoney()));

    return hours != 0;
}

fs_knl::MissionBriefing * GameController::loadBriefing(int n) {
    return missionMgr_.loadBriefing(n);
}

/*!
 * Load a mission based on the selection by the player
 * @return 
 */
bool GameController::loadSelectedMission() {
    Block block = session_->getSelectedBlock();
    fs_knl::Mission *pMission = missionMgr_.loadMission(block.mis_id, block.paletteId);

    return pMission != nullptr;
}

void GameController::handle_mission_end(fs_knl::Mission *p_mission) {
    uint32_t elapsed = p_mission->stats()->missionDuration();
    updateTime(elapsed);

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

void GameController::transferAgentToCryoChamber(fs_knl::Mission *pMission) {
    // Update for squad
    for (size_t i = fs_knl::Squad::kSlot1; i < fs_knl::Squad::kMaxSlot; i++) {
        fs_knl::PedInstance *pPedAgent = pMission->getSquad()->member(i);
        if (pPedAgent) {
            fs_knl::Agent *pAg = agents().squadMember(i);
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
    if (pMission->stats()->nbAgentCaptured() > 0 && pMission->completed()) {
        for (size_t i = pMission->getSquad()->size(); i < pMission->numPeds(); i++) {
            fs_knl::PedInstance *pPed = pMission->ped(i);
            if (pPed->objGroupDef() == fs_knl::PedInstance::og_dmAgent) {
                fs_knl::Agent *pAg = agents().createAgent(false);
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
    std::list<int> blocks_per_synd[kMaxSyndicate - 1];
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
            uint8_t synd_from_id = session_->getRandomEnemySyndicateId();
            // Distribution for choosing a random country
            std::uniform_int_distribution<std::size_t> countryDist(0,
                                                        blocks_per_synd[synd_from_id].size() - 1);
            if (!blocks_per_synd[synd_from_id].empty()) {
                // randomly chose one of its countries
                size_t m_pos = countryDist(session_->getRandomGenerator());
                size_t i = 0;
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
                            uint8_t synd_to_id;
                            do {
                                // find another syndicate that owns countries.
                                // synds that do not have countries are out
                                synd_to_id = session_->getRandomEnemySyndicateId();
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

    fs_utl::PortableFile outfile;
    std::string path;

    fs_utl::File::getFullPathForSaveSlot(fileSlot, path);
    LOG(Log::k_FLG_IO, "GameController", "saveGameToFile", ("Saving to file %s", path.c_str()))

    outfile.open_to_overwrite(path.c_str());

    if (outfile) {
        // write file format version
        outfile.write8(1); // major
        outfile.write8(3); // minor

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
    fs_utl::PortableFile infile;

    fs_utl::File::getFullPathForSaveSlot(fileSlot, path);

    infile.open_to_read(path.c_str());

    if (infile) {
        // FIXME: detect original game saves

        // Read version
        unsigned char vMaj = infile.read8();
        unsigned char vMin = infile.read8();
        fs_utl::FormatVersion v(vMaj, vMin);

        // validate that this is a supported version.
        if (v.gt(1,3)) {
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
    for (uint8_t agent = 0; agent < fs_knl::AgentManager::MAX_AGENT; agent++) {
        fs_knl::Agent *pAgent = agents().agent(agent);
        if (pAgent) {
            pAgent->clearSlots();

            pAgent->addMod(mods().getMod(fs_knl::Mod::MOD_LEGS, fs_knl::Mod::MOD_V3));
            pAgent->addMod(mods().getMod(fs_knl::Mod::MOD_ARMS, fs_knl::Mod::MOD_V3));
            pAgent->addMod(mods().getMod(fs_knl::Mod::MOD_EYES, fs_knl::Mod::MOD_V3));
            pAgent->addMod(mods().getMod(fs_knl::Mod::MOD_BRAIN, fs_knl::Mod::MOD_V3));
            pAgent->addMod(mods().getMod(fs_knl::Mod::MOD_CHEST, fs_knl::Mod::MOD_V3));
            pAgent->addMod(mods().getMod(fs_knl::Mod::MOD_HEART, fs_knl::Mod::MOD_V3));
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
    g_Session.currentTime().cheatAccelerateTime();
}

void GameController::cheatFemaleRecruits() {
    agents().reset(true);

    for (uint8_t i = 0; i < fs_knl::Squad::kMaxSlot; i++)
        agents().setSquadMember(i, agents().agent(i));
}

void GameController::cheatEquipFancyWeapons() {
    for (uint8_t i = 0; i < fs_knl::AgentManager::MAX_AGENT; i++) {
        if (agents().agent(i)) {
        agents().agent(i)->destroyAllWeapons();
#ifdef _DEBUG
        agents().agent(i)->addWeapon(
            fs_knl::WeaponInstance::createInstance(weaponManager().getWeapon(fs_knl::Weapon::Minigun)));
        agents().agent(i)->addWeapon(
            fs_knl::WeaponInstance::createInstance(weaponManager().getWeapon(fs_knl::Weapon::TimeBomb)));
        agents().agent(i)->addWeapon(
            fs_knl::WeaponInstance::createInstance(weaponManager().getWeapon(fs_knl::Weapon::GaussGun)));
        agents().agent(i)->addWeapon(
            fs_knl::WeaponInstance::createInstance(weaponManager().getWeapon(fs_knl::Weapon::Flamer)));
        agents().agent(i)->addWeapon(
            fs_knl::WeaponInstance::createInstance(weaponManager().getWeapon(fs_knl::Weapon::Uzi)));
        agents().agent(i)->addWeapon(
            fs_knl::WeaponInstance::createInstance(weaponManager().getWeapon(fs_knl::Weapon::Persuadatron)));
        agents().agent(i)->addWeapon(
            fs_knl::WeaponInstance::createInstance(weaponManager().getWeapon(fs_knl::Weapon::EnergyShield)));
        agents().agent(i)->addWeapon(
            fs_knl::WeaponInstance::createInstance(weaponManager().getWeapon(fs_knl::Weapon::Shotgun)));
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


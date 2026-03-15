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

#ifndef CORE_GAMECONTROLLER_H_
#define CORE_GAMECONTROLLER_H_

#include <cassert>
#include <list>
#include <memory>

#include "fs-utils/misc/singleton.h"
#include "fs-kernel/mgr/agentmanager.h"
#include "fs-kernel/mgr/weaponmanager.h"
#include "fs-kernel/mgr/modmanager.h"
#include "fs-kernel/mgr/missionmanager.h"
#include "core/gamesession.h"

/*!
 * The game controller holds the game logic.
 */
class GameController : public Singleton < GameController > {
  public:
    GameController();
    virtual ~GameController();

    //! initialize the game controller
    bool initialize();
    /*!
     * Resets controller.
     * \returns True if reset has succeeded
     */
    bool reset();

    //! Delete all ressources. Called by App:destroy()
    void destroy();

    //*************************************
    // Managers
    //*************************************
    fs_knl::AgentManager &agents() {
        return agents_;
    }

    fs_knl::WeaponManager &weaponManager() {
        return weaponMgr_;
    }

    fs_knl::ModManager &mods() {
        return mods_;
    }

    //*************************************
    // Game services
    //*************************************
    void setCheatCode(const char *name);

    //! Changes the user preferences (from the config menu)
    void change_user_infos(const char *company_name, const char *player_name,
                            int logo, int color);

    //! Do all time related updates
    bool updateTime(uint32_t elapsed);
    
    //! Loads briefing for the given mission id
    fs_knl::MissionBriefing *loadBriefing(int n);
    //! Loads mission
    bool loadSelectedMission();
    //! Checks if mission is completed and updates game state
    void handle_mission_end(fs_knl::Mission *p_mission);

    //! Save game to a file
    bool saveGameToFile(int fileSlot, std::string name);
    //! Load game from a file
    bool loadGameFromFile(int fileSlot);

private:
    //! Sync the returning agents with the cryo chamber roster
    void transferAgentToCryoChamber(fs_knl::Mission *pMission);
    //! Simulates syndicates fighting for countries
    void simulate_enemy_moves();
    // helper method
    int get_nb_mvt_for_active_synds(int nb_active_synds);

    void cheatFunds();
    void cheatRepeatOrCompleteMission();
    void cheatWeaponsAndMods();
    void cheatEquipAllMods();
    void cheatAnyMission();
    void cheatResurrectAgents();
    void cheatOwnAllCountries();
    void cheatAccelerateTime();
    void cheatFemaleRecruits();
    void cheatEquipFancyWeapons();
    void cheatEquipMiniguns();

private:
    /*!
     * Manager of agent.
     */
    fs_knl::AgentManager agents_;
    /*! Manager of weapons.*/
    fs_knl::WeaponManager weaponMgr_;
    /*! Manager of mods.*/
    fs_knl::ModManager mods_;
    //! For loading tiles
    fs_eng::TileManager tileMgr_;
    /*! Manager of missions.*/
    fs_knl::MissionManager missionMgr_;
    /*! A structure to hold player information.*/
    std::unique_ptr<GameSession> session_;
};

#define g_gameCtrl    GameController::singleton()

#endif  // CORE_GAMECONTROLLER_H_

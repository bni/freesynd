/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#include "app.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <iostream>
#include <fstream>
#include <set>

#ifdef __APPLE__
// Carbon includes an AIFF header which conflicts with fliplayer.h
// So we will redefine ChunkHeader temporarily to work around that.
#define ChunkHeader CarbonChunkHeader
#include <Carbon/Carbon.h>
#undef ChunkHeader
#endif

#include "fs-utils/log/log.h"
#include "fs-utils/io/configfile.h"
#include "fs-utils/io/portablefile.h"
#include "menus/gamemenufactory.h"
#include "menus/gamemenuid.h"

App::App()
    : BaseApp(new GameMenuFactory()),
      game_ctlr_(std::make_unique<GameController>(&maps_))
{
#ifdef _DEBUG
    debug_breakpoint_trigger_ = 0;
#endif
}

App::~App() {
}

/*!
 * Initialize application.
 * \param iniPath The path to the config file.
 * \return True if initialization is ok.
 */
bool App::doInitialize(const CliParam& param) {

    LOG(Log::k_FLG_INFO, "App", "initialize", ("loading game tileset..."))
    if (!maps().initialize()) {
        return false;
    }

    LOG(Log::k_FLG_INFO, "App", "initialize", ("Loading game data..."))
    return game_ctlr_->reset();
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
void App::setCheatCode(const char *name) {

    // Repeat mission with previously obtained items, press 'C' or 'Ctrl-C'
    // to instantly complete a mission
    if (!strcmp(name, "DO IT AGAIN"))
        game_ctlr_->cheatRepeatOrCompleteMission();
    else if (!strcmp(name, "NUK THEM")) {
        // Select any mission, resurrect dead agents
        game_ctlr_->cheatAnyMission();
        game_ctlr_->cheatResurrectAgents();
    }
    else if (!strcmp(name, "OWN THEM")) {
        // Own all countries
        game_ctlr_->cheatOwnAllCountries();
    }
    else if (!strcmp(name, "ROB A BANK")) {
        // $100 000 000 in funds
        game_ctlr_->cheatFunds();
    }
    else if (!strcmp(name, "TO THE TOP")) {
        // $100 000 000 in funds, select any mission
        game_ctlr_->cheatFunds();
        game_ctlr_->cheatAnyMission();
    }
    else if (!strcmp(name, "COOPER TEAM")) {
        // $100 000 000 in funds, select any mission, all weapons and mods
        game_ctlr_->cheatFemaleRecruits();
        game_ctlr_->cheatFunds();
        game_ctlr_->cheatAnyMission();
        game_ctlr_->cheatWeaponsAndMods();
        game_ctlr_->cheatEquipAllMods();
        game_ctlr_->cheatEquipFancyWeapons();
    }
    else if (!strcmp(name, "WATCH THE CLOCK")) {
        // Accelerate time for faster research completion
        game_ctlr_->cheatAccelerateTime();
    }
}

/*!
 * Destroy the application.
 */
void App::doDestroy() {
     game_ctlr_->destroy();
}

/*!
 * This method defines the application loop.
 * \param start_mission Mission id used to start the application in debug mode
 * In standard mode start_mission is always -1.
 */
/*!
 * This method returns the menu Id used to start the app.
 */
int App::getStartMenuId(const CliParam& param) {
    if (param.getStartingMission() == -1) {
        if (context_->isPlayIntro()) {
            // Update intro flag so intro won't be played next time
            context_->updateIntroFlag();
            return fs_game_menus::kMenuIdFliIntro;
        } else {
            // play title before going to main menu
            return fs_game_menus::kMenuIdFliTitle;
        }
    }
    else {
        // Debug scenario : start directly with the brief menu
        // in the given mission
        // First, we find the block associated with the given
        // mission number
        for (int i = 0; i < 50; i++) {
            if (g_Session.getBlock(i).mis_id == param.getStartingMission()) {
                g_Session.setSelectedBlockId(i);
                break;
            }
        }
        // Then we go to the brief menu
        return fs_game_menus::kMenuIdBrief;
    }
}

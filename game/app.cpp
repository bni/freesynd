/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> 
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>
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

GameCliParam::GameCliParam():startMission_(0), cheatCodes_("") {}

void GameCliParam::addOptions(CLI::App &app) {
    app.add_option("-m,--mission", startMission_, "Jump directly to the specified mission.")->option_text("<id>");
    app.add_option("-c,--cheat", startMission_, "Set the list of cheatcodes separated by colon.")->option_text("<codes>");

    // TODO : define validator
    /*if (mission >= 0 && mission < 50) {
                startMission_ = mission;
            }*/
}

App::App()
    : BaseApp(new GameMenuFactory()),
      game_ctlr_(std::make_unique<GameController>())
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
bool App::doInitialize() {
    // setting the cheat codes
        if (cliParam_.hasCheatCodes()) {
            std::string cheatCodes(cliParam_.getCheatCodes());
            char *cheats = (char *)cheatCodes.c_str();
            char *token = strtok(cheats, ":");
            while ( token != NULL ) {
                LOG(Log::k_FLG_INFO, "Main", "main", ("Cheat code activated : %s", token))
                //setCheatCode(token);
                token = strtok(NULL, ":");
            }
        }

    return game_ctlr_->initialize();
}


/*!
 * Activate a cheat code with the given name. 
 * \param name The name of a cheat code.
 */
void App::setCheatCode(const char *name) {
    game_ctlr_->setCheatCode(name);
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
int App::getStartMenuId() {
    if (cliParam_.getStartingMission() == -1) {
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
            if (g_Session.getBlock(i).mis_id == cliParam_.getStartingMission()) {
                g_Session.setSelectedBlockId(i);
                break;
            }
        }
        // Then we go to the brief menu
        return fs_game_menus::kMenuIdBrief;
    }
}

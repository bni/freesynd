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

#ifndef APP_H
#define APP_H

#include <memory>

#include "fs-utils/common.h"
#include "fs-engine/base_app.h"
#include "fs-engine/appcontext.h"
#include "fs-kernel/mgr/mapmanager.h"
#include "core/gamecontroller.h"

class GameCliParam : public fs_eng::CliParam {

public:
    GameCliParam();

    int getStartingMission() const { return startMission_; }

    std::string getCheatCodes() const { return cheatCodes_; }
    //! Returns true if user has set cheatcodes
    bool hasCheatCodes() { return cheatCodes_.length() != 0; }

protected:
    void addOptions(CLI::App &app) override;

private:
    /*!
     * Use in development mode to start directly on a mission and skip menus.
     * Set the variable to a mission id using CLI param "-m".
     * Note : the argument is the index of the block in the structure g_MissionNumbers
     * as defined in briefmenu.cpp and not the mission number itself.
     */
    int startMission_;

    /*!
     * This parameter is used to specify cheat codes on command line (option -c).
     * You can specify multiple codes using the ':' as a separator.
     * See available cheat codes in App::setCheatCode()
     * example -c "DO IT AGAIN:NUK THEM"
     */
    std::string cheatCodes_;

};

/*!
 * Application class.
 * Used for managing game settings and workflows.
 */
class App : public fs_eng::BaseApp {
  public:
    App();
    virtual ~App();

    void setCheatCode(const char *name);

#ifdef _DEBUG
public:
    uint8 debug_breakpoint_trigger_;
#endif

protected:
    //! Initialize application
    bool doInitialize() override;
    //! Destroy all components
    void doDestroy() override;
    //! Define the menuid that will be displayed at the application's start
    int getStartMenuId() override;

    fs_eng::CliParam & getCliParam() override {
        return cliParam_;
    };

private:
    /*! Controls the game logic. */
    std::unique_ptr<GameController> game_ctlr_;
    GameCliParam cliParam_;
};

#endif  // APP_H


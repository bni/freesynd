/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
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

#ifndef ENGINE_BASEAPP_H
#define ENGINE_BASEAPP_H

#include <string>
#include <memory>
#include <concepts>

#include "CLI/CLI.hpp"

#include "fs-utils/log/log.h"
#include "fs-engine/appcontext.h"
#include "fs-engine/gfx/animationmanager.h"
#include "fs-engine/system/system.h"
#include "fs-engine/sound/soundmanager.h"
#include "fs-engine/sound/musicmanager.h"
#include "fs-engine/menus/menumanager.h"
#include "fs-engine/events/default_events.h"

namespace fs_eng {

class CliParam {

public:
    CliParam();
    virtual ~CliParam() {}

    //! Parse command line parameters
    int parseCommandLine(int argc, char *argv[]);

    bool isSoundDisabled() const { return disableSound_; }

    std::string getLogMask() const { return logMask_; }

    std::string getIniPath() const { return iniPath_; }
    std::string getUserConfDir() const { return userConfPath_; }
protected:
    virtual void addOptions(CLI::App &app) {};

private:
    /*!
     * Set to true to mute the sound and music using CLI param "--nosound".
     */
    bool disableSound_;
    //! This variable stores the log mask to init log. By default we activate all logs
    std::string logMask_ = "ALL";
    //! The path to the folder containing the application level parameters
    std::string iniPath_;
    //! The path to the folder containing the user level parameters
    std::string userConfPath_;
};

/*!
 * Base class for a Freesynd application.
 */
class BaseApp {
public:
    //! Constructor
    BaseApp(MenuFactory *pMenuFactory);
    //! Destructor
    virtual ~BaseApp();

    //! Destroy all components
    virtual void destroy() final;

    int run(int argc, char *argv[]);

    MenuManager &menus() {
        return menus_;
    }

protected:
    //! Initialize application
    virtual bool initialize() final;
    //! running loop
    int run();
    //! Child class implements this method for initialization
    virtual bool doInitialize() { return true; }
    //! Child class overloads this method for destroying resources
    virtual void doDestroy();
    //! Define the menuid that will be displayed at the application's start
    virtual int getStartMenuId() = 0;
    //! Return true is intro resources must be loaded
    virtual bool isLoadIntroResources();

    virtual fs_eng::CliParam & getCliParam() = 0;

    //! Returns true if the application is running
    bool isRunning() const {
        return running_;
    }

    void waitForKeyPress();

    //! Stop the running loop
    void onQuitHandler(QuitEvent *evt);

    /*! A structure to hold general application information.*/
    std::unique_ptr<AppContext> context_;
    std::unique_ptr<System> system_;

private:
    bool running_;
    AnimationManager animationManager_;
    SoundManager soundManager_;
    MusicManager music_;
    MenuManager menus_;
};

}

#endif // ENGINE_BASEAPP_H

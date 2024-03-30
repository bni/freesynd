/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef ENGINE_BASEAPP_H
#define ENGINE_BASEAPP_H

#include <string>
#include <memory>

#include "fs-engine/appcontext.h"
#include "fs-engine/gfx/screen.h"
#include "fs-engine/gfx/spritemanager.h"
#include "fs-engine/system/system.h"
#include "fs-engine/sound/soundmanager.h"
#include "fs-engine/sound/musicmanager.h"
#include "fs-engine/menus/menumanager.h"
#include "fs-engine/events/default_events.h"

class CliParam {

public:
    CliParam();

    //! Parse command line parameters
    int parseCommandLine(int argc, char *argv[]);

    int getStartingMission() const { return startMission_; }
    bool isSoundDisabled() const { return disableSound_; }

    std::string getLogMask() const { return logMask_; }
    std::string getCheatCodes() const { return cheatCodes_; }
    //! Returns true if user has set cheatcodes
    bool hasCheatCodes() { return cheatCodes_.length() != 0; }

    std::string getIniPath() const { return iniPath_; }
    std::string getUserConfDir() const { return userConfPath_; }
private:
    void printUsage();

private:
    /*!
     * Use in development mode to start directly on a mission and skip menus.
     * Set the variable to a mission id using CLI param "-m".
     */
    int startMission_;
    /*!
     * Set to true to mute the sound and music using CLI param "--nosound".
     */
    bool disableSound_;
    //! This variable stores the log mask to init log. By default we activate all logs
    std::string logMask_ = "ALL";
    /*!
     * This parameter is used to specify cheat codes on command line (option -c).
     * You can specify multiple codes using the ':' as a separator.
     * See available cheat codes in App::setCheatCode()
     * example -c "DO IT AGAIN:NUK THEM"
     */
    std::string cheatCodes_;
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

    //! Initialize application
    virtual bool initialize(const CliParam& param) final;

    //! Destroy all components
    virtual void destroy() final;

    //! running loop
    void run(const CliParam& param);

    //! Returns true if the application is running
    bool isRunning() const {
        return running_;
    }

    MenuManager &menus() {
        return menus_;
    }

protected:
    //! Child class implements this method for initialization
    virtual bool doInitialize(const CliParam& param);
    //! Child class overloads this method for destroying resources
    virtual void doDestroy();
    //! Define the menuid that will be displayed at the application's start
    virtual int getStartMenuId(const CliParam& param) = 0;

    void waitForKeyPress();

    //! Stop the running loop
    void onQuitHandler(QuitEvent *evt);

    /*! A structure to hold general application information.*/
    std::unique_ptr<AppContext> context_;
    std::unique_ptr<Screen> screen_;
    std::unique_ptr<System> system_;

private:
    bool running_;
    GameSpriteManager game_sprites_;
    SoundManager soundManager_;
    MusicManager music_;
    MenuManager menus_;
};

#endif // ENGINE_BASEAPP_H

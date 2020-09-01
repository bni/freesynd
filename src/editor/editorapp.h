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

#ifndef EDITORAPP_H
#define EDITORAPP_H

#include <memory>
#include <vector>


#include "common.h"
#include "system.h"
#include "gfx/screen.h"
#include "path.h"
#include "menus/menumanager.h"
#include "mapmanager.h"
#include "sound/soundmanager.h"
#include "sound/musicmanager.h"
#include "appcontext.h"
#include "core/gamecontroller.h"

/*!
 * Editor Application class.
 */
class EditorApp : public Singleton < EditorApp > {
  public:
    EditorApp(bool disable_sound);
    virtual ~EditorApp();

    //! Initialize application
    bool initialize(const std::string& iniPath);

    GameSpriteManager &gameSprites() {
        return game_sprites_;
    }

    MenuManager &menus() {
        return menus_;
    }

    MapManager &maps() {
        return maps_;
    }

    SoundManager &introSounds() {
        return intro_sounds_;
    }

    SoundManager &gameSounds() {
        return game_sounds_;
    }

    MusicManager &music() {
        return music_;
    }

    //! Main application method
    void run();

    void quit() {
        running_ = false;
    }

    bool isRunning() const {
        return running_;
    }

    //! Destroy all components
    void destroy();

    void waitForKeyPress();

    static std::string defaultIniFolder();
    //! Return the list of missions found in the search menu
    std::list<int> & getMissionResultList() { return searchResLst_;}

#ifdef _DEBUG
public:
    uint8 debug_breakpoint_trigger_;
#endif

private:
    //! Reads the configuration file
    bool readConfiguration();

    //! Tests Syndicate original data for existence and correctness
    bool testOriginalData();

    //! Sets the intro flag to false in the config file
    void updateIntroFlag();

private:
    bool running_;

    std::auto_ptr<Screen> screen_;
    std::auto_ptr<System> system_;
    /*! A structure to hold general application informations.*/
    std::auto_ptr<AppContext> context_;
    /*! Controls the game logic. */
    std::auto_ptr<GameController> game_ctlr_;

    std::string iniPath_;

    GameSpriteManager game_sprites_;
    MenuManager menus_;
    MapManager maps_;
    SoundManager intro_sounds_;
    SoundManager game_sounds_;
    MusicManager music_;
    /*!
     * Use to store id of missions that are found in the search menu.
     */
    std::list<int> searchResLst_;
};

#define g_App   EditorApp::singleton()

#endif // EDITORAPP_H

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

#include "editorapp.h"

#ifdef WIN32
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

#include "fs-engine/gfx/spritemanager.h"
#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "fs-utils/io/configfile.h"
#include "fs-utils/io/portablefile.h"
#include "editor/editormenufactory.h"
#include "editor/editormenuid.h"

EditorApp::EditorApp()
    : BaseApp(),
      game_ctlr_(new GameController),
#ifdef SYSTEM_SDL
      system_(new SystemSDL())
#else
#error A suitable System object has not been defined!
#endif
    , intro_sounds_(), game_sounds_(), music_(),
    menus_(new EditorMenuFactory(), &game_sounds_)
{
    running_ = true;
#ifdef _DEBUG
    debug_breakpoint_trigger_ = 0;
#endif
}

EditorApp::~EditorApp() {
}

/*!
 * Destroy the application.
 */
void EditorApp::doDestroy() {
    menus_.destroy();
}


/*!
 * Initialize application.
 * \param iniPath The path to the config file.
 * \return True if initialization is ok.
 */
bool EditorApp::doInitialize(const std::string& iniPath, bool disable_sound) {

    LOG(Log::k_FLG_INFO, "EditorApp", "initialize", ("initializing system..."))
    if (!system_->initialize(context_->isFullScreen())) {
        return false;
    }

    LOG(Log::k_FLG_INFO, "EditorApp", "initialize", ("initializing menus..."))
    if (!menus_.initialize(false)) {
        return false;
    }

    LOG(Log::k_FLG_INFO, "EditorApp", "initialize", ("loading game sprites..."))
    if (!gameSprites().loaded())
        gameSprites().load();

    LOG(Log::k_FLG_INFO, "EditorApp", "initialize", ("loading game tileset..."))
    if (!maps().initialize()) {
        return false;
    }

    LOG(Log::k_FLG_INFO, "EditorApp", "initialize", ("Loading game sounds..."))
    game_sounds_.initialize(disable_sound, system_->getAudio(), SoundManager::SAMPLES_GAME);

    LOG(Log::k_FLG_INFO, "EditorApp", "initialize", ("Loading music..."))
    music_.initialize(disable_sound, system_->getAudio());

    g_gameCtrl.reset();

    return true;
}

/*!
 * This method defines the application loop.
 * \param start_mission Mission id used to start the application in debug mode
 * In standard mode start_mission is always -1.
 */
void EditorApp::run() {

    // load palette
    menus().setDefaultPalette();

#if 0
    system_->updateScreen();
    int nx = 0, ny = 0, my = 0;
    for (int i = 0; i < tabSize / 6; i++) {
        Sprite *s = menu_sprites_.sprite(i);
        if (nx + s->width() >= Screen::kScreenWidth) {
            nx = 0;
            ny += my;
            my = 0;
        }
        if (ny + s->height() > Screen::kScreenHeight)
            break;
        s->draw(nx, ny, 0);
        system_->updateScreen();
        nx += s->width();
        if (s->height() > my)
            my = s->height();
    }

    waitForKeyPress();
    exit(1);
#endif

    // play title before going to main menu
    menus_.gotoMenu(fs_edit_menus::kMenuIdMain);

    int lasttick = SDL_GetTicks();
    while (running_) {
        int curtick = SDL_GetTicks();
        int diff_ticks = curtick - lasttick;
        menus_.updtSinceMouseDown(diff_ticks);
        menus_.handleEvents();
        if (diff_ticks < 30) {
            SDL_Delay(30 - diff_ticks);
            continue;
        }
        menus_.handleTick(diff_ticks);
        menus_.renderMenu();
        lasttick = curtick;
        system_->updateScreen();
    }

#ifdef GP2X
#ifndef WIN32
    // return to the menu
    chdir("/usr/gp2x");
    execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif
#endif
}


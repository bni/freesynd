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
#include "fs-engine/gfx/screen.h"
#include "fs-engine/sound/audio.h"
#include "fs-utils/crc/ccrc32.h"
#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "fs-utils/io/configfile.h"
#include "fs-utils/io/portablefile.h"
#include "editor/editormenufactory.h"
#include "editor/editormenuid.h"

EditorApp::EditorApp(bool disable_sound):
    context_(new AppContext), game_ctlr_(new GameController),
    screen_(new Screen(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT))
#ifdef SYSTEM_SDL
    , system_(new SystemSDL())
#else
#error A suitable System object has not been defined!
#endif
    , intro_sounds_(disable_sound), game_sounds_(disable_sound), music_(disable_sound),
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
 * Destory the application.
 */
void EditorApp::destroy() {
    menus_.destroy();
}


/*!
 * Initialize application.
 * \param iniPath The path to the config file.
 * \return True if initialization is ok.
 */
bool EditorApp::doInitialize(const std::string& iniPath) {

    LOG(Log::k_FLG_INFO, "EditorApp", "initialize", ("reading configuration..."))
    if (!context_->readConfiguration(iniPath)) {
        FSERR(Log::k_FLG_IO, "EditorApp", "initialize", ("failed to read configuration..."))
        return false;
    }

    if (context_->isTestFiles()) {
        if (!File::testOriginalData()) {
            return false;
        }
        // do not tests files from now
        context_->deactivateTestFlag();
    }

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
    if (!game_sounds_.loadSounds(SoundManager::SAMPLES_GAME)) {
        return false;
    }

    LOG(Log::k_FLG_INFO, "EditorApp", "initialize", ("Loading music..."))
    music_.loadMusic();

    g_gameCtrl.reset();

    return true;
}

void EditorApp::waitForKeyPress() {

    while (running_) {
        // small pause while waiting for key, also mouse event
        SDL_Delay(20);
        menus_.handleEvents();
    }
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
        if (nx + s->width() >= GAME_SCREEN_WIDTH) {
            nx = 0;
            ny += my;
            my = 0;
        }
        if (ny + s->height() > GAME_SCREEN_HEIGHT)
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


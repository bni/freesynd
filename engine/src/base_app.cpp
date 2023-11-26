/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2023  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "fs-engine/base_app.h"

#include "fs-utils/log/log.h"
#include "fs-utils/io/file.h"

#ifdef HAVE_SDL_MIXER

#include "mixer/sdlmixeraudio.h"
#endif // HAVE_SDL_MIXER

BaseApp::BaseApp(MenuFactory *pMenuFactory)
    : context_(std::make_unique<AppContext>()),
      screen_(std::make_unique<Screen>(Screen::kScreenWidth, Screen::kScreenHeight)),
      system_(System::createSystem()),
      game_sprites_(),
      soundManager_(),
      music_(),
      menus_(pMenuFactory, &soundManager_) {
    running_ = false;
}

BaseApp::~BaseApp() {}

bool BaseApp::initialize(const CliParam& param) {
    LOG(Log::k_FLG_INFO, "BaseApp", "initialize", ("App initialization started..."))
    if (!context_->readConfiguration(param.iniPath)) {
        FSERR(Log::k_FLG_IO, "BaseApp", "initialize", ("failed to read configuration : %s", param.iniPath.c_str()))
        return false;
    }

    if (context_->isTestFiles()) {
        if (!File::testOriginalData()) {
            return false;
        }
        // do not tests files from now
        context_->deactivateTestFlag();
    }

    if (!system_->initialize(context_->isFullScreen())) {
        return false;
    }

    if (!menus_.initialize(context_->isPlayIntro())) {
        return false;
    }

    if (!game_sprites_.loaded()) {
        game_sprites_.load();
    }

    soundManager_.initialize(system_->getAudio(), param.disableSound, context_->isPlayIntro());

    music_.initialize(param.disableSound, system_->getAudio());

    bool resInit = doInitialize(param);
    if (resInit) {
        LOG(Log::k_FLG_INFO, "BaseApp", "initialize", ("App initialized with success"))
    }

    return resInit;
}

bool BaseApp::doInitialize(const CliParam& param) {
   return true;
}

void BaseApp::destroy() {
    menus_.destroy();

    doDestroy();
}

void BaseApp::doDestroy() {}

/*!
 * This method defines the application loop.
 * \param start_mission Mission id used to start the application in debug mode
 * In standard mode start_mission is always -1.
 */
void BaseApp::run(const CliParam& param) {

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

    // Then we go to the brief menu
    menus_.gotoMenu(getStartMenuId(param));

    running_ = true;
    int lasttick = system_->getTicks();
    while (running_) {
        int curtick = system_->getTicks();
        int diff_ticks = curtick - lasttick;
        menus_.updtSinceMouseDown(diff_ticks);
        menus_.handleEvents();
        if (diff_ticks < 30) {
            system_->delay(30 - diff_ticks);
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

void BaseApp::waitForKeyPress() {

    while (isRunning()) {
        // small pause while waiting for key, also mouse event
        system_->delay(20);
        menus().handleEvents();
    }
}


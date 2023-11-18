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
      game_sounds_(), music_(),
      menus_(pMenuFactory, &game_sounds_) {
}

BaseApp::~BaseApp() {}

bool BaseApp::initialize(const std::string& iniPath, bool disable_sound) {
    LOG(Log::k_FLG_INFO, "BaseApp", "initialize", ("App initialization started..."))
    if (!context_->readConfiguration(iniPath)) {
        FSERR(Log::k_FLG_IO, "BaseApp", "initialize", ("failed to read configuration : %s", iniPath.c_str()))
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

    if (!gameSprites().loaded()) {
        gameSprites().load();
    }

    LOG(Log::k_FLG_INFO, "BaseApp", "initialize", ("Loading game sounds..."))
    game_sounds_.initialize(disable_sound, system_->getAudio(), SoundManager::SAMPLES_GAME);

    music_.initialize(disable_sound, system_->getAudio());

    bool resInit = doInitialize(iniPath, disable_sound);
    if (resInit) {
        LOG(Log::k_FLG_INFO, "BaseApp", "initialize", ("App initialized with success"))
    }

    return resInit;
}

bool BaseApp::doInitialize(const std::string& iniPath, bool disable_sound) {
   return true;
}

void BaseApp::destroy() {
    doDestroy();
}

void BaseApp::doDestroy() {}


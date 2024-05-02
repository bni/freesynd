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

#include "fs-engine/events/event.h"
#include "fs-engine/events/default_events.h"

#ifdef HAVE_SDL_MIXER

#include "mixer/sdlmixeraudio.h"
#endif // HAVE_SDL_MIXER

CliParam::CliParam() {
    startMission_ = -1;
    disableSound_ = false;
}

int CliParam::parseCommandLine(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
#ifdef _DEBUG
        // This parameter is used in debug phase to accelerate the starting
        // of a game and to jump directly to a mission
        // Note : the argument is the index of the block in the structure g_MissionNumbers
        // as defined in briefmenu.cpp and not the mission number itself.
        if (0 == strcmp("-m", argv[i]) || 0 == strcmp("--mission", argv[i])) {
            int mission = atoi(argv[i + 1]);
            if (mission >= 0 && mission < 50) {
                startMission_ = mission;
            }
            i++;
        }

        // Find cheatcodes
        if (0 == strcmp("-c", argv[i]) || 0 == strcmp("--cheat", argv[i])) {
            cheatCodes_ = argv[i + 1];
            i++;
        }

        // This parameter is used to specify debug flags on command line.
        // You can specify multiple flags using the ':' as a separator.
        // example -l "INFO:GFX"
        if (0 == strcmp("-l", argv[i]) || 0 == strcmp("--log", argv[i])) {
            i++;
            logMask_ = argv[i];

        }
#endif

        if (0 == strcmp("-i", argv[i]) || 0 == strcmp("--ini", argv[i])) {
            i++;
            iniPath_ = argv[i];
        }

        if (0 == strcmp("-u", argv[i]) || 0 == strcmp("--user", argv[i])) {
            i++;
            userConfPath_ = argv[i];
        }

        if (0 == strcmp("--nosound", argv[i])) {
            disableSound_ = true;
        }

        if (0 == strcmp("-h", argv[i]) || 0 == strcmp("--help", argv[i])) {
            printUsage();
            return 1;
        }
    }

    return 0;
}

void CliParam::printUsage() {
    printf("usage: freesynd [options...]\n");
    printf("    -h, --help            display this help and exit.\n");
    printf("    -i, --ini <path>      specify the location of the FreeSynd config file.\n");
    printf("    -u, --user <path>      specify the location of the user.conf file.\n");
    printf("    --nosound             disable all sound.\n");

#ifdef _WIN32
    printf(" (default: freesynd.ini in the same folder as freesynd.exe)\n");
#elif defined(__APPLE__)
    printf(" (default: $HOME/Library/Application Support/FreeSynd/freesynd.ini)\n");
#else
    printf(" (default: $HOME/.freesynd/freesynd.ini)\n");
#endif

#ifdef _DEBUG
    printf("    -m, --mission <num>   jump directly to the specified mission.\n");
    printf("    -c, --cheat <codes>   apply the specified cheat codes.\n");
    printf("                          separate multiple codes with a colon.\n");
    printf("    -l, --log <flags>     apply the specified log flags separated by colon.\n");
#endif
}

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
    if (!context_->readConfiguration(param.getIniPath(), param.getUserConfDir())) {
        FSERR(Log::k_FLG_IO, "BaseApp", "initialize", ("failed to read configuration : %s", param.getIniPath().c_str()))
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

    soundManager_.initialize(system_->getAudio(), param.isSoundDisabled(), context_->isPlayIntro());

    music_.initialize(param.isSoundDisabled(), system_->getAudio());

    bool resInit = doInitialize(param);
    if (resInit) {
        LOG(Log::k_FLG_INFO, "BaseApp", "initialize", ("App initialized with success"))
    }

    EventManager::listen<QuitEvent>(this, &BaseApp::onQuitHandler);

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

    // Let the concrete app decide what menu to start with
    menus_.gotoMenu(getStartMenuId(param));

    running_ = true;
    int lasttick = system_->getTicks();
    while (running_) {
        int curtick = system_->getTicks();
        int diff_ticks = curtick - lasttick;
        menus_.updtSinceMouseDown(diff_ticks);

        FS_Event fsEvt;
        while(system_->pumpEvents(fsEvt)) {
            menus_.handleEvent(fsEvt);
        }
        if (diff_ticks < 30) {
            system_->delay(30 - diff_ticks);
            continue;
        }
        menus_.handleTick(diff_ticks);
        menus_.renderMenu();
        lasttick = curtick;
        system_->updateScreen();
    }
}

void BaseApp::waitForKeyPress() {

    while (isRunning()) {
        // small pause while waiting for key, also mouse event
        system_->delay(20);
        FS_Event fsEvt;
        while(system_->pumpEvents(fsEvt)) {
            menus_.handleEvent(fsEvt);
        }
    }
}

void BaseApp::onQuitHandler(QuitEvent *evt) {
    LOG(Log::k_FLG_INFO, "BaseApp", "onQuitHandler", ("Received Quit Evt : quitting"))
    running_ = false;
}



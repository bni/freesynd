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

#include <memory>

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "fs-utils/common.h"
#include "app.h"
#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "version.h"

#ifdef SYSTEM_SDL
#ifdef _WIN32
#undef main
#else
#include <SDL_main.h>           //This is required on OSX for remapping of main()
#endif
#endif

//#define CHEAP_LEAK_DETECTION

#ifdef CHEAP_LEAK_DETECTION

#define BLKSZ 65536
int blocks[BLKSZ];

void cleanupLeakDetection() {
    int unfreed = 0;
    int mfreed = 0;

    for (int i = 0; i < BLKSZ; i++) {
        if (blocks[i] > 0) {
            unfreed++;
            printf("%x ", i);
        }
        else if (blocks[i] < 0) {
            mfreed++;
            printf("m%x ", i);
        }
    }

    printf("\n%i unfreed blocks\n", unfreed);
    printf("%i multiple freed blocks\n", mfreed);
}

void initLeakDetection() {
    int ginits = 0;

    for (int i = 0; i < BLKSZ; i++) {
        if (blocks[i] > 0)
            ginits++;
        else
            assert(blocks[i] == 0);
    }

    printf("%i blocks allocated before main\n", ginits);
    atexit(&cleanupLeakDetection);
}

void *operator new(size_t n) {
    void *p = malloc(n);
    int x = ((int) p) % BLKSZ;

    if (x == 0x1ffff) {         // plug a reported hash in here and set a breakpoint
        printf("boo\n");
    }

    blocks[x]++;
    return p;
}

void operator delete(void *p) {
    if (p == 0) {
        printf("freeing NULL\n");
        return;
    }

    int x = ((int) p) % BLKSZ;

    if (x == 0x1ffff) {         // plug a reported hash in here and set a breakpoint
        printf("mboo\n");
    }

    blocks[x]--;
    free(p);
}
#endif

int main(int argc, char *argv[]) {

    printf("Freesynd v%i.%i (may 2024)\n", FS_VERSION_MAJOR, FS_VERSION_MINOR);

#ifdef CHEAP_LEAK_DETECTION
    initLeakDetection();
#endif

#ifdef _WIN32
    srand(GetTickCount());
#else
    srand((unsigned) time(NULL));
#endif
    CliParam param;

    if (param.parseCommandLine(argc, argv)) {
        return 1;
    }

    // Initialize log
    Log::initialize(param.getLogMask(), "game.log");

    LOG(Log::k_FLG_INFO, "Main", "main", ("----- Initializing application..."))
    auto app = std::make_unique<App>();

    if (app->initialize(param)) {
        // setting the cheat codes
        if (param.hasCheatCodes()) {
            std::string cheatCodes(param.getCheatCodes());
            char *cheats = (char *)cheatCodes.c_str();
            char *token = strtok(cheats, ":");
            while ( token != NULL ) {
                LOG(Log::k_FLG_INFO, "Main", "main", ("Cheat code activated : %s", token))
                app->setCheatCode(token);
                token = strtok(NULL, ":");
            }
        }

        LOG(Log::k_FLG_INFO, "Main", "main", ("----- Initializing application completed"))
        LOG(Log::k_FLG_INFO, "Main", "main", ("----- Starting game loop"))
        app->run(param);
    } else {
        LOG(Log::k_FLG_INFO, "Main", "main", ("----- Initializing application failed"))
    }

    // Destroy application
    LOG(Log::k_FLG_INFO, "Main", "main", ("----- End of game loop. Destroy application"))
    app->destroy();

#ifdef _DEBUG
    // Close log
    Log::close();
#endif

    return 0;
}

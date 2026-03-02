/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
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

    printf("FreeSynd v%i.%i (mar 2026)\n", FS_VERSION_MAJOR, FS_VERSION_MINOR);

#ifdef CHEAP_LEAK_DETECTION
    initLeakDetection();
#endif

#ifdef _WIN32
    srand(GetTickCount());
#else
    srand((unsigned) time(NULL));
#endif
    
    App app;

    int res = app.run(argc, argv);

    app.destroy();

    return res;
}

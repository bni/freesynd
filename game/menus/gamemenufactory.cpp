/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2013  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "menus/gamemenufactory.h"
#include "menus/gamemenuid.h"
#include "menus/mainmenu.h"
#include "menus/confmenu.h"
#include "menus/mapmenu.h"
#include "menus/briefmenu.h"
#include "menus/selectmenu.h"
#include "menus/researchmenu.h"
#include "menus/loadsavemenu.h"
#include "menus/loadingmenu.h"
#include "menus/gameplaymenu.h"
#include "menus/debriefmenu.h"
#include "fs-engine/menus/flimenu.h"
#include "fs-utils/log/log.h"

using fs_eng::NO_SOUND;

//! This defines the list of event for the intro animation
const fs_eng::FrameEvent intro[] = {
    {   1,  fs_eng::MusicManager::kMusicSongIntro, 17,               0x0, "" }, // Play track
    {  15, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_0" },
    {  29, fs_eng::MusicManager::kMusicSongNoSong, 0,                0x0, NULL },  // Sound car
    {  39, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x1, "" }, // clear subtitle
    {  44, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_1" },
    {  50, fs_eng::MusicManager::kMusicSongNoSong, 1,                0x1, NULL }, // Sound train
    {  62, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" }, // clear subtitle
    {  67, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_2" },
    {  85, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" }, // clear
    {  90, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_3" },
    { 117, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" }, // clear
    { 121, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_4" },
    { 135, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" }, // clear
    { 138, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_5" },
    { 153, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_6" },
    { 165, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_7" },
    { 167, fs_eng::MusicManager::kMusicSongNoSong, 10,               0x1, NULL }, // Sound car start
    { 174, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" }, // Clear
    { 176, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_8" },
    { 181, fs_eng::MusicManager::kMusicSongNoSong, 9,               0x0, NULL }, // Sound car run
    { 200, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" }, // clear
    { 215, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 1
    { 222, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 2
    { 229, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 3
    { 234, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_9" },
    { 236, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 4
    { 243, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 5
    { 250, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 6
    { 257, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 7
    { 264, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 8
    { 269, fs_eng::MusicManager::kMusicSongNoSong, 11,               0x0, NULL }, // car accelerate
    { 271, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" },//clear
    { 291, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_10" },
    { 314, fs_eng::MusicManager::kMusicSongNoSong, 12,               0x0, NULL }, // 1st Door open
    { 327, fs_eng::MusicManager::kMusicSongNoSong, 12,               0x0, NULL }, // 2nd Door closed
    { 333, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" }, // Clear
    { 345, fs_eng::MusicManager::kMusicSongNoSong, 12,               0x1, NULL }, // 2nd Door open
    { 359, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_11" },
    { 406, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" }, // clear
    { 421, fs_eng::MusicManager::kMusicSongNoSong, 4,               0x1, NULL }, // Lab door open
    { 442, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_12" },
    { 444, fs_eng::MusicManager::kMusicSongNoSong, 16,               0x0, NULL }, // glow
    { 455, fs_eng::MusicManager::kMusicSongNoSong, 14,               0x1, NULL }, // levitate
    { 466, fs_eng::MusicManager::kMusicSongNoSong, 15,               0x1, NULL }, // end levitate
    { 467, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" }, // clear
    { 473, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_13" },
    { 482, fs_eng::MusicManager::kMusicSongNoSong, 6,               0x0, NULL }, // round
    { 502, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" }, // clear
    { 520, fs_eng::MusicManager::kMusicSongNoSong, 7,               0x0, NULL }, // leg
    { 530, fs_eng::MusicManager::kMusicSongNoSong, 8,               0x0, NULL }, // leg
    { 540, fs_eng::MusicManager::kMusicSongNoSong, 7,               0x0, NULL }, // leg
    { 548, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_14" },
    { 550, fs_eng::MusicManager::kMusicSongNoSong, 6,               0x0, NULL }, // round back
    { 562, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" },
    { 569, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_15" },
    { 576, fs_eng::MusicManager::kMusicSongNoSong, 3,               0x0, NULL }, // weld
    { 586, fs_eng::MusicManager::kMusicSongNoSong, 3,               0x0, NULL }, // weld
    { 598, fs_eng::MusicManager::kMusicSongNoSong, 3,               0x0, NULL }, // weld
    { 610, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" },
    { 612, fs_eng::MusicManager::kMusicSongNoSong, 3,               0x0, NULL }, // weld
    { 617, fs_eng::MusicManager::kMusicSongNoSong, 13,               0x0, NULL }, // chip
    { 634, fs_eng::MusicManager::kMusicSongNoSong, 17,               0x0, NULL }, // Menu
    { 639, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_16" },
    { 645, fs_eng::MusicManager::kMusicSongNoSong, 24,               0x1, NULL }, // Menu screen 1
    { 659, fs_eng::MusicManager::kMusicSongNoSong, 23,               0x1, NULL }, // Menu screen 3
    { 670, fs_eng::MusicManager::kMusicSongNoSong, 21,               0x1, NULL }, // Screen
    { 673, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" },
    { 679, fs_eng::MusicManager::kMusicSongNoSong, 25,               0x1, NULL }, // Screen
    { 691, fs_eng::MusicManager::kMusicSongNoSong, 26,               0x1, NULL }, // Screen
    { 700, fs_eng::MusicManager::kMusicSongNoSong, 27,               0x1, NULL }, // Screen
    { 702, fs_eng::MusicManager::kMusicSongNoSong, 29,               0x1, NULL }, // Screen
    { 710, fs_eng::MusicManager::kMusicSongNoSong, 27,               0x1, NULL }, // Screen
    { 714, fs_eng::MusicManager::kMusicSongNoSong, 29,               0x1, NULL }, // Screen
    { 720, fs_eng::MusicManager::kMusicSongNoSong, 27,               0x1, NULL }, // Screen
    { 724, fs_eng::MusicManager::kMusicSongNoSong, 29,               0x1, NULL }, // Screen
    { 730, fs_eng::MusicManager::kMusicSongNoSong, 13,               0x1, NULL }, // Screen
    { 750, fs_eng::MusicManager::kMusicSongNoSong, 29,               0x1, NULL }, // Screen
    { 760, fs_eng::MusicManager::kMusicSongNoSong, 30,               0x0, NULL }, // Bip 1
    { 764, fs_eng::MusicManager::kMusicSongNoSong, 30,               0x0, NULL }, // Bip 1
    { 768, fs_eng::MusicManager::kMusicSongNoSong, 30,               0x0, NULL }, // Bip 1
    { 772, fs_eng::MusicManager::kMusicSongNoSong, 30,               0x0, NULL }, // Bip 1
    { 776, fs_eng::MusicManager::kMusicSongNoSong, 30,               0x0, NULL }, // Bip 1
    { 780, fs_eng::MusicManager::kMusicSongNoSong, 30,               0x0, NULL }, // Bip 1
    { 784, fs_eng::MusicManager::kMusicSongNoSong, 18,               0x0, NULL }, // car
    { 796, fs_eng::MusicManager::kMusicSongNoSong, 19,               0x1, NULL }, // train
    { 848, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_17" },
    { 868, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 1
    { 874, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 2
    { 880, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 3
    { 886, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 4
    { 892, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" },
    { 893, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 5
    { 899, fs_eng::MusicManager::kMusicSongNoSong, 2,               0x0, NULL }, // Bip 6
    { 904, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "INTRO_18" },
    { 910, fs_eng::MusicManager::kMusicSongNoSong, 24,               0x0, NULL }, // Gun out
    { 911, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, "" },
    { 928, fs_eng::MusicManager::kMusicSongNoSong, 31,               0x0, NULL }, // Gun shot
    { 932, fs_eng::MusicManager::kMusicSongNoSong, 32,               0x0, NULL }, // Letter S
    { 935, fs_eng::MusicManager::kMusicSongNoSong, 32,               0x0, NULL }, // Letter Y
    { 938, fs_eng::MusicManager::kMusicSongNoSong, 32,               0x0, NULL }, // Letter N
    { 941, fs_eng::MusicManager::kMusicSongNoSong, 32,               0x0, NULL }, // Letter D
    { 944, fs_eng::MusicManager::kMusicSongNoSong, 32,               0x0, NULL }, // Letter I
    { 947, fs_eng::MusicManager::kMusicSongNoSong, 32,               0x0, NULL }, // Letter C
    { 950, fs_eng::MusicManager::kMusicSongNoSong, 32,               0x0, NULL }, // Letter A
    { 953, fs_eng::MusicManager::kMusicSongNoSong, 32,               0x0, NULL }, // Letter T
    { 956, fs_eng::MusicManager::kMusicSongNoSong, 32,               0x0, NULL }, // Letter E
    { 966, fs_eng::MusicManager::kMusicSongNoSong, 33,               0x0, NULL }, // 
    {  -1, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, NULL }
};

//! This defines the list of event for the mission won animation
const fs_eng::FrameEvent mission_win[] = {
    { 1, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::APPLAUSE,              0x1, NULL },
    { 1, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::APPLAUSE_ZOOM,         0x0, NULL },
    { 10, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::FIREWORKS,            0x1, NULL },
    { 30, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::FIREWORKS_APPLAUSE,   0x1, NULL },
    { -1, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound,              0x0, NULL }
};

//! This defines the list of event for the mission lost animation
const fs_eng::FrameEvent mission_failed[] = {
    { 1, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, NULL },
    { 35, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::PRESS_BUTTON, 0x0, NULL },
    { 52, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::MENU_AFTER_MISSION, 0x0, NULL },
    { 80, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::SPEECH_NO, 0x0, NULL },
    { 108, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::MONITOR_IMPACT, 0x0, NULL },
    { 118, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::GLASS_BREAKING_2, 0x0, NULL },
    { -1, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, NULL }
};

//! This defines the list of event for the game lost animation
const fs_eng::FrameEvent game_lost[] = {
    { 1, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, NULL },
    { 2, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::FALLING_COMMAND_SHIP, 0x0, NULL },
    { 34, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::EXPLOSION_BIG, 0x0, NULL },
    { -1, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, NULL }
};

//! This defines the list of event for the menu up animation
const fs_eng::FrameEvent menu_up[] = {
    { 1, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::MENU_UP,   0x1, NULL },
    { -1, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound,  0x0, NULL }
};

//! This defines the list of no event
const fs_eng::FrameEvent title[] = {
    { -1, fs_eng::MusicManager::kMusicSongNoSong, fs_eng::kNoSound, 0x0, NULL }
};

/*!
 * Create an instance of a Menu based on its id
 * @param menuId The id of the menu
 * @return Null of id is unknown
 */
fs_eng::Menu * GameMenuFactory::createCustomMenu(const int menuId) {
    fs_eng::Menu *pMenu = nullptr;

    if (menuId == fs_game_menus::kMenuIdMain) {
        pMenu =  new MainMenu(pManager_);
    } else if (menuId == fs_game_menus::kMenuIdBrief) {
        pMenu =  new BriefMenu(pManager_);
    } else if (menuId == fs_game_menus::kMenuIdConf) {
        pMenu =  new ConfMenu(pManager_);
    } else if (menuId == fs_game_menus::kMenuIdDebrief) {
        pMenu =  new DebriefMenu(pManager_);
    } else if (menuId == fs_game_menus::kMenuIdGameplay) {
        pMenu =  new GameplayMenu(pManager_);
    } else if (menuId == fs_game_menus::kMenuIdLoading) {
        pMenu =  new LoadingMenu(pManager_);
    } else if (menuId == fs_game_menus::kMenuIdResearch) {
        pMenu =  new ResearchMenu(pManager_);
    } else if (menuId == fs_game_menus::kMenuIdSelect) {
        pMenu =  new SelectMenu(pManager_);
    } else if (menuId == fs_game_menus::kMenuIdLdSave) {
        pMenu =  new LoadSaveMenu(pManager_);
    } else if (menuId == fs_game_menus::kMenuIdMap) {
        pMenu =  new MapMenu(pManager_);
    } else if (menuId == fs_game_menus::kMenuIdFliSuccess ||
        menuId == fs_game_menus::kMenuIdFliFailedMission ||
        menuId == fs_game_menus::kMenuIdFliTitle||
        menuId == fs_game_menus::kMenuIdFliIntro ||
        menuId == fs_game_menus::kMenuIdFliMissionAborted) {
        pMenu =  createFliMenu(menuId);
        ;
    } else {
        FSERR(Log::k_FLG_UI, "GameMenuFactory", "createMenu", ("Cannot create Menu : unknown id (%d)", menuId));
    }

    return pMenu;
}


/*!
 * Create an instance of a FliMenu.
 * @param menuId The id for the menu
 * @return A new instance of FliMenu
 */
fs_eng::Menu * GameMenuFactory::createFliMenu(const int menuId) {
    fs_eng::FliMenu *pMenu = new fs_eng::FliMenu(pManager_, menuId, fs_game_menus::kMenuIdMain);
    if (pMenu->getId() == fs_game_menus::kMenuIdFliSuccess || menuId == fs_game_menus::kMenuIdFliFailedMission) {
        if (menuId == fs_game_menus::kMenuIdFliSuccess) {
            pMenu->addFliDesc("mgamewin.dat", 66, false, true, false, mission_win);
        } else if (menuId == fs_game_menus::kMenuIdFliFailedMission) {
            pMenu->addFliDesc("mendlose.dat", 66, false, true, false, mission_failed);
        } else if (menuId == fs_game_menus::kMenuIdFliGameLost) {
            // All availiable agents are dead
            // TODO: use it
            pMenu->addFliDesc("mlosegam.dat", 66, false, true, false, game_lost);
        }
        // common fli to prepare next menu show
        pMenu->addFliDesc("mscrenup.dat", 50, false, false, false, menu_up);
        pMenu->setNextMenu(fs_game_menus::kMenuIdDebrief);
    } else if (menuId == fs_game_menus::kMenuIdFliMissionAborted) {
        pMenu->addFliDesc("mscrenup.dat", 50, false, false, false, menu_up);
        pMenu->setNextMenu(fs_game_menus::kMenuIdDebrief);
    } else if (menuId == fs_game_menus::kMenuIdFliTitle || menuId == fs_game_menus::kMenuIdFliIntro) {
        if ( menuId == fs_game_menus::kMenuIdFliIntro) {
            // Display the intro animation
            pMenu->addFliDesc("intro.dat", 95, false, true, true, intro, fs_eng::kSampleIntro);
        }
        // Display the splash screen
        pMenu->addFliDesc("mtitle.dat", 66, true, true, false, title, fs_eng::kSampleIntro);
        // common fli to prepare next menu show
        pMenu->addFliDesc("mscrenup.dat", 50, false, false, false, menu_up);
        pMenu->setNextMenu(fs_game_menus::kMenuIdMain);
    }

    return pMenu;
}

/*!
 * Return the name of the file for the show animation per Menu
 * @param menuId The id of the menu
* @return Empty string if no animation
 */
const char* GameMenuFactory::getShowAnimation(int menuId) {
    if (menuId == fs_game_menus::kMenuIdMain) {
        return "moption.dat";
    } else if (menuId == fs_game_menus::kMenuIdBrief) {
        return "mbrief.dat";
    } else if (menuId == fs_game_menus::kMenuIdConf) {
        return "mconfup.dat";
    } else if (menuId == fs_game_menus::kMenuIdDebrief) {
        return "mdebrief.dat";
    } else if (menuId == fs_game_menus::kMenuIdResearch) {
        return "mresrch.dat";
    } else if (menuId == fs_game_menus::kMenuIdSelect) {
        return "mselect.dat";
    } else if (menuId == fs_game_menus::kMenuIdLdSave) {
        return "mlosa.dat";
    } else if (menuId == fs_game_menus::kMenuIdMap) {
        return "mmap.dat";
    } else {
        return MenuFactory::getShowAnimation(menuId);
    }
}

/*!
 * Return the name of the file for the leave animation per Menu
 * @param menuId The id of the menu
 * @return Empty string if no animation
 */
const char* GameMenuFactory::getLeaveAnimation(int menuId) {
    if (menuId == fs_game_menus::kMenuIdMain) {
        return "moptout.dat";
    } else if (menuId == fs_game_menus::kMenuIdBrief) {
        return "mbrieout.dat";
    } else if (menuId == fs_game_menus::kMenuIdConf) {
        return "mconfout.dat";
    } else if (menuId == fs_game_menus::kMenuIdDebrief) {
        return "mdeout.dat";
    } else if (menuId == fs_game_menus::kMenuIdResearch) {
        return "mresout.dat";
    } else if (menuId == fs_game_menus::kMenuIdSelect) {
        return "mselout.dat";
    } else if (menuId == fs_game_menus::kMenuIdLdSave) {
        return "mlosaout.dat";
    } else if (menuId == fs_game_menus::kMenuIdMap) {
        return "mmapout.dat";
    } else {
        return MenuFactory::getLeaveAnimation(menuId);
    }
}
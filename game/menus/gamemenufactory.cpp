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

//! This defines the list of event for the intro animation
const FrameEvent intro[] = {
    { 1,  msc::TRACK_INTRO, NO_SOUND, 0x0, NULL }, // Play track
    { 15, msc::NO_TRACK, NO_SOUND,        0x0, "INTRO_0" },
    { 39, msc::NO_TRACK, NO_SOUND,        0x0, NULL }, // clear subtitle
    { 44, msc::NO_TRACK, NO_SOUND,        0x0, "INTRO_1" },
    { 62, msc::NO_TRACK, NO_SOUND,        0x0, NULL }, // clear subtitle
    { 67, msc::NO_TRACK, NO_SOUND,        0x0, "INTRO_2" },
    { 85, msc::NO_TRACK, NO_SOUND,        0x0, NULL }, // clear
    { 90, msc::NO_TRACK, NO_SOUND,        0x0, "INTRO_3" },
    { 117, msc::NO_TRACK, NO_SOUND,    0x0, NULL }, // clear
    { 121, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_4" },
    { 135, msc::NO_TRACK, NO_SOUND,    0x0, NULL }, // clear
    { 138, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_5" },
    { 153, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_6" },
    { 165, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_7" },
    { 178, msc::NO_TRACK, NO_SOUND,    0x0, NULL }, // Clear
    { 180, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_8" },
    { 200, msc::NO_TRACK, NO_SOUND,    0x0, NULL }, // clear
    { 234, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_9" },
    { 271, msc::NO_TRACK, NO_SOUND,    0x0, NULL },//clear
    { 291, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_10" },
    { 333, msc::NO_TRACK, NO_SOUND,    0x0, NULL }, // Clear
    { 359, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_11" },
    { 406, msc::NO_TRACK, NO_SOUND,    0x0, NULL }, // clear
    { 442, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_12" },
    { 467, msc::NO_TRACK, NO_SOUND,    0x0, NULL }, // clear
    { 473, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_13" },
    { 502, msc::NO_TRACK, NO_SOUND,    0x0, NULL }, // clear
    { 548, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_14" },
    { 562, msc::NO_TRACK, NO_SOUND,    0x0, NULL },
    { 569, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_15" },
    { 610, msc::NO_TRACK, NO_SOUND,    0x0, NULL },
    { 639, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_16" },
    { 673, msc::NO_TRACK, NO_SOUND,    0x0, NULL },
    { 848, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_17" },
    { 892, msc::NO_TRACK, NO_SOUND,    0x0, NULL },
    { 904, msc::NO_TRACK, NO_SOUND,    0x0, "INTRO_18" },
    { 960, msc::NO_TRACK, NO_SOUND,    0x0, NULL },
    {(uint16)-1, msc::NO_TRACK, NO_SOUND, 0x0, NULL }
};

//! This defines the list of event for the mission won animation
const FrameEvent mission_win[] = {
    { 1, msc::NO_TRACK, APPLAUSE,              0x1, NULL },
    { 1, msc::NO_TRACK, APPLAUSE_ZOOM,         0x0, NULL },
    { 10, msc::NO_TRACK, FIREWORKS,            0x1, NULL },
    { 30, msc::NO_TRACK, FIREWORKS_APPLAUSE,   0x1, NULL },
    {(uint16)-1, msc::NO_TRACK, NO_SOUND,              0x0, NULL }
};

//! This defines the list of event for the mission lost animation
const FrameEvent mission_failed[] = {
    { 1, msc::NO_TRACK, NO_SOUND, 0x0, NULL },
    { 35, msc::NO_TRACK, PRESS_BUTTON, 0x0, NULL },
    { 52, msc::NO_TRACK, MENU_AFTER_MISSION, 0x0, NULL },
    { 80, msc::NO_TRACK, SPEECH_NO, 0x0, NULL },
    { 108, msc::NO_TRACK, MONITOR_IMPACT, 0x0, NULL },
    { 118, msc::NO_TRACK, GLASS_BREAKING_2, 0x0, NULL },
    {(uint16)-1, msc::NO_TRACK, NO_SOUND, 0x0, NULL }
};

//! This defines the list of event for the game lost animation
const FrameEvent game_lost[] = {
    { 1, msc::NO_TRACK, NO_SOUND, 0x0, NULL },
    { 2, msc::NO_TRACK, FALLING_COMMAND_SHIP, 0x0, NULL },
    { 34, msc::NO_TRACK, EXPLOSION_BIG, 0x0, NULL },
    {(uint16)-1, msc::NO_TRACK, NO_SOUND, 0x0, NULL }
};

//! This defines the list of event for the menu up animation
const FrameEvent menu_up[] = {
    { 1, msc::NO_TRACK, MENU_UP,   0x1, NULL },
    {(uint16)-1, msc::NO_TRACK, NO_SOUND,  0x0, NULL }
};

//! This defines the list of no event
const FrameEvent no_event[] = {
    {(uint16)-1, msc::NO_TRACK, NO_SOUND, 0x0, NULL }
};

/*!
 * Create an instance of a Menu based on its id
 * @param menuId The id of the menu
 * @return Null of id is unknown
 */
Menu * GameMenuFactory::createCustomMenu(const int menuId) {
    Menu *pMenu = nullptr;

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
Menu * GameMenuFactory::createFliMenu(const int menuId) {
    FliMenu *pMenu = new FliMenu(pManager_, menuId, fs_game_menus::kMenuIdMain);
    if (pMenu->getId() == fs_game_menus::kMenuIdFliSuccess || menuId == fs_game_menus::kMenuIdFliFailedMission) {
        if (menuId == fs_game_menus::kMenuIdFliSuccess) {
            pMenu->addFliDesc("mgamewin.dat", 66, false, true, mission_win);
        } else if (menuId == fs_game_menus::kMenuIdFliFailedMission) {
            pMenu->addFliDesc("mendlose.dat", 66, false, true, mission_failed);
        } else if (menuId == fs_game_menus::kMenuIdFliGameLost) {
            // All availiable agents are dead
            // TODO: use it
            pMenu->addFliDesc("mlosegam.dat", 66, false, true, game_lost);
        }
        // common fli to prepare next menu show
        pMenu->addFliDesc("mscrenup.dat", 50, false, false, menu_up);
        pMenu->setNextMenu(fs_game_menus::kMenuIdDebrief);
    } else if (menuId == fs_game_menus::kMenuIdFliMissionAborted) {
        pMenu->addFliDesc("mscrenup.dat", 50, false, false, menu_up);
        pMenu->setNextMenu(fs_game_menus::kMenuIdDebrief);
    } else if (menuId == fs_game_menus::kMenuIdFliTitle || menuId == fs_game_menus::kMenuIdFliIntro) {
        if ( menuId == fs_game_menus::kMenuIdFliIntro) {
            // Display the intro animation
            pMenu->addFliDesc("intro.dat", 66, false, true, intro);
        }
        // Display the splash screen
        pMenu->addFliDesc("mtitle.dat", 66, true, true, no_event);
        // common fli to prepare next menu show
        pMenu->addFliDesc("mscrenup.dat", 50, false, false, menu_up);
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
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
#include "menus/logoutmenu.h"
#include "menus/flimenu.h"
#include "utils/log.h"

Menu * GameMenuFactory::createMenu(const int menuId) {
    Menu *pMenu = NULL;

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
    } else if (menuId == Menu::kMenuIdLogout) {
        pMenu =  new LogoutMenu(pManager_);
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
        menuId == fs_game_menus::kMenuIdFliIntro) {
        pMenu =  new FliMenu(pManager_, menuId);
    } else {
        FSERR(Log::k_FLG_UI, "GameMenuFactory", "createMenu", ("Cannot create Menu : unknown id (%d)", menuId));
    }

    return pMenu;
}
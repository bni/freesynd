/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2013, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "editormenufactory.h"

#include "fs-utils/log/log.h"

#include "editormenuid.h"
#include "mainmenu.h"
#include "fontmenu.h"
#include "animmenu.h"
#include "audiomenu.h"
#include "searchmissionmenu.h"
#include "listmissionmenu.h"

fs_eng::Menu * EditorMenuFactory::createCustomMenu(const int menuId) {
    fs_eng::Menu *pMenu = NULL;

    if (menuId == fs_edit_menus::kMenuIdMain) {
        pMenu =  new MainMenu(pManager_);
    } else if (menuId == fs_edit_menus::kMenuIdFont) {
        pMenu =  new FontMenu(pManager_);
    } else if (menuId == fs_edit_menus::kMenuIdAnim) {
        pMenu =  new AnimMenu(pManager_);
    } else if (menuId == fs_edit_menus::kMenuIdSrchMis) {
        pMenu = new SearchMissionMenu(pManager_);
    } else if (menuId == fs_edit_menus::kMenuIdListMis) {
        pMenu = new ListMissionMenu(pManager_);
    } else if (menuId == fs_edit_menus::kMenuIdAudio) {
        pMenu = new AudioMenu(pManager_);
    } else {
        FSERR(Log::k_FLG_UI, "EditorMenuFactory", "createMenu", ("Cannot create Menu : unknown id (%d)", menuId));
    }

    return pMenu;
}

//! Return the name of the animation to play before showing a menu
const char* EditorMenuFactory::getShowAnimation(int menuId) {
    if (menuId == fs_edit_menus::kMenuIdMain) {
        return "mscrenup.dat";
    } else if (menuId == fs_edit_menus::kMenuIdFont) {
        return "";
    } else if (menuId == fs_edit_menus::kMenuIdAnim) {
        return "mscrenup.dat";
    } else if (menuId == fs_edit_menus::kMenuIdSrchMis) {
        return "mscrenup.dat";
    } else if (menuId == fs_edit_menus::kMenuIdListMis) {
        return "mscrenup.dat";
    } else if (menuId == fs_edit_menus::kMenuIdAudio) {
        return "";
    } else {
        return MenuFactory::getShowAnimation(menuId);
    }
}

//! Return the name of the animation to play after leaving a menu
const char* EditorMenuFactory::getLeaveAnimation(int menuId) {
    return MenuFactory::getLeaveAnimation(menuId);
}

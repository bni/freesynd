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

#include "mainmenu.h"

#include <stdio.h>
#include <assert.h>

#include "fs-engine/system/system.h"
#include "core/gamecontroller.h"
#include "menus/gamemenuid.h"

using fs_eng::FontManager;

MainMenu::MainMenu(fs_eng::MenuManager * m):fs_eng::Menu(m, fs_game_menus::kMenuIdMain, fs_game_menus::kMenuIdMain, true)
{
    isCachable_ = false;
    cursorOnShow_ = kMenuCursor;
    addStatic(0, 40, fs_eng::kScreenWidth, "#MAIN_TITLE", FontManager::SIZE_4, false);

    int id = addOption(201, 130, 300, 25, "#MAIN_CONF", FontManager::SIZE_3, fs_game_menus::kMenuIdConf, true, false);
    registerHotKey(fs_eng::kKeyCode_F1, id);
    id = addOption(201, 164, 300, 25, "#MAIN_BEGIN", FontManager::SIZE_3, fs_game_menus::kMenuIdMap, true, false);
    registerHotKey(fs_eng::kKeyCode_F2, id);
    id = addOption(201, 198, 300, 25, "#MAIN_LOAD_SAVE", FontManager::SIZE_3, fs_game_menus::kMenuIdLdSave, true, false);
    registerHotKey(fs_eng::kKeyCode_F3, id);
    resetButId_ = addOption(201, 232, 300, 25, "#MAIN_RESET", FontManager::SIZE_3, fs_game_menus::kMenuIdMain, true, false);
    registerHotKey(fs_eng::kKeyCode_F4, resetButId_);
    quitButId_ = addOption(201, 266, 300, 25, "#MAIN_QUIT", FontManager::SIZE_3, kMenuIdNoMenu, true, false);
    registerHotKey(fs_eng::kKeyCode_F5, quitButId_);
}

void MainMenu::handleAction(const ActionDesc &action)
{
    if (action.id == resetButId_)
        g_gameCtrl.reset();
    if (action.id == quitButId_)
        menu_manager_->gotoMenu(Menu::kMenuIdLogout);
}

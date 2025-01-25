/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2011, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
 *   Copyright (C) 2011  Joey Parrish <joey.parrish@gmail.com>
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

#include "loadsavemenu.h"

#include <stdio.h>
#include <assert.h>

#include "fs-utils/io/file.h"
#include "fs-engine/appcontext.h"
#include "fs-engine/system/system.h"
#include "core/gamecontroller.h"
#include "menus/gamemenuid.h"

using fs_eng::FontManager;

const int LoadSaveMenu::X_ORIGIN = 165;
const int LoadSaveMenu::Y_ORIGIN = 100;
const size_t LoadSaveMenu::NAME_MAX_SIZE = 31;

LoadSaveMenu::LoadSaveMenu(fs_eng::MenuManager * m):
        Menu(m, fs_game_menus::kMenuIdLdSave, fs_game_menus::kMenuIdMain, true) {
    isCachable_ = false;
    cursorOnShow_ = kMenuCursor;
    // Title
    addStatic(0, 40, fs_eng::kScreenWidth, "#LS_TITLE", FontManager::SIZE_4, false);

    // Load button
    loadButId_ = addOption(46, 346, 99, 25, "#LS_LOAD_BUT", FontManager::SIZE_2);
    // Save button
    saveButId_ = addOption(147, 346, 99, 25, "#LS_SAVE_BUT", FontManager::SIZE_2);
    // Main menu button
    addOption(501, 346, 126, 25, "#MENU_MAIN_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdMain);

    std::string label;
    g_Ctx.getMessage("MENU_LB_EMPTY", label);
    fs_eng::TextField::setEmptyLabel(label);

    int y = Y_ORIGIN;
    for (int i=0; i<10; i++) {
        pTextFields_[i] = addTextField(X_ORIGIN, y, 370, 22, FontManager::SIZE_2, NAME_MAX_SIZE, true, true);
        y += 24;
    }

    // by default no line is edited
    editNameId_ = -1;
}

bool LoadSaveMenu::handleBeforeShow()
{
    // List of savefile names.
    std::vector<std::string> files;

    // initialize with 10 blanks
    for (int i=0; i<10; i++) {
        files.push_back("");
    }

    fs_utl::File::getGameSavedNames(files);

    for (int i=0; i<10; i++) {
        pTextFields_[i]->setText(files[i].c_str());
    }

    return true;
}

void LoadSaveMenu::handleLeave() {
    if (editNameId_ != -1) {
        pTextFields_[editNameId_]->setHighlighted(false);
    }
    editNameId_ = -1;
}

void LoadSaveMenu::handleAction(const int actionId, void *ctx) {
    if (actionId == loadButId_) {
        if (editNameId_ != -1) {
            if (g_gameCtrl.loadGameFromFile(editNameId_)) {
                editNameId_ = -1;
                menu_manager_->gotoMenu(fs_game_menus::kMenuIdMain);
            }
        }
    } else if (actionId == saveButId_) {
        if (editNameId_ != -1 && pTextFields_[editNameId_]->getText().size() != 0) {
            if (g_gameCtrl.saveGameToFile(editNameId_, pTextFields_[editNameId_]->getText())) {
                editNameId_ = -1;
                menu_manager_->gotoMenu(fs_game_menus::kMenuIdMain);
            }
        }
    }
}

bool LoadSaveMenu::handleMouseDown(Point2D point, int button) {
    for (int i=0; i<10; i++) {
        if (pTextFields_[i]->isMouseOver(point)) {
            editNameId_ = i;
            break;
        }
    }

    return false;
}

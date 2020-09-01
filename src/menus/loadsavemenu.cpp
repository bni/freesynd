/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2011  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2011  Joey Parrish <joey.parrish@gmail.com>          *
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

#include <stdio.h>
#include <assert.h>
#include "app.h"
#include "loadsavemenu.h"
#include "menus/gamemenuid.h"
#include "utils/file.h"

const int LoadSaveMenu::X_ORIGIN = 165;
const int LoadSaveMenu::Y_ORIGIN = 100;
const size_t LoadSaveMenu::NAME_MAX_SIZE = 31;

LoadSaveMenu::LoadSaveMenu(MenuManager * m):Menu(m, fs_game_menus::kMenuIdLdSave, fs_game_menus::kMenuIdMain, "mlosa.dat",
     "mlosaout.dat")
{
    isCachable_ = false;
    // Title
    addStatic(0, 40, g_Screen.gameScreenWidth(), "#LS_TITLE", FontManager::SIZE_4, false);

    // Load button
    loadButId_ = addOption(46, 346, 99, 25, "#LS_LOAD_BUT", FontManager::SIZE_2);
    // Save button
    saveButId_ = addOption(147, 346, 99, 25, "#LS_SAVE_BUT", FontManager::SIZE_2);
    // Main menu button
    addOption(501, 346, 126, 25, "#MENU_MAIN_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdMain);

    std::string label;
    g_Ctx.getMessage("MENU_LB_EMPTY", label);
    TextField::setEmptyLabel(label);
    
    int y = Y_ORIGIN;
    for (int i=0; i<10; i++) {
        pTextFields_[i] = addTextField(X_ORIGIN, y, 370, 22, FontManager::SIZE_2, NAME_MAX_SIZE, true, true);
        y += 24;
    }

    // by default no line is edited
    editNameId_ = -1;
}

void LoadSaveMenu::handleShow()
{
    // List of savefile names.
    std::vector<std::string> files;
    for (int i=0; i<10; i++) {
        files.push_back("");
    }
    File::getGameSavedNames(files);

    for (int i=0; i<10; i++) {
        pTextFields_[i]->setText(files[i].c_str());
    }

    menu_manager_->saveBackground();
    g_System.showCursor();
}

void LoadSaveMenu::handleLeave() {
    g_System.hideCursor();
    if (editNameId_ != -1) {
        pTextFields_[editNameId_]->setHighlighted(false);
    }
    editNameId_ = -1;
}

void LoadSaveMenu::handleAction(const int actionId, void *ctx, const int modKeys) {
    if (actionId == loadButId_) {
        if (editNameId_ != -1) {
            if (g_App.loadGameFromFile(editNameId_)) {
                editNameId_ = -1;
                menu_manager_->gotoMenu(fs_game_menus::kMenuIdMain);
            }
        }
    } else if (actionId == saveButId_) {
        if (editNameId_ != -1 && pTextFields_[editNameId_]->getText().size() != 0) {
            if (g_App.saveGameToFile(editNameId_, pTextFields_[editNameId_]->getText())) {
                editNameId_ = -1;
                menu_manager_->gotoMenu(fs_game_menus::kMenuIdMain);
            }
        }
    }
}

bool LoadSaveMenu::handleMouseDown(int x, int y, int button, const int modKeys) {
    for (int i=0; i<10; i++) {
        if (pTextFields_[i]->isMouseOver(x, y)) {
            editNameId_ = i;
            break;
        }
    }

    return false;
}

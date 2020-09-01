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

#include "menus/menu.h"
#include "menus/menumanager.h"
#include "editor/mainmenu.h"
#include "editor/editormenuid.h"
#include "gfx/screen.h"
#include "system.h"

MainMenu::MainMenu(MenuManager * m):Menu(m, fs_edit_menus::kMenuIdMain, fs_edit_menus::kMenuIdMain, "mscrenup.dat", "")
{
    isCachable_ = false;
    addStatic(0, 40, g_Screen.gameScreenWidth(), "GAME EDITOR", FontManager::SIZE_4, false);

    addStatic(201, 130, 100, "GRAPHICS", FontManager::SIZE_3, false);
    addOption(201, 155, 130, 25, "- MENU SPRITES", FontManager::SIZE_2, fs_edit_menus::kMenuIdFont, true, false);
    addOption(201, 180, 130, 25, "- ANIMATIONS", FontManager::SIZE_2, fs_edit_menus::kMenuIdAnim, true, false);

    addStatic(201, 210, 100, "MISSIONS", FontManager::SIZE_3, false);
    addOption(210, 235, 130, 25, "- SEARCH", FontManager::SIZE_2, fs_edit_menus::kMenuIdSrchMis, true, false);
    quitButId_ = addOption(201, 300, 300, 25, "#MAIN_QUIT", FontManager::SIZE_3, MENU_NO_MENU, true, false);
}

void MainMenu::handleShow()
{
    // If we came from the intro, the cursor is invisible
    // otherwise, it does no harm
    g_System.useMenuCursor();
    g_System.showCursor();
}

void MainMenu::handleLeave() {
    g_System.hideCursor();
}

void MainMenu::handleAction(const int actionId, void *ctx, const int modKeys)
{
    if (actionId == quitButId_)
        menu_manager_->gotoMenu(Menu::kMenuIdLogout);
}

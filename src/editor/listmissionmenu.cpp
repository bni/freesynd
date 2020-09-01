/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2015  Benoit Blancard <benblan@users.sourceforge.net>*
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
#include "editor/listmissionmenu.h"
#include "editor/editormenuid.h"
#include "editor/editorapp.h"
#include "gfx/screen.h"
#include "system.h"

ListMissionMenu::ListMissionMenu(MenuManager * m):
    Menu(m, fs_edit_menus::kMenuIdListMis, fs_edit_menus::kMenuIdSrchMis, "mscrenup.dat", "")
{
    isCachable_ = false;
    addStatic(0, 40, g_Screen.gameScreenWidth(), "MISSIONS FOUND", FontManager::SIZE_4, false);

    // Display list of missions found in search menu
    int nbRes = 0;
    int x = 20;
    int y, topY = 100;

    for (std::list < int >::iterator it = g_App.getMissionResultList().begin();
         it != g_App.getMissionResultList().end(); it++) {

             int missionId = *it;
             char label[50];

             if (missionId < 10) {
                sprintf(label, "#CNTRY_0%d", missionId);
             } else {
                sprintf(label, "#CNTRY_%d", missionId);
             }

             if (nbRes % 10 == 0) {
                y = topY;
                x += (nbRes == 0 ? 0 : 130);
             } else {
                 y += 25;
             }
             addOption(x, y, 130, 25, label, FontManager::SIZE_2, fs_edit_menus::kMenuIdSrchMis);
             nbRes++;
    }

    // Back button
    addOption(17, 347, 128, 25, "BACK", FontManager::SIZE_2, fs_edit_menus::kMenuIdSrchMis);
}

void ListMissionMenu::handleShow()
{
    // If we came from the intro, the cursor is invisible
    // otherwise, it does no harm
    g_System.useMenuCursor();
    g_System.showCursor();
}

void ListMissionMenu::handleLeave() {
    g_System.hideCursor();
}

void ListMissionMenu::handleAction(const int actionId, void *ctx, const int modKeys)
{

}

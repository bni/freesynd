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

#include "editor/editormenufactory.h"
#include "editor/editormenuid.h"
#include "utils/log.h"
#include "editor/mainmenu.h"
#include "editor/logoutmenu.h"
#include "editor/fontmenu.h"
#include "editor/animmenu.h"
#include "editor/searchmissionmenu.h"
#include "editor/listmissionmenu.h"

Menu * EditorMenuFactory::createMenu(const int menuId) {
    Menu *pMenu = NULL;

    if (menuId == fs_edit_menus::kMenuIdMain) {
        pMenu =  new MainMenu(pManager_);
    } else if (menuId == Menu::kMenuIdLogout) {
        pMenu =  new LogoutMenu(pManager_);
    } else if (menuId == fs_edit_menus::kMenuIdFont) {
        pMenu =  new FontMenu(pManager_);
    } else if (menuId == fs_edit_menus::kMenuIdAnim) {
        pMenu =  new AnimMenu(pManager_);
    } else if (menuId == fs_edit_menus::kMenuIdSrchMis) {
        pMenu = new SearchMissionMenu(pManager_);
    } else if (menuId == fs_edit_menus::kMenuIdListMis) {
        pMenu = new ListMissionMenu(pManager_);
    } else {
        FSERR(Log::k_FLG_UI, "EditorMenuFactory", "createMenu", ("Cannot create Menu : unknown id (%d)", menuId));
    }

    return pMenu;
}

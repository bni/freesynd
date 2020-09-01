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

#include "editor/fontmenu.h"
#include "menus/menumanager.h"
#include "editor/editormenuid.h"
#include "gfx/screen.h"
#include "system.h"

FontMenu::FontMenu(MenuManager * m):
    Menu(m, fs_edit_menus::kMenuIdFont, fs_edit_menus::kMenuIdMain, "", "")
{
    isCachable_ = false;

}

void FontMenu::handleShow()
{
    // If we came from the intro, the cursor is invisible
    // otherwise, it does no harm
    g_System.useMenuCursor();
    g_System.showCursor();

    g_Screen.clear(0);
    menu_manager_->setDefaultPalette();
    displayFont();
}

void FontMenu::handleLeave() {
    g_System.hideCursor();
}

void FontMenu::displayFont() {

    int spriteX = 0;
    int spriteY = 0;
    int lineMaxHeight = 0;
    for (int s = 0; s < menuSprites().spriteCount(); s++) {
        int sprh = menuSprites().sprite(s)->height();
        int sprw = menuSprites().sprite(s)->width();

        if (sprw != 0 && sprh != 0) {
            if (sprh > lineMaxHeight) {
                lineMaxHeight = sprh;
            }

            if (spriteX + sprw > g_Screen.gameScreenWidth()) {
                spriteX = 0;
                spriteY += lineMaxHeight;
                lineMaxHeight = sprh;
            }

            if (spriteY + sprh > g_Screen.gameScreenHeight()) {
                printf("Fin de boucle\n");
                break;
            }

            menuSprites().drawSpriteXYZ(s, spriteX, spriteY, 0);
            spriteX += sprw;
        }
    }
}


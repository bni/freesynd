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

#include "fontmenu.h"

#include "fs-engine/menus/menumanager.h"
#include "fs-engine/system/system.h"

#include "editormenuid.h"

using fs_eng::MenuManager;
using fs_eng::Menu;
using fs_eng::FontManager;

const size_t FontMenu::kSpriteMaxObjects = 84;

FontMenu::FontMenu(MenuManager * m):
    Menu(m, fs_edit_menus::kMenuIdFont, fs_edit_menus::kMenuIdMain)
{
    isCachable_ = false;
    cursorOnShow_ = kMenuCursor;

    addStatic(0, 40, fs_eng::kScreenWidth, "MENUS SPRITES", FontManager::SIZE_4, false);
    idButtonSprites_ = addOption(510, 155, 120, 25, "SPRITES", FontManager::SIZE_2, -1, true, true);
    idButtonFonts_ = addOption(510, 185, 120, 25, "FONTS", FontManager::SIZE_2, -1, true, true);
    idButtonSpriteset_ = addOption(510, 225, 120, 25, "SPRSET", FontManager::SIZE_2, -1, true, true);

    addStatic(0, 375, fs_eng::kScreenWidth, "MORE", FontManager::SIZE_1, false);
}

void FontMenu::handleShow()
{
    menu_manager_->getColorFromMenuPalette(fs_eng::kMenuPaletteColorDarkGreen, defaultColor_);
    panelId_ = 0;
}

void FontMenu::handleLeave() {
    g_System.hideCursor();
}

void FontMenu::handleAction(const int actionId, [[maybe_unused]] void *ctx) {
    panelId_ = actionId;
}

void FontMenu::handleRender(DirtyList &dirtyList) {
    g_System.drawRect({10, 70}, 490, 300, defaultColor_);

    if (panelId_ == idButtonSprites_) {
        displayObjects();
    }else if (panelId_ == idButtonFonts_) {
        displayFonts();
    } else if (panelId_ == idButtonSpriteset_) {
        displaySpriteset();
    }
}

void FontMenu::displayObjects() {
    displaySprite({12, 72}, 0, kSpriteMaxObjects);
}

void FontMenu::displayFonts() {
    // Font 1 : 254, 117
    int offset1 = 116;
    int offset2 = 254;
    int lastY = displaySprite({12, 72}, 84, 137);
    lastY = displaySprite({12, lastY + 10}, 221, 137);

    // Font 2 : 528, 391
    offset1 = 391;
    offset2 = 528;
    lastY = displaySprite({12, lastY + 10}, 358, 137);
    lastY = displaySprite({12, lastY + 10}, 495, 137);

    // Font 3 : 802, 665
    offset1 = 665;
    offset2 = 802;
    lastY = displaySprite({12, lastY + 10}, 632, 137);
    lastY = displaySprite({12, lastY + 10}, 769, 137);

    // Font 4 : 1076, 939
    offset1 = 939;
    offset2 = 1076;
    lastY = displaySprite({12, lastY + 10}, 906, 137);
    lastY = displaySprite({12, lastY + 10}, offset2 - 33, 137);
}

int FontMenu::displaySprite(Point2D startAt, int fromId, int range) {
    int startX = startAt.x;
    Point2D drawAt = startAt;
    int lineMaxHeight = 0;
    int totalHeight = 0;
    for (int s = fromId; s < fromId + range; s++) {
        int sprh = menuSprites().sprite(s)->height();
        int sprw = menuSprites().sprite(s)->width();

        if (sprw != 0 && sprh != 0) {
            if (sprh > lineMaxHeight) {
                lineMaxHeight = sprh;
            }

            if (drawAt.x + sprw > 500) {
                drawAt.x = startX;
                drawAt.y += lineMaxHeight + 1;
                lineMaxHeight = sprh;
            }

            if (drawAt.y + sprh > 370) {
                printf("overflow\n");
                break;
            }

            menuSprites().drawSprite(s, drawAt.x, drawAt.y);
            drawAt.x += sprw + 1;
            totalHeight = drawAt.y + sprh;
        }
    }

    return totalHeight;
}

void FontMenu::displaySpriteset() {
    menuSprites().drawTexture({0, 0}, {12, 72}, 486, 296);
}

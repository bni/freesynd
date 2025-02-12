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

#include "animmenu.h"

#include "fs-engine/menus/menumanager.h"
#include "fs-engine/system/system.h"
#include "fs-engine/gfx/animationmanager.h"

#include "editormenuid.h"

using fs_eng::MenuManager;
using fs_eng::Menu;
using fs_eng::FontManager;

AnimMenu::AnimMenu(MenuManager * m)
        : Menu(m, fs_edit_menus::kMenuIdFont, fs_edit_menus::kMenuIdMain, true) {
    isCachable_ = false;
    cursorOnShow_ = kMenuCursor;
    animId_ = 416;
    frameId_ = 0;

    addStatic(0, 40, fs_eng::kScreenWidth, "ANIMATIONS", FontManager::SIZE_4, false);
    // Accept button
    addOption(17, 347, 128, 25, "BACK", FontManager::SIZE_2, fs_edit_menus::kMenuIdMain);

    // Animation id
    addStatic(180, 300, "ANIMATION:", FontManager::SIZE_2, true);
    txtAnimId_ = addStatic(300, 300, "416", FontManager::SIZE_2, true);

    // frame id
    addStatic(180, 350, "FRAME:", FontManager::SIZE_2, true);
    txtFrameId_ = addStatic(300, 350, "0", FontManager::SIZE_2, true);
}

void AnimMenu::handleRender()
{
    //g_Screen.drawRect(150, 110, 350, 150);
    g_System.drawRect({150, 110}, 350, 150, menu_manager_->kMenuColorBlack);
    Point2D pos = {310, 180};
    g_SpriteMgr.drawFrame(animId_, frameId_, pos);

}

void AnimMenu::handleLeave() {
    g_System.hideCursor();
}

void AnimMenu::displayFont() {


}

bool AnimMenu::handleUnMappedKey(const fs_eng::FS_Key key) {
    bool change = false;
    if (key.keyCode == fs_eng::kKeyCode_Up) {
        if (animId_ < g_SpriteMgr.numAnims() - 1) {
            animId_++;
            frameId_ = 0;
            change = true;
        }
    } else if (key.keyCode == fs_eng::kKeyCode_Down) {
        if (animId_ > 0) {
            animId_--;
            frameId_ = 0;
            change = true;
        }
    } else if (key.keyCode == fs_eng::kKeyCode_Right) {
        frameId_++;
        if (frameId_ > g_SpriteMgr.lastFrame(animId_)) {
            frameId_ = 0;
        }
        change = true;
    } else if (key.keyCode == fs_eng::kKeyCode_Left) {
        frameId_--;
        if (frameId_ < 0) {
            frameId_ = g_SpriteMgr.lastFrame(animId_);
        }
        change = true;
    }

    if (change) {
        //printf("Animation #%d, frame #%d\n", animId_, frameId_);
        char tmp[100];
        sprintf(tmp, "%d", animId_);
        getStatic(txtAnimId_)->setText(tmp);

        sprintf(tmp, "%d", frameId_);
        getStatic(txtFrameId_)->setText(tmp);

        needRendering();
    }

    return change;
}


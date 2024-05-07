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

#include "animmenu.h"

#include "fs-engine/menus/menumanager.h"
#include "fs-engine/gfx/screen.h"
#include "fs-engine/system/system.h"
#include "fs-engine/gfx/spritemanager.h"

#include "editormenuid.h"

AnimMenu::AnimMenu(MenuManager * m):
    Menu(m, fs_edit_menus::kMenuIdFont, fs_edit_menus::kMenuIdMain, "mscrenup.dat", "")
{
    isCachable_ = false;
    animId_ = 416;
    frameId_ = 0;

    addStatic(0, 40, g_Screen.gameScreenWidth(), "ANIMATIONS", FontManager::SIZE_4, false);
    // Accept button
    addOption(17, 347, 128, 25, "BACK", FontManager::SIZE_2, fs_edit_menus::kMenuIdMain);

    // Animation id
    addStatic(180, 300, "ANIMATION:", FontManager::SIZE_2, true);
    txtAnimId_ = addStatic(300, 300, "416", FontManager::SIZE_2, true);

    // frame id
    addStatic(180, 350, "FRAME:", FontManager::SIZE_2, true);
    txtFrameId_ = addStatic(300, 350, "0", FontManager::SIZE_2, true);
}

void AnimMenu::handleShow()
{
    // If we came from the intro, the cursor is invisible
    // otherwise, it does no harm
    g_System.useMenuCursor();
    g_System.showCursor();

    menu_manager_->setDefaultPalette();

    menu_manager_->saveBackground();
}

void AnimMenu::handleRender(DirtyList &dirtyList)
{
    g_Screen.drawRect(150, 110, 350, 150);
    Point2D pos = {310, 180};
    g_SpriteMgr.drawFrame(animId_, frameId_, pos);
}

void AnimMenu::handleLeave() {
    g_System.hideCursor();
}

void AnimMenu::displayFont() {


}

bool AnimMenu::handleUnMappedKey(const FS_Key key) {
    bool change = false;
    if (key.keyCode == kKeyCode_Up) {
        if (animId_ < g_SpriteMgr.numAnims() - 1) {
            animId_++;
            frameId_ = 0;
            change = true;
        }
    } else if (key.keyCode == kKeyCode_Down) {
        if (animId_ > 0) {
            animId_--;
            frameId_ = 0;
            change = true;
        }
    } else if (key.keyCode == kKeyCode_Right) {
        frameId_++;
        if (frameId_ > g_SpriteMgr.lastFrame(animId_)) {
            frameId_ = 0;
        }
        change = true;
    } else if (key.keyCode == kKeyCode_Left) {
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


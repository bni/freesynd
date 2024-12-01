/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2011  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2011  Joey Parrish <joey.parrish@gmail.com>          *
 *   Copyright (C) 2010-2024
 *                       Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "confmenu.h"

#include "fs-engine/menus/menumanager.h"
#include "menus/gamemenuid.h"
#include "core/gamecontroller.h"
#include "core/gamesession.h"
#include "fs-engine/gfx/screen.h"
#include "fs-engine/system/system.h"


ConfMenu::ConfMenu(MenuManager *m) :
        Menu(m, fs_game_menus::kMenuIdConf, fs_game_menus::kMenuIdMain, true) {
    isCachable_ = false;
    cursorOnShow_ = kMenuCursor;
    toAcceptLogo_ = 0;
    toAcceptColourId_ = 0;
    tempLogo_ = 0;
    tempColourId_ = 0;

    panelMsgId_ = addStatic(280, 32, 330, "#CONF_MAIN_MSG", FontManager::SIZE_2, true);

    logoButId_ = addOption(325, 65, 240, 20, "#CONF_COL_LOGO_BUT", FontManager::SIZE_2, MENU_NO_MENU, true, false, Sprite::MSPR_BULLET_D, Sprite::MSPR_BULLET_L);
    compNameButId_ = addOption(325, 90, 240, 20, "#CONF_COM_NAME_BUT", FontManager::SIZE_2, MENU_NO_MENU, true, false, Sprite::MSPR_BULLET_D, Sprite::MSPR_BULLET_L);
    userNameButId_ = addOption(325, 115, 240, 20, "#CONF_YOUR_NAME_BUT", FontManager::SIZE_2, MENU_NO_MENU, true, false, Sprite::MSPR_BULLET_D, Sprite::MSPR_BULLET_L);
    // Accept button
    acceptButId_ = addOption(17, 347, 128, 25, "#MENU_ACC_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdMain);
    // Main menu button
    menuButId_ = addOption(500, 347,  128, 25, "#MENU_MAIN_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdMain);

    createPanels();

    // Sub panel Ok and Cancel buttons
    okButId_ = addOption(291, 122, 125, 23, "#CONF_OK_BUT", FontManager::SIZE_2, MENU_NO_MENU, false);
    cancelButId_ = addOption(476, 122, 123, 23, "#MENU_CANCEL_BUT", FontManager::SIZE_2, MENU_NO_MENU, false);

    toAcceptCmpNameTxtId_ = addStatic(32, 93, "", FontManager::SIZE_1, false);
    toAcceptUsrNameTxtId_ = addStatic(32, 115, "", FontManager::SIZE_1, false);

    currPanel_ = kConfPanelMain;
}

ConfMenu::~ConfMenu() {
}

void ConfMenu::createPanels() {
    // Color picker
    colStaticId_ = addStatic(475, 60, "#CONF_COL_TITLE", FontManager::SIZE_2, true);
    getStatic(colStaticId_)->setVisible(false);
    leftColButId_ = addImageOption(405, 58, Sprite::MSPR_LEFT_ARROW_D, Sprite::MSPR_LEFT_ARROW_L, false);
    rightColButId_ = addImageOption(435, 58, Sprite::MSPR_RIGHT_ARROW_D, Sprite::MSPR_RIGHT_ARROW_L, false);

    // Logo picker
    leftLogoButId_ = addImageOption(405, 94, Sprite::MSPR_LEFT_ARROW_D, Sprite::MSPR_LEFT_ARROW_L, false);
    rightLogoButId_ = addImageOption(435, 94, Sprite::MSPR_RIGHT_ARROW_D, Sprite::MSPR_RIGHT_ARROW_L, false);
    logoStaticId_ = addStatic(475, 96, "#CONF_LOGO_TITLE", FontManager::SIZE_2, true);
    getStatic(logoStaticId_)->setVisible(false);

    // Change names textfields
    pUserNameTF_ = addTextField(312, 79, 255, 21, FontManager::SIZE_2, GameSession::kNameMaxSize);
    pCompNameTF_ = addTextField(312, 79, 255, 21, FontManager::SIZE_2, GameSession::kNameMaxSize);
}

void ConfMenu::handleRender(DirtyList &dirtyList) {
    // Draw the current logo
    g_LogoMgr.draw({28, 22}, toAcceptLogo_, toAcceptColourId_, true);

    if (currPanel_ == kConfPanelLogo) {
        // Draw the selected logo
        g_LogoMgr.draw({336, 55}, tempLogo_, tempColourId_, true);
    } else if (currPanel_ == kConfPanelPlayerCompanyName || currPanel_ == kConfPanelPlayerName) {
        // draw a frame around the textfield
        g_System.drawRect({300, 77}, 272, 26, menu_manager_->kMenuColorDarkGreen);
        g_System.drawRect({301, 78}, 270, 24, menu_manager_->kMenuColorDarkGreen);
    }

    if (currPanel_ != kConfPanelMain) {
        // draw frame around ok and cancel buttons
        g_System.drawRect({284, 122}, 136, 26, menu_manager_->kMenuColorDarkGreen);
        g_System.drawRect({285, 123}, 134, 24, menu_manager_->kMenuColorDarkGreen);
        g_System.drawRect({468, 122}, 136, 26, menu_manager_->kMenuColorDarkGreen);
        g_System.drawRect({469, 123}, 134, 24, menu_manager_->kMenuColorDarkGreen);
    }
}

void ConfMenu::handleShow() {

    toAcceptLogo_ = g_Session.getLogo();
    toAcceptColourId_ = g_Session.getLogoColour();

    getStatic(toAcceptUsrNameTxtId_)->setText(g_Session.getUserName(), false);
    getStatic(toAcceptCmpNameTxtId_)->setText(g_Session.getCompanyName(), false);
}

void ConfMenu::handleLeave() {
    showMainPanel();
}

void ConfMenu::handleAction(const int actionId, void *ctx) {
    if (actionId == logoButId_) {
        showLogoPanel();
    } else if (actionId == userNameButId_) {
        showUserNamePanel();
    } else if (actionId == compNameButId_) {
        showCompanyNamePanel();
    } else if (actionId == leftColButId_) {
        tempColourId_--;
        if (tempColourId_ < 0) {
            tempColourId_ = g_LogoMgr.kMaxColour - 1;
        }
        redrawPanel();
    } else if (actionId == rightColButId_) {
        tempColourId_ = (tempColourId_ + 1) % g_LogoMgr.kMaxColour;
        redrawPanel();
    } else if (actionId == leftLogoButId_) {
        tempLogo_--;
        if (tempLogo_ < 0 ) {
            tempLogo_ = g_LogoMgr.numLogos() - 1;
        }
        redrawPanel();
    } else if (actionId == rightLogoButId_) {
        tempLogo_ = (tempLogo_ + 1) % g_LogoMgr.numLogos();
        redrawPanel();
    } else if (actionId == okButId_) {
        if (currPanel_ == kConfPanelLogo) {
            toAcceptColourId_ = tempColourId_;
            toAcceptLogo_ = tempLogo_;
        } else if (currPanel_ == kConfPanelPlayerName) {
            getStatic(toAcceptUsrNameTxtId_)->setText(pUserNameTF_->getText().c_str(), false);
        } else {
            getStatic(toAcceptCmpNameTxtId_)->setText(pCompNameTF_->getText().c_str(), false);
        }
        redrawLogo();
        showMainPanel();
    } else if (actionId == cancelButId_) {
        showMainPanel();
    } else if (actionId == acceptButId_) {
        // Effectively change the user's informations
        g_gameCtrl.change_user_infos(getStatic(toAcceptCmpNameTxtId_)->getText().c_str(),
            getStatic(toAcceptUsrNameTxtId_)->getText().c_str(),
            toAcceptLogo_,
            toAcceptColourId_);
    }
}

bool ConfMenu::handleUnMappedKey(const FS_Key key) {
    if (currPanel_ != kConfPanelMain) {
        if (key.keyCode == kKeyCode_Escape) {
            showMainPanel();
            return true;
        } else if (key.keyCode == kKeyCode_Return) {
            if (currPanel_ == kConfPanelLogo) {
                toAcceptColourId_ = tempColourId_;
                toAcceptLogo_ = tempLogo_;
            } else if (currPanel_ == kConfPanelPlayerName) {
                getStatic(toAcceptUsrNameTxtId_)->setText(pUserNameTF_->getText().c_str(), false);
            } else {
                getStatic(toAcceptCmpNameTxtId_)->setText(pCompNameTF_->getText().c_str(), false);
            }
            redrawLogo();
            showMainPanel();
            return true;
        }
    }

    return false;
}

void ConfMenu::showMainPanel() {
    getOption(logoButId_)->setVisible(true);
    getOption(userNameButId_)->setVisible(true);
    getOption(compNameButId_)->setVisible(true);
    getStatic(panelMsgId_)->setText("#CONF_MAIN_MSG");

    getOption(okButId_)->setVisible(false);
    getOption(cancelButId_)->setVisible(false);
    getOption(acceptButId_)->setWidgetEnabled(true);
    getOption(menuButId_)->setWidgetEnabled(true);

    if (currPanel_ == kConfPanelLogo) {
        getOption(leftColButId_)->setVisible(false);
        getOption(rightColButId_)->setVisible(false);
        getOption(leftLogoButId_)->setVisible(false);
        getOption(rightLogoButId_)->setVisible(false);

        getStatic(colStaticId_)->setVisible(false);
        getStatic(logoStaticId_)->setVisible(false);
    } else if (currPanel_ == kConfPanelPlayerName) {
        pUserNameTF_->setVisible(false);
        pUserNameTF_->setText("");
        captureInputBy(NULL);
    } else if (currPanel_ == kConfPanelPlayerCompanyName) {
        pCompNameTF_->setVisible(false);
        pCompNameTF_->setText("");
        captureInputBy(NULL);
    }

    redrawPanel();
    currPanel_ = kConfPanelMain;
}

void ConfMenu::hideMainPanel() {
    getOption(logoButId_)->setVisible(false);
    getOption(userNameButId_)->setVisible(false);
    getOption(compNameButId_)->setVisible(false);

    getOption(acceptButId_)->setWidgetEnabled(false);
    getOption(menuButId_)->setWidgetEnabled(false);

    redrawPanel();
}

void ConfMenu::showLogoPanel() {
    currPanel_ = kConfPanelLogo;
    getStatic(panelMsgId_)->setText("#CONF_COL_LOGO_MSG");
    getOption(leftColButId_)->setVisible(true);
    getOption(rightColButId_)->setVisible(true);
    getOption(leftLogoButId_)->setVisible(true);
    getOption(rightLogoButId_)->setVisible(true);

    getOption(okButId_)->setVisible(true);
    getOption(cancelButId_)->setVisible(true);

    getStatic(colStaticId_)->setVisible(true);
    getStatic(logoStaticId_)->setVisible(true);

    tempColourId_ = toAcceptColourId_;
    tempLogo_ = toAcceptLogo_;

    hideMainPanel();
}

void ConfMenu::showUserNamePanel() {
    currPanel_ = kConfPanelPlayerName;
    getStatic(panelMsgId_)->setText("#CONF_YOUR_NAME_MSG");

    pUserNameTF_->setText(getStatic(toAcceptUsrNameTxtId_)->getText().c_str());
    pUserNameTF_->setVisible(true);
    captureInputBy(pUserNameTF_);

    getOption(okButId_)->setVisible(true);
    getOption(cancelButId_)->setVisible(true);

    hideMainPanel();
}

void ConfMenu::showCompanyNamePanel() {
    currPanel_ = kConfPanelPlayerCompanyName;
    getStatic(panelMsgId_)->setText("#CONF_COM_NAME_MSG");

    pCompNameTF_->setText(getStatic(toAcceptCmpNameTxtId_)->getText().c_str());
    pCompNameTF_->setVisible(true);
    captureInputBy(pCompNameTF_);

    getOption(okButId_)->setVisible(true);
    getOption(cancelButId_)->setVisible(true);

    hideMainPanel();
}

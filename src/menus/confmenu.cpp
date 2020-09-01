/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2011  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#include "confmenu.h"
#include "menus/menumanager.h"
#include "menus/gamemenuid.h"
#include "core/gamecontroller.h"
#include "core/gamesession.h"
#include "utils/file.h"
#include "gfx/screen.h"
#include "system.h"

#define NAME_MAX_SIZE 16
#define MAX_COLOUR 8

int g_Colours[MAX_COLOUR] = { 6, 7, 14, 3, 11, 12, 13, 15 };


ConfMenu::ConfMenu(MenuManager *m) :
Menu(m, fs_game_menus::kMenuIdConf, fs_game_menus::kMenuIdMain, "mconfup.dat", "mconfout.dat") {
    isCachable_ = false;
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

    currPanel_ = PNL_MAIN;

    // Initialize data for the buttons frame
    memset(butFrameData_, 255, 68 * 13);
    for (int i = 4; i < 66; i++) {
        butFrameData_[i + 0 * 68] = 16;
        butFrameData_[i + 11 * 68] = 16;
    }

    for (int j = 0; j < 12; j++) {
        butFrameData_[4 + j * 68] = 16;
        butFrameData_[66 + j * 68] = 16;
    }

    // Initialize data for the text field frame
    memset(tfFrameData_, 255, 136 * 13);

    for (int i = 4; i < 134; i++) {
        tfFrameData_[i + 0 * 136] = 16;
        tfFrameData_[i + 11 * 136] = 16;
    }

    for (int j = 0; j < 12; j++) {
        tfFrameData_[4 + j * 136] = 16;
        tfFrameData_[134 + j * 136] = 16;
    }
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
    pUserNameTF_ = addTextField(312, 79, 255, 21, FontManager::SIZE_2, NAME_MAX_SIZE);
    pCompNameTF_ = addTextField(312, 79, 255, 21, FontManager::SIZE_2, NAME_MAX_SIZE);
}

void ConfMenu::handleRender(DirtyList &dirtyList) {
    // Draw the current logo
    g_Screen.drawLogo(28, 22, toAcceptLogo_, g_Colours[toAcceptColourId_]);

    if (currPanel_ == PNL_LOGO) {
        // Draw the selected logo
        g_Screen.drawLogo(336, 55, tempLogo_, g_Colours[tempColourId_]);
    } else if (currPanel_ == PNL_CMPNM || currPanel_ == PNL_USRNM) {
        // draw a frame around the textfield
        g_Screen.scale2x(300, 77, 136, 13, tfFrameData_);
    }

    if (currPanel_ != PNL_MAIN) {
        // draw frame around ok and cancel buttons
        g_Screen.scale2x(283, 122, 68, 13, butFrameData_);
        g_Screen.scale2x(468, 122, 68, 13, butFrameData_);
    }
}

void ConfMenu::handleShow() {
    menu_manager_->saveBackground();

    toAcceptLogo_ = g_Session.getLogo();

    for (unsigned int i = 0; i < sizeof(g_Colours) / sizeof(int); i++) {
        if (g_Colours[i] == g_Session.getLogoColour())
            toAcceptColourId_ = i;
    }

    getStatic(toAcceptUsrNameTxtId_)->setText(g_Session.getUserName());
    getStatic(toAcceptCmpNameTxtId_)->setText(g_Session.getCompanyName());

    g_System.showCursor();
}

void ConfMenu::handleLeave() {
    showMainPanel();

    g_System.hideCursor();
}

void ConfMenu::handleAction(const int actionId, void *ctx, const int modKeys) {
    if (actionId == logoButId_) {
        showLogoPanel();
    } else if (actionId == userNameButId_) {
        showUserNamePanel();
    } else if (actionId == compNameButId_) {
        showCompanyNamePanel();
    } else if (actionId == leftColButId_) {
        tempColourId_--;
        if (tempColourId_ < 0) {
            tempColourId_ = MAX_COLOUR - 1;
        }
        redrawPanel();
    } else if (actionId == rightColButId_) {
        tempColourId_ = (tempColourId_ + 1) % MAX_COLOUR;
        redrawPanel();
    } else if (actionId == leftLogoButId_) {
        tempLogo_--;
        if (tempLogo_ < 0 ) {
            tempLogo_ = g_Screen.numLogos() - 1;
        }
        redrawPanel();
    } else if (actionId == rightLogoButId_) {
        tempLogo_ = (tempLogo_ + 1) % g_Screen.numLogos();
        redrawPanel();
    } else if (actionId == okButId_) {
        if (currPanel_ == PNL_LOGO) {
            toAcceptColourId_ = tempColourId_;
            toAcceptLogo_ = tempLogo_;
        } else if (currPanel_ == PNL_USRNM) {
            getStatic(toAcceptUsrNameTxtId_)->setText(pUserNameTF_->getText().c_str());
        } else {
            getStatic(toAcceptCmpNameTxtId_)->setText(pCompNameTF_->getText().c_str());
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
            g_Colours[toAcceptColourId_]);
    }
}

bool ConfMenu::handleUnknownKey(Key key, const int modKeys) {
    if (currPanel_ != PNL_MAIN) {
        if (key.keyFunc == KFC_ESCAPE) {
            showMainPanel();
            return true;
        } else if (key.keyFunc == KFC_RETURN) {
            if (currPanel_ == PNL_LOGO) {
                toAcceptColourId_ = tempColourId_;
                toAcceptLogo_ = tempLogo_;
            } else if (currPanel_ == PNL_USRNM) {
                getStatic(toAcceptUsrNameTxtId_)->setText(pUserNameTF_->getText().c_str());
            } else {
                getStatic(toAcceptCmpNameTxtId_)->setText(pCompNameTF_->getText().c_str());
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

    if (currPanel_ == PNL_LOGO) {
        getOption(leftColButId_)->setVisible(false);
        getOption(rightColButId_)->setVisible(false);
        getOption(leftLogoButId_)->setVisible(false);
        getOption(rightLogoButId_)->setVisible(false);

        getStatic(colStaticId_)->setVisible(false);
        getStatic(logoStaticId_)->setVisible(false);
    } else if (currPanel_ == PNL_USRNM) {
        pUserNameTF_->setVisible(false);
        pUserNameTF_->setText("");
        captureInputBy(NULL);
    } else if (currPanel_ == PNL_CMPNM) {
        pCompNameTF_->setVisible(false);
        pCompNameTF_->setText("");
        captureInputBy(NULL);
    }

    redrawPanel();
    currPanel_ = PNL_MAIN;
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
    currPanel_ = PNL_LOGO;
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
    currPanel_ = PNL_USRNM;
    getStatic(panelMsgId_)->setText("#CONF_YOUR_NAME_MSG");

    pUserNameTF_->setText(getStatic(toAcceptUsrNameTxtId_)->getText().c_str());
    pUserNameTF_->setVisible(true);
    captureInputBy(pUserNameTF_);

    getOption(okButId_)->setVisible(true);
    getOption(cancelButId_)->setVisible(true);

    hideMainPanel();
}

void ConfMenu::showCompanyNamePanel() {
    currPanel_ = PNL_CMPNM;
    getStatic(panelMsgId_)->setText("#CONF_COM_NAME_MSG");

    pCompNameTF_->setText(getStatic(toAcceptCmpNameTxtId_)->getText().c_str());
    pCompNameTF_->setVisible(true);
    captureInputBy(pCompNameTF_);

    getOption(okButId_)->setVisible(true);
    getOption(cancelButId_)->setVisible(true);

    hideMainPanel();
}

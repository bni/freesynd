/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010, 2024-2025 Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef CONFMENU_H
#define CONFMENU_H

#include "fs-engine/menus/menu.h"
/*!
 * Configuration Menu class.
 */
class ConfMenu : public fs_eng::Menu {
public:
    ConfMenu(fs_eng::MenuManager *m);
    ~ConfMenu();

    virtual void handleRender(DirtyList &dirtyList) override;
    bool handleBeforeShow() override;
    void handleLeave() override;

    void handleAction(const int actionId, void *ctx) override;

protected:

    bool handleUnMappedKey(const fs_eng::FS_Key key) override;

    void createPanels();
    void showLogoPanel();
    void showUserNamePanel();
    void showCompanyNamePanel();
    void showMainPanel();
    void hideMainPanel();

    void redrawLogo() { addDirtyRect(28, 22, 150, 110); }
    void redrawPanel() { addDirtyRect(278, 20, 340 , 135); }

protected:
    /*!
     * This enum identifies the panel which is currently displayed.
     */
    enum EPanel {
        //! Main panel that displays options to modify logo or names
        kConfPanelMain = 0,
        //! Panel to select the logo
        kConfPanelLogo = 1,
        //! Panel to edit the player name
        kConfPanelPlayerName = 2,
        //! Panel to edit the player company name
        kConfPanelPlayerCompanyName = 3
    };

    /*! keep track of the current panel.*/
    EPanel currPanel_;
    /*! Button to open the change logo panel.*/
    int logoButId_;
    /*! Button to open the change company name panel.*/
    int compNameButId_;
    /*! Button to open the change user name panel.*/
    int userNameButId_;
    /*! Button in the main window to accept change.*/
    int acceptButId_;
    /*! Button in the main window to cancel change.*/
    int menuButId_;
    int panelMsgId_;

    // Logo Panel
    int leftColButId_;
    int rightColButId_;
    int leftLogoButId_;
    int rightLogoButId_;
    int logoStaticId_;
    int colStaticId_;

    // Change names panel
    fs_eng::TextField *pUserNameTF_;
    fs_eng::TextField *pCompNameTF_;

    // Common buttons
    int okButId_;
    int cancelButId_;

    /*! Id of the menu text that stores the username that will be accepted.*/
    int toAcceptUsrNameTxtId_;
    /*! Id of the menu text that stores the company name that will be accepted.*/
    int toAcceptCmpNameTxtId_;
    int toAcceptLogo_;
    int toAcceptColourId_;
    int tempLogo_;
    int tempColourId_;
};

#endif

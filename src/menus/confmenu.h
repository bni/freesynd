/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef CONFMENU_H
#define CONFMENU_H

#include "menus/menu.h"
/*!
 * Configuration Menu class.
 */
class ConfMenu : public Menu {
public:
    ConfMenu(MenuManager *m);
    ~ConfMenu();

    virtual void handleRender(DirtyList &dirtyList);
    void handleShow();
    void handleLeave();

    void handleAction(const int actionId, void *ctx, const int modKeys);

protected:

    bool handleUnknownKey(Key key, const int modKeys);

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
        PNL_MAIN = 0,
        PNL_LOGO = 1,
        PNL_USRNM = 2,
        PNL_CMPNM = 3
    };

    /*! This array holds the values for drawing a frame
     *  around the ok and cancel button in the sub panels. */
    uint8 butFrameData_[68 * 13];
    /*! This array holds the values for drawing a frame
     *  around the textfield in the user and company name panels. */
    uint8 tfFrameData_[136 * 13];

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
    TextField *pUserNameTF_;
    TextField *pCompNameTF_;

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

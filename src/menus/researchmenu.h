/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#ifndef RESEARCHMENU_H
#define RESEARCHMENU_H

#include "core/gameevent.h"
#include "menus/menu.h"

class Research;
/*!
 * Research class.
 */
class ResearchMenu : public Menu, public GameEventListener {
public:
    ResearchMenu(MenuManager *m);

    void handleTick(int elapsed);
    void handleShow();
    void handleRender(DirtyList &dirtyList);
    void handleLeave();
    void handleAction(const int actionId, void *ctx, const int modKeys);
    void handleGameEvent(GameEvent evt);

protected:
    void updateClock();

    void showFieldList();
    void hideFieldList();

    void showDetailsList();
    void hideDetailsList();

    void showResInfo();
    void showResGraph(Research *pRes);
    //! Draws infos on the selected weapon
    void drawSelectedWeaponInfos(int x, int y);
    //! Draws infos on the selected mod
    void drawSelectedModInfos(int x, int y);

    void redrawGraph() { addDirtyRect(200, 110, 250, 250); } 

protected:
    enum ETab {
        TAB_MODS = 0,
        TAB_EQUIPS = 1
    };

    ETab tab_;
    /*! Weapon selected in the Equips tab.*/
    Weapon *pSelectedWeapon_;
    /*! Mod selected in the Mods tab.*/
    Mod *pSelectedMod_;
    /*! Research selected from the field list*/
    Research *pSelectedRes_;
    /*! Research used for displaying graph.*/
    Research *pResForGraph_;
    /*! Id of the text widget for time.*/
    int txtTimeId_;
    /*! Id of the mods toogle button.*/
    int modsButId_;
    /*! Id of the equipment toogle button.*/
    int equipButId_;
    /*! Id of the cancel search button.*/
    int cancelSearchId_;
    /*! Id of the search field button.*/
    int researchId_;
    /*! Id of the cancel description tab button.*/
    int cancelDescId_;
    /*! Id of the text field displaying current search field.*/
    int fieldTxtId_;
    /*! Id of the text field displaying current amount of money.*/
    int moneyTxtId_;
    int fundMinTxtId_;
    int fundMaxTxtId_;
    int fundMinLblId_;
    int fundMaxLblId_;
    /*! Id of the increase funding button.*/
    int incrFundId_;
    /*! Id of the decrease funding button.*/
    int decrFundId_;
    /*! Id of the text field displaying current search funding.*/
    int fundCurrLblId_;
    /*! Id of the text field displaying current search title.*/
    int searchTitleLblId_;
    /*! List box for all search on equips.*/
    ListBox *pFieldEquipLBox_;
    /*! List box for all search on mods.*/
    ListBox *pFieldModsLBox_;
    /*! List box for all available mods.*/
    ListBox *pModsLBox_;
    /*! List box for all available euips.*/
    ListBox *pEquipsLBox_;
};

#endif

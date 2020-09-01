/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2011  Mark <mentor66@users.sourceforge.net>          *
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

#ifndef SELECTMENU_H
#define SELECTMENU_H

#include "menus/menu.h"

class ListBox;
class WeaponInstance;
class Weapon;
class Mod;

/*!
 * Select Menu class.
 */
class SelectMenu : public Menu {
public:
    SelectMenu(MenuManager *m);
    ~SelectMenu();

    void handleTick(int elapsed);
    void handleShow();
    void handleRender(DirtyList &dirtyList);
    void handleLeave();
    void handleAction(const int actionId, void *ctx, const int modKeys);

protected:
    void handleMouseMotion(int x, int y, int state, const int modKeys);
    void handleMouseUp(int x, int y, int button, const int modKeys);
    bool handleMouseDown(int x, int y, int button, const int modKeys);
    void handleClickOnAgentSelector(const int agent_no, int button);

    void updateClock();
    //! Draws a focus around the selected agent picture
    void drawAgentSelector(int x, int y);
    void drawAgent();
    void drawSelectedWeaponInfos(int x, int y);
    void drawSelectedModInfos(int x, int y);

    //! Utility method to force redraw of agent selectors
    void dirtyAgentSelector() { addDirtyRect(16, 80, 130, 155); }

    void toggleAgent(int n);
    void updateAcceptEnabled();

    void showModWeaponPanel();
    void showItemList();
    void updateSelectedWeapon();

protected:
    enum ETab {
        TAB_MODS = 0,
        TAB_EQUIPS = 1,
        TAB_TEAM = 2
    };

    ETab tab_;
    /*! Id of the currently selected agent.*/
    int cur_agent_;
    /*! Counter to update the rnd_ field.*/
    int tick_count_;
    /*! A counter used to draw the agent selector.*/
    int rnd_;
    bool sel_all_;
    /*! Selected weapon on the weapon list.*/
    Weapon *pSelectedWeap_;
    /*! Selected weapon instance id on the current agent inventory.*/
    int selectedWInstId_;
    /*! Selected mod on the mods list.*/
    Mod *pSelectedMod_;
    /*! Id of the text widget presenting the selected agent name.*/
    int txtAgentId_;
    /*! Id of the text widget for time.*/
    int txtTimeId_;
    /*! Id of the text field displaying current amount of money.*/
    int moneyTxtId_;
    /*! Id of the team toogle button.*/
    int teamButId_;
    /*! Id of the mods toogle button.*/
    int modsButId_;
    /*! Id of the equipment toogle button.*/
    int equipButId_;
    /*! Id of the cancel button.*/
    int cancelButId_;
    /*! Id of the reload button.*/
    int reloadButId_;
    /*! Id of the purchase button.*/
    int purchaseButId_;
    /*! Id of the sell button.*/
    int sellButId_;
    /*! Id of the accept button.*/
    int acceptButId_;
    /*! The team list box.*/
    TeamListBox *pTeamLBox_;
    /*! The weapons list.*/
    ListBox *pWeaponsLBox_;
    /*! The mods list.*/
    ListBox *pModsLBox_;

    WeaponInstance *weapon_dragged_;
    struct screenPoint{
        int x;
        int y;
    };
    screenPoint weapon_pos_;
};

#endif

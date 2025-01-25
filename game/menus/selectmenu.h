/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
 *   Copyright (C) 2011  Mark <mentor66@users.sourceforge.net>
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

#ifndef SELECTMENU_H
#define SELECTMENU_H

#include "fs-engine/menus/menu.h"
#include "fs-utils/misc/timer.h"

class ListBox;
class WeaponInstance;
class Weapon;
class Mod;

/*!
 * Select Menu class.
 */
class SelectMenu : public fs_eng::Menu {
public:
    SelectMenu(fs_eng::MenuManager *m);
    ~SelectMenu();

    bool handleTick(uint32_t elapsed) override;
    bool handleBeforeShow() override;
    void handleRender(DirtyList &dirtyList) override;
    void handleLeave() override;
    void handleAction(const int actionId, void *ctx) override;

protected:
    void handleMouseMotion(Point2D point, uint32_t state) override;
    void handleMouseUp(Point2D point, int button) override;
    bool handleMouseDown(Point2D point, int button) override;
    void handleClickOnAgentSelector(const int agent_no, int button);

    void updateClock();
    //! Draws a focus around the selected agent picture
    void drawAgentSelector(Point2D pos);
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

    //! Size of a segment for the dotted line of the agent selector
    static const int kSegmentSize;

    ETab tab_;
    /*! Id of the currently selected agent.*/
    int cur_agent_;
    /*! Counter to update the rnd_ field.*/
    fs_utl::Timer timerSelector_;
    /*! This offset is used to draw the dash line for the selector.
     * It's incremented each time the timer is updated.
     */
    int dashOffset_;
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
    fs_eng::TeamListBox *pTeamLBox_;
    /*! The weapons list.*/
    fs_eng::ListBox *pWeaponsLBox_;
    /*! The mods list.*/
    fs_eng::ListBox *pModsLBox_;

    WeaponInstance *weapon_dragged_;
    Point2D weapon_pos_;
};

#endif

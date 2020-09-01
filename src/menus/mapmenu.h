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

#ifndef MAPMENU_H
#define MAPMENU_H

#include <assert.h>

#include "menus/menu.h"

//! Displays the mission selection map.
/*!
 * This menu displays a world map on which the player
 * selects the next mission to play.
 */
class MapMenu : public Menu {
public:
    MapMenu(MenuManager *m);
    ~MapMenu();

    void handleTick(int elapsed);
    void handleShow();
    void handleLeave();
    void handleAction(const int actionId, void *ctx, const int modKeys);

protected:
    void handleRender(DirtyList &dirtyList);
    bool handleUnknownKey(Key key, const int modKeys);
    bool handleMouseDown(int x, int y, int button, const int modKeys);
    //! Utility method to draw the mission selector
    void drawSelector();
    //! Utility method to update mission informations
    void handleBlockSelected();
    //! Update the game time display
    void updateClock();

protected:

    int mapblk_size_;
    /*! Contains the images of the differents blocks.*/
    uint8 *mapblk_data_;
    /*! A counter for the blinking line of the selector.*/
    int select_tick_count_;
    /*! A counter for the blinking available blocks. */
    int blk_tick_count_;
    /*! */
    bool blink_status_;

    /*! Id of the text widget for time.*/
    int txtTimeId_;
    /*! Id of the text widget for Own label status.*/
    int txtOwnLblId_;
    /*! Id of the text widget for Own status.*/
    int txtOwnId_;
    /*! Id of the text widget for Country name.*/
    int txtCountryId_;
    /*! Id of the text widget for population.*/
    int txtPopId_;
    /*! Id of the text widget for tax value.*/
    int txtTaxValueId_;
    /*! Id of the text widget for tax percentage.*/
    int txtTaxPctId_;

    /*! Id of the increment tax percentage button.*/
    int incrTaxButId_;
    /*! Id of the decrement tax percentage button.*/
    int decrTaxButId_;

    /*! Id of the briefing button.*/
    int briefButId_;
};

#endif

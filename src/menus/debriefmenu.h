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

#ifndef DEBRIEFMENU_H
#define DEBRIEFMENU_H

#include "menus/menu.h"
#include "core/gameevent.h"

class Mission;

/*!
 * Menu displaying statistics about the last mission played.
 */
class DebriefMenu : public Menu, public GameEventListener {
public:
    DebriefMenu(MenuManager *m);

    void handleShow();
    void handleRender(DirtyList &dirtyList);
    void handleLeave();

    void handleGameEvent(GameEvent evt);

    void updateStatsFields(Mission *pMission);
    void checkNewWeaponFound();

protected:
    /*! Id of the text widget for mission status.*/
    int txtStatusId_;
    /*! Id of the text widget for number of agents used.*/
    int txtUsedId_;
    /*! Id of the text widget for mission duration.*/
    int txtTimeId_;
    /*! Id of the text widget for number of ennemy agents captured.*/
    int txtAgentCapturedId_;
    /*! Id of the text widget for number of ennemy agents killed.*/
    int txtAgentKilledId_;
    /*! Id of the text widget for number of criminal killed.*/
    int txtCrimKilledId_;
    /*! Id of the text widget for number of civilian killed.*/
    int txtCivilKilledId_;
    /*! Id of the text widget for number of policemen killed.*/
    int txtPoliceKilledId_;
    /*! Id of the text widget for number of guard killed.*/
    int txtGardKilledId_;
    /*! Id of the text widget for number of people convinced.*/
    int txtConvincedId_;
    /*! Id of the text widget for shooting precision.*/
    int txtPrecisionId_;
    /*! Size of the line separator between title and statistics. */
    int separatorSize_;
    /*! Id of the text widget for displaying research alert.*/
    int txtSearchId_;
    /*! Id of the text widget for displaying new weapon found alert.*/
    int txtNewWeap1Id_;
    int txtNewWeap2Id_;
};

#endif

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#ifndef PEDMANAGER_H
#define PEDMANAGER_H

#include <vector>

#include "common.h"
#include "ped.h"
#include "model/leveldata.h"


/*!
 * Pedestrians manager class.
 */
class PedManager {
public:
    PedManager();
    virtual ~PedManager() {}

    PedInstance *loadInstance(const LevelData::People & ped_data, uint16 ped_idx, int map, uint32 playerGroupId);
protected:
    void initAnimation(Ped *pedanim, unsigned short baseAnim);
    //! Initialize the ped instance as our agent
    void initOurAgent(Agent *p_agent, unsigned int obj_group_id, PedInstance *pPed);
    //! Initialize the ped instance as an enemy agent
    void initEnemyAgent(PedInstance *pPed);
    //! Initialize the ped instance as a guard
    void initGuard(PedInstance *pPed);
    //! Initialize the ped instance as a police
    void initPolice(PedInstance *pPed);
    //! Initialize the ped instance as a civilian
    void initCivilian(PedInstance *pPed);
    //! Initialize the ped instance as a criminal
    void initCriminal(PedInstance *pPed);
};

#endif

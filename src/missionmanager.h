/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#ifndef MISSIONMANAGER_H
#define MISSIONMANAGER_H

#include <map>

#include "common.h"
#include "model/leveldata.h"
#include "ia/actions.h"

class Mission;
class MissionBriefing;
class WeaponInstance;
class VehicleInstance;
class PedInstance;

/*!
 * Mission manager class.
 * Stores information about all missions.
 */
class MissionManager {
public:
    MissionManager();
    //! Loads mission for the given mission id
    Mission *loadMission(int n);
    //! Loads briefing for the given mission id
    MissionBriefing *loadBriefing(int n);

private:
    /*!
     * NOTE: Original objects data is based on offsets, but our objects are different
     * in size and are not in a single memory block.
     * Because of this indexes in our data, "arrays" are mirrored for object's
     * position within original array.
     */
    struct DataIndex {
        // indexes within vehicle array
        uint16 vindx[64];
        // indexes within peds array
        uint16 pindx[256];
        // contains indexes for driver's vehicle
        uint16 driverindx[256];
        //uint16 windx[512];
        WeaponInstance * weapons[512];

        DataIndex() : weapons() {}
    };

private:
    //! When loading missions, possibly adds some info to the data
    void hackMissions(int n, uint8 *data);
    //! Reads the mission file and return a representation of that file
    bool load_level_data(int n, LevelData::LevelDataAll &level_data);
    // Instanciate a mission from the data file
    Mission * create_mission(LevelData::LevelDataAll &level_data);
    //! Creates all weapons
    void createWeapons(const LevelData::LevelDataAll &level_data, DataIndex &di, Mission *pMission);
    //! Creates a weapon from the game data
    WeaponInstance * create_weapon_instance(const LevelData::Weapons &gamdata);
    //! Creates all vehicles
    void createVehicles(const LevelData::LevelDataAll &level_data,
                            DataIndex &di, Mission *pMission);
    //! Creates a vehicle from the game data
    Vehicle * createVehicleInstance(const LevelData::Cars &gamdata, uint16 id, uint16 map);
    //! Creates all peds
    void createPeds(const LevelData::LevelDataAll &level_data,
                            DataIndex &di, Mission *pMission);
    void createScriptedActionsForPed(Mission *pMission,
                                        DataIndex &di,
                                        const LevelData::LevelDataAll &level_data,
                                        PedInstance *pPed);
    //! Creates objectives
    void createObjectives(const LevelData::LevelDataAll &level_data,
                            DataIndex &di, Mission *pMission);

    //! Export data for debug (will be moved in editor)
    void exportMissionData(LevelData::LevelDataAll &level_data, Mission *pMission);
};

#endif

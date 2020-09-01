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

#include <stdio.h>
#include <assert.h>
#include "mapmanager.h"
#include "utils/file.h"
#include "utils/log.h"

MapManager::MapManager()
{
}

MapManager::~MapManager()
{
    for (unsigned int i = 0; i < maps_.size(); i++)
        delete maps_[i];
}

/*!
 * Currently loads all tiles.
 * \return True if everything is ok
 */
bool MapManager::initialize()
{
    return tileManager_.loadTiles();
}

/*!
 * Loads the given map.
 * First look in the map cache if the map already exists.
 * If the map exists, returns it. Otherwise, creates a new one.
 * \param i_mapNum The map id.
 * \return NULL if map could not be loaded
 */
Map * MapManager::loadMap(uint16 i_mapNum)
{
    char tmp[100];
    int size;

    LOG(Log::k_FLG_IO, "MapManager", "loadMap()", ("loading map %i", i_mapNum));
    // First look in cache
    if (maps_.find(i_mapNum) != maps_.end()) {
        LOG(Log::k_FLG_IO, "MapManager", "loadMap()", ("Map is already in cache"));
        return maps_[i_mapNum];
    }
    
    // Not found so construct new one
    LOG(Log::k_FLG_IO, "MapManager", "loadMap()", ("Load new map"));
    sprintf(tmp, "map%02d.dat", i_mapNum);
    uint8 *mapData = File::loadOriginalFile(tmp, size);
    if (mapData == NULL) {
        return NULL;
    }

    maps_[i_mapNum] = new Map(&tileManager_, i_mapNum);
    maps_[i_mapNum]->loadMap(mapData);
    // patch for "YUKON" map
    if (i_mapNum == 0x27) {
        maps_[i_mapNum]->patchMap(60, 63, 1, 0x27);
        maps_[i_mapNum]->patchMap(61, 63, 1, 0x27);
        maps_[i_mapNum]->patchMap(62, 63, 1, 0x27);
        maps_[i_mapNum]->patchMap(60, 64, 1, 0x27);
        maps_[i_mapNum]->patchMap(61, 64, 1, 0x27);
        maps_[i_mapNum]->patchMap(62, 64, 1, 0x27);
        maps_[i_mapNum]->patchMap(60, 65, 1, 0x27);
        maps_[i_mapNum]->patchMap(61, 65, 1, 0x27);
        maps_[i_mapNum]->patchMap(62, 65, 1, 0x27);
        maps_[i_mapNum]->patchMap(60, 66, 1, 0x27);
        maps_[i_mapNum]->patchMap(61, 66, 1, 0x27);
        maps_[i_mapNum]->patchMap(62, 66, 1, 0x27);
        maps_[i_mapNum]->patchMap(60, 67, 1, 0x27);
        maps_[i_mapNum]->patchMap(61, 67, 1, 0x27);
        maps_[i_mapNum]->patchMap(62, 67, 1, 0x27);
    }
    // patch for "INDONESIA" map
    // TODO: find better way to block access for our agents
    if (i_mapNum == 0x5B) {
        maps_[i_mapNum]->patchMap(49, 27, 2, 0);
        maps_[i_mapNum]->patchMap(49, 28, 2, 0);
        maps_[i_mapNum]->patchMap(49, 29, 2, 0);
    }

    delete[] mapData;

    return maps_[i_mapNum];
}

/*!
 * Returns the map if found in cache.
 * The map must have been loaded first.
 * \param i_mapNum Map id.
 * \return NULL if map is not found
 */
Map *MapManager::map(int i_mapNum)
{
    //make sure the map is loaded
    if (maps_.find(i_mapNum) == maps_.end()) {
        LOG(Log::k_FLG_IO, "MapManager", "map", ("map %d not in cache", i_mapNum));
        return NULL;
    }

    return maps_[i_mapNum];
}



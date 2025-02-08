/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-kernel/mgr/mapmanager.h"

#include <format>

#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"

namespace fs_knl {

MapManager::MapManager(fs_eng::TileManager *pTileManager) : pTileManager_(pTileManager)
{
}

MapManager::~MapManager()
{
    for (unsigned int i = 0; i < maps_.size(); i++)
        delete maps_[i];
}

/*!
 * Loads the given map.
 * First look in the map cache if the map already exists.
 * If the map exists, returns it. Otherwise, creates a new one.
 * \param i_mapNum The map id.
 * \return NULL if map could not be loaded
 */
Map * MapManager::loadMap(uint16_t i_mapNum)
{
    LOG(Log::k_FLG_IO, "MapManager", "loadMap()", ("loading map %i", i_mapNum));
    // First look in cache
    if (maps_.find(i_mapNum) != maps_.end()) {
        LOG(Log::k_FLG_IO, "MapManager", "loadMap()", ("Map is already in cache"));
        return maps_[i_mapNum];
    }

    // Not found so construct new one
    std::string filename = std::format("map{:02}.dat", i_mapNum);
    LOG(Log::k_FLG_IO, "MapManager", "loadMap()", ("Load new map from file %s", filename.c_str()));
    size_t size;
    uint8 *mapData = fs_utl::File::loadOriginalFile(filename, size);
    if (mapData == NULL) {
        return NULL;
    }

    maps_[i_mapNum] = new Map(pTileManager_, i_mapNum);
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
 * @brief 
 * @param paletteId 
 * @return 
 */
bool MapManager::loadPalette(int paletteId) {    
    return pTileManager_->setPalette(paletteId);
}

}

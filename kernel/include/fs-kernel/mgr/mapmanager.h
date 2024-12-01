/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include <map>
#include "fs-utils/common.h"
#include "fs-kernel/model/map.h"
#include "fs-engine/gfx/tilemanager.h"

/*!
 * Map manager class.
 */
class MapManager {
public:
    MapManager(TileManager *pTileManager);
    ~MapManager();

    //! Load the map with the given id if not in cache
    Map * loadMap(uint16 i_mapNum);
    //! Sets the palette for the given mission
    bool loadPalette(int paletteId);

protected:
    //! a cache of the latest maps
    std::map<int, Map *> maps_;
    TileManager *pTileManager_;
};

#endif

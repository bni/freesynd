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

#ifndef TILEMANAGER_H
#define TILEMANAGER_H

#include "common.h"
#include "tile.h"

/*!
 * Tile manager loads and holds all the game tiles.
 */
class TileManager {
public:
    /*! The total number of tiles.*/
    static const int kNumOfTiles;

    TileManager();
    ~TileManager();
    //! Loads tiles from the file
    bool loadTiles();

    //! Returns tile with the given index
    Tile * getTile(uint8 index);

protected:
    //! Load a given tile
    Tile * loadTile(uint8 *tileData, uint8 id, Tile::EType type);
    //! Returns the good enum for the given data
    Tile::EType toTileType(uint8 data);

protected:
    //! All the tiles in the game
    Tile **a_tiles_;
};

#endif

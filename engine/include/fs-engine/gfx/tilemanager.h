/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2024  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "fs-utils/common.h"
#include "fs-engine/gfx/tile.h"

/*!
 * Tile manager loads and holds all the game tiles.
 */
class TileManager {
public:
    /*! The total number of tiles.*/
    static const int kNumOfTiles;
    //! The number of sub-tiles horizontally for a tile
    static const int kSubTilePerWidth;
    //! The number of sub-tiles vertically for a tile
    static const int kSubTilePerHeight;
    //! The total number of sub-tiles for a tile
    static const int kSubTilePerTile;
    /*!
     * The size in bytes of the indexes of subtiles for tile in the header file.
     * There are 6 subtiles of 4 bytes for each tile.
     */
    static const int kTileIndexSize;
    //! The length in bytes of the header : there are 256 tiles.
    static const int kTileHeaderLength;
    //! SUBTILE_WIDTH / PIXELS_PER_BLOCK
    static const int kBlocksPerSubTileRow;
    //! (COLOR_BYTES_PER_BLOCK + ALPHA_BYTES_PER_BLOCK) * BLOCKS_PER_SUBTILE_ROW
    static const int kSubTileRowLength;

    TileManager();
    ~TileManager();
    //! Loads tiles from the file
    bool loadTiles();

    //! Returns tile with the given index
    Tile * getTile(uint8 index);
    //! Draws the tile to the screen
    bool drawTile(const Tile *tile, int x, int y);

protected:
    //! Load a given tile
    void loadTile(int id, const uint8_t * tileData, Tile::EType type);
    //! Load a given sub-tile
    void loadSubTile(const uint8_t * data, int offset, int index, int stride, uint8_t * pixels);
    //! Returns the good enum for the given data
    Tile::EType toTileType(uint8_t data);

protected:
    //! All the tiles in the game
    Tile **tiles_;
};

#endif

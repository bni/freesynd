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

#include "fs-engine/gfx/tilemanager.h"

#include <stdio.h>
#include <assert.h>

#include "fs-utils/log/log.h"
#include "fs-utils/io/file.h"

/*!
 *
 */
void unpackBlocks4(const uint8_t * data, uint8_t * pixels)
{
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 8; ++i) {
            if (bitSet(data[j], 7 - i)) {
                pixels[j * 8 + i] = 255;        // transparent
            } else {
                pixels[j * 8 + i] =
                    static_cast < uint8_t >
                    ((bitValue(data[4 + j], 7 - i) << 0) & 0xff)
                    | static_cast < uint8_t >
                    ((bitValue(data[8 + j], 7 - i) << 1) & 0xff)
                    | static_cast < uint8_t >
                    ((bitValue(data[12 + j], 7 - i) << 2) & 0xff)
                    | static_cast < uint8_t >
                    ((bitValue(data[16 + j], 7 - i) << 3) & 0xff);
            }
        }
    }
}

/*!
 *
 */
void loadSubTile(const uint8_t * data, int offset, int index,
                 int stride, uint8_t * pixels)
{
    if (offset < TileManager::kTileHeaderLength)
        return;

    data += offset;

    for (int i = 0; i < Tile::kSubileHeight; ++i) {
        unpackBlocks4(data,
                      pixels + index + (Tile::kSubileHeight - 1 - i) * stride);
        data += TileManager::kSubtileRowLength;
    }
}

const int TileManager::kNumOfTiles = 256;
const int TileManager::kSubtilePerWidth = 2;
const int TileManager::kSubtilePerHeight = 3;
const int TileManager::kSubtilePerTile = 6;
const int TileManager::kTileIndexSize = 6 * 4;  // total of subtiles per tile * number of bytes for a subtile
const int TileManager::kTileHeaderLength = 256 * kTileIndexSize; // There a 256 tiles
const int TileManager::kBlocksPerSubtileRow = 32 / 8;
const int TileManager::kSubtileRowLength = (4 + 1) * kBlocksPerSubtileRow;

/*!
 * Default constructor.
 */
TileManager::TileManager()
{
    tiles_ = new Tile*[kNumOfTiles];
    memset(tiles_, 0, sizeof(Tile*) * kNumOfTiles);
}

/*!
 * Default destructor.
 */
TileManager::~TileManager()
{
    for (uint32 i = 0; i < (uint32)kNumOfTiles; i++) {
        delete tiles_[i];
    }
    delete [] tiles_;
}

/*!
 * Loads a tile from the tile data.
 * \param tileData Data containing all tiles
 * \param id Id of the tile to load
 * \param type The tile type
 * \return The loaded tile.
 */
Tile * TileManager::loadTile(const uint8_t * tilesData, int id, Tile::EType type)
{
    int offset = id * kTileIndexSize;
    uint8_t tilePixels[Tile::kTileWidth * Tile::kTileHeight];
    memset(tilePixels, 255, Tile::kTileWidth * Tile::kTileHeight);

    for (int i = 0; i < kSubtilePerWidth; ++i) {
        for (int j = 0; j < kSubtilePerHeight; ++j) {
            int subTileOffset =
                READ_LE_INT32(tilesData + offset +
                               (i * kSubtilePerHeight + j) * 4);
            loadSubTile(tilesData, subTileOffset,
                        (kSubtilePerHeight - 1 - j) * Tile::kSubileHeight * Tile::kTileWidth + i * Tile::kSubtileWidth,
                        Tile::kTileWidth, tilePixels);
        }
    }

    // If at least one pixel is not transparent
    // not_alpha will be true and so tile will be drawn
    bool notAlpha = false;
    for (int h = 0; h < Tile::kTileHeight; h++) {
        for (int w = 0; w < Tile::kTileWidth; w++)
            if (tilePixels[h * Tile::kTileHeight + w] != 255) {
                notAlpha = true;
                break;
            }
    }

    Tile *tile = new Tile(id, tilePixels, notAlpha, type);
    return tile;
}

/*!
 * Returns a constant from the enumeration EType for the
 * given value.
 * \param data This data comes from the mapcolumn file.
 * \return kNone if data is unkonwn
 */
Tile::EType TileManager::toTileType(uint8 data)
{
    switch(data) {
    case 0x01:
        return Tile::kSlopeSN;
    case 0x02:
        return Tile::kSlopeNS;
    case 0x03:
        return Tile::kSlopeEW;
    case 0x04:
        return Tile::kSlopeWE;
    case 0x05:
        return Tile::kGround;
    case 0x06:
        return Tile::kRoadSideEW;
    case 0x07:
        return Tile::kRoadSideWE;
    case 0x08:
        return Tile::kRoadSideSN;
    case 0x09:
        return Tile::kRoadSideNS;
    case 0x0A:
        return Tile::kWall;
    case 0x0B:
        return Tile::kRoadCurve;
    case 0x0C:
        return Tile::kHandrailLight;
    case 0x0D:
        return Tile::kRoof;
    case 0x0E:
        return Tile::kRoadPedCross;
    case 0x0F:
        return Tile::kRoadMark;
    case 0x10:
        return Tile::kUnknown;
    default:
        return Tile::kNone;
    }
}

/*!
 * Loads all tile from the file.
 */
bool TileManager::loadTiles()
{
    size_t size;
    uint8 *type_data;

    // first reads types
    //#define TILE_TYPES "col01.dat"
    type_data = File::loadOriginalFile("col01.dat", size);
    if (!type_data) {
        return false;
    }

    // then reads tiles
    uint8 *tileData = File::loadOriginalFile("hblk01.dat", size);

    if (!tileData) {
        FSERR(Log::k_FLG_IO, "TileManager", "loadTiles", ("Failed to load tiles data\n"));
        delete[] type_data;
        return false;
    }

    // Loads all tiles
    for (int i = 0; i < kNumOfTiles; ++i) {
        tiles_[i] = loadTile(tileData, i, toTileType(type_data[i]));
    }

    delete[] type_data;
    delete[] tileData;
    return true;
}

/*!
 * Returns the tile with the given index.
 * \param tileNum The tile index in the tile manager.
 * \return The tile or null if no tile is found for the index.
 */
Tile * TileManager::getTile(uint8 tileNum) {
    //make sure the tile is loaded
    // TODO : tileNum always < kNumOfTiles! uint16 tileNum?
#if 0
    if (tileNum >= kNumOfTiles) {
        FSERR(Log::k_FLG_GFX, "TileManager", "getTile", ("tile %d not loaded!", tileNum));
        return NULL;
    }
#endif

    return tiles_[tileNum];
}

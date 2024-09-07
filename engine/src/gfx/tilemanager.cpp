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
#include "fs-engine/gfx/screen.h"

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

const int TileManager::kNumOfTiles = 256;
const int TileManager::kTilesPerWidth = 16;
const int TileManager::kSubTilePerWidth = 2;
const int TileManager::kSubTilePerHeight = 3;
const int TileManager::kSubTilePerTile = 6;
const int TileManager::kTileIndexSize = 6 * 4;  // total of subtiles per tile * number of bytes for a subtile
const int TileManager::kTileHeaderLength = 256 * kTileIndexSize; // There a 256 tiles
const int TileManager::kBlocksPerSubTileRow = 32 / 8;
const int TileManager::kSubTileRowLength = (4 + 1) * kBlocksPerSubTileRow;

/*!
 * Default constructor.
 */
TileManager::TileManager()
{
    tiles_ = new Tile*[kNumOfTiles];
    memset(tiles_, 0, sizeof(Tile*) * kNumOfTiles);
    tilesPixels_ = new uint8_t[kNumOfTiles * Tile::kTileHeight * Tile::kTileWidth];
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

    delete [] tilesPixels_;
}

/*!
 * Loads a tile from the tile data.
 * \param id Id of the tile to load
 * \param tilesData Data containing all tiles
 * \param type The tile type
 */
void TileManager::loadTile(int id, const uint8_t * tilesData, Tile::EType type)
{
    int offset = id * kTileIndexSize;
    uint8_t tilePixels[Tile::kTileWidth * Tile::kTileHeight];
    memset(tilePixels, 255, Tile::kTileWidth * Tile::kTileHeight);

    for (int i = 0; i < kSubTilePerWidth; ++i) {
        for (int j = 0; j < kSubTilePerHeight; ++j) {
            int subTileOffset =
                READ_LE_INT32(tilesData + offset +
                               (i * kSubTilePerHeight + j) * 4);
            loadSubTile(tilesData, subTileOffset,
                        (kSubTilePerHeight - 1 - j) * Tile::kSubTileHeight * Tile::kTileWidth + i * Tile::kSubTileWidth,
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

    tiles_[id] = new Tile(id, tilePixels, notAlpha, type);
}

/*!
 *
 * \param
 * \param
 * \return
 *
 */
void TileManager::loadSubTile(const uint8_t * data, int subTileOffset, int index,
                 int stride, uint8_t * pixels)
{
    if (subTileOffset < TileManager::kTileHeaderLength)
        return;

    data += subTileOffset;

    for (int i = 0; i < Tile::kSubTileHeight; ++i) {
        unpackBlocks4(data,
                      pixels + index + (Tile::kSubTileHeight - 1 - i) * stride);
        data += TileManager::kSubTileRowLength;
    }
}

void TileManager::copyTilePixelsToSurface(int id, const uint8_t tilePixels) {
    if (id < 0 || id >= kNumOfTiles) {
        return;
    }

    Point2D dest(id % kTilesPerWidth * Tile::kTileWidth,
                 id / kTilesPerWidth * Tile::kTileHeight);

    for (int j = 0; j < Tile::kTileHeight; ++j) { // On parcourt les lignes d'une tile
        uint8 *cp_ptr_a_pixels = ptr_a_pixels;    // Sauve les pixels
        ptr_a_pixels -= kTileWidth;               // On recule de la largeur d'une tuile
        uint8 *cp_ptr_screen = ptr_screen;        // On copie ce qu'il y avait sur le screen
        ptr_screen += swidth;                     // On avance le pointeur de la taille de la largeur clipse
        for (int i = 0; i < Tile::kTileWidth; ++i) {      // Pour chaque pixel de la ligne
            int offset = dest.y * Tile::kTileHeight * kTilesPerWidth + i;
            uint8 c = *cp_ptr_a_pixels++;         // on prend la couleur du pixel de la tile
            if (c != 255)                         // S'il est pas transparent on le copie
                *cp_ptr_screen = c;
            ++cp_ptr_screen;                      // On avance le pointeur
        }
    }
/*
    int xlow = x < 0 ? 0 : x;
    int clipped_w = kTileWidth - (xlow - x);
    int xhigh = xlow + clipped_w >= swidth ? swidth : xlow + clipped_w;
    int ylow = y < 0 ? 0 : y;
    int clipped_h = kTileHeight - (ylow - y);
    int yhigh = ylow + clipped_h >= sheight ? sheight : ylow + clipped_h;

    uint8 *ptr_a_pixels = pixels_ + ((kTileHeight - 1) - (ylow - y)) * kTileWidth;
    uint8 *ptr_screen = screen + ylow * swidth + xlow;
    for (int j = ylow; j < yhigh; ++j)
    {
        uint8 *cp_ptr_a_pixels = ptr_a_pixels;
        ptr_a_pixels -= kTileWidth;
        uint8 *cp_ptr_screen = ptr_screen;
        ptr_screen += swidth;
        for (int i = xlow; i < xhigh; ++i) {
            uint8 c = *cp_ptr_a_pixels++;
            if (c != 255)
                *cp_ptr_screen = c;
            ++cp_ptr_screen;
        }
    }
    return true;*/
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
    uint8 *typesData;

    // first reads tile types
    typesData = File::loadOriginalFile("col01.dat", size);
    if (!typesData) {
        return false;
    }

    // then reads tiles
    uint8 *tilesData = File::loadOriginalFile("hblk01.dat", size);

    if (!tilesData) {
        FSERR(Log::k_FLG_IO, "TileManager", "loadTiles", ("Failed to load tiles data\n"));
        delete[] typesData;
        return false;
    }

    // Loads all tiles
    for (int i = 0; i < kNumOfTiles; ++i) {
        loadTile(i, tilesData, toTileType(typesData[i]));
    }

    delete[] typesData;
    delete[] tilesData;
    return true;
}

/*!
 * Returns the tile with the given index.
 * \param tileNum The tile index in the tile manager.
 * \return The tile or null if no tile is found for the index.
 */
Tile * TileManager::getTile(uint8 tileNum) {
    return tiles_[tileNum];
}

/*! \brief
 *
 * \param tile const Tile*
 * \param x int
 * \param y int
 * \return bool
 *
 */
bool TileManager::drawTile(const Tile *tile, int x, int y) {
    return tile->drawTo((uint8*) g_Screen.pixels(), g_Screen.gameScreenWidth(), g_Screen.gameScreenHeight(), x, y);
}

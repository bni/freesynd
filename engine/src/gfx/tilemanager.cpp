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
#include <format>

#include "fs-utils/log/log.h"
#include "fs-utils/io/file.h"
#include "fs-engine/system/system.h"

/*!
 *
 */
void unpackBlocks4(const uint8_t * data, uint8_t * pixels)
{
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 8; ++i) {
            if (fs_utl::bitSet(data[j], 7 - i)) {
                pixels[j * 8 + i] = 255;        // transparent
            } else {
                pixels[j * 8 + i] =
                    static_cast < uint8_t >
                    ((fs_utl::bitValue(data[4 + j], 7 - i) << 0) & 0xff)
                    | static_cast < uint8_t >
                    ((fs_utl::bitValue(data[8 + j], 7 - i) << 1) & 0xff)
                    | static_cast < uint8_t >
                    ((fs_utl::bitValue(data[12 + j], 7 - i) << 2) & 0xff)
                    | static_cast < uint8_t >
                    ((fs_utl::bitValue(data[16 + j], 7 - i) << 3) & 0xff);
            }
        }
    }
}

const int TileManager::kNumOfTiles = 256;
const int TileManager::kSubTilePerWidth = 2;
const int TileManager::kSubTilePerHeight = 3;
const int TileManager::kSubTilePerTile = 6;
const int TileManager::kTileIndexSize = 6 * 4;  // total of subtiles per tile * number of bytes for a subtile
const int TileManager::kTileHeaderLength = 256 * kTileIndexSize; // There a 256 tiles
const int TileManager::kBlocksPerSubTileRow = 32 / 8;
const int TileManager::kSubTileRowLength = (4 + 1) * kBlocksPerSubTileRow;
const int TileManager::kNumOfTilesPerRow = 16;
const int TileManager::kNumOfTilesPerCol = 16;
const int TileManager::kTileColorKeyIndex = 255;

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
 * \param id Id of the tile to load
 * \param tilesData Data containing all tiles
 * \param type The tile type
 * \param tilesetBuffer The tile will be copied in this buffer
 */
void TileManager::loadTile(int id, const uint8_t * tilesData, uint8 *typesData, uint8_t *tilesetBuffer)
{
    int offset = id * kTileIndexSize;
    uint8_t *tilePixels = new uint8_t[Tile::kTileWidth * Tile::kTileHeight];
    memset(tilePixels, 255, Tile::kTileWidth * Tile::kTileHeight);

    for (int i = 0; i < kSubTilePerWidth; ++i) {
        for (int j = 0; j < kSubTilePerHeight; ++j) {
            int subTileOffset =
                fs_utl::READ_LE_INT32(tilesData + offset +
                               (i * kSubTilePerHeight + j) * 4);
            loadSubTile(tilesData, subTileOffset,
                        (kSubTilePerHeight - 1 - j) * Tile::kSubTileHeight * Tile::kTileWidth + i * Tile::kSubTileWidth,
                        Tile::kTileWidth, tilePixels);
        }
    }

    copyTilePixelsToBuffer(id, tilePixels, tilesetBuffer);

    // If at least one pixel is not transparent
    // not_alpha will be true and so tile will be drawn
    bool notAlpha = false;
    for (int h = 0; h < Tile::kTileHeight; h++) {
        for (int w = 0; w < Tile::kTileWidth; w++) {
            if (tilePixels[h * Tile::kTileHeight + w] != kTileColorKeyIndex) {
                notAlpha = true;
                break;
            }
        }
    }

    delete [] tilePixels;

    Point2D textureLoc {
        (id % kNumOfTilesPerRow) * Tile::kTileWidth,
        (id / kNumOfTilesPerCol) * Tile::kTileHeight};
    tiles_[id] = new Tile(id, notAlpha, toTileType(typesData[id]), textureLoc);
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

/*!
 * Copies the content of a tile to a common temporary buffer that will be used
 * to initialize the tile texture. The buffer stores tiles as a 16x16 tileset.
 * Each tile is copied to the buffer at a position given by its id.
 * @param id Id of the tile
 * @param tilePixels 
 * @param tilesetBuffer Buffer to copy the tile to.
 */
void TileManager::copyTilePixelsToBuffer(int id, const uint8_t *tilePixels, uint8_t *tilesetBuffer) {
    if (id < 0 || id >= kNumOfTiles) {
        return;
    }

    // Coords in the destination surface
    int row = id / kNumOfTilesPerRow;
    int col = id % kNumOfTilesPerCol;
    // start of the tile pixels in the destination surface (upper left corner of tile)
    int tileOffsetDest = (col * Tile::kTileWidth) + (row * kNumOfTilesPerRow * Tile::kTileWidth * Tile::kTileHeight);

    // Copy line by line
    for (int j=0; j < Tile::kTileHeight; j++) {
        int lineOffsetDest = j * Tile::kTileWidth * kNumOfTilesPerRow;
        int lineOffsetSrc = (Tile::kTileHeight -1 - j) * Tile::kTileWidth;
        memcpy(tilesetBuffer + tileOffsetDest + lineOffsetDest, tilePixels + lineOffsetSrc, Tile::kTileWidth);
    }
}


/*!
 * Returns a constant from the enumeration EType for the
 * given value.
 * \param data This data comes from the mapcolumn file.
 * \return kNone if data is unkonwn
 */
Tile::EType TileManager::toTileType(uint8_t data)
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
    typesData = fs_utl::File::loadOriginalFile("col01.dat", size);
    if (!typesData) {
        return false;
    }

    // then reads tiles
    uint8 *tilesData = fs_utl::File::loadOriginalFile("hblk01.dat", size);

    if (!tilesData) {
        FSERR(Log::k_FLG_IO, "TileManager", "loadTiles", ("Failed to load tiles data\n"));
        delete[] typesData;
        return false;
    }

    // Loads all tiles and put pixels in a temporaty buffer
    uint8_t *tilesPixels = new uint8_t[kNumOfTiles * Tile::kTileHeight * Tile::kTileWidth];
    for (int i = 0; i < kNumOfTiles; ++i) {
        loadTile(i, tilesData, typesData, tilesPixels);
    }

    // Then init texture with the buffer
    tilesTexture_ = g_System.createTexture();
    bool res = tilesTexture_->create8bitsSurfaceFromData(tilesPixels, 
                                            kNumOfTilesPerRow * Tile::kTileWidth, 
                                            kNumOfTilesPerCol * Tile::kTileHeight,
                                            kTileColorKeyIndex);

    delete[] typesData;
    delete[] tilesData;
    delete [] tilesPixels;

    return res;
}


/*!
 * 
 * @param missionId Id of the mission to find the right palette 
 * @param sixbit 6 bits or 8 bits palette
 * @return true is ok
 */
//bool TileManager::setPaletteForMission(int missionId, bool sixbit) {
bool TileManager::setPalette(int paletteId) {
    std::string fname = std::format("hpal0{}.dat", paletteId);

    LOG(Log::k_FLG_GFX, "TileManager", "setPalette", ("Setting palette : %s", fname.c_str()))
    size_t size;
    uint8_t *paletteData = fs_utl::File::loadOriginalFile(fname, size);

    if (!paletteData) {
        FSERR(Log::k_FLG_GFX, "TileManager", "setPalette", ("Could not read file %s", fname.c_str()))
        return false;
    }

    for (size_t i = 0; i < fs_eng::kPaletteMaxColor; ++i) {
        uint8_t r = paletteData[i * 3 + 0];
        uint8_t g = paletteData[i * 3 + 1];
        uint8_t b = paletteData[i * 3 + 2];

        // multiply by 255 divide by 63 isn't good enough?
        palette_[i].r = (r << 2) | (r >> 4);
        palette_[i].g = (g << 2) | (g >> 4);
        palette_[i].b = (b << 2) | (b >> 4);
    }

    if (tilesTexture_->setPalette(palette_)) {
        return tilesTexture_->loadTextureFromSurface();
    }

    return false;
}

/*!
 * Returns the tile with the given index.
 * \param tileNum The tile index in the tile manager.
 * \return The tile or null if no tile is found for the index.
 */
Tile * TileManager::getTile(uint8 tileNum) {
    return tiles_[tileNum];
}

/*!
 * TODO : to be complemented
 * @param tile 
 * @param x 
 * @param y 
 * @return 
 */
bool TileManager::drawTile(const Tile *tile, int x, int y) {
    tilesTexture_->render(tile->textureLocation(), {x, y}, Tile::kTileWidth, Tile::kTileHeight);

    return true;
}

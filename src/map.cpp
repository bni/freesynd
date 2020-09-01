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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "map.h"
#include "utils/log.h"
#include "gfx/tilemanager.h"
#include "gfx/screen.h"

Map::Map(TileManager * tileManager, uint16 anId) : tile_manager_(tileManager)
{
    id_ = anId;
    a_tiles_ = NULL;
}

Map::~Map()
{
    delete[] a_tiles_;
}

bool Map::loadMap(uint8 * mapData)
{
    LOG(Log::k_FLG_GFX, "Map", "loadMap", ("Loading Map %d.", id_));
    max_x_ = READ_LE_UINT32(mapData + 0);
    max_y_ = READ_LE_UINT32(mapData + 4);
    max_z_ = READ_LE_UINT32(mapData + 8);

    LOG(Log::k_FLG_GFX, "Map", "loadMap",
        ("Map size in tiles: max_x = %d, max_y = %d, max_z = %d.", max_x_, max_y_, max_z_));

    uint32 *lookup = new uint32[max_x_ * max_y_];
    // NOTE : increased map height by 1 to enable range check on higher tiles
    a_tiles_ = new Tile*[max_x_ * max_y_ * (max_z_ + 1)];

    for (int i = 0; i < max_x_ * max_y_; i++)
        lookup[i] = READ_LE_UINT32(mapData + 12 + i * 4);
    for (int h = 0, z_real = max_z_ + 1; h < max_y_; h++)
        for (int w = 0; w < max_x_; w++) {
            int idx = h * max_x_ + w;

            for (int z = 0; z < max_z_; z++) {
                uint8 tileNum = *(mapData + 12 + lookup[idx] + z);
                a_tiles_[idx * z_real + z] = tile_manager_->getTile(tileNum);

            }
        }
    delete[] lookup;

    max_z_++;
    for (int h = 0, z = max_z_ - 1; h < max_y_; h++) {
        for (int w = 0; w < max_x_; w++) {
            a_tiles_[(h * max_x_ + w) * max_z_ + z] = tile_manager_->getTile(0);
        }
    }

    map_width_ = (max_x_ + max_y_) * (TILE_WIDTH / 2);
    map_height_ = (max_x_ + max_y_ + max_z_) * TILE_HEIGHT / 3;
    LOG(Log::k_FLG_GFX, "Map", "loadMap",
        ("Map size in pixels: width = %d, height = %d.", map_width_, map_height_));

    LOG(Log::k_FLG_GFX, "Map", "loadMap", ("Loading finished"));

    return true;
}

void Map::mapDimensions(int *x, int *y, int *z)
{
    *x = maxX();
    *y = maxY();
    *z = maxZ();
}

/*!
 * Clip x,y,z to map dimensions.
 */
void Map::adjXYZ(int &x, int &y, int &z) {
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (z < 0 || z >= maxZ())
        z = 0;
    if (x >= maxX())
        x = maxX() - 1;
    if (y >= maxY())
        y = maxY() - 1;
}

void Map::clip(Point2D *pPoint) {
    if (pPoint->x < 0) {
        pPoint->x = 0;
    } else if (pPoint->x >= maxX()) {
        pPoint->x = maxX();
    }

    if (pPoint->y < 0) {
        pPoint->y = 0;
    } else if (pPoint->y >= maxY()) {
        pPoint->y = maxY();
    }
}

void Map::clip(TilePoint *pPoint) {
    if (pPoint->tx < 0) {
        pPoint->tx = 0;
    } else if (pPoint->tx >= maxX()) {
        pPoint->tx = maxX() - 1;
    }

    if (pPoint->ty < 0) {
        pPoint->ty = 0;
    } else if (pPoint->ty >= maxY()) {
        pPoint->ty = maxY() - 1;
    }

    if (pPoint->tz < 0 || pPoint->tz >= maxZ()) {
        pPoint->tz = 0;
    }
}

float scalexPx = 256.0f;
float scalexPy = 256.0f;
float scaleyPx = 256.0f;
float scaleyPy = 256.0f;


void Map::tileToScreenPoint(int x, int y, int z, int pX, int pY, Point2D *pScp)
{
    float fx = x + pX / scalexPx;
    float fy = y + pY / scalexPy;

    pScp->x = (int) ((max_x_ * TILE_WIDTH / 2) + (fx - fy) * TILE_WIDTH / 2
                  + TILE_WIDTH / 2);

    pScp->y = (int) ((max_z_ + 1) * TILE_HEIGHT / 3 + (fx + fy) * TILE_HEIGHT / 3);
}

void Map::tileToScreenPoint(const TilePoint &tPt, Point2D *pScp) {
    tileToScreenPoint(tPt.tx, tPt.ty, tPt.tz, tPt.ox, tPt.oy, pScp);
}

/*!
 * Returns the tile position corresponding to the screen position.
 * Usually after the user clicked on the map, this method is used
 * to find what tile has been clicked.
 * \param x The x position on the screen (in pixel).
 * \param y The y position on the screen (in pixel).
 */
TilePoint Map::screenToTilePoint(int x, int y)
{
    TilePoint mtp;

    x -= (max_x_ + 1) * (TILE_WIDTH / 2);
    // x now equals fx * TILE_WIDTH / 2 - fy * TILE_WIDTH / 2
    // which equals TILE_WIDTH/2 * (fx - fy)
    y -= (max_z_ + 1) * (TILE_HEIGHT / 3);
    // y now equals (fx + fy) * TILE_HEIGHT / 3
    float dx = (float) x / (TILE_WIDTH / 2);
    float dy = (float) y / (TILE_HEIGHT / 3);

    // dx equals fx - fy
    // dy equals fx + fy
    float f_tx = (dx + dy) / 2;
    mtp.tx = (int) f_tx;
    mtp.ox = (int) ((f_tx - mtp.tx) * 256.0f);

    float f_ty = (dy - dx) / 2;
    mtp.ty = (int) f_ty;
    mtp.oy = (int) ((f_ty - mtp.ty) * 256.0f);

    return mtp;
}

int Map::maxZAt(int x, int y)
{
    assert(x < max_x_);
    assert(y < max_y_);
    /*
    int idx = y * max_x_ + x;
    int mz = 0;
    for (int z = 0; z < max_z_; z++) {
        int tile = map_data_[idx * max_z_ + z];
        if (tile > 5) {
            mz = z;
        }
    }
    */
    // TODO: disabling this thing, causes a lot of speed drain
    // find a better for such optimization, not all objects are drawn
    // that is why I disabled it
    return max_z_ - 1;
}

Tile * Map::getTileAt(int x, int y, int z)
{
    if (x < 0 || x >= max_x_ || y < 0 || y >= max_y_) {
        return tile_manager_->getTile(z < 2 ? 6 : 0);
    }

    if (z < 0 || z >= max_z_) {
        return tile_manager_->getTile(0);
    }

    return a_tiles_[(y * max_x_ + x) * max_z_ + z];
}

int Map::tileAt(int x, int y, int z)
{
    if (x < 0 || x >= max_x_)
        return z < 2 ? 6 : 0;
    if (y < 0 || y >= max_y_)
        return z < 2 ? 6 : 0;
    if (z < 0 || z >= max_z_)
        return 0;

    return a_tiles_[(y * max_x_ + x) * max_z_ + z]->id();
}

void Map::patchMap(int x, int y, int z, uint8 tileNum)
{
    assert((x >= 0 && x < max_x_)
        && (y >= 0 && y < max_y_)
        && (z >= 0 && z < max_z_));
    a_tiles_[(y * max_x_ + x) * max_z_ + z] = tile_manager_->getTile(tileNum);
}


/**
 * Return true if tile at given position is traversable by a car.
 * \param x int X coordinate
 * \param y int Y coordinate
 * \param z int Z coordinate
 * \return bool
 *
 */
bool Map::isTileWalkableByCar(int x, int y, int z)
{
    Tile *pTile = getTileAt(x, y, z);
    uint8 tileId = pTile->id();

    if(tileId == 80) {
        Tile::EType near_type = getTileAt(x, y - 1, z)->type();
        if((near_type < Tile::kRoadSideEW || near_type > Tile::kRoadSideNS)
            && near_type != Tile::kRoadPedCross) {
            return false;
        }
        near_type = getTileAt(x, y + 1, z)->type();
         if((near_type < Tile::kRoadSideEW || near_type > Tile::kRoadSideNS)
             && near_type != Tile::kRoadPedCross)
         {
            return false;
         }
        return true;
    }
    if(tileId == 81) {
        Tile::EType near_type = getTileAt(x - 1, y, z)->type();
         if((near_type < Tile::kRoadSideEW || near_type > Tile::kRoadSideNS)
             && near_type != Tile::kRoadPedCross)
         {
            return false;
         }
        near_type = getTileAt(x + 1, y, z)->type();
         if((near_type < Tile::kRoadSideEW || near_type > Tile::kRoadSideNS)
             && near_type != Tile::kRoadPedCross)
         {
            return false;
         }
        return true;
    }
    if(tileId == 72) {
        return false;
    }

    if(tileId == 119) {
        return false;
    }
    return  pTile->isRoad();
}

const uint8 MiniMap::kOverlayNone = 0;
const uint8 MiniMap::kOverlayOurAgent = 1;
const uint8 MiniMap::kOverlayEnemyAgent = 2;

/*!
 * Construct the minimap from the given map.
 * \param p_map
 */
MiniMap::MiniMap(Map *p_map) {
    // walkdata based colours
    uint8 minimap_colours_[] = {
        8,  7,  7,  7,
        7,  7, 10, 10,
       10, 10,  0, 10,
       15, 15, 10, 10,
       0,   0,  0,  0,
    };

    mmax_x_ = p_map->maxX();
    mmax_y_ = p_map->maxY();

    a_minimap_ = (uint8 *)( malloc(mmax_x_ * mmax_y_) );
    if(a_minimap_ == NULL) {
        FSERR(Log::k_FLG_MEM, "MiniMap", "MiniMap", ("memory allocation failed"));
        return;
    }
    for (unsigned short y = 0; y < mmax_y_; y++) {
        unsigned short yadd = y * mmax_x_;
        for (unsigned short x = 0; x < mmax_x_; x++) {
            Tile::EType type = p_map->getTileAt(x, y, 0)->type();
            a_minimap_[x + yadd] = minimap_colours_[type];
        }
    }
}

MiniMap::~MiniMap() {
    if (a_minimap_) {
        free(a_minimap_);
        a_minimap_ = NULL;
    }
}

/*!
 * Return the color at the given point.
 * \param x
 * \param y
 */
uint8 MiniMap::getColourAt(int x, int y) {

    if (x < 0 || y < 0 || x >= mmax_x_ || y >= mmax_y_)
        return 0;

    if (a_minimap_ != 0)
        return a_minimap_[x + y * mmax_x_];
    return 0;
}

//! Defines a source on the minimap for the signal
void MiniMap::setTarget(MapObject *pTarget) {
    p_target_ = pTarget;
}
//! Clear the target source
void MiniMap::clearTarget() {
    p_target_ = NULL;
}

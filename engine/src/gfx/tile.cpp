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

#include "fs-engine/gfx/tile.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

namespace fs_eng {

const int Tile::kTileWidth = 64;
const int Tile::kTileHeight = 48;
const int Tile::kSubTileWidth = 32;
const int Tile::kSubTileHeight = 16;


/*!
 * @brief 
 * @param id 
 * @param notAlpha 
 * @param type 
 * @param textLoc 
 */
Tile::Tile(int id, bool notAlpha, EType type, Point2D textLoc)
{
    id_ = id;
    type_ = type;
    notAlpha_ = notAlpha;
    textureLocation_ = textLoc;
}

/*bool Tile::drawTo(uint8 * screen, int swidth, int sheight, int x, int y) const
{
    if (x + kTileWidth < 0 || y + kTileHeight < 0
        || x >= swidth || y >= sheight)
    {
        return false;
    }

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
    return true;
}*/


/*! \brief
 *
 * \return uint8
 *
 */
uint8 Tile::getWalkData() {
    // little patch to enable full surface description
    // and eliminate unnecessary data
    // 0x10 - non-surface/non-walkable, always above train stop
    // 0x11, 0x12 - train entering surface
    switch (id_) {
    case 0x80 :
        return 0x11;
    case 0x81 :
        return 0x12;
    case 0x8F :
        return 0x00;
    case 0x93 :
        return 0x00;
    default:
        // else return the type of the tile
        return type_;
    }
}

}


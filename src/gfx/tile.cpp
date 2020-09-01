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
#include <string.h>
#include <assert.h>

#include "tile.h"
#include "gfx/screen.h"


Tile::Tile(uint8 id_set, uint8 *tile_Data, bool not_alpha, EType type_set)
{
    i_id_ = id_set;
    e_type_ = type_set;
    a_pixels_ = new uint8[TILE_WIDTH * TILE_HEIGHT];
    memcpy(a_pixels_, tile_Data, TILE_WIDTH * TILE_HEIGHT);
    not_alpha_ = not_alpha;
}

Tile::~Tile()
{
    delete[] a_pixels_;
}

bool Tile::drawTo(uint8 * screen, int swidth, int sheight, int x, int y)
{
    if (x + TILE_WIDTH < 0 || y + TILE_HEIGHT < 0
        || x >= swidth || y >= sheight)
    {
        return false;
    }

    int xlow = x < 0 ? 0 : x;
    int clipped_w = TILE_WIDTH - (xlow - x);
    int xhigh = xlow + clipped_w >= swidth ? swidth : xlow + clipped_w;
    int ylow = y < 0 ? 0 : y;
    int clipped_h = TILE_HEIGHT - (ylow - y);
    int yhigh = ylow + clipped_h >= sheight ? sheight : ylow + clipped_h;

    uint8 *ptr_a_pixels = a_pixels_ + ((TILE_HEIGHT - 1) - (ylow - y)) * TILE_WIDTH;
    uint8 *ptr_screen = screen + ylow * swidth + xlow;
    for (int j = ylow; j < yhigh; ++j)
    {
        uint8 *cp_ptr_a_pixels = ptr_a_pixels;
        ptr_a_pixels -= TILE_WIDTH;
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
}

bool Tile::drawToScreen(int x, int y)
{
    return drawTo((uint8*) g_Screen.pixels(), g_Screen.gameScreenWidth(), g_Screen.gameScreenHeight(), x, y);
}

uint8 Tile::getWalkData() {
    // little patch to enable full surface description
    // and eliminate unnecessary data
    // 0x10 - non-surface/non-walkable, always above train stop
    // 0x11, 0x12 - train entering surface
    switch (i_id_) {
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
        return e_type_;
    }
}

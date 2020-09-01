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

#ifndef TILE_H
#define TILE_H

#include "common.h"

// TODO: Convert these to const int's -- we are using C++, yes? :-)
#define TILE_WIDTH              64
#define TILE_HEIGHT             48

#define SUBTILE_WIDTH           32
#define SUBTILE_HEIGHT          16

#define SUBTILES_PERtile__X     (TILE_WIDTH / SUBTILE_WIDTH)
#define SUBTILES_PERtile__Y     (TILE_HEIGHT / SUBTILE_HEIGHT)
#define SUBTILES_PERtile_       (SUBTILES_PERtile__X * SUBTILES_PERtile__Y)

#define BLOCKS_PER_SUBTILE_ROW  (SUBTILE_WIDTH / PIXELS_PER_BLOCK)
#define SUBTILE_ROW_LENGTH      (BLOCK_LENGTH * BLOCKS_PER_SUBTILE_ROW)

#define TILE_INDEX_SIZE         (SUBTILES_PERtile_ * 4)
#define TILE_HEADER_LENGTH      (256 * TILE_INDEX_SIZE)

/*!
 * Tile class.
 */
class Tile {
public:

    /*!
     * A tile type.
     */
    enum EType {
        kNone = 0x00,       // non-surface/non-walkable(if above surface is walkable)
        kSlopeSN = 0x01,    // stairs that climbs along the Y axis from left to right
        kSlopeNS = 0x02,    // stairs that climbs along the Y axis from right to left
        kSlopeEW = 0x03,    // stairs that climbs along the X axis from left to right
        kSlopeWE = 0x04,    // stairs that climbs along the X axis from right to left
        kGround = 0x05,
        kRoadSideEW = 0x06,
        kRoadSideWE = 0x07,
        kRoadSideSN = 0x08,
        kRoadSideNS = 0x09,
        kWall = 0x0A,
        kRoadCurve = 0x0B,
        kHandrailLight = 0x0C,
        kRoof = 0x0D,
        kRoadPedCross = 0x0E,
        kRoadMark = 0x0F,
        kUnknown = 0x10,   // non-surface/non-walkable, always above train stop
        kNbTypes  = 0x11,
    };

    Tile(uint8 id_set, uint8 *tile_Data, bool not_alpha, EType type_set);
    ~Tile();

    //! Returns the tile id
    uint8 id() { return i_id_; }
    //! Returns the tile type
    EType type() { return e_type_; }
    //! Convenience method to tell whether this tile is a road type or not
    bool isRoad() {
        return  (e_type_ == kRoadCurve || e_type_ == kRoadPedCross ||
                (e_type_ > kGround && e_type_ < kWall));
    }
    //! Convenience method to tell whether this tile is a stair type or not
    bool isStairs() {
        return e_type_ > kNone && e_type_ < kGround;
    }

    uint8 getWalkData();

    //! Draws the tile to the given surface
    bool drawTo(uint8 *screen, int swidth, int sheight, int x, int y);
    //! Draws the tile to the screen
    bool drawToScreen(int x, int y);

    inline bool notTransparent() { return not_alpha_; }

protected:
    /*! Each tile has a unique id.*/
    uint8 i_id_;
    /*! The pixels that compose the tile.*/
    uint8 *a_pixels_;
    /*! A quick flag to tell that all pixel are transparent.*/
    bool not_alpha_;
    /*! The tile type. */
    EType e_type_;
};

#endif

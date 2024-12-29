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

#ifndef ENGINE_GFX_TILE_H
#define ENGINE_GFX_TILE_H

#include "fs-utils/common.h"
#include "fs-engine/enginecommon.h"

namespace fs_eng {

/*!
 * Tile class.
 */
class Tile {
public:
    //! Width of a tile
    static const int kTileWidth;
    //! Height of a tile
    static const int kTileHeight;
    //! Width of a component of a tile
    static const int kSubTileWidth;
    //! Height of a component of a tile
    static const int kSubTileHeight;

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

    //! Constructor
    Tile(int id, bool notAlpha, EType type, Point2D textLoc);
    ~Tile() {}

    //! Returns the tile id
    int id() const { return id_; }
    //! Returns the tile type
    EType type() const { return type_; }
    //! Returns the position in the tileset texture
    Point2D textureLocation() const { return textureLocation_; }

    //! Convenience method to tell whether this tile is a road type or not
    bool isRoad() {
        return  (type_ == kRoadCurve || type_ == kRoadPedCross ||
                (type_ > kGround && type_ < kWall));
    }
    //! Convenience method to tell whether this tile is a stair type or not
    bool isStairs() {
        return type_ > kNone && type_ < kGround;
    }

    uint8 getWalkData();

    //! Return true if there is at least one pixel to draw because it's not transparent
    inline bool notTransparent() { return notAlpha_; }

protected:
    /*! Each tile has a unique id.*/
    int id_;
    /*! A quick flag to tell that all pixel are transparent.*/
    bool notAlpha_;
    /*! The tile type. */
    EType type_;
    //! Position of tile in the tileset texture
    Point2D textureLocation_;
};

}

#endif

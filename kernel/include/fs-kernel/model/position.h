/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2016  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef MODEL_POSITION_H_
#define MODEL_POSITION_H_

#include <iostream>
#include <sstream>

/*!
 * This a convenient structure to store a position
 * in map tile coordinates.
 */
class TilePoint {
public:
    /*! The tile's X coord.*/
    int tx;
    /*! The tile's Y coord.*/
    int ty;
    /*! The tile's Z coord.*/
    int tz;
    /*! X Offset inside the tile. (varies between 0 and 256)*/
    int ox;
    /*! Y Offset inside the tile. (varies between 0 and 256)*/
    int oy;
    /*! Z Offset inside the tile. (varies between 0 and 128)*/
    int oz;

    TilePoint() {
        reset();
    }

    TilePoint(int tile_x, int tile_y, int tile_z, int off_x = 128,
            int off_y = 128, int off_z = 0) {
        tx = tile_x;
        ty = tile_y;
        tz = tile_z;
        ox = off_x;
        oy = off_y;
        oz = off_z;
    }

    TilePoint(const TilePoint &tp) {
        tx = tp.tx;
        ty = tp.ty;
        tz = tp.tz;
        ox = tp.ox;
        oy = tp.oy;
        oz = tp.oz;
    }

    void reset() {
        tx = 0;
        ty = 0;
        tz = 0;
        ox = 0;
        oy = 0;
        oz = 0;
    }

    /*!
     *
     */
    void initFrom(const TilePoint &tp) {
        tx = tp.tx;
        ty = tp.ty;
        tz = tp.tz;
        ox = tp.ox;
        oy = tp.oy;
        oz = tp.oz;
    }

    /*!
     * Return true if this point matches the other
     */
    bool equals(const TilePoint &otherTp) {
        return otherTp.tx == tx
                && otherTp.ty == ty
                && otherTp.tz == tz
                && otherTp.oy == oy
                && otherTp.oz == oz;
    }

    bool operator<(const TilePoint &other) const {
        int a = tx | (ty << 16);
        int b = other.tx | (other.ty << 16);
        return a < b;
    }

    void toString(std::string *buffer) const {
        std::ostringstream out;

        out << "(" << tx << ", " << ty << ", " << tz << " - " << ox << ", " << oy << ", " << oz << ")";
        buffer->append(out.str());
    }
};

/*!
 * This structure stores a position
 * in absolute coordinates.
 */
class WorldPoint {
public:
    int x;
    int y;
    int z;

    WorldPoint() {
        reset();
    }

    WorldPoint(const TilePoint &tp) {
        convertFromTilePoint(tp);
    }

    void reset() {
        x = 0;
        y = 0;
        z = 0;
    }

    void convertFromTilePoint(const TilePoint &tp) {
        x = tp.tx * 256 + tp.ox;
        y = tp.ty * 256 + tp.oy;
        z = tp.tz * 128 + tp.oz;
    }

    void convertToTilePoint(TilePoint *pTp) {
        pTp->tx = x / 256;
        pTp->ty = y / 256;
        pTp->tz = z / 128;
        pTp->ox = x % 256;
        pTp->oy = y % 256;
        pTp->oz = z % 128;
    }

    /*!
     * Return true if this point matches the other
     */
    bool equals(const WorldPoint &otherWp) {
        return otherWp.x == x
                && otherWp.y == y
                && otherWp.z == z;
    }
};

#endif // MODEL_POSITION_H_

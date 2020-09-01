/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
 *   Copyright (C) 2012  Ryan Cocks <ryan@ryancocks.net>                *
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

#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <cstring>
#include <cassert>

#include "utils/singleton.h"
#include "version.h"

// TODO: Add a configuration scheme to allow setting these during runtime
#define MENU_SCREEN_WIDTH       320
#define MENU_SCREEN_HEIGHT      200
#define GAME_SCREEN_WIDTH       640
#define GAME_SCREEN_HEIGHT      400

#define STUB_FUNC               printf("STUB: %s\n", __PRETTY_FUNCTION__)

// TODO: Convert these to const int's -- we are using C++, yes? :-)
#define PIXELS_PER_BLOCK        8
#define COLOR_BYTES_PER_BLOCK   (PIXELS_PER_BLOCK / 2)
#define ALPHA_BYTES_PER_BLOCK   (PIXELS_PER_BLOCK / 8)
#define BLOCK_LENGTH            (COLOR_BYTES_PER_BLOCK + ALPHA_BYTES_PER_BLOCK)

typedef signed char int8;
typedef unsigned char uint8;

typedef signed short int16;
typedef unsigned short uint16;

typedef signed int int32;
typedef unsigned int uint32;

typedef signed long long int64;
typedef unsigned long long uint64;

inline int ceil8(int v) {
    if (v <= 0)
        return 0;

    return (v % 8) ? ((v / 8) + 1) * 8: v;
}

inline uint16 READ_LE_UINT16(const uint8 *data) {
    return (data[1] << 8) | data[0];
}

inline uint32 READ_LE_UINT32(const uint8 *data) {
    return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
}

inline uint16 READ_BE_UINT16(const uint8 *data) {
    return (data[0] << 8) | data[1];
}

inline uint32 READ_BE_UINT32(const uint8 *data) {
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

inline int16 READ_LE_INT16(const uint8 *data) {
    return (data[1] << 8) | data[0];
}

inline int32 READ_LE_INT32(const uint8 *data) {
    return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
}

inline int16 READ_BE_INT16(const uint8 *data) {
    return (data[0] << 8) | data[1];
}

inline int32 READ_BE_INT32(const uint8 *data) {
    return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

inline void WRITE_LE_UINT16(uint8 *data, uint16 num) {
    data[0] = (uint8)(num & 0xFF);
    data[1] = (uint8)(num >> 8);
}

inline uint32 mirror(uint32 value, int count) {
    uint32 top = 1 << (count - 1), bottom = 1;

    while (top > bottom) {
        uint32 mask = top | bottom;
        uint32 masked = value & mask;

        if (masked != 0 && masked != mask)
            value ^= mask;

        top >>= 1;
        bottom <<= 1;
    }

    return value;
}

inline uint32 bitValue(const uint32 value, int index) {
    return (value >> index) & 1;
}

inline bool bitSet(const uint32 value, int index) {
    return bitValue(value, index) == 0 ? false : true;
}

inline bool bitClear(const uint32 value, int index) {
    return !bitSet(value, index);
}

const double PI = 3.141592653589793;

namespace fs_cmn {

    // Those values are the index of the color in the current pallette.
    /*! Color constant : Black */
    static const uint8 kColorBlack = 0;
    /*! Color constant : Light Green */
    static const uint8 kColorLightGreen = 3;
    /*! Color constant : Light grey */
    static const uint8 kColorLightGrey = 4;
    /*! Color constant : Dark Brown */
    static const uint8 kColorDarkBrown = 5;
    /*! Color constant : Dark Red */
    static const uint8 kColorDarkRed = 6;
    /*! Color constant : Blue Grey */
    static const uint8 kColorBlueGrey = 7;
    /*! Color constant : Yellow */
    static const uint8 kColorYellow = 11;
    /*! Color constant : White */
    static const uint8 kColorWhite = 12;
    /*! Color constant : Light Brown */
    static const uint8 kColorLightBrown = 13;
    /*! Color constant : Light Red */
    static const uint8 kColorLightRed = 14;
    /*! Color constant : Blue */
    static const uint8 kColorBlue = 15;
    /*! Color constant : Dark Green */
    static const uint8 kColorDarkGreen = 16;

    /*!
     * Turn bits given by mask on in the given bitfield.
     * \param bitfield the bitfield to update
     * \param mask The bitmask to apply
     */
    inline void setBitsWithMask(uint32 *bitfield, const uint32 mask) {
        *bitfield |= mask;
    }

    /*!
     * Return true is bits identified by mask are set in the bitfield
     * \param bitfield the bitfield to query
     * \param mask The bitmask to apply
     */
    inline bool isBitsOnWithMask(const uint32 bitfield, const uint32 mask) {
        return (bitfield & mask) != 0;
    }

    /*!
     * Return true is bits identified by mask are set in the bitfield
     * \param bitfield 8 bits bitfield to query
     * \param mask 8 bits bitmask to apply
     */
    inline bool isBitsOnWithMask(const uint8 bitfield, const uint8 mask) {
        return (bitfield & mask) != 0;
    }
};

inline void boxify(int &left, int &width, int x1, int x2)
{
    width = x1 < x2 ? x2 - x1 : x1 - x2;
    left = (x1 < x2) ? x1 : x2;
}

/*!
 * A structure to store a point defined with 2 coordinates.
 */
struct Point2D {
    int x;
    int y;
};

#endif

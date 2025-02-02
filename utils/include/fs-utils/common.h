/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>
 *   Copyright (C) 2012  Ryan Cocks <ryan@ryancocks.net>
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

#ifndef COMMON_H
#define COMMON_H

#include <cstdint>
#include <string>
#include <cstring>
#include <cassert>


typedef signed char int8;
typedef unsigned char uint8;

typedef signed short int16;
typedef unsigned short uint16;

namespace fs_utl {

    //! This type is used for characters in Code Page 437
    typedef unsigned char cp437char_t;

    inline constexpr double kPI { 3.141592653589793 };

    inline uint16_t READ_LE_UINT16(const uint8_t *data) {
        return (data[1] << 8) | data[0];
    }

    inline uint32_t READ_LE_UINT32(const uint8_t *data) {
        return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
    }

    inline uint16_t READ_BE_UINT16(const uint8_t *data) {
        return (data[0] << 8) | data[1];
    }

    inline uint32_t READ_BE_UINT32(const uint8_t *data) {
        return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    }

    inline int16_t READ_LE_INT16(const uint8_t *data) {
        return (data[1] << 8) | data[0];
    }

    inline int32_t READ_LE_INT32(const uint8_t *data) {
        return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
    }

    inline int16_t READ_BE_INT16(const uint8_t *data) {
        return (data[0] << 8) | data[1];
    }

    inline int32_t READ_BE_INT32(const uint8_t *data) {
        return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    }

    inline void WRITE_LE_UINT16(uint8_t *data, uint16_t num) {
        data[0] = (uint8_t)(num & 0xFF);
        data[1] = (uint8_t)(num >> 8);
    }
    /*!
     * Return an integer where the bit at index is at the lowest bit
     * @param value 
     * @param index 
     * @return 
     */
    inline uint32_t bitValue(const uint32_t value, int index) {
        return (value >> index) & 1;
    }
    /*!
     * Return true if the bit at given index is set.
     * @param value 
     * @param index 
     * @return 
     */
    inline bool bitSet(const uint32_t value, int index) {
        return bitValue(value, index) == 0 ? false : true;
    }

    /*!
    * Unset the bit at given position.
    * @param value 
    * @param index 
    * @return 
    */
    inline bool bitClear(const uint32_t value, int index) {
        return !bitSet(value, index);
    }
    /*!
     * Turn bits given by mask on in the given bitfield.
     * \param bitfield the bitfield to update
     * \param mask The bitmask to apply
     */
    inline void setBitsWithMask(uint32_t *bitfield, const uint32_t mask) {
        *bitfield |= mask;
    }

    /*!
     * Return true is bits identified by mask are set in the bitfield
     * \param bitfield the bitfield to query
     * \param mask The bitmask to apply
     */
    inline bool isBitsOnWithMask(const uint32_t bitfield, const uint32_t mask) {
        return (bitfield & mask) != 0;
    }

    /*!
     * Return true is bits identified by mask are set in the bitfield
     * \param bitfield 8 bits bitfield to query
     * \param mask 8 bits bitmask to apply
     */
    inline bool isBitsOnWithMask(const uint8_t bitfield, const uint8_t mask) {
        return (bitfield & mask) != 0;
    }

    /*!
     * 
     * @param left 
     * @param width 
     * @param x1 
     * @param x2 
     */
    inline void boxify(int &left, int &width, int x1, int x2) {
        width = x1 < x2 ? x2 - x1 : x1 - x2;
        left = (x1 < x2) ? x1 : x2;
    }
};

#endif

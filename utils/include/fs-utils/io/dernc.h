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

#ifndef DERNC_H
#define DERNC_H

#include "fs-utils/common.h"

namespace rnc {

    //! This enumeration contains the different return codes for RNC functions
    enum RncRetCode {
        kOk = 0,
        kFileIsNotRNC = -1,
        kHufDecodeError = -2,
        kFileSizeMismatch = -3,
        kPackedCrcError = -4,
        kUnpackedCrcError = -5
    };

    //! Return true if the file is compressed with RNC format
    bool isRncCompressed(const uint8_t *data);
    //! Return a string description of the return code
    const char * errorString(RncRetCode returnCode);
    //! Read the length from the RNC header
    RncRetCode unpackedLength(const uint8_t *packed_data, size_t &length);
    //! Error control
    uint16 crc(uint8_t *packed_data, size_t packed_length);
    //! Decompress the data
    RncRetCode unpack(uint8_t *packed_data, uint8_t *unpacked_data, size_t &length);

}

#endif

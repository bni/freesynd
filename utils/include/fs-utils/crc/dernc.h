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

#ifndef DERNC_H
#define DERNC_H

#include "fs-utils/common.h"

#define RNC_SIGNATURE   0x524E4301      // "RNC\001"

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

    const char *const errorString(RncRetCode returnCode);
    RncRetCode unpackedLength(const uint8_t *packed_data, size_t &length);
    uint16 crc(uint8 *packed_data, int packed_length);
    RncRetCode unpack(uint8_t *packed_data, uint8_t *unpacked_data, size_t &length);

}

#endif

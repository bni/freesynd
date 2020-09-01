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

#include "common.h"

#define RNC_SIGNATURE   0x524E4301      // "RNC\001"

namespace rnc {

    enum {
        FILE_IS_NOT_RNC = -1,
        HUF_DECODE_ERROR = -2,
        FILE_SIZE_MISMATCH = -3,
        PACKED_CRC_ERROR = -4,
        UNPACKED_CRC_ERROR = -5
    };

    const char *const errorString(int error_code);
    int unpackedLength(uint8 *packed_data);
    uint16 crc(uint8 *packed_data, int packed_length);
    int unpack(uint8 *packed_data, uint8 *unpacked_data);

}

#endif

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

#ifndef KEYS_H
#define KEYS_H

#include "fs-utils/common.h"

/*!
 * All usable key codes for the game. A key code is dependent on keyboard layout.
 * We have defined only the keys that used in the game (plus some more in case).
 */
enum FS_KeyCode {
    kKeyCode_Unknown = 0,
    kKeyCode_Text = 99,
    KFC_ESCAPE = 1,
    KFC_BACKSPACE = 2,
    kKeyCode_Space = 3,
    KFC_RETURN = 4,

    // Arrows + Home/End pad
    KFC_UP = 11,
    KFC_DOWN = 12,
    KFC_RIGHT = 13,
    KFC_LEFT = 14,
    KFC_INSERT = 15,
    KFC_HOME = 16,
    KFC_END = 17,
    KFC_PAGEUP = 18,
    KFC_PAGEDOWN = 19,
    KFC_DELETE = 20,

    // Function keys
    KFC_F1 = 21,
    KFC_F2 = 22,
    KFC_F3 = 23,
    KFC_F4 = 24,
    KFC_F5 = 25,
    KFC_F6 = 26,
    KFC_F7 = 27,
    KFC_F8 = 28,
    KFC_F9 = 29,
    KFC_F10 = 30,
    KFC_F11 = 31,
    KFC_F12 = 32,

    kKeyCode_D = 33,
    kKeyCode_P = 34,

    kKeyCode_0 = 35,
    kKeyCode_1 = 36,
    kKeyCode_2 = 37,
    kKeyCode_3 = 38,
    kKeyCode_4 = 39
};

//! Valid key modifiers
enum FS_KeyMod {
    KMD_NONE = 0x0000,
    KMD_LSHIFT = 0x0001,
    KMD_RSHIFT = 0x0002,
    KMD_LCTRL = 0x0040,
    KMD_RCTRL = 0x0080,
    KMD_LALT = 0x0100,
    KMD_RALT = 0x0200
};

const int KMD_CTRL = KMD_LCTRL | KMD_RCTRL;
const int KMD_SHIFT = KMD_LSHIFT | KMD_RSHIFT;
const int KMD_ALT = KMD_LALT | KMD_RALT;


/*!
 * Represents a key that has been pressed.
 */
struct FS_Key {
    FS_KeyCode keyCode;
    uint16 unicode;            /**< Unicode for printable characters. */
    char text[32];
};

#endif

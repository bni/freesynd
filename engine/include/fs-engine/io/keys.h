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

#include "utf8.h"

#include "fs-utils/common.h"

/*!
 * All usable key codes for the game. A key code is dependent on keyboard layout.
 * We have defined only the keys that used in the game (plus some more in case).
 * But when entering text in a textfield, we use the kKeyCode_Text to have the
 * unicode codepoint of the key. Only for printable keys.
 */
enum FS_KeyCode {
    kKeyCode_Unknown = 0,
    kKeyCode_Text = 99,
    kKeyCode_Escape = 1,
    kKeyCode_Backspace = 2,
    kKeyCode_Space = 3,
    kKeyCode_Return = 4,

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

    kKeyCode_A = 33,
    kKeyCode_B = 34,
    kKeyCode_C = 35,
    kKeyCode_D = 36,
    kKeyCode_E = 37,
    kKeyCode_F = 38,
    kKeyCode_G = 39,
    kKeyCode_H = 40,
    kKeyCode_I = 41,
    kKeyCode_J = 42,
    kKeyCode_K = 43,
    kKeyCode_L = 44,
    kKeyCode_M = 45,
    kKeyCode_N = 46,
    kKeyCode_O = 47,
    kKeyCode_P = 48,
    kKeyCode_Q = 49,
    kKeyCode_R = 50,
    kKeyCode_S = 51,
    kKeyCode_T = 52,
    kKeyCode_U = 53,
    kKeyCode_V = 54,
    kKeyCode_W = 55,
    kKeyCode_X = 56,
    kKeyCode_Y = 57,
    kKeyCode_Z = 58,

    kKeyCode_0 = 59,
    kKeyCode_1 = 60,
    kKeyCode_2 = 61,
    kKeyCode_3 = 62,
    kKeyCode_4 = 63
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
    /*!
     * The keyCode represents the virtual key pressed.
     * Except when the user in entering text in textfield.
     * In that case, this field is set with kKeyCode_Text.
     */
    FS_KeyCode keyCode;
    /*! When keyCode is set to kKeyCode_Text, then
     * codePoint is set with the unicode codepoint of the character
     * corresponding to the key.
     */
    utf8::utfchar32_t codePoint;
};

#endif

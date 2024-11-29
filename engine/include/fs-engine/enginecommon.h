/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
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

#ifndef ENGINE_ENGINECOMMON_H
#define ENGINE_ENGINECOMMON_H

#include <array>

/*!
* @brief Namespace for the engine library
 */
namespace fs_eng
{
    //! The maximum number of colors in a palette
    inline constexpr int kPaletteMaxColor { 256 };


    /*!
    * A structure to store color.
    */
    struct FSColor {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        // Operator < for storing in map
        bool operator<(const FSColor& other) const {
            if (r != other.r) {
                return r < other.r;
            } else if (g != other.g) {
                return g < other.g;
            } else {
                return b < other.b;
            }
        }
    };
 
    //! This type represents a Palette of kPaletteMaxColor
    typedef std::array<FSColor, kPaletteMaxColor> Palette;

    //! This type represents index in the menu palette for major colors
    enum PaletteMenuColorIndex {
        kMenuPaletteColorLogo1 = 6,
        kMenuPaletteColorLogo2 = 7,
        kMenuPaletteColorLogo3 = 14, 
        kMenuPaletteColorLogo4 = 3,
        kMenuPaletteColorLogo5 = 11,
        kMenuPaletteColorLogo6 = 12,
        kMenuPaletteColorLogo7 = 13,
        kMenuPaletteColorLogo8 = 15,
        kMenuPaletteColorDarkGreen = 16,
        kMenuPaletteColorLightGreen = 252
    };
}

#endif // ENGINE_ENGINECOMMON_H
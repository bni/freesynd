/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
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

#ifndef ENGINE_ENGINECOMMON_H
#define ENGINE_ENGINECOMMON_H

#include <array>

/*!
* @brief Namespace for the engine library
 */
namespace fs_eng
{
    //! List of supported languages in the application
    enum FS_Lang {
        ENGLISH,
        FRENCH,
        ITALIAN,
        GERMAN
    };
    
    //! The maximum number of colors in a palette
    inline constexpr int kPaletteMaxColor { 256 };

    /*! Width of the screen in pixels.*/
    inline constexpr int kScreenWidth {640};
    /*! Height of the screen in pixels.*/
    inline constexpr int kScreenHeight {400};

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
        kMenuPaletteColorBrown = 5,
        kMenuPaletteColorRed = 6,
        kMenuPaletteColorBlueGrey = 7,
        kMenuPaletteColorGrey = 10,
        kMenuPaletteColorOrange = 14, 
        kMenuPaletteColorLightOrange = 3,
        kMenuPaletteColorYellow = 11,
        kMenuPaletteColorWhiteBlue = 12,
        kMenuPaletteColorLightBrown = 13,
        kMenuPaletteColorDarkBlue = 15,
        kMenuPaletteColorDarkGreen = 16,
        kMenuPaletteColorLightGreen = 252,
        kMenuPaletteColorWhite = 204
    };

    enum PaletteGameColorIndex {
         /*! Color constant : Black */
        kPaletteGameColorBlack = 0,
        /*! Color constant : Light Green */
        kPaletteGameColorLightGreen = 3,
        /*! Color constant : Light grey */
        kPaletteGameColorLightGrey = 4,
        /*! Color constant : Dark Brown */
        kPaletteGameColorDarkBrown = 5,
        /*! Color constant : Dark Red */
        kPaletteGameColorDarkRed = 6,
        /*! Color constant : Blue Grey */
        kPaletteGameColorBlueGrey = 7,
        /*! Color constant : Yellow */
        kPaletteGameColorYellow = 11,
        /*! Color constant : White */
        kPaletteGameColorWhite = 12,
        /*! Color constant : Light Brown */
        kPaletteGameColorLightBrown = 13,
        /*! Color constant : Light Red */
        kPaletteGameColorLightRed = 14,
        /*! Color constant : Blue */
        kPaletteGameColorBlue = 15
    };
}

/*!
 * A structure to store a point defined with 2 coordinates.
 */
struct Point2D {
    int x;
    int y;

    Point2D add(int offsetX, int offsetY) const {
        Point2D res;
        res.x = x + offsetX;
        res.y = y + offsetY;

        return res;
    }
};

#endif // ENGINE_ENGINECOMMON_H
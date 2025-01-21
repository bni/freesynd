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

#ifndef ENGINE_LOGOMANAGER_H
#define ENGINE_LOGOMANAGER_H

#include <memory>

#include "fs-utils/common.h"
#include "fs-engine/enginecommon.h"
#include "fs-utils/misc/singleton.h"
#include "fs-engine/gfx/fstexture.h"

namespace fs_eng {

/*!
 * This class manages the logo sprites and proposes a method to display a logo.
 * Logos are uploaded from original game files.
 */
class LogoManager : public Singleton<LogoManager>{
public:
    /*! Max number of colors for logos.*/
    static const int kMaxColour;

    explicit LogoManager();
    ~LogoManager();

    //! Initialize logoManager by loading the logos
    bool loadLogos(const fs_eng::Palette &palette);

    //! Return the color value at the given index
    uint8_t getColorAtIndex(int colourIdx);
    //! Returns the number of available logos
    int numLogos();
    //! Draws a logo at given position with the given color in big format or not
    void draw(Point2D pos, int logoId, int colourIdx, bool big);

private:
    //! Copies logos data found in original files into a buffer before copying it to a surface
    void copyLogosPixelsToBuffer(const uint8_t *bigLogosData, uint8_t *logosBuffer);

private:
    /*! Width for a normal logo*/
    static const int kLogoBigWidth;
    /*! Width for a small logo*/
    static const int kLogoSmallWidth;
    /*! How many logos on a line in a texture.*/
    static const int kNumOfLogosPerRow;
    
    //! Number of logos in the game
    int numberLogo_;
    //! This contains the pixels data for all logos and are loaded from a file
    uint8 *data_all_logos_;

    //! A texture that stores the logos big and small
    std::unique_ptr<FSTexture> logosTexture_;
    //! An array of positions for each logo in the texture
    Point2D **logoPositions_;
};

}

#define g_LogoMgr    fs_eng::LogoManager::singleton()

#endif //ENGINE_LOGOMANAGER_H

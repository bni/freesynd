/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#ifndef ENGINE_LOGOMANAGER_H
#define ENGINE_LOGOMANAGER_H

#include "fs-utils/common.h"
#include "fs-utils/misc/singleton.h"

/*!
 * This class manages the logo sprites and proposes a method to display a logo.
 * Logos are uploaded from original game files.
 */
class LogoManager : public Singleton<LogoManager>{
public:
    /*! Max number of colors for logos.*/
    static const int kMaxColour;
    /*! Width for a normal logo*/
    static const int kLogoBigWidth;
    /*! Width for a small logo*/
    static const int kLogoSmallWidth;

    explicit LogoManager();
    ~LogoManager();

    //! Return the color value at the given index
    uint8_t getColorAtIndex(int colourIdx);
    //! Returns the number of available logos
    int numLogos();
    //! Draws a logo at given position with the given color
    void drawLogo(int x, int y, int logo, int colourIdx, bool mini = false);

private:
    //! Number of logos in the game
    int numberLogo_;
    //! This contains the pixels data for all logos and are loaded from a file
    uint8 *data_all_logos_;
    //! This contains the data for one logo that will be drawn on screen
    uint8_t *data_logo_;
    //! This contains the pixels data for all mini logos and are loaded from a file
    uint8 *data_all_mini_logos_;
    //! This contains the data for one mini logo that will be drawn on screen
    uint8_t *data_mini_logo_;
};

#define g_LogoMgr    LogoManager::singleton()

#endif //ENGINE_LOGOMANAGER_H

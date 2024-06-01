/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#include "fs-engine/gfx/logomanager.h"

#include "fs-utils/io/file.h"
#include "fs-engine/gfx/screen.h"

uint8_t g_Colours[8] = { 6, 7, 14, 3, 11, 12, 13, 15 };

const int LogoManager::kMaxColour = 8;
const int LogoManager::kLogoBigWidth = 32;
const int LogoManager::kLogoSmallWidth = 16;

LogoManager::LogoManager()
: data_all_logos_(NULL), data_logo_(NULL)
, data_all_mini_logos_(NULL), data_mini_logo_(NULL)
{
}

LogoManager::~LogoManager()
{
    if (data_all_logos_)
        delete[] data_all_logos_;
    if (data_logo_)
        delete[] data_logo_;
    if (data_all_mini_logos_)
        delete[] data_all_mini_logos_;
    if (data_mini_logo_)
        delete[] data_mini_logo_;
}

/*!
 *
 * \param colourIdx int must be positive and less than kMaxColour
 * \return uint8_t The color value
 *
 */
uint8_t LogoManager::getColorAtIndex(int colourIdx) {
    if (colourIdx < 0 || colourIdx >= kMaxColour) {
        colourIdx = 0;
    }
    return g_Colours[colourIdx];
}

int LogoManager::numLogos()
{
    return numberLogo_;
}

void LogoManager::drawLogo(int x, int y, int logo, int colour, bool mini)
{
    size_t fileSize;
    if (data_all_logos_ == NULL) {
        data_all_logos_ = File::loadOriginalFile("mlogos.dat", fileSize);
        numberLogo_ = fileSize / (kLogoBigWidth * kLogoBigWidth);
        data_logo_ = new uint8[kLogoBigWidth * kLogoBigWidth];
    }
    if (data_all_mini_logos_ == NULL) {
        data_all_mini_logos_ = File::loadOriginalFile("mminlogo.dat", fileSize);
        data_mini_logo_ = new uint8[kLogoSmallWidth * kLogoSmallWidth];
    }

    // Fill the data_logo_ array with the current logo and given color
    int offsetBigLogo = logo * kLogoBigWidth * kLogoBigWidth;
    for (int i = 0; i < kLogoBigWidth * kLogoBigWidth; i++) {
        if (data_all_logos_[offsetBigLogo + i] == 0xFE)
            data_logo_[i] = g_Colours[colour];
        else
            data_logo_[i] = data_all_logos_[offsetBigLogo + i];
    }

    // Fill the data_small_logo_ array with the current logo and given color
    int offsetSmallLogo = logo * kLogoSmallWidth * kLogoSmallWidth;
    for (int i = 0; i < kLogoSmallWidth * kLogoSmallWidth; i++) {
        if (data_all_mini_logos_[offsetSmallLogo + i] == 0xFE)
            data_mini_logo_[i] = g_Colours[colour];
        else
            data_mini_logo_[i] = data_all_mini_logos_[offsetSmallLogo + i];
    }

    if (mini)
        g_Screen.scale2x(x, y, kLogoSmallWidth, kLogoSmallWidth, data_mini_logo_, kLogoSmallWidth);
    else
        g_Screen.scale2x(x, y, kLogoBigWidth, kLogoBigWidth, data_logo_, kLogoBigWidth);
}

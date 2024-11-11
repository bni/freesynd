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

#include "fs-utils/log/log.h"
#include "fs-utils/io/file.h"
#include "fs-engine/gfx/screen.h"
#include "fs-engine/system/system.h"

uint8_t g_Colours[8] = { 6, 7, 14, 3, 11, 12, 13, 15 };

const int LogoManager::kMaxColour = 8;
const int LogoManager::kLogoBigWidth = 32;
const int LogoManager::kLogoSmallWidth = 16;
const int LogoManager::kNumOfLogosPerRow = 32;

LogoManager::LogoManager()
: data_all_logos_(nullptr),
  logoPositions_(nullptr)
{
}

LogoManager::~LogoManager()
{
    if (data_all_logos_)
        delete[] data_all_logos_;

    if (logoPositions_)
        delete[] logoPositions_;
}

/*!
 * Load logos from original game files
 * @param paletteColors Palette used for logos
 * @param nbColors number of colors in the palette
 * @return True if everything is ok
 */
bool LogoManager::loadLogos(const uint8_t * paletteColors, int nbColors) {
    size_t fileSize;

    LOG(Log::k_FLG_GFX, "LogoManager", "loadLogos", ("Loading logos..."))

    data_all_logos_ = File::loadOriginalFile("mlogos.dat", fileSize);
    if (data_all_logos_ == nullptr) {
        FSERR(Log::k_FLG_GFX, "LogoManager", "loadLogos", ("Could not read mlogos.dat"))
        return false;
    }

    numberLogo_ = int(fileSize) / (kLogoBigWidth * kLogoBigWidth);

    // We store all logos in a temporaty buffer before copying everything in the texture
    // We only use the big logos and use SDL to strech texture
    const int textureWidth = kNumOfLogosPerRow * kLogoBigWidth;
    // there are 40 logos : so we use 64px height to get a power of 2
    const int textureHeight = 64;
    
    uint8_t *logosBuffer = new uint8_t[textureWidth * textureWidth];
    logoPositions_ = new Point2D*[numberLogo_];

    copyLogosPixelsToBuffer(data_all_logos_, logosBuffer);

    logosTexture_ = g_System.createTexture();
    bool res = logosTexture_->createSurfaceFromData(logosBuffer, 
                                            textureWidth, 
                                            textureHeight, 254);
    
    if (res) {
        logosTexture_->setPalette6b3(paletteColors, nbColors);
        logosTexture_->loadTextureFromSurface();
    }

    if (res) {
        LOG(Log::k_FLG_GFX, "LogoManager", "loadLogos", ("%i logos loaded\n", numberLogo_))
    }
    
    delete [] logosBuffer;

    return true;
}

/*!
 * Copies the content of a tile to a common temporary buffer that will be used
 * to initialize the tile texture. The buffer stores tiles as a 16x16 tileset.
 * Each tile is copied to the buffer at a position given by its id.
 * @param bigLogosData pixels of all big logos
 * @param logosBuffer Buffer to copy the tile to.
 */
void LogoManager::copyLogosPixelsToBuffer(const uint8_t *bigLogosData, uint8_t *logosBuffer) {
    for (int i=0; i < numberLogo_; i++) {
        // Coords in the destination surface
        int row = i / kNumOfLogosPerRow;
        int col = i - (row * kNumOfLogosPerRow);

        // start of the logo pixels in the destination surface (upper left corner of logo)
        int logoOffsetDest = (col * kLogoBigWidth) + (row * kNumOfLogosPerRow * kLogoBigWidth * kLogoBigWidth);
        int logoOffsetSrc = i * kLogoBigWidth * kLogoBigWidth;

        // Copy pixels line by line
        for (int j=0; j < kLogoBigWidth; j++) {
            int lineOffsetDest = j * kLogoBigWidth * kNumOfLogosPerRow;
            int lineOffsetSrc = j * kLogoBigWidth;
            memcpy(logosBuffer + logoOffsetDest + lineOffsetDest, bigLogosData + logoOffsetSrc + lineOffsetSrc, kLogoBigWidth);
        }

        logoPositions_[i] = new Point2D();
        logoPositions_[i]->x = col * kLogoBigWidth;
        logoPositions_[i]->y = row * kLogoBigWidth;
    }
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

/*!
 * Draw a logo at the given position and with the given color
 * @param pos Coordinate on screen where to draw the logo
 * @param logoId Id of the logo to draw
 * @param colourId Id of the color in the g_Colours array
 * @param big True to draw a 64x64 logo. Else it will be 32x32
 */
void LogoManager::draw(Point2D pos, int logoId, int colorId, bool big) {
    FSColor color;
    if (colorId > kMaxColour || !logosTexture_->getColorFromPalette(g_Colours[colorId], color)) {
        FSERR(Log::k_FLG_GFX, "LogoManager", "draw", ("Could not get color at index %i", colorId))
        return;
    }
    // In both case, first we draw a background of the given color
    // and then on top the logo
    if (big) {
        g_System.drawFillRect(pos, kLogoBigWidth*2, kLogoBigWidth*2, color);
        logosTexture_->renderStretch(*logoPositions_[logoId], pos, kLogoBigWidth, kLogoBigWidth, 2);
    } else {
        g_System.drawFillRect(pos, kLogoBigWidth, kLogoBigWidth, color);
        logosTexture_->render(*logoPositions_[logoId], pos, kLogoBigWidth, kLogoBigWidth);
    }
}

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2024 Benoit Blancard <benblan@users.sourceforge.net> *
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

#ifndef ENGINE_FSTEXTURE_H
#define ENGINE_FSTEXTURE_H

#include "fs-utils/common.h"
#include "fs-engine/enginecommon.h"

/*! \brief An abstract representing a texture used to render on screen using material acceleration.
 *
 */
class FSTexture {
public:

    virtual ~FSTexture() {}
    //! Create a 32bits texture with streaming access
    virtual bool createStreamingTexture(int width, int height) = 0;
    //! Update the pixels in the texture with the given pixels index and a palette
    virtual bool updateStreamingTexture(const uint8_t *pixels, FSColor *colorPalette_) = 0;
    //! Create a texture to draw on it
    virtual bool createRenderTargetTexture(int width, int height) = 0;
    //! Set this texture to be the render target.
    virtual bool setAsRenderTarget() = 0;
    //! Import a raw pixel array into a 8 bits surface of given width and height, with the color key
    virtual bool create8bitsSurfaceFromData(const uint8_t *srcPixels, int width, int height, uint8_t colorKey) = 0;
    [[deprecated("remove this method and use setPalette")]]
    virtual bool setPalette6b3(const uint8_t * pal, int cols) = 0;
    [[deprecated("remove this method and use setPalette")]]
    virtual bool setPalette8b3(const uint8_t * pal, int cols) = 0;
    //! Create a texture from an already loaded surface. Delete preexisting texture
    virtual bool loadTextureFromSurface() = 0;
    //! Renders a rectangle from the current texture to the current renderer
    virtual void render(Point2D src, Point2D dst, int width, int height) = 0;
    //! Renders a portion of the current texture to the destination with a given ration
    virtual void renderStretch(Point2D src, Point2D dst, int width, int height, int ratio) = 0;
    //! Render the full texture at 0,0 on screen with the given destination size
    virtual void renderFullTextureStrech(int width, int height) = 0;
    //! Return the color stored at given index in the palette store in this texture
    virtual bool getColorFromPalette(const int index, FSColor& color) = 0;
    //! Set a new palette for a existing surface.
    virtual bool setPalette(const fs_eng::Palette &palette) = 0;
    //! Set a color in the palette
    virtual void setColorInPalette(int index, fs_eng::FSColor color) = 0;
    //! Set the color for color modulation
    virtual void setColorModulation( fs_eng::FSColor color ) = 0;
};

#endif

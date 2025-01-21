/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
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

#ifndef ENGINE_FSTEXTURE_SDL_H
#define ENGINE_FSTEXTURE_SDL_H

#include <SDL.h>

#include "fs-engine/gfx/fstexture.h"

namespace fs_eng {

/*! \brief
 *
 */
class FSTextureSDL : public FSTexture {
public:
    FSTextureSDL(SDL_Renderer *pRenderer);
    ~FSTextureSDL();

    void render(Point2D src, Point2D dst, int width, int height) override;
    void renderStretch(Point2D src, Point2D dst, int width, int height, int ratio) override;
    void renderFullTextureStrech(int width, int height) override;
    //! @copydoc FSTexture::renderExtended()
    void renderExtended(Point2D src, Point2D dst, int width, int height, int ratio, bool flipped) override;

    //! Create a 32bits texture with streaming access
    bool createStreamingTexture(int width, int height) override;
    //! @copydoc FSTexture::updateStreamingTexture()
    bool updateStreamingTexture(const uint8_t *pixels, const fs_eng::Palette &colorPalette) override;

    //! Creates a texure to be used as a render target
    bool createRenderTargetTexture(int width, int height) override;
    //! @copydoc FSTexture::setAsRenderTarget()
    bool setAsRenderTarget() override;

    //! Creates a surface initialized with the array of pixel
    bool create8bitsSurfaceFromData(const uint8_t *srcPixels, int width, int height, uint8_t colorKey) override;
    //! Create a texture from the surface (a palette should have been defined first)
    bool loadTextureFromSurface() override;
    //! Return the color from the palette at given index
    bool getColorFromPalette(const int index, fs_eng::FSColor& color) override;
    //! @copydoc FSTexture::setPalette()
    bool setPalette(const fs_eng::Palette &palette) override;
    //! @copydoc FSTexture::setColorInPalette()
    void setColorInPalette(int index, fs_eng::FSColor color) override;
    //! @copydoc FSTexture::setColorModulation()
    void setColorModulation( fs_eng::FSColor color ) override;

private:
    //! Deallocates surface
    void freeSurface();

    //! Deallocates texture
    void freeTexture();

private:
    //! The renderer is necessary to manipulate SDL_Texture and use graphic acceleration
    SDL_Renderer *pRenderer_;
    //! This surface is loaded from original file and used for storing palette
    SDL_Surface *pSurface_;
    //! The actual hardware texture. It can be loaded multiple time
    SDL_Texture* pTexture_;
    //! Store the pixel format for the texture
    SDL_PixelFormat *pFormat_;
    //Image dimensions
    int width_;
    int height_;
};

}

#endif

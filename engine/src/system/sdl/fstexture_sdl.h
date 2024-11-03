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

#ifndef ENGINE_FSTEXTURE_SDL_H
#define ENGINE_FSTEXTURE_SDL_H

#include <SDL.h>

#include "fs-engine/gfx/fstexture.h"

/*! \brief
 *
 */
class FSTextureSDL : public FSTexture {
public:
    FSTextureSDL(SDL_Renderer *pRenderer);
    ~FSTextureSDL();

    void render(Point2D src, Point2D dst, int width, int height) override;
    void renderStretch(Point2D src, Point2D dst, int width, int height, int ratio) override;
    //! Creates a sufrace initialized with the array of pixel
    bool createSurfaceFromData(const uint8_t *srcPixels, int width, int height, uint8_t colorKey) override;
    //! Set a palette for the surface
    bool setPalette6b3(const uint8_t * pal, int cols) override;
    bool setPalette8b3(const uint8_t * pal, int cols) override;
    //! Create a texture from the surface (a palette should have been defined first)
    bool loadTextureFromSurface();
    //! Return the color from the palette at given index
    bool getColorFromPalette(const int index, FSColor& color) override;

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
    //Image dimensions
    int width_;
    int height_;
};

#endif

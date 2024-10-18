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

    bool importTilesetInSurface(const uint8_t *tilesPixels, int width, int height) override;

    bool setPalette6b3(const uint8_t * pal, int cols) override;
    bool setPalette8b3(const uint8_t * pal, int cols) override;

    bool loadTextureFromSurface();

private:
    //! Deallocates surface
    void freeSurface();

    //! Deallocates texture
    void freeTexture();

private:
    //! Index in the palette for the transparent color
    static const int kColorKeyIndex;

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

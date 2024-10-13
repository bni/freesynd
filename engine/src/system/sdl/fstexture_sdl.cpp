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

#include "fstexture_sdl.h"

#include "fs-utils/log/log.h"

/*!
 * @brief 
 * @param pRenderer 
 */
FSTextureSDL::FSTextureSDL(SDL_Renderer *pRenderer) {
    pRenderer_ = pRenderer;
    pSurface_ = nullptr;
    pTexture_ = nullptr;
    width_ = 0;
    height_ = 0;
}

/*!
 * @brief 
 */
FSTextureSDL::~FSTextureSDL() {
    freeSurface();
    freeTexture();
    pRenderer_ = nullptr;
}

/*!
 * @brief 
 */
void FSTextureSDL::freeSurface() {
    if( pSurface_ != NULL ) {
        SDL_FreeSurface(pSurface_);
        pSurface_ = NULL;
        width_ = 0;
        height_ = 0;
    }
}

/*!
 * @brief 
 */
void FSTextureSDL::freeTexture() {
    if( pTexture_ != NULL ) {
        SDL_DestroyTexture( pTexture_ );
        pTexture_ = NULL;
    }
}

/*!
 * @brief 
 * @param src 
 * @param dst 
 * @param width 
 * @param height 
 */
void FSTextureSDL::render(Point2D src, Point2D dst, int width, int height) {
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { dst.x, dst.y, width, height };
    SDL_Rect tileQuad = { src.x, src.y, width, height };
    if (!SDL_RenderCopy( pRenderer_, pTexture_, &tileQuad, &renderQuad )) {
        //printf("Failed to copy: %s\n", SDL_GetError());
    }
}

/*!
 * 
 * @param tilesPixels 
 * @param width 
 * @param height 
 * @return 
 */
bool FSTextureSDL::importTilesetInSurface(const uint8_t *tilesPixels, int width, int height) {
    bool res = true;
    // Initialize an indexed surface
    freeSurface();
    pSurface_ = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    if (pSurface_ == nullptr) {
        FSERR(Log::k_FLG_GFX, "FSTextureSDL", "importTilesetInSurface", ("Critical error, Tileset surface could not be created! SDL Error : %s", SDL_GetError()))
        return false;
    }
    width_ = width;
    height_ = height;

    if (SDL_MUSTLOCK(pSurface_) && !SDL_LockSurface(pSurface_)) {
        res = false;
        FSERR(Log::k_FLG_GFX, "FSTextureSDL", "importTilesetInSurface", ("Critical error, Could not lock surface! SDL Error : %s", SDL_GetError()))
    }

    if (res) {
        Uint8 *dstPixels = (Uint8 *) (pSurface_->pixels);
        for (int i = 0; i < width * height; i++) {
            dstPixels[i] = tilesPixels[i];
        }

        if (SDL_MUSTLOCK(pSurface_)) {
            SDL_UnlockSurface(pSurface_);
        }
    }
    
    return res;
}

/*!
 * @brief 
 * @param pal 
 * @param cols 
 * @return 
 */
bool FSTextureSDL::setPalette6b3(const uint8_t * pal, int cols) {
    static SDL_Color palette[256];

    for (int i = 0; i < cols; ++i) {
        uint8_t r = pal[i * 3 + 0];
        uint8_t g = pal[i * 3 + 1];
        uint8_t b = pal[i * 3 + 2];

        // multiply by 255 divide by 63 isn't good enough?
        palette[i].r = (r << 2) | (r >> 4);
        palette[i].g = (g << 2) | (g >> 4);
        palette[i].b = (b << 2) | (b >> 4);

#if 0
        if (like(palette[i].r, 28) && like(palette[i].g, 144)
            && like(palette[i].b, 0))
            printf("col %i = %i, %i, %i\n", i, palette[i].r, palette[i].g,
                   palette[i].b);
#endif
    }

    if (SDL_SetPaletteColors(pSurface_->format->palette, palette, 0, cols)) {
        FSERR(Log::k_FLG_GFX, "SystemSDL", "setPalette6b3", ("Could not set palette6b3 with %i colors! SDL Error : %s", cols, SDL_GetError()))
        return false;
    }
    return true;
}

/*!
 * @brief 
 * @param pal 
 * @param cols 
 * @return 
 */
bool FSTextureSDL::setPalette8b3(const uint8_t * pal, int cols) {
    static SDL_Color palette[256];

    for (int i = 0; i < cols; ++i) {
        palette[i].r = pal[i * 3 + 0];
        palette[i].g = pal[i * 3 + 1];
        palette[i].b = pal[i * 3 + 2];
    }

    if (SDL_SetPaletteColors(pSurface_->format->palette, palette, 0, cols)) {
        FSERR(Log::k_FLG_GFX, "SystemSDL", "setPalette8b3", ("Could not set palette8b3 with %i colors! SDL Error : %s", cols, SDL_GetError()))
        return false;
    }
    return true;
}

/*!
 * @brief 
 * @return 
 */
bool FSTextureSDL::loadTextureFromSurface() {
    freeTexture();
    pTexture_ = SDL_CreateTextureFromSurface(pRenderer_, pSurface_);
    if (pTexture_ == nullptr) {
        FSERR(Log::k_FLG_GFX, "FSTextureSDL", "loadTextureFromSurface", ("Critical error, Could create texture from surface! SDL Error : %s", SDL_GetError()))
    }
    return pTexture_ != nullptr;
}
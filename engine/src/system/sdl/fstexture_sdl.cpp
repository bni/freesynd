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
    pFormat_ = nullptr; 
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

    if (pFormat_ != nullptr) {
        SDL_FreeFormat(pFormat_);
        pFormat_ = nullptr;
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

void FSTextureSDL::renderStretch(Point2D src, Point2D dst, int width, int height, int ratio) {
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { dst.x, dst.y, width*ratio, height*ratio };
    SDL_Rect tileQuad = { src.x, src.y, width, height};
    if (!SDL_RenderCopy( pRenderer_, pTexture_, &tileQuad, &renderQuad )) {
        //printf("Failed to copy: %s\n", SDL_GetError());
    }
}

/*!
 * Render a texture clip to a destination rect and position with possible ratio and horizontal flip.
 * @param src Origin of the clip in source texture
 * @param dst Position on the screen to copy the texture
 * @param destWidth Width of destination
 * @param destHeight Height of destination
 * @param ratio A multiplication factor that change the size of destination rect
 * @param flipped True means to flip the texture in horizontal
 */
void FSTextureSDL::renderExtended(Point2D src, Point2D dst, int width, int height, int ratio, bool flipped) {
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { dst.x, dst.y, width*ratio, height*ratio };
    SDL_Rect tileQuad = { src.x, src.y, width, height};
    SDL_RendererFlip flip = flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    if (!SDL_RenderCopyEx( pRenderer_, pTexture_, &tileQuad, &renderQuad, 0, NULL, flip )) {
        //printf("Failed to copy: %s\n", SDL_GetError());
    }
}

/*!
 * Render this texture as a whole to the target at position (0,0) and with the given size.
 * @param destWidth Width of destination
 * @param destHeight Height of destination
 */
void FSTextureSDL::renderFullTextureStrech(int destWidth, int destHeight) {
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { 0, 0, destWidth, destHeight };
    SDL_Rect tileQuad = { 0, 0, width_, height_};
    if (!SDL_RenderCopy( pRenderer_, pTexture_, &tileQuad, &renderQuad )) {
        //printf("Failed to copy: %s\n", SDL_GetError());
    }
}

/*!
 * Create a texture with pixel format of RGBA8888 for a stream access.
 * The resulting format of the texture is kept in pFormat_ member.
 * @param width Width of the new texture
 * @param height Height of the new texture
 * @return true if creation is ok
 */
bool FSTextureSDL::createStreamingTexture(int width, int height) {
    freeTexture();
    
    pTexture_ = SDL_CreateTexture(pRenderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (pTexture_ == nullptr) {
        FSERR(Log::k_FLG_GFX, "FSTextureSDL", "createStreamingTexture", ("Critical error, Could create texture! SDL Error : %s", SDL_GetError()))
    }

    width_ = width;
    height_ = height;
    pFormat_ = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

    return pTexture_ != nullptr;
}

/*!
 * Update the whole texture with the given pixels.
 * Pixels are expected to be index in the given color palette.
 * @param pixels 
 * @param colorPalette_ 
 * @return 
 */
bool FSTextureSDL::updateStreamingTexture(const uint8_t *pixels, fs_eng::FSColor *colorPalette_) {
    int pitch;
    Uint32 *destPixels;
    if (SDL_LockTexture(pTexture_, nullptr, (void **)&destPixels, &pitch)) {
        FSERR(Log::k_FLG_GFX, "FSTextureSDL", "updateStreamingTexture", ("Critical error, Could not lock texture! SDL Error : %s", SDL_GetError()))
        return false;
    }

    const uint8_t *srcPixels = pixels;

    for (int i = 0; i < width_ * height_; i++) {
        uint8_t index = srcPixels[i];
        fs_eng::FSColor color = colorPalette_[index];

        destPixels[i] = SDL_MapRGBA(pFormat_, color.r, color.g, color.b, color.a);
    }

    SDL_UnlockTexture(pTexture_);

    return true;
}

/*!
 * @brief 
 * @param width 
 * @param height 
 * @return 
 */
bool FSTextureSDL::createRenderTargetTexture(int width, int height) {
    freeTexture();

    pTexture_ = SDL_CreateTexture(pRenderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
    if (pTexture_ == nullptr) {
        FSERR(Log::k_FLG_GFX, "FSTextureSDL", "createRenderTargetTexture", ("Critical error, Could not create texture! SDL Error : %s", SDL_GetError()))
    }

    width_ = width;
    height_ = height;
    pFormat_ = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

    return pTexture_ != nullptr;
}

/*!
 * Set this texture to be the target
 * @return true if operation succeeded
 */
bool FSTextureSDL::setAsRenderTarget() {
    if (SDL_SetRenderTarget( pRenderer_, pTexture_ )) {
        FSERR(Log::k_FLG_GFX, "FSTextureSDL", "setAsRenderTarget", ("Critical error, Could not set target texture! SDL Error : %s", SDL_GetError()))
        return false;
    }

    return true;
}

/*!
 * Copy an array of pixel in the surface stored inside this class.
 * The surface is an 8 bits pixel so an empty palette is created.
 * @param srcPixels Array of pixel. Must be the same size as width and height
 * @param width Width of the surface
 * @param height Height of the surface
 * @param colorKey The index in the palette of the color key to use
 * @return True if import is ok
 */
bool FSTextureSDL::create8bitsSurfaceFromData(const uint8_t *srcPixels, int width, int height, uint8_t colorKey) {
    LOG(Log::k_FLG_GFX, "FSTextureSDL", "importSurface", ("Importing pixels in surface..."))
    bool res = true;
    // Initialize an indexed surface
    freeSurface();
    pSurface_ = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    if (pSurface_ == nullptr) {
        FSERR(Log::k_FLG_GFX, "FSTextureSDL", "importSurface", ("Critical error, Tileset surface could not be created! SDL Error : %s", SDL_GetError()))
        return false;
    }
    width_ = width;
    height_ = height;

    if (SDL_MUSTLOCK(pSurface_) && !SDL_LockSurface(pSurface_)) {
        res = false;
        FSERR(Log::k_FLG_GFX, "FSTextureSDL", "importSurface", ("Critical error, Could not lock surface! SDL Error : %s", SDL_GetError()))
    }

    if (res) {
        Uint8 *dstPixels = (Uint8 *) (pSurface_->pixels);
        for (int i = 0; i < width * height; i++) {
            dstPixels[i] = srcPixels[i];
        }

        if (SDL_MUSTLOCK(pSurface_)) {
            SDL_UnlockSurface(pSurface_);
        }
    }
    // Set the color at given index in the palette as a transparent color
    SDL_SetColorKey(pSurface_, SDL_TRUE, colorKey);
    
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
 * Sets a new palette. This texture must have been created with create8bitsSurfaceFromData().
 * @param palette A palette of color
 * @return True if everything is ok
 */
bool FSTextureSDL::setPalette(const fs_eng::Palette &palette) {
    SDL_Color sdlPalette[fs_eng::kPaletteMaxColor];
    size_t i = 0;

    for (const auto& color : palette) {
        sdlPalette[i].r = color.r;
        sdlPalette[i].g = color.g;
        sdlPalette[i].b = color.b;
        sdlPalette[i].a = 0xFF;
        i++;
    }

    if (SDL_SetPaletteColors(pSurface_->format->palette, sdlPalette, 0, fs_eng::kPaletteMaxColor)) {
        FSERR(Log::k_FLG_GFX, "FSTextureSDL", "setPalette", ("Could not set palette with %i colors! SDL Error : %s", fs_eng::kPaletteMaxColor, SDL_GetError()))
        return false;
    }
    return true;
}

void FSTextureSDL::setColorInPalette(int index, fs_eng::FSColor color) {
    SDL_Color newColor {color.r, color.g, color.b, color.a};

    if (SDL_SetPaletteColors(pSurface_->format->palette, &newColor, index, 1)) {
        FSERR(Log::k_FLG_GFX, "FSTextureSDL", "setPalette", ("Could not set color in palette! SDL Error : %s", SDL_GetError()))
    }
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
    if (pSurface_) {
        pTexture_ = SDL_CreateTextureFromSurface(pRenderer_, pSurface_);
        if (pTexture_ == nullptr) {
            FSERR(Log::k_FLG_GFX, "FSTextureSDL", "loadTextureFromSurface", ("Critical error, Could create texture from surface! SDL Error : %s", SDL_GetError()))
        }
    }
    return pTexture_ != nullptr;
}

/*!
 * Returns the color in the palette at the given index
 * @param index Index of the color in the palette
 * @param color Resulting color
 * @return True if color was found
 */
bool FSTextureSDL::getColorFromPalette(const int index, fs_eng::FSColor& color) {
    if (index < 0 || index > pSurface_->format->palette->ncolors) {
        return false;
    }

    SDL_Color sdlColor = pSurface_->format->palette->colors[index];
    color.r = sdlColor.r;
    color.g = sdlColor.g;
    color.b = sdlColor.b;
    color.a = sdlColor.a;
    
    return true;
}

/*!
 * Set the color factor for color modulation
 * @param color The color factor
 */
void FSTextureSDL::setColorModulation( fs_eng::FSColor color ) {
    //Modulate texture
    SDL_SetTextureColorMod( pTexture_, color.r, color.g, color.b );
}
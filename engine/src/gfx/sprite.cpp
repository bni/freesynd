/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
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

#include "fs-engine/gfx/sprite.h"

#include <assert.h>
#include <stdio.h>
#include <png.h>

#include "fs-utils/log/log.h"
#include "fs-engine/gfx/screen.h"

void unpackBlocks1(const uint8_t * data, uint8_t * pixels)
{
    for (int i = 0; i < 8; ++i) {
        if (bitSet(data[0], 7 - i)) {
            pixels[i] = 255;    // transparent
        } else {
            pixels[i] =
                static_cast < uint8_t >
                ((bitValue(data[1], 7 - i) << 0) & 0xff)
                | static_cast < uint8_t >
                ((bitValue(data[2], 7 - i) << 1) & 0xff)
                | static_cast < uint8_t >
                ((bitValue(data[3], 7 - i) << 2) & 0xff)
                | static_cast < uint8_t >
                ((bitValue(data[4], 7 - i) << 3) & 0xff);
        }
    }
}

const int Sprite::kPixelPerBlock = 8;
const int Sprite::kBlockLength = kPixelPerBlock / 2 + kPixelPerBlock / 8;
const int Sprite::kTabEntrySize = 6;
const int Sprite::MSPR_SELECT_1 = 1;
const int Sprite::MSPR_SELECT_2 = 2;
const int Sprite::MSPR_SELECT_3 = 3;
const int Sprite::MSPR_SELECT_4 = 4;
const int Sprite::MSPR_LEFT_ARROW_D = 5;
const int Sprite::MSPR_LEFT_ARROW_L = 8;
const int Sprite::MSPR_RIGHT_ARROW_D = 6;
const int Sprite::MSPR_RIGHT_ARROW_L = 9;
const int Sprite::MSPR_BULLET_D = 7;
const int Sprite::MSPR_BULLET_L = 10;
const int Sprite::MSPR_TAX_DECR = 11;
const int Sprite::MSPR_TAX_INCR = 12;
const int Sprite::MSPR_LEFT_ARROW2_D = 82;
const int Sprite::MSPR_LEFT_ARROW2_L = 80;
const int Sprite::MSPR_RIGHT_ARROW2_D = 81;
const int Sprite::MSPR_RIGHT_ARROW2_L = 79;

Sprite::Sprite()
:  width_(0)
    , height_(0)
    , stride_(0)
    , sprite_data_(NULL)
{
}

Sprite::~Sprite()
{
    if (sprite_data_)
        delete[] sprite_data_;

    width_ = height_ = stride_ = 0;
    sprite_data_ = NULL;
}

void Sprite::loadSpriteFromPNG(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        //fprintf(stderr, "unable to open %s.\n", filename);
        return;
    }
    png_byte header[8];
    size_t shead = fread(header, 1, 8, fp);
    if (shead == 0 || png_sig_cmp(header, 0, 8) != 0) {
        FSERR(Log::k_FLG_IO, "Sprite", "loadSpriteFromPNG", ("Error loading sprite : %s is not a png.", filename))
        fclose(fp);
        return;
    }
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png_ptr) {
        FSERR(Log::k_FLG_IO, "Sprite", "loadSpriteFromPNG", ("Error creating png read struct for %s.", filename))
        fclose(fp);
        return;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, 0, 0);
        FSERR(Log::k_FLG_IO, "Sprite", "loadSpriteFromPNG", ("Error creating png info struct for %s.", filename))
        fclose(fp);
        return;
    }
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

    png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);

    png_uint_32 w, h;
    int depth, color_type, interlace_type, compression_type, filter_method;

    png_get_IHDR(png_ptr, info_ptr, &w, &h, &depth, &color_type, &interlace_type,
                    &compression_type, &filter_method);

    if (depth != 8) {
        FSERR(Log::k_FLG_IO, "Sprite", "loadSpriteFromPNG", ("Error loading sprite : expected 8 bit depth from %s.", filename))
    } else {
        if (sprite_data_)
            delete[] sprite_data_;

        sprite_data_ = new uint8_t[w * h];
        width_ = int(w);
        height_ = int(h);
        stride_ = int(w);
        for (int i = 0; i < height_; i++)
            memcpy(sprite_data_ + i * stride_, row_pointers[i], w);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, 0);

    fclose(fp);
}

bool Sprite::loadSprite(uint8_t * tabData, uint8_t * spriteData, int offset,
                        bool rle)
{
    if (tabData == nullptr || spriteData == nullptr) {
        FSERR(Log::k_FLG_IO, "Sprite", "loadSprite", ("Error loading sprite : sprite data is null."))
        return false;
    } else if (offset < 0) {
        FSERR(Log::k_FLG_IO, "Sprite", "loadSprite", ("Error loading sprite : offset is negative %d.", offset))
        return false;
    }

    uint8_t *tabEntry = tabData + offset * kTabEntrySize;

    uint32_t spriteOffset = READ_LE_UINT32(tabEntry);

    tabEntry += 4;
    width_ = *tabEntry;
    tabEntry += 1;
    height_ = *tabEntry;

    if (width_ == 0 || height_ == 0)
        return true;

    stride_ = ceil8(width_);
    uint8_t *spriteBlocks = spriteData + spriteOffset;

    sprite_data_ = new uint8_t[stride_ * height_];
    memset(sprite_data_, 255, size_t(stride_ * height_));

    uint8_t *currentPixel;

    if (rle) {
        for (int i = 0; i < height_; ++i) {
            int spriteWidth = width_;
            currentPixel = sprite_data_ + i * stride_;

            uint8_t b = *spriteBlocks++;
            int runLength = b < 128 ? b : -(256 - b);
            while (runLength != 0) {
                spriteWidth -= runLength;

                if (runLength > 0) {
                    if (currentPixel < sprite_data_)
                        currentPixel = sprite_data_;
                    if (currentPixel + runLength >
                        sprite_data_ + height_ * stride_)
                        runLength =
                            sprite_data_ + height_ * stride_ -
                            currentPixel;
                    // pixel run
                    for (int j = 0; j < runLength; ++j)
                        *currentPixel++ = *spriteBlocks++;

                } else if (runLength < 0) {
                    // transparent run
                    runLength *= -1;
                    if (currentPixel < sprite_data_)
                        currentPixel = sprite_data_;
                    if (currentPixel + runLength >
                        sprite_data_ + height_ * stride_)
                        runLength =
                            sprite_data_ + height_ * stride_ -
                            currentPixel;
                    memset(currentPixel, 255, size_t(runLength));
                    currentPixel += runLength;

                } else if (runLength == 0) {
                    // end of the row
                    spriteWidth = 0;
                }

                b = *spriteBlocks++;
                runLength = b < 128 ? b : -(256 - b);
            }
        }
    } else {
        for (int j = 0; j < height_; ++j) {
            currentPixel = sprite_data_ + j * stride_;

            for (int i = 0; i < width_; i += kPixelPerBlock) {
                unpackBlocks1(spriteBlocks, currentPixel);

                spriteBlocks += kBlockLength;
                currentPixel += kPixelPerBlock;
            }
        }
    }

    return true;
}

void Sprite::draw(int x, int y, int z, bool flipped, bool x2)
{
    if (x2)
        g_Screen.scale2x(x, y, width_, height_, sprite_data_, stride_);
    else
        g_Screen.blit(x, y, width_, height_, sprite_data_, flipped,
                      stride_);
}

void Sprite::data(uint8_t * spr_data) const
{
    for (int j = 0; j < height_; j++) {
        memcpy(spr_data + j * width_, sprite_data_ + j * stride_, (size_t) width_);
    }
}

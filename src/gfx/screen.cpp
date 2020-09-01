/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#include "common.h"
#include "screen.h"
#include "utils/file.h"

const int Screen::kScreenWidth = 640;
const int Screen::kScreenHeight = 400;
const int Screen::kScreenPanelWidth = 129;

Screen::Screen(int width, int height)
:width_(width)
, height_(height)
, pixels_(NULL)
, dirty_(false)
, data_logo_(NULL), data_logo_copy_(NULL)
, data_mini_logo_(NULL), data_mini_logo_copy_(NULL)
{
    assert(width_ > 0);
    assert(height_ > 0);

    pixels_ = new uint8[width_ * height_];
}

Screen::~Screen()
{
    delete[] pixels_;
    if (data_logo_)
        delete[] data_logo_;
    if (data_logo_copy_)
        delete[] data_logo_copy_;
    if (data_mini_logo_)
        delete[] data_mini_logo_;
    if (data_mini_logo_copy_)
        delete[] data_mini_logo_copy_;
}

void Screen::clear(uint8 color)
{
    memset(pixels_, color, width_ * height_);
    dirty_ = true;
}
/*!
 * Blits data to screen
 * @param x position by x coord
 * @param y position by y coord
 * @param width data's width
 * @param height data's height
 * @param pixeldata pointer to data to be blitted
 * @param flipped draw flipped
 * @param stride actual data width (Sprite class related)
 * @sa sprite.h
 */
void Screen::blit(int x, int y, int width, int height,
                  const uint8 * pixeldata, bool flipped, int stride)
{
    if (x + width < 0 || y + height < 0 || x >= width_ || y >= height_)
        return;

    int clipped_x = x < 0 ? 0 : x;
    int clipped_y = y < 0 ? 0 : y;

    int sx = x < 0 ? -x : 0;
    int sy = y < 0 ? -y : 0;

    int w = x < 0 ? x + width : x + width > width_ ? width_ - x : width;
    int h = y < 0
        ? y + height : y + height > height_ ? height_ - y : height;

    stride = (stride == 0 ? width : stride);
    int ofs = (flipped ? w - 1 : 0) + clipped_x;
    uint8 *d = pixels_ + clipped_y * width_ + ofs;

    if (flipped) {
        const uint8 *s = pixeldata + sy * stride + sx + (width - w);
        for (int j = 0; j < h; ++j) {

            const uint8 *cp_s = s;
            s += stride;
            uint8 *cp_d = d;
            d += width_;
            for (int i = 0; i < w; ++i) {
                uint8 c = *cp_s++;

                if (c != 255)
                    *cp_d = c;
                cp_d--;
            }
        }
    } else {
        const uint8 *s = pixeldata + sy * stride + sx;
        for (int j = 0; j < h; ++j) {

            const uint8 *cp_s = s;
            s += stride;
            uint8 *cp_d = d;
            d += width_;
            for (int i = 0; i < w; ++i) {
                uint8 c = *cp_s++;

                if (c != 255)
                    *cp_d = c;
                cp_d++;
            }
        }
    }

    dirty_ = true;
}

/*!
 * Blits a portion of the source data to the screen a given position.
 */
void Screen::blitRect(int x, int y, int width, int height,
                  const uint8 * pixeldata, bool flipped, int stride)
{
    if (x + width < 0 || y + height < 0 || x >= width_ || y >= height_)
        return;

    int dest_x = x < 0 ? 0 : x;
    int dest_y = y < 0 ? 0 : y;

    int clipped_w = x < 0 ? x + width : x + width > width_ ? width_ - x : width;
    int clipped_h = y < 0
        ? y + height : y + height > height_ ? height_ - y : height;

    stride = (stride == 0 ? width : stride);
    int ofs = (flipped ? clipped_w - 1 : 0) + dest_x;
    uint8 *d = pixels_ + dest_y * width_ + ofs;

    if (flipped) {
        const uint8 *s = pixeldata + y * stride + x + (width - clipped_w);
        for (int j = 0; j < clipped_h; ++j) {

            const uint8 *cp_s = s;
            s += stride;
            uint8 *cp_d = d;
            d += width_;
            for (int i = 0; i < clipped_w; ++i) {
                uint8 c = *cp_s++;

                if (c != 255)
                    *cp_d = c;
                cp_d--;
            }
        }
    } else {
        const uint8 *s = pixeldata + y * stride + x;
        for (int j = 0; j < clipped_h; ++j) {

            const uint8 *cp_s = s;
            s += stride;
            uint8 *cp_d = d;
            d += width_;
            for (int i = 0; i < clipped_w; ++i) {
                uint8 c = *cp_s++;

                if (c != 255)
                    *cp_d = c;
                cp_d++;
            }
        }
    }

    dirty_ = true;
}

void Screen::scale2x(int x, int y, int width, int height,
                     const uint8 * pixeldata, int stride, bool transp)
{
    stride = (stride == 0 ? width : stride);

    for (int j = 0; j < height; ++j) {
        uint8 *d = pixels_ + (y + j * 2) * width_ + x;

        for (int i = 0; i < width; ++i, d += 2) {
            uint8 c = *(pixeldata + i);
            if (c != 255 || !transp) {
                *(d + 0) = c;
                *(d + 1) = c;
                *(d + 0 + width_) = c;
                *(d + 1 + width_) = c;
            }
        }

        pixeldata += stride;
    }

    dirty_ = true;
}

void Screen::drawVLine(int x, int y, int length, uint8 color)
{
    if (x < 0 || x >= width_ || y + length < 0 || y >= height_)
        return;

    length = y + length >= height_ ? height_ - y : length;
    if (y < 0) {
        length += y;
        y = 0;
    }
    if (length < 1)
        return;

    uint8 *pixel = pixels_ + y * width_ + x;
    while (length--) {
        *pixel = color;
        pixel += width_;
    }

    dirty_ = true;
}

void Screen::drawHLine(int x, int y, int length, uint8 color)
{
    if ((x + length) < 0 || x >= width_ || y < 0 || y >= height_)
        return;

    length = x + length >= width_ ? width_ - x : length;
    if (x < 0) {
        length += x;
        x = 0;
    }
    if (length < 1)
        return;

    uint8 *pixel_ptr = pixels_ + y * width_ + x;
    while (length--)
        *pixel_ptr++ = color;

    dirty_ = true;
}

int Screen::numLogos()
{
    return size_logo_ / (32 * 32);
}

void Screen::drawLogo(int x, int y, int logo, int colour, bool mini)
{
    if (data_logo_ == NULL) {
        data_logo_ = File::loadOriginalFile("mlogos.dat", size_logo_);
        data_logo_copy_ = new uint8[size_logo_];
    }
    if (data_mini_logo_ == NULL) {
        data_mini_logo_ = File::loadOriginalFile("mminlogo.dat", size_mini_logo_);
        data_mini_logo_copy_ = new uint8[size_mini_logo_];
    }

    for (int i = 0; i < size_logo_; i++)
        if (data_logo_[i] == 0xFE)
            data_logo_copy_[i] = colour;
        else
            data_logo_copy_[i] = data_logo_[i];
    for (int i = 0; i < size_mini_logo_; i++)
        if (data_mini_logo_[i] == 0xFE)
            data_mini_logo_copy_[i] = colour;
        else
            data_mini_logo_copy_[i] = data_mini_logo_[i];
    if (mini)
        scale2x(x, y, 16, 16, data_mini_logo_copy_ + logo * 16 * 16, 16);
    else
        scale2x(x, y, 32, 32, data_logo_copy_ + logo * 32 * 32, 32);

    dirty_ = true;
}

// Taken from SDL_gfx
#define ABS(a) (((a)<0) ? -(a) : (a))
void Screen::drawLine(int x1, int y1, int x2, int y2, uint8 color,
                      int skip, int off)
{
    int pixx, pixy;
    int x, y;
    int dx, dy;
    int sx, sy;
    int swaptmp;
    uint8 *pixel;

    /*
     * Variable setup
     */
    dx = x2 - x1;
    dy = y2 - y1;
    sx = (dx >= 0) ? 1 : -1;
    sy = (dy >= 0) ? 1 : -1;

    /*
     * No alpha blending - use fast pixel routines
     */

    /*
     * More variable setup
     */
    dx = sx * dx + 1;
    dy = sy * dy + 1;
    pixx = 1;
    pixy = GAME_SCREEN_WIDTH;
    pixel = pixels_ + pixx * (int) x1 + pixy * (int) y1;
    pixx *= sx;
    pixy *= sy;
    if (dx < dy) {
        swaptmp = dx;
        dx = dy;
        dy = swaptmp;
        swaptmp = pixx;
        pixx = pixy;
        pixy = swaptmp;
    }

    /*
     * Draw
     */
    x = 0;
    y = 0;
    int count = 0;
    for (; x < dx; x++, pixel += pixx) {
        if (skip == 0 || !(((off + count++) / skip) & 1))
            if (pixel >= pixels_ && pixel < pixels_ + width_ * height_)
                *pixel = color;
        y += dy;
        if (y >= dx) {
            y -= dx;
            pixel += pixy;
        }
    }

    dirty_ = true;
}

void Screen::setPixel(int x, int y, uint8 color)
{
    if (x < 0 || y < 0 || x >= width_ || y >= height_)
        return;
    pixels_[y * width_ + x] = color;
    dirty_ = true;
}


void Screen::drawRect(int x, int y, int width, int height, uint8 color)
{
    if (x < 0 || y < 0 || (x + width) > width_
        || (y + height) > height_ || width <= 0 || height <= 0)
        return;
    // NOTE: we don't handle properly clipping by (x,y), do we need it?


    for (int i = 0; i < height; i++) {
        uint8 *p_pixels = pixels_ + x + width_ * (y + i);
        for (int w = 0; w != width; w++)
            *p_pixels++ = color;
    }
    dirty_ = true;
}

int Screen::gameScreenHeight()
{
    return GAME_SCREEN_HEIGHT;
}

int Screen::gameScreenWidth()
{
    return GAME_SCREEN_WIDTH;
}

int Screen::gameScreenLeftMargin()
{
    return 129;
}


/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#ifndef SCREEN_H
#define SCREEN_H

#include "common.h"

/*!
 * Screen class.
 */
class Screen : public Singleton<Screen> {
public:
    /*! Width of the screen in pixels.*/
    static const int kScreenWidth;
    /*! Height of the screen in pixels.*/
    static const int kScreenHeight;
    /*! Width of the left control panel*/
    static const int kScreenPanelWidth;

    explicit Screen(int width, int height);
    ~Screen();

    void clear(uint8 color = 0);

    const uint8 *pixels() const { return pixels_; }
    bool dirty() { return dirty_; }
    void clearDirty() { dirty_ = false; }

    void blit(int x, int y, int width, int height, const uint8 *pixeldata,
            bool flipped = false, int stride = 0);
    void blitRect(int x, int y, int width, int height,
                  const uint8 * pixeldata, bool flipped = false, int stride = 0);
    void scale2x(int x, int y, int width, int height, const uint8 *pixeldata,
            int stride = 0, bool transp = true);

    void drawVLine(int x, int y, int length, uint8 color);
    void drawHLine(int x, int y, int length, uint8 color);

    int numLogos();
    void drawLogo(int x, int y, int logo, int colour, bool mini = false);
    void drawLine(int x1, int y1, int x2, int y2, uint8 color, int skip = 0,
            int off = 0);

    void setPixel(int x, int y, uint8 color);
    void drawRect(int x, int y, int width, int height, uint8 color = 0);

    int gameScreenHeight();
    int gameScreenWidth();
    int gameScreenLeftMargin();

protected:
    int width_;
    int height_;
    uint8 *pixels_;
    bool dirty_;
    int size_logo_;
    uint8 *data_logo_, *data_logo_copy_;
    int size_mini_logo_;
    uint8 *data_mini_logo_, *data_mini_logo_copy_;

    Screen();
};

#define g_Screen    Screen::singleton()

#endif

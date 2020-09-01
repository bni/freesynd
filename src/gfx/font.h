/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
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

#ifndef FONT_H
#define FONT_H

#include "common.h"
#include "spritemanager.h"
#include <map>

/*!
 * Font range description for 8-bit character sets.
 */
class FontRange {
public:
    FontRange();
    FontRange(const std::string& valid_chars);

    inline bool in_range(unsigned char c) {
        return (char_present_[c / 32] & (1 << (c % 32))) != 0;
    }

private:
    unsigned int char_present_[8]; // 256 bits
};

/*!
 * Font class.
 */
class Font {
public:
    Font() {}
    virtual ~Font() {}

    void setSpriteManager(SpriteManager *sprites, int offset, char base,
            const std::string& valid_chars);
    void setSpriteManager(SpriteManager *sprites, int offset, char base,
            const FontRange& range);
    // If dos is true, the text is in cp437, otherwise it's utf-8.
    void drawText(int x, int y, const char *text, bool dos, bool x2 = true);
    int textWidth(const char *text, bool dos, bool x2 = true);
    int textHeight(bool x2 = true);

    // returns true if given code point is printable with the font
    bool isPrintable(uint16 unicode);

protected:
    static unsigned char decode(const unsigned char * &c, bool dos);
    static int decodeUTF8(const unsigned char * &c);
    virtual Sprite *getSprite(unsigned char dos_char);

    SpriteManager *sprites_;
    int offset_;
    FontRange range_;
};

/*! 
 * Font used in menu and widget. Text can be highlighted or dark.
 * Text can be in UTF-8 or Cp437.
 */
class MenuFont : public Font {
public:
    MenuFont();

    void setSpriteManager(SpriteManager *sprites, int darkOffset, int lightOffset, char base,
            const std::string& valid_chars);

    //! draws a UTF-8 text at the given position
    void drawText(int x, int y, const char *text, bool lighted, bool x2 = true) {
        drawText(x, y, false, text, lighted, x2);
    };
    //! draws a Cp437 text at the given position
    void drawTextCp437(int x, int y, const char *text, bool lighted, bool x2 = true) {
        drawText(x, y, true, text, lighted, x2);
    };

protected:
    //! returns the sprite which can be highlighted or not
    virtual Sprite *getSprite(unsigned char dos_char, bool highlighted);
    //! draws a text at the given position
    void drawText(int x, int y, bool dos, const char *text, bool lighted, bool x2 = true);

protected:
    int lightOffset_;
};

/*! 
 * GameFont is the font used during gameplay (for displaying hints).
 * It uses the same sprites as the menu fonts but only in size 3 and it
 * changes its color to a specified color.
 * It uses only UTF-8 text in input.
 */
class GameFont : public Font {
public:
    GameFont();

    //! draw a UTF-8 text at the given position with the given color
    void drawText(int x, int y, const char *text, uint8 toColor);
};

class HChar {
public:
    HChar();
    ~HChar();

    void set(int w, int h, uint8 *data);

    int draw(int x, int y, uint8 color);

protected:
    int width_;
    int height_;
    bool *bits_;
};

class HFont {
public:
    HFont();
    ~HFont();

    void load();

    void drawText(int x, int y, const char *str, uint8 color);

protected:
    std::map<char, HChar> characters;
};

#endif

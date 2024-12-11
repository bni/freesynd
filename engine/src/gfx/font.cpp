/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
 *   Copyright (C) 2024  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "fs-engine/gfx/font.h"

#include <stdlib.h>

#include "fs-engine/gfx/screen.h"
#include "fs-engine/gfx/cp437.h"
#include "fs-utils/io/file.h"


/*! \brief
 *
 * \param valid_chars const std::string&
 *
 */
FontRange::FontRange(const std::string& valid_chars)
{
    // turn description valid_chars into bitfield char_present_
    memset(char_present_, 0, sizeof(char_present_));
    size_t pos = 0;
    while (pos < valid_chars.size()) {
        // skip commas if we are pointing to any.
        while (pos < valid_chars.size() && valid_chars[pos] == ',') pos++;

        size_t it = valid_chars.find(',', pos);
        size_t next_pos;
        if (it == std::string::npos) {
            // there is no next section.
            next_pos = valid_chars.size();
        } else {
            // there is a next section, and we'll point to
            // the comma on the start of the next loop.
            next_pos = it;
        }

        // each section can be a single number, or a range of numbers.
        // numbers can be in decimal or hex with the prefix 0x.
        std::string section(valid_chars, pos, next_pos - pos);

        it = section.find('-');
        if (it == std::string::npos) {
            // single value.
            char *endptr = NULL;
            unsigned char value = (uint8_t)strtol(section.c_str(), &endptr, 0);
            if (endptr == NULL || *endptr != '\0') {
                fprintf(stderr, "Invalid font range specified: %s\n", valid_chars.c_str());
                return;
            }
            char_present_[value / 32] |= 1 << (value % 32);
        } else {
            // range of values.
            char *endptr = NULL;
            unsigned char start = (uint8_t)strtol(section.c_str(), &endptr, 0);
            if (endptr == NULL || *endptr != '-') {
                fprintf(stderr, "Invalid font range specified: %s\n", valid_chars.c_str());
                return;
            }
            unsigned char end = (uint8_t)strtol(section.c_str() + it + 1, &endptr, 0);
            if (endptr == NULL || *endptr != '\0') {
                fprintf(stderr, "Invalid font range specified: %s\n", valid_chars.c_str());
                return;
            }
            unsigned char value;
            for (value = start; value <= end; value++) {
                char_present_[value / 32] |= 1 << (value % 32);
            }
        }

        pos = next_pos;
    }
}

/*!
 * Constructor for the Font class.
 * \param sprites SpriteManager*
 * \param offset int
 * \param base char
 * \param valid_chars const std::string&
 */
Font::Font(SpriteManager *sprites, int offset, char base, const std::string& valid_chars) : range_(valid_chars) {
    sprites_ = sprites;
    offset_ = offset - base;
    Sprite *pDef = getSpriteForCodepoint(0x0041);
    defaultHeight_ = pDef->height();
    defaultWidth_ = pDef->width();
}

/*!
 * Change the palette of the sprite texture and reload it.
 * @param newPalette The new palette to set
 * @return 
 */
bool Font::setPalette(const fs_eng::Palette &newPalette) {
    return sprites_->setPalette(newPalette);
}

/**
 * Return the sprite font for the given unicode codepoint
 * \param codePoint utf8::utfchar32_t The codepoint
 * \return Sprite* Null if no sprite was found
 *
 */
Sprite * Font::getSpriteForCodepoint(utf8::utfchar32_t codePoint) {
    // We look for the corresponding character in cp437 table
    if (codePoint > sizeof(unicodeToCp437)) {
        return nullptr; // out-of-range
    }

    cp437char_t charCp437 = unicodeToCp437[codePoint];

    if (!range_.in_range(charCp437)) {
        // use '?' as default character.
        if (range_.in_range('?')) {
            return sprites_->sprite('?' + offset_);
        } else {
            // NULL causes the missing glyph to be skipped.
            // no space will be consumed on-screen.
            return nullptr;
        }
    }
    return sprites_->sprite(charCp437 + offset_);
}

/**
 * Draw text with this font
 * \param x int X Coordinate on screen
 * \param y int Y Coordinate on screen
 * \param text const std::string The text to display. Must be a UTF-8 encoded string
 * \param x2 bool Option to multiply the size of the font by 2
 * \return void
 *
 */
void Font::drawText(int x, int y, const std::string& text, bool x2) {
    int sc = x2 ? 2 : 1;
    int ox = x;

    std::string newText(text);
    std::string::iterator b = newText.begin();
    std::string::iterator e = newText.end();
    while ( b != e ) {
        try {
            auto codePoint = utf8::next(b,e);

            if (codePoint == 0x0020) {
                // If char is a space, only move the drawing origin to the left
                x += defaultWidth_ * sc - sc;
                continue;
            }
            if (codePoint == 0x000A) {
                // If char is a line feed, only move the drawing origin to the next line
                x = ox;
                y += textHeight() - sc;
                continue;
            }

            Sprite *sprite = getSpriteForCodepoint(codePoint);
            if (sprite) {
                int y_offset = 0;
                if (codePoint == ':') {
                    y_offset = sc;
                } else if (codePoint == '.' || codePoint == ',') {
                    y_offset = 4 * sc;
                } else if (codePoint == '-') {
                    y_offset = 2 * sc;
                }

                sprites_->drawSprite(sprite->id(), x, y + y_offset, false, x2);

                x += sprite->width() * sc - sc;
            }
        } catch (const utf8::invalid_utf8& exc) {
            continue;
        }
    } //end of while
}

int Font::textWidth(const std::string& text, bool x2) {
    int sc = x2 ? 2 : 1;
    int x = 0;

    std::string newText(text);
    std::string::iterator b = newText.begin();
    std::string::iterator e = newText.end();
    while ( b != e ) {
        try {
            auto codePoint = utf8::next(b,e);

            if (codePoint == ' ') {
                x += defaultWidth_ * sc - sc;
                continue;
            }
            Sprite *s = getSpriteForCodepoint(codePoint);
            if (s) {
                x += s->width() * sc - sc;
            }
        } catch (const utf8::invalid_utf8& exc) {
            continue;
        }
    } //end of while

    return x;
}

int Font::textHeight(bool x2) {
    int sc = x2 ? 2 : 1;
    return defaultHeight_ * sc;
}

/*!
 * Returns true if given code point is printable with this font.
 * To be printable, the character must have an equivalent in the
 * code page 437 character set and be in the character range for the font.
 * \param codePoint utf8::utfchar32_t
 * \return bool True means the character is printable
 *
 */
bool Font::isPrintable(utf8::utfchar32_t codePoint) {
    // We look for the corresponding character in cp437 table
    if (codePoint > sizeof(unicodeToCp437)) {
        return false; // out-of-range
    }

    cp437char_t charCp437 = unicodeToCp437[codePoint];

    return range_.in_range(charCp437);
}

MenuFont::MenuFont(SpriteManager *sprites, int darkOffset, int lightOffset, char base,
            const std::string& valid_chars) : Font(sprites, darkOffset, base, valid_chars) {
    lightOffset_ = lightOffset - base;
}


/*!
 * Returns the sprite for the given codepoint and highlighted or not.
 * \param codePoint utf8::utfchar32_t
 * \param highlighted bool
 * \return Sprite* null if no sprite was found
 *
 */
Sprite *MenuFont::getSpriteForCodepoint(utf8::utfchar32_t codePoint, bool highlighted) {
    // We look for the corresponding character in cp437 table
    if (codePoint > sizeof(unicodeToCp437)) {
        return nullptr; // out-of-range
    }

    cp437char_t charCp437 = unicodeToCp437[codePoint];

    if (!range_.in_range(charCp437)) {
        // use '?' as default character.
        if (range_.in_range('?')) {
            return sprites_->sprite('?' + (highlighted ? lightOffset_ : offset_));
        } else {
            // NULL causes the missing glyph to be skipped.
            // no space will be consumed on-screen.
            return nullptr;
        }
    }
    return sprites_->sprite(charCp437 + (highlighted ? lightOffset_ : offset_));
}

/**
 * Draw text with this font
 * \param x int X Coordinate on screen
 * \param y int Y Coordinate on screen
 * \param text const std::string The text to display. Must be a UTF-8 encoded string
 * \param highlighted bool True to display the highlighted font
 * \param x2 bool Option to multiply the size of the font by 2
 * \return void
 */
void MenuFont::drawText(int x, int y, const std::string& text, bool highlighted, bool x2) {
    int sc = x2 ? 2 : 1;
    int ox = x;

    std::string newText(text);
    std::string::iterator b = newText.begin();
    std::string::iterator e = newText.end();
    while ( b != e ) {
        try {
            auto codePoint = utf8::next(b,e);

            if (codePoint == 0x0020) {
                // If char is a space, only move the drawing origin to the left
                x += defaultWidth_ * sc - sc;
                continue;
            }
            if (codePoint == 0x000A) {
                // If char is a line feed, only move the drawing origin to the next line
                x = ox;
                y += textHeight() - sc;
                continue;
            }

            Sprite *sprite = getSpriteForCodepoint(codePoint, highlighted);
            if (sprite) {
                int y_offset = 0;
                if (codePoint == ':') {
                    y_offset = sc;
                } else if (codePoint == '.' || codePoint == ',' || codePoint == '-' || codePoint == '_')
                    y_offset = defaultHeight_ *sc - sprite->height() * sc;
                else if (codePoint == '/') {
                    y_offset = (defaultHeight_ *sc)/2 - (sprite->height() * sc) / 2;
                }

                //sprite->draw(x, y + y_offset, 0, false, x2);
                sprites_->drawSprite(sprite->id(), x, y + y_offset, false, x2);

                x += sprite->width() * sc - sc;
            }
        } catch (const utf8::invalid_utf8& exc) {
            continue;
        }
    } //end of while
}

/*!
 * Constructor for the class.
 * \param sprites SpriteManager that holds the font sprites
 * \param offset The first sprite in the sprite set for this font
 * \param base
 * \param validChars List of characters that are displayable in this font
 */
GameFont::GameFont(SpriteManager *sprites, int offset, char base,
            const std::string& validChars) :Font(sprites, offset, base, validChars) {}

/*!
 * Draw text at the given position. Text will have the specified color.
 * \param x X location
 * \param y Y location
 * \param text The text to draw. It must be in UTF-8.
 * \param toColor The color used to draw the text.
 */
void GameFont::drawText(int x, int y, const std::string& text, fs_eng::FSColor toColor) {
    int sc = 1;
    int ox = x;
    uint8_t fromColor = 252;

    std::string newText(text);
    std::string::iterator b = newText.begin();
    std::string::iterator e = newText.end();
    while ( b != e ) {
        try {
            auto codePoint = utf8::next(b,e);

            if (codePoint == 0x0020) {
                // If char is a space, only move the drawing origin to the left
                x += defaultWidth_ * sc - sc;
                continue;
            }
            if (codePoint == 0x000A) {
                // If char is a line feed, only move the drawing origin to the next line
                x = ox;
                y += textHeight() - sc;
                continue;
            }

            // get the sprite for the character
            Sprite *sprite = getSpriteForCodepoint(codePoint);
            if (sprite) {
                int y_offset = 0;
                // Add some offset correct for special characters as ':' '.' ',' '-'
                if (codePoint == ':') {
                    y_offset = sc;
                } else if (codePoint == '.' || codePoint == ',' || codePoint == '-') {
                    y_offset = defaultHeight_ *sc - sprite->height() * sc;
                } else if (codePoint == '/') {
                    y_offset = (defaultHeight_ *sc)/2 - (sprite->height() * sc) / 2;
                }

                sprites_->setColorModulation(toColor);
                sprites_->drawSprite(sprite->id(), x, y + y_offset);

                x += sprite->width() * sc - sc;
            }
        } catch (const utf8::invalid_utf8& exc) {
            continue;
        }
    } //end of while
}

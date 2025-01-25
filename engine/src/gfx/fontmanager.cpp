/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>
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

#include "fs-engine/gfx/fontmanager.h"

#include <cassert>

#include "fs-utils/log/log.h"

namespace fs_eng {

FontManager::FontManager()
{
    for (int i = 0; i < 4; i++)
        menuFonts_[i] = NULL;

    pIntroFont_ = NULL;
    pGameFont_ = NULL;
}

FontManager::~FontManager()
{
    for (int i = 0; i < 4; i++) {
        delete menuFonts_[i];
    }

    if (pGameFont_)
        delete pGameFont_;
}

/*!
 * @brief 
 * @param pMenuSprites 
 * @param menuPalette 
 * @param loadIntroFont 
 * @throw InitializationFailedException if any problem when loading fonts
 */
void FontManager::loadFonts(SpriteManager *pMenuSprites, fs_eng::Palette &menuPalette, bool loadIntroFont) {
    assert(pMenuSprites);

    // Valid char : ' ,-/ A-Z backslash(Ox5c) ` 0x80-DCS
    menuFonts_[SIZE_4] = createMenuFontForSize(pMenuSprites, 1076, 939, 'A', "0x27,0x2c-0x2f,0x41-0x5a,0x5c,0x60,0x80-0x90,0x93-0x9a,0xa0-0xa7");
    menuFonts_[SIZE_3] = createMenuFontForSize(pMenuSprites, 802, 665, 'A', "0x21-0x5a,0x80-0x90,0x93-0x9a,0xa0-0xa8");
    menuFonts_[SIZE_2] = createMenuFontForSize(pMenuSprites, 528, 391, 'A', "0x21-0x60,0x80-0xa8");
    menuFonts_[SIZE_1] = createMenuFontForSize(pMenuSprites, 254, 117, 'A', "0x21-0x60,0x80-0xa8");

    createGameFont(menuPalette);

    if (loadIntroFont) {
        createIntroFont();
    }
}

/*!
 * Create Font used during the mission gameplay
 * @param menuPalette 
 * @throw InitializationFailedException if any problem when loading fonts
 */
void FontManager::createGameFont(fs_eng::Palette &menuPalette) {

    // loads intro sprites
    LOG(Log::k_FLG_GFX, "FontManager", "loadFonts", ("Creating Game Font ..."))

    pGameFontSprites_ = std::make_unique<SpriteManager>(true, SpriteManager::kMenuSpritesTextureWidth);
    // We use a custom palette to set the color used in font to white
    // so it's easier to use color modulation to change the color when displaying the font
    fs_eng::Palette gameFontPalette = menuPalette;
    gameFontPalette[252] = {254, 254, 254, 255};
    gameFontPalette[18] = {0, 0, 0, 0};
    pGameFontSprites_->loadSprites("mspr-0.tab", "mspr-0.dat", gameFontPalette);

    pGameFont_ = new GameFont(pGameFontSprites_.get(), 665, 'A', "0x21-0x5a,0x80-0x90,0x93-0x9a,0xa0-0xa8");
}

/*!
 * @brief 
 * @throw InitializationFailedException if any problem when loading fonts
 */
void FontManager::createIntroFont() {

    // loads intro sprites
    LOG(Log::k_FLG_GFX, "FontManager", "loadFonts", ("Creating Intro font ..."))

    pIntroFontSprites_ = std::make_unique<SpriteManager>(true, SpriteManager::kMenuSpritesTextureWidth);
    fs_eng::Palette emptyPalette;
    pIntroFontSprites_->loadSprites("mfnt-0.tab", "mfnt-0.dat", emptyPalette);

    pIntroFont_ = std::make_unique<Font>(pIntroFontSprites_.get(), 1, '!', "0x21-0x60,0x80-0xa8");
}

MenuFont *FontManager::createMenuFontForSize(SpriteManager *sprites,
                                             int darkOffset, int lightOffset, char base, const std::string &valid_chars)
{
    MenuFont * pFont = new MenuFont(sprites, darkOffset, lightOffset, base, valid_chars);

    return pFont;
}

}

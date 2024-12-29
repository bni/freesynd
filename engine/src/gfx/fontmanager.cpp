/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
 *   Copyright (C) 2011  Benoit Blancard <benblan@users.sourceforge.net>*
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

bool FontManager::loadFonts(SpriteManager *pMenuSprites, fs_eng::Palette &menuPalette, bool loadIntroFont) {
    assert(pMenuSprites);

    // Valid char : ' ,-/ A-Z backslash(Ox5c) ` 0x80-DCS
    menuFonts_[SIZE_4] = createMenuFontForSize(pMenuSprites, 1076, 939, 'A', "0x27,0x2c-0x2f,0x41-0x5a,0x5c,0x60,0x80-0x90,0x93-0x9a,0xa0-0xa7");
    menuFonts_[SIZE_3] = createMenuFontForSize(pMenuSprites, 802, 665, 'A', "0x21-0x5a,0x80-0x90,0x93-0x9a,0xa0-0xa8");
    menuFonts_[SIZE_2] = createMenuFontForSize(pMenuSprites, 528, 391, 'A', "0x21-0x60,0x80-0xa8");
    menuFonts_[SIZE_1] = createMenuFontForSize(pMenuSprites, 254, 117, 'A', "0x21-0x60,0x80-0xa8");

    if (!createGameFont(menuPalette)) {
        return false;
    }

    if (loadIntroFont) {
        if (!createIntroFont()) {
            return false;
        }
    }

    return true;
}

bool FontManager::createGameFont(fs_eng::Palette &menuPalette) {

    // loads intro sprites
    LOG(Log::k_FLG_GFX, "FontManager", "loadFonts", ("Creating Game Font ..."))

    pGameFontSprites_ = std::make_unique<SpriteManager>(true, SpriteManager::kMenuSpritesTextureWidth);
    // We use a custom palette to set the color used in font to white
    // so it's easier to use color modulation to change the color when displaying the font
    fs_eng::Palette gameFontPalette = menuPalette;
    gameFontPalette[252] = {254, 254, 254, 255};
    gameFontPalette[18] = {0, 0, 0, 0};
    if (!pGameFontSprites_->loadSprites("mspr-0.tab", "mspr-0.dat", gameFontPalette)) {
        return false;
    }

    pGameFont_ = new GameFont(pGameFontSprites_.get(), 665, 'A', "0x21-0x5a,0x80-0x90,0x93-0x9a,0xa0-0xa8");

    return true;
}

bool FontManager::createIntroFont() {

    // loads intro sprites
    LOG(Log::k_FLG_GFX, "FontManager", "loadFonts", ("Creating Intro font ..."))

    pIntroFontSprites_ = std::make_unique<SpriteManager>(true, SpriteManager::kMenuSpritesTextureWidth);
    fs_eng::Palette emptyPalette;
    if (!pIntroFontSprites_->loadSprites("mfnt-0.tab", "mfnt-0.dat", emptyPalette))
    {
        return false;
    }

    pIntroFont_ = std::make_unique<Font>(pIntroFontSprites_.get(), 1, '!', "0x21-0x60,0x80-0xa8");

    return true;
}

MenuFont *FontManager::createMenuFontForSize(SpriteManager *sprites,
                                             int darkOffset, int lightOffset, char base, const std::string &valid_chars)
{
    MenuFont * pFont = new MenuFont(sprites, darkOffset, lightOffset, base, valid_chars);

    return pFont;
}

}

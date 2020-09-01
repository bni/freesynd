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

#include <stdio.h>
#include <assert.h>
#include "fontmanager.h"

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

    if (pIntroFont_) {
        delete pIntroFont_;
    }
    if (pGameFont_)
        delete pGameFont_;
}

bool FontManager::loadFonts(SpriteManager *pMenuSprites, SpriteManager *pIntroFontSprites_) {
    assert(pMenuSprites);

    menuFonts_[SIZE_4] = createMenuFontForSize(pMenuSprites, FontManager::SIZE_4, 1076, 939, 'A', "0x27,0x2c-0x2f,0x41-0x5a,0x5c,0x60,0x80-0x90,0x93-0x9a,0xa0-0xa7");
    menuFonts_[SIZE_3] = createMenuFontForSize(pMenuSprites, FontManager::SIZE_3, 802, 665, 'A', "0x21-0x5a,0x80-0x90,0x93-0x9a,0xa0-0xa8");
    menuFonts_[SIZE_2] = createMenuFontForSize(pMenuSprites, FontManager::SIZE_2, 528, 391, 'A', "0x21-0x60,0x80-0xa8");
    menuFonts_[SIZE_1] = createMenuFontForSize(pMenuSprites, FontManager::SIZE_1, 254, 117, 'A', "0x21-0x60,0x80-0xa8");

    pGameFont_ = new GameFont();
    pGameFont_->setSpriteManager(pMenuSprites, 665, 'A', "0x21-0x5a,0x80-0x90,0x93-0x9a,0xa0-0xa8");

    if (pIntroFontSprites_) {
        pIntroFont_ = new Font();
        pIntroFont_->setSpriteManager(pIntroFontSprites_, 1, '!', "0x21-0x60,0x80-0xa8");
    }

    return true;
}

MenuFont * FontManager::createMenuFontForSize(SpriteManager * sprites, EFontSize size,
                           int darkOffset, int lightOffset, char base, const std::string& valid_chars)
{
    MenuFont * pFont = new MenuFont();
    pFont->setSpriteManager(sprites, darkOffset, lightOffset, base, valid_chars);

    return pFont;
}


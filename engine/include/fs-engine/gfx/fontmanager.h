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

#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "fs-utils/common.h"
#include "fs-engine/gfx/font.h"

namespace fs_eng {

class SpriteManager;

/*!
 * Manager for all fonts used in the application.
 */
class FontManager {
public:
    /*! Size of font : 1 is the smaller.*/
    enum EFontSize {
        SIZE_1 = 0,  // smallest font
        SIZE_2 = 1,  // Regular buttons and labels
        SIZE_3 = 2,  // Big buttons in Main Menu
        SIZE_4 = 3   // For Menu titles
    };

    FontManager();
    ~FontManager();

    //! Creates all fonts
    void loadFonts(SpriteManager *pMenuSprites, fs_eng::Palette &menuPalette, bool loadIntroFont);

    /*!
     * Returns the font used in menus.
     * \param size Size of the font
     */
    MenuFont * getMenuFont(FontManager::EFontSize size) { return menuFonts_[size]; }

    /*!
     * Returns the font used in the gameplay menu.
     */
    GameFont *gameFont() {
        return pGameFont_;
    }

    /*!
     * Returns the font used in the intro animation.
     */
    Font * introFont() {
        return pIntroFont_.get();
    }

private:
    //! Create a menu font for the given size
    MenuFont * createMenuFontForSize(SpriteManager *sprites, int darkOffset, int lightOffset,
            char base, const std::string& valid_chars);

    void createGameFont(fs_eng::Palette &menuPalette);
    void createIntroFont();

private:
    /*!
     * Menu fonts have different sizes.
     */
    MenuFont * menuFonts_[4];
    /*! Sprite manager for intro font. */
    std::unique_ptr<SpriteManager> pGameFontSprites_;
    // TODO : use unique pointer
    GameFont *pGameFont_;
    std::unique_ptr<Font> pIntroFont_;
    /*! Sprite manager for intro font. */
    std::unique_ptr<SpriteManager> pIntroFontSprites_;
};

}

#endif

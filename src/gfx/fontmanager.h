/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
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

#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "common.h"
#include "font.h"

class SpriteManager;

/*!
 * Manager for all fonts used in the application.
 */
class FontManager {
public:
    /*! Size of font : 1 is the smaller.*/
    enum EFontSize {
        SIZE_1 = 0,
        SIZE_2 = 1,
        SIZE_3 = 2,
        SIZE_4 = 3
    };

    FontManager();
    ~FontManager();

    //! Creates all fonts
    bool loadFonts(SpriteManager *pMenuSprites, SpriteManager *pIntroFontSprites_);

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
        return pIntroFont_;
    }

protected:
    //! Create a menu font for the given size
    MenuFont * createMenuFontForSize(SpriteManager *sprites, EFontSize size, int darkOffset, int lightOffset,
            char base, const std::string& valid_chars);

protected:
    /*!
     * Menu fonts have different sizes.
     */
    MenuFont * menuFonts_[4];
    GameFont *pGameFont_;
    Font *pIntroFont_;
};

#endif

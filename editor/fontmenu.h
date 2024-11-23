/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2013  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef EDITOR_FONTMENU_H_
#define EDITOR_FONTMENU_H_

#include "fs-engine/menus/menu.h"
/*!
 * Font menu.
 */
class FontMenu : public Menu {
public:
    FontMenu(MenuManager *m);

    void handleShow();
    void handleRender(DirtyList &dirtyList) override;
    void handleLeave();

    void handleAction([[maybe_unused]] const int actionId, [[maybe_unused]] void *ctx) override;

private:
    void displayObjects();
    void displayFonts();

    int  displaySprite(Point2D startAt, int fromId, int range);

    void displaySpriteset();

private:
    static const size_t kSpriteMaxObjects;

    FSColor defaultColor_;
    int panelId_;
    int idButtonSprites_;
    int idButtonFonts_;
    int idButtonSpriteset_;
};

#endif // EDITOR_FONTMENU_H_

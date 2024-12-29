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

#ifndef MENUS_GAMEMENUFACTORY_
#define MENUS_GAMEMENUFACTORY_

#include "fs-utils/common.h"
#include "fs-engine/menus/menumanager.h"

/*!
 * Menu factory for the Game.
 */
class GameMenuFactory : public fs_eng::MenuFactory {
protected:
    /*!
     * Instanciate menus for the Game.
     * @param menuId A menu ID
     * @return null if menu ID is unknown
     */
    fs_eng::Menu * createCustomMenu(const int menuId) override;

    //! Create an instance of a FliMenu to display the intro and game title
    fs_eng::Menu * createFliMenu(const int menuId);

    //! Return the name of the animation to play before showing a menu
    const char* getShowAnimation(int menuId) override;

    //! Return the name of the animation to play after leaving a menu
    const char* getLeaveAnimation(int menuId) override;
};

#endif // MENUS_GAMEMENUFACTORY_

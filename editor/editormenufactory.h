/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2013, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef EDITOR_EDITORMENUFACTORY_H_
#define EDITOR_EDITORMENUFACTORY_H_

#include "fs-utils/common.h"
#include "fs-engine/menus/menumanager.h"

/*!
 * Creates menus for the editor application.
 */
class EditorMenuFactory : public fs_eng::MenuFactory {
protected:
    /*!
     * Instanciate menus for the Editor.
     * @param menuId A menu ID
     * @return null if menu ID is unknown
     */
    fs_eng::Menu * createCustomMenu(const int menuId);

    //! Return the name of the animation to play before showing a menu
    const char* getShowAnimation(int menuId) override;

    //! Return the name of the animation to play after leaving a menu
    const char* getLeaveAnimation(int menuId) override;
};

#endif // EDITOR_EDITORMENUFACTORY_H_

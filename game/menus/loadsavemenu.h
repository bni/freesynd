/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2011, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef LOADSAVEMENU_H
#define LOADSAVEMENU_H

#include "fs-engine/menus/menumanager.h"

/*!
 * Load/Save Menu class.
 */
class LoadSaveMenu : public fs_eng::Menu {
public:
    LoadSaveMenu(fs_eng::MenuManager *m);

    bool handleBeforeShow() override;
    void handleLeave() override;

    void handleAction(const int actionId, void *ctx) override;

    bool handleMouseDown(Point2D point, int button) override;

protected:
    static const int X_ORIGIN;
    static const int Y_ORIGIN;
    static const size_t NAME_MAX_SIZE;

    /*! Id of the load button.*/
    int loadButId_;
    /*! Id of the save button.*/
    int saveButId_;
    /*! The id of the line currently being edited. -1 if no line selected.*/
    short editNameId_;
    /*! There are 10 text fields in the menu to enter file names.*/
    fs_eng::TextField *pTextFields_[10];
};

#endif

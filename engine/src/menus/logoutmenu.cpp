/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-engine/menus/logoutmenu.h"

#include "fs-engine/events/event.h"
#include "fs-engine/events/default_events.h"
#include "fs-engine/enginecommon.h"

namespace fs_eng {
LogoutMenu::LogoutMenu(MenuManager * m):Menu(m, kMenuIdLogout, kMenuIdNoMenu, true),
timerLogout_(2000)
{
    isCachable_ = false;
    addStatic(0, 180, fs_eng::kScreenWidth, "#LGOUT_TITLE", FontManager::SIZE_4, true);
}

bool LogoutMenu::handleTick(uint32_t elapsed)
{
    if (timerLogout_.update(elapsed))
        EventManager::fire<QuitEvent>(0);

    return true;
}

}


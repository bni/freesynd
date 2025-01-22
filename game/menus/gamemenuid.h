/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2013, 2024-2025 Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef MENUS_GAMEMENUID_H_
#define MENUS_GAMEMENUID_H_

namespace fs_game_menus {
    static const int kMenuIdMain = 0;
    static const int kMenuIdBrief = 1;
    static const int kMenuIdConf = 2;
    static const int kMenuIdDebrief = 3;
    static const int kMenuIdGameplay = 4;
    static const int kMenuIdLoading = 5;
    static const int kMenuIdResearch = 7;
    static const int kMenuIdSelect = 8;
    static const int kMenuIdLdSave = 9;
    static const int kMenuIdMap = 10;
    static const int kMenuIdFliSuccess = 11;
    static const int kMenuIdFliFailedMission = 12;
    static const int kMenuIdFliTitle = 13;
    static const int kMenuIdFliIntro = 14;
    static const int kMenuIdFliGameLost = 15;
    static const int kMenuIdFliMissionAborted = 16;
};

#endif  // MENUS_GAMEMENUID_H_
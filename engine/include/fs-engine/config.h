/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
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

#ifndef CONFIG_H
#define CONFIG_H

// Set this to enable speed measurement of code execution
// 0 > disable
// 1 > map renderer
#define EXEC_SPEED_TIME 0

#if EXEC_SPEED_TIME == 1
#define DEBUG_SPEED_INIT int start_mesure_ticks = SDL_GetTicks();
#define DEBUG_SPEED_LOG(module) printf("%s - speed : %i\n", module, SDL_GetTicks() - start_mesure_ticks);
#else
#define DEBUG_SPEED_INIT
#define DEBUG_SPEED_LOG(module)
#endif

#endif

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
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

#ifndef CONFIG_H
#define CONFIG_H

// Define this to enable sound effects and music
//#undef HAVE_SDL_MIXER
#define HAVE_SDL_MIXER          1

#define USE_INTRO_OGG           1
#define USE_ASSASSINATE_OGG     1

// Define this to display frame rate during gameplay
//define TRACK_FPS  1

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

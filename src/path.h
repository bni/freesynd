/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#ifndef PATH_H
#define PATH_H

#include "common.h"
#include "pathsurfaces.h"

/*!
 * This structure is used to store movement informations
 * while ped is moving to a direction.
 * Used by moveTodir().
 */
struct DirMoveType {
        int32 dir_orig;
        int32 dir_last;
        int32 dir_closest;
        int32 dir_closer;
        int32 dir_modifier;
        int32 modifier_value;
        // directional movement only
        // to decide whether to continue movement by changing
        // direction or not
        bool bounce;
        // walkn only on "safe" tiles
        bool safe_walk;
        // when closest is used and first movement is successful
        bool on_new_tile;
        void clear() {
            dir_last = -1;
            dir_modifier = 0;
            modifier_value = 0;
            bounce = false;
            on_new_tile = false;
            safe_walk = true;
        }
    };

#endif

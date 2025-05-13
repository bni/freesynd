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

#ifndef MOD_H
#define MOD_H

#include <string>

namespace fs_knl {
/*!
 * Mod class.
 */
class Mod {
public:
    /*!
     * Different types of Mod.
     */
    enum EModType {
        MOD_LEGS = 0,
        MOD_ARMS = 1,
        MOD_CHEST = 2,
        MOD_HEART = 3,
        MOD_EYES = 4,
        MOD_BRAIN = 5,
        Unknown = 6
    } ;

    enum EModVersion {
        MOD_V1 = 0,
        MOD_V2 = 1,
        MOD_V3 = 2
    };

    Mod(const std::string& mod_name, EModType type, EModVersion version, int mod_cost, const std::string& mod_desc,
            int mod_icon, int mod_iconF = 0);

    const char *getName() { return name_.c_str(); }
    //! Return modification type
    EModType getType() { return type_; }
    //! Return modification version
    EModVersion getVersion() { return ver_; }
    int cost() { return cost_; }
    const char *desc() { return desc_.c_str(); }
    int icon(bool isMale) { return isMale ? icon_ : icon_f_; }

protected:
    std::string name_, desc_;
    /*! Type of modification.*/
    EModType type_;
    /*! Version of modification.*/
    EModVersion ver_;
    int cost_, icon_, icon_f_;
};

};

#endif

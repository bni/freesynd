/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2007  Davor Ocelic <docelic@mail.inet.hr>            *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2017  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef MODEL_DAMAGE_H_
#define MODEL_DAMAGE_H_

#include "fs-kernel/model/position.h"

class ShootableMapObject;
class WeaponInstance;

namespace fs_dmg {
    /*!
     * These are all types of damage caused by weapons.
     */
    enum DamageType {
        kDmgTypeNone = 0x0000,
        kDmgTypeBullet = 0x0001,
        kDmgTypeLaser = 0x0002,
        kDmgTypeBurn = 0x0004,
        kDmgTypeExplosion = 0x0008,
        kDmgTypeCollision = 0x0010, // By car or door
        kDmgTypePersuasion = 0x0020,
    };

    /*!
     * This structure holds informations on the damage inflicted to a ShootableMapObject.
     */
    struct DamageToInflict {
        //! The type of damage
        DamageType dtype;
        //! Range of damage
        double range;
        //! The value of the damage
        int dvalue;
        //! direction damage comes from, should be angle 256 degree based
        int ddir;
        //! Location of aimed point
        WorldPoint aimedLocW;
        //! Location of origin of shot
        WorldPoint originLocW;
        //! The object that inflicted the damage
        ShootableMapObject * d_owner;
        //! The weapon that generated this damage
        WeaponInstance *pWeapon;
    };
};
#endif // MODEL_DAMAGE_H_

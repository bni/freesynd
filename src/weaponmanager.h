/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2011  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
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

#ifndef WEAPONMANAGER_H
#define WEAPONMANAGER_H

#include <vector>
#include <fstream>

#include "common.h"
#include "model/weapon.h"
#include "utils/seqmodel.h"
#include "utils/portablefile.h"

/*!
 * Weapon manager class.
 */
class WeaponManager {
public:
    //! Constructor
    WeaponManager();
    //! Destructor
    ~WeaponManager();
    //! Resources destruction
    void destroy();
    //! Reset data
    void reset();
    //! Cheating mode to enable all weapons
    void cheatEnableAllWeapons();
    //! Enable weapon of given type
    void enableWeapon(Weapon::WeaponType wt);
    //! Returns the list of currently available weapons
    SequenceModel * getAvailableWeapons() { return &availableWeapons_; }
    //! Returns a weapon of given type whether it is available or not
    Weapon *getWeapon(Weapon::WeaponType wt);
    //! Returns true is the given weapon is available for agents
    bool isAvailable(Weapon *pWeapon);

    //! Returns pointer if required weapon type is availiable
    Weapon * getAvailable(Weapon::WeaponType wpn);
    //! Creates a list of available weapons of required damage
    void getAvailable(uint32 dmg_type, std::vector <Weapon *> &wpns);

    //! Save instance to file
    bool saveToFile(PortableFile &file);
    //! Load instance from file
    bool loadFromFile(PortableFile &infile, const FormatVersion& v);

    //! checks existing weapons that can do such damage and sets whether they can shoot
    //! strict check
    bool checkDmgTypeCanShootStrict(uint32 dmg, bool &can_shoot);
    //! checks existing weapons that can do such damage and sets whether they can shoot
    //! non strict check
    bool checkDmgTypeCanShootNonStrict(uint32 dmg, bool &can_shoot);
protected:
    //! Loads the weapon from file
    Weapon *loadWeapon(Weapon::WeaponType wt);

protected:
    std::vector<Weapon *> all_game_weapons_;
    /*! This vector is used to store necessary but unavailable weapons until they
     * are made available.*/
    std::vector<Weapon *> preFetch_;
    /*! This is the list of all weapons available to the user.*/
    VectorModel<Weapon *> availableWeapons_;
};

#endif

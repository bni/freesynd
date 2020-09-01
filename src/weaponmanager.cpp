/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#include <stdio.h>
#include <assert.h>

#include "app.h"
#include "utils/log.h"
#include "utils/file.h"
#include "utils/configfile.h"

WeaponManager::WeaponManager() {
    all_game_weapons_.reserve(18);
}

void WeaponManager::destroy() {
    preFetch_.clear();
    availableWeapons_.clear();
}

WeaponManager::~WeaponManager() {
    destroy();
    for (std::vector<Weapon *>::iterator it = all_game_weapons_.begin();
        it != all_game_weapons_.end();it++)
    {
        delete (*it);
    }
    all_game_weapons_.clear();

}

void WeaponManager::reset() {
    destroy();

    all_game_weapons_.clear();
    all_game_weapons_.push_back(loadWeapon(Weapon::Persuadatron));
    all_game_weapons_.push_back(loadWeapon(Weapon::Pistol));
    all_game_weapons_.push_back(loadWeapon(Weapon::Shotgun));
    all_game_weapons_.push_back(loadWeapon(Weapon::Uzi));
    all_game_weapons_.push_back(loadWeapon(Weapon::Scanner));
    all_game_weapons_.push_back(loadWeapon(Weapon::MediKit));
    all_game_weapons_.push_back(loadWeapon(Weapon::Minigun));
    all_game_weapons_.push_back(loadWeapon(Weapon::Flamer));
    all_game_weapons_.push_back(loadWeapon(Weapon::LongRange));
    all_game_weapons_.push_back(loadWeapon(Weapon::EnergyShield));
    all_game_weapons_.push_back(loadWeapon(Weapon::Laser));
    all_game_weapons_.push_back(loadWeapon(Weapon::GaussGun));
    all_game_weapons_.push_back(loadWeapon(Weapon::AccessCard));
    all_game_weapons_.push_back(loadWeapon(Weapon::TimeBomb));

    // Enables default weapons
    enableWeapon(Weapon::Persuadatron);
    enableWeapon(Weapon::Pistol);
    enableWeapon(Weapon::Shotgun);
    enableWeapon(Weapon::Scanner);
    enableWeapon(Weapon::MediKit);
}

void WeaponManager::cheatEnableAllWeapons() {
    enableWeapon(Weapon::Persuadatron);
    enableWeapon(Weapon::Pistol);
    enableWeapon(Weapon::Shotgun);
    enableWeapon(Weapon::Uzi);
    enableWeapon(Weapon::Scanner);
    enableWeapon(Weapon::MediKit);
    enableWeapon(Weapon::Minigun);
    enableWeapon(Weapon::Flamer);
    enableWeapon(Weapon::LongRange);
    enableWeapon(Weapon::EnergyShield);
    enableWeapon(Weapon::Laser);
    enableWeapon(Weapon::GaussGun);
    enableWeapon(Weapon::AccessCard);
    enableWeapon(Weapon::TimeBomb);
}

void WeaponManager::enableWeapon(Weapon::WeaponType wt) {
    // First check if weapon is not already available
    for (unsigned i = 0; i != availableWeapons_.size(); ++i) {
        if (wt == availableWeapons_.get(i)->getType())
            return;
    }

    // Then get weapon
    Weapon *pWeapon = getWeapon(wt);

    // removes it from cache
    for (std::vector < Weapon * >::iterator it = preFetch_.begin();
         it != preFetch_.end(); it++) {
             if (pWeapon == *it) {
                preFetch_.erase(it);
                break;
             }
    }

    // make it available
    for (unsigned i = 0; i != availableWeapons_.size(); ++i) {
        if (pWeapon->getType() < availableWeapons_.get(i)->getType()) {
            // The new weapon is inserted in the order of the WeaponType constants
            availableWeapons_.insertAt(i, pWeapon);
            return;
        }
    }
    // If we're here, it's because the new weapon comes at the end of the list
    availableWeapons_.add(pWeapon);
}

Weapon * WeaponManager::getWeapon(Weapon::WeaponType wt) {
    // Search in prefetched weapons first
    for (unsigned int i = 0; i < preFetch_.size(); i++) {
        if (wt == preFetch_[i]->getType())
            return preFetch_[i];
    }

    // Then search in available weapons
    for (unsigned i = 0; i != availableWeapons_.size(); ++i) {
        if (wt == availableWeapons_.get(i)->getType())
            return availableWeapons_.get(i);
    }

    // Weapon was not found so loads it
    Weapon *pWeapon = NULL;

    for (std::vector<Weapon *>::iterator it = all_game_weapons_.begin();
        it != all_game_weapons_.end();it++)
    {
        if ((*it)->getType() == wt) {
            pWeapon = *it;
            break;
        }
    }
    // Stores it in cache
    preFetch_.push_back(pWeapon);

    return pWeapon;
}

bool WeaponManager::isAvailable(Weapon *pWeapon) {
    // search in available weapons
    for (unsigned i = 0; i != availableWeapons_.size(); ++i) {
        if (pWeapon->getType() == availableWeapons_.get(i)->getType())
            return true;
    }

    // No weapon of that type has been found
    return false;
}

Weapon * WeaponManager::getAvailable(Weapon::WeaponType wpn) {
    // search in available weapons
    for (unsigned i = 0; i != availableWeapons_.size(); ++i) {
        if (wpn == availableWeapons_.get(i)->getType())
            return availableWeapons_.get(i);
    }

    // No weapon of that type has been found
    return NULL;
}

void WeaponManager::getAvailable(uint32 dmg_type, std::vector <Weapon *> &wpns)
{
    // search in available weapons
    for (unsigned i = 0; i != availableWeapons_.size(); ++i) {
        if ((availableWeapons_.get(i)->dmgType() & dmg_type) != 0)
            wpns.push_back(availableWeapons_.get(i));
    }
}

Weapon * WeaponManager::loadWeapon(Weapon::WeaponType wt) {
    /* NOTE: small icon 27 exists and looks like an N with an arrow above it.
       the corresponding large icon is actually the "all" button on the
       select menu.  It would appear Bullfrog was going to have another
       weapon here but it got scrapped early on and they used the large
       icon space to implement the all button.
     */
    std::string filename(File::dataFullPath("ref/weapons.dat"));
    try {
        // Loads configuration file
        ConfigFile conf(filename);
        // TODO:calibrate weapon time for shot and reload, angle, accuracy
        return new Weapon(wt, conf);
    } catch (...) {
        FSERR(Log::k_FLG_IO, "WeaponManager", "loadWeapon", ("File %s cannot be read", filename.c_str()))
        return NULL;
    }
}

bool WeaponManager::saveToFile(PortableFile &file) {
    file.write32(availableWeapons_.size());

    for(unsigned int i=0; i<availableWeapons_.size(); i++) {
        Weapon *pWeapon = availableWeapons_.get(i);
        file.write32(pWeapon->getType());
    }

    return true;
}

bool WeaponManager::loadFromFile(PortableFile &infile, const FormatVersion& v) {
    int nbWeap = infile.read32();

    for (int i = 0; i < nbWeap; i++) {
        int type = infile.read32();
        Weapon::WeaponType wt = Weapon::Unknown;
        switch (type) {
            case Weapon::Persuadatron:
                wt = Weapon::Persuadatron;
                break;
            case Weapon::Pistol:
                wt = Weapon::Pistol;
                break;
            case Weapon::GaussGun:
                wt = Weapon::GaussGun;
                break;
            case Weapon::Shotgun:
                wt = Weapon::Shotgun;
                break;
            case Weapon::Uzi:
                wt = Weapon::Uzi;
                break;
            case Weapon::Minigun:
                wt = Weapon::Minigun;
                break;
            case Weapon::Laser:
                wt = Weapon::Laser;
                break;
            case Weapon::Flamer:
                wt = Weapon::Flamer;
                break;
            case Weapon::LongRange:
                wt = Weapon::LongRange;
                break;
            case Weapon::Scanner:
                wt = Weapon::Scanner;
                break;
            case Weapon::MediKit:
                wt = Weapon::MediKit;
                break;
            case Weapon::TimeBomb:
                wt = Weapon::TimeBomb;
                break;
            case Weapon::AccessCard:
                wt = Weapon::AccessCard;
                break;
            case Weapon::EnergyShield:
                wt = Weapon::EnergyShield;
                break;
            default:
                wt = Weapon::Unknown;
        }
        // enable weapon
        enableWeapon(wt);
    }

    return true;
}

bool WeaponManager::checkDmgTypeCanShootStrict(uint32 dmg, bool &can_shoot)
{
    bool found = false;
    can_shoot = false;

    for (std::vector<Weapon *>::iterator it = all_game_weapons_.begin();
        it != all_game_weapons_.end();it++)
    {
        if ((*it)->dmgType() == dmg) {
            found = true;
            if (((*it)->shotProperty() & Weapon::spe_CanShoot) != 0)
                can_shoot = true;
        }
    }

    return found;
}

bool WeaponManager::checkDmgTypeCanShootNonStrict(uint32 dmg, bool &can_shoot)
{
    bool found = false;
    can_shoot = false;

    for (std::vector<Weapon *>::iterator it = all_game_weapons_.begin();
        it != all_game_weapons_.end();it++)
    {
        if (((*it)->dmgType() & dmg) != 0) {
            found = true;
            if ((*it)->canShoot())
                can_shoot = true;
        }
    }

    return found;

}


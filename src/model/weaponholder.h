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

#ifndef WEAPON_HOLDER_H
#define WEAPON_HOLDER_H

#include <vector>

#include "weapon.h"

/*!
 * A weapon holder is an object that can hold weapons.
 * Creation/destruction of weapons holds by the WeaponHolder is
 * not his responsibility.
 */
class WeaponHolder {
public:
    //! This constat indicates that there is no weapon selected.
    static const int kNoWeaponSelected;
    /*! Defines the maximum number of weapons an agent can carry.*/
    static const uint8 kMaxHoldedWeapons;

    WeaponHolder();
    virtual ~WeaponHolder();

    uint8 numWeapons() { return weapons_.size(); }

    WeaponInstance *weapon(uint8 n) {
        assert(n < weapons_.size());
        return weapons_[n];
    }

    void addWeapon(WeaponInstance *w);

    //! Removes the weapon from the inventory at the given index.
    WeaponInstance *removeWeaponAtIndex(uint8 n);

    //! Removes the given weapon from the inventory.
    void removeWeapon(WeaponInstance *w);
    //! Removes and delete all weapons in the inventory
    void destroyAllWeapons();

    //! Selects the weapon at given index in the inventory
    void selectWeapon(uint8 n);
    //! Selects the weapon in the inventory
    void selectWeapon(const WeaponInstance &weaponToSelect);
    //! Deselects a selected weapon if any
    WeaponInstance * deselectWeapon();

    //! Returns the currently used weapon or null if no weapon is used
    WeaponInstance *selectedWeapon() {
        return selected_weapon_ >= 0
                && selected_weapon_ < (int) weapons_.size()
            ? weapons_[selected_weapon_] : NULL;
    }

    //! Select any shooting weapon with ammo
    void selectShootingWeaponWithAmmo();
    //! Select a shooting weapon of same type or another type if there is no of first type
    void selectShootingWeaponWithSameTypeFirst(WeaponInstance *pLeaderWeapon);
    void selectMedikitOrShield(Weapon::WeaponType weaponType);

protected:
    struct WeaponSelectCriteria {
        union {
            //! weapon index from weapons_ in mission_
            uint32 indx;
            //! use only this weapon for attack
            WeaponInstance *wi;
            //! use only this type of weapon
            Weapon::WeaponType wpn_type;
            //! use weapon that inflicts this type of damage
            //! MapObject::DamageType
            uint32 dmg_type;
        } criteria;

        enum CriteriaType {
            kCritPointer = 2,           // wi
            kCritWeaponType = 3,        // wpn_type
            kCritDamageStrict = 4,      // type == dmg_type
            kCritDamageNonStrict = 5,   // type & dmg_type != 0
            kCritPlayerSelection = 6,   // Manage selection from weapon selector
            kCritLoadedShoot = 7        // select weapon who can shoot and has ammo
        };
        //! Union descriptor
        CriteriaType desc;
        //! Search weapon based on the rank attribute
        bool use_ranks;
    };

    /*!
     * Called before a weapon is selected to check if weapon can be selected.
     * \param wi The weapon to select
     */
    virtual bool canSelectWeapon(WeaponInstance *pNewWeapon) { return true;}
    /*!
     * Called when a weapon has been deselected.
     * \param wi The deselected weapon
     */
    virtual void handleWeaponDeselected(WeaponInstance * wi) {}
    /*!
     * Called when a weapon has been selected.
     * \param wi The selected weapon
     * \param previousWeapon The previous selected weapon (can be null if no weapon was selected)
     */
    virtual void handleWeaponSelected(WeaponInstance * wi, WeaponInstance * previousWeapon) {}

    //! Selects a weapon based on the given criteria
    bool selectRequiredWeapon(const WeaponSelectCriteria &criteria);
protected:
    /*!
     * The list of weapons carried by the holder.
     */
    std::vector<WeaponInstance *> weapons_;
    /*!
     * The currently selected weapon inside the inventory.
     */
    int selected_weapon_;
    /*!
     * On automatic weapon selection, weapon will be selected upon
     * this criteria.
     */
    WeaponSelectCriteria prefered_weapon_;
};

#endif

/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> 
 *   Copyright (C) 2013, 2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef WEAPON_HOLDER_H
#define WEAPON_HOLDER_H

#include <vector>

#include "fs-kernel/model/weapon.h"

/*!
 * A weapon holder is an object that can hold weapons.
 * Creation/destruction of weapons holds by the WeaponHolder is
 * not his responsibility.
 */
class WeaponHolder {
public:
    /*! Defines the maximum number of weapons an agent can carry.*/
    static const uint8 kMaxHoldedWeapons;

    WeaponHolder();
    virtual ~WeaponHolder();

    size_t numWeapons() { return weapons_.size(); }

    WeaponInstance *weapon(size_t n) {
        assert(n < weapons_.size());
        return weapons_[n];
    }

    void addWeapon(WeaponInstance *w);

    //! Removes the weapon from the inventory at the given index.
    WeaponInstance *removeWeaponAtIndex(size_t n);

    //! Removes the given weapon from the inventory.
    void removeWeapon(WeaponInstance *w);

    //! Selects the weapon at given index in the inventory
    void selectWeapon(size_t n);
    //! Selects the weapon in the inventory
    void selectWeapon(WeaponInstance *pWeaponToSelect);
    //! Deselects a selected weapon if any
    WeaponInstance * deselectWeapon();

    //! Returns the currently used weapon or null if no weapon is used
    WeaponInstance *selectedWeapon() {
        return selectedWeapon_;
    }
    //! Return true if there is a weapon selected
    bool isAnyWeaponSelected() { return selectedWeapon_ != nullptr; }
    //! Return true if the given weapon is currently selected
    bool isWeaponSelected(WeaponInstance *pWeapon);

    //! Select any shooting weapon with ammo
    void selectShootingWeaponWithAmmo();
    //! Select a shooting weapon of same type or another type if there is no of first type
    void selectShootingWeaponWithSameTypeFirst(WeaponInstance *pLeaderWeapon);
    void selectMedikitOrShield(Weapon::WeaponType weaponType);

    void transferWeapons(WeaponHolder &anotherHolder);

protected:
    struct WeaponSelectCriteria {
        union {
            //! weapon index from weapons_ in mission_
            uint32_t indx;
            //! use only this weapon for attack
            WeaponInstance *wi;
            //! use only this type of weapon
            Weapon::WeaponType wpn_type;
            //! use weapon that inflicts this type of damage
            //! MapObject::DamageType
            uint32_t dmg_type;
        } criteria;

        enum CriteriaType {
            //! Search weapon based on a given type
            kCritWeaponType = 3,
            //! Search weapon that inflicts strict damage
            kCritDamageStrict = 4,
            //! Search weapon that inflicts non strict damage
            kCritDamageNonStrict = 5,
            //! Search weapon that has the same type as the current selection
            kCritPlayerSelection = 6,
            //! Search weapon who can shoot and has ammo
            kCritLoadedShoot = 7
        };
        //! Union descriptor
        CriteriaType desc;
        //! Select the final weapon based on the rank attribute
        bool use_ranks;
    };

    /*!
     * Called before a weapon is selected to check if weapon can be selected.
     * \param wi The weapon to select
     */
    virtual bool canSelectWeapon(WeaponInstance *pNewWeapon) { return pNewWeapon->isSelectable();}
    /*!
     * Called when a weapon has been deselected.
     * \param wi The deselected weapon
     */
    virtual void handleWeaponDeselected([[maybe_unused]] WeaponInstance * wi) {}
    /*!
     * Called when a weapon has been selected.
     * \param wi The selected weapon
     * \param previousWeapon The previous selected weapon (can be null if no weapon was selected)
     */
    virtual void handleWeaponSelected([[maybe_unused]] WeaponInstance * wi, [[maybe_unused]] WeaponInstance * previousWeapon) {}

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
    WeaponInstance * selectedWeapon_;
    /*!
     * On automatic weapon selection, weapon will be selected upon
     * this criteria.
     */
    WeaponSelectCriteria prefered_weapon_;
};

#endif

/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net>
 *   Copyright (C) 2013, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-kernel/model/weaponholder.h"

namespace fs_knl {
//*************************************
// Constant definition
//*************************************
const uint8_t WeaponHolder::kMaxHoldedWeapons = 8;

WeaponHolder::WeaponHolder() {
    // Starts with no weapon selected
    selectedWeapon_ = nullptr;
}

WeaponHolder::~WeaponHolder() {
    selectedWeapon_ = nullptr;
    weapons_.clear();
}

/*!
 * Adds the givent weapon to the inventory.
 * Weapon is placed at the end of the inventory.
 * When a weapon is in inventory, it is not rendered on screen.
 * \param w The weapon to add
 */
void WeaponHolder::addWeapon(WeaponInstance *w) {
    if (w && weapons_.size() < kMaxHoldedWeapons) {
        auto it = std::find(weapons_.begin(), weapons_.end(), w);
        if (it == weapons_.end()) {
            w->setDrawable(false);
            weapons_.push_back(w);
        }
    }
}

/*!
 * Removes the weapon at the given index in the inventory.
 * Caller is responsible for freeing the returned value.
 * \param n The index. Must be less than the total number of weapons.
 * \return The removed weapon or NULL if not found.
 */
WeaponInstance *WeaponHolder::removeWeaponAtIndex(size_t n) {
    if (n < weapons_.size()) {
        WeaponInstance *w = weapons_[n];
        removeWeapon(w);
        return w;
    }

    return NULL;
}

/*!
 * Deselects and removes the given weapon from the inventory.
 * Caller is responsible for freeing the returned value.
 * \param w The weapon instance.
 */
void WeaponHolder::removeWeapon(WeaponInstance *wi) {
    
    if (isWeaponSelected(wi)) {
        deselectWeapon();
    }

    if (std::erase(weapons_, wi)) {
        wi->setOwner(NULL);
    }
}

void WeaponHolder::transferWeapons(WeaponHolder &anotherHolder) {
    while (numWeapons()) {
        WeaponInstance *wi = removeWeaponAtIndex(0);
        wi->deactivate();
        // auto-reload for pistol
        if (wi->isInstanceOf(Weapon::Pistol))
            wi->reload();
        anotherHolder.addWeapon(wi);
    }
}

/*!
 * Return true if given weapon is selected.
 * @param pWeapon The weapon to check
 * @return true if pWeapon is not null and equal to the selected weapon.
 */
bool WeaponHolder::isWeaponSelected(WeaponInstance *pWeapon) {
    return pWeapon != nullptr && selectedWeapon_ == pWeapon;
}

/*!
 * Selects the weapon at given index in the inventory.
 * Calls onWeaponDeselected() and onWeaponSelected().
 */
void WeaponHolder::selectWeapon(size_t n) {
    if (n < weapons_.size()) {
        WeaponInstance *pNewWeapon = weapons_[n];
        selectWeapon(pNewWeapon);
    }
}

/**
 * Selects the given weapon if it is in the inventory.
 * \param weapon The weapon to select
 * \return void
 *
 */
void WeaponHolder::selectWeapon(WeaponInstance *pWeaponToSelect) {
    if (!isWeaponSelected(pWeaponToSelect) && canSelectWeapon(pWeaponToSelect)) {
        // First deselect current weapon if any
        WeaponInstance *prevSelectedWeapon = deselectWeapon();

        selectedWeapon_ = pWeaponToSelect;

        handleWeaponSelected(pWeaponToSelect, prevSelectedWeapon);
    }
}

/*!
 * Deselects a selected weapon if any.
 * Calls onWeaponDeselected().
 * \return the deselected weapon.
 */
WeaponInstance * WeaponHolder::deselectWeapon() {
    WeaponInstance *wi = NULL;
    if (isAnyWeaponSelected()) {
        wi = selectedWeapon_;
        selectedWeapon_ = nullptr;
        handleWeaponDeselected(wi);
    }

    return wi;
}

/*!
 * Selects a weapon based on given criteria.
 * \param criteria The criteria.
 * \return True if selection has changed
 */
bool WeaponHolder::selectRequiredWeapon(const WeaponSelectCriteria &criteria) {
    std::vector < WeaponInstance *> found_weapons;

    switch (criteria.desc) {
        case WeaponSelectCriteria::kCritWeaponType:
            for (const auto& weapon : weapons_) {
                if (weapon->isInstanceOf(criteria.criteria.wpn_type)) {
                    if (weapon->usesAmmo()) {
                        if (weapon->ammoRemaining()) {
                            found_weapons.push_back(weapon);
                            break;
                        }
                    } else {
                        found_weapons.push_back(weapon);
                        break;
                    }
                }
            }

            break;
        case WeaponSelectCriteria::kCritPlayerSelection:
            if (criteria.criteria.wi->canShoot()) {
                // If the selected weapon was a shooting one
                // select a shooting weapon for the agent, choosing
                // first a weapon of same type then any shooting weapon
                for (const auto& pWeapon : weapons_) {
                    if (pWeapon->canShoot() && pWeapon->ammoRemaining() > 0)
                    {
                        if (pWeapon->hasSameTypeAs(*(criteria.criteria.wi))) {
                            //We found a weapon of same type with ammo
                            // so remove all already found weapons
                            found_weapons.clear();
                            // and give the new one
                            found_weapons.push_back(pWeapon);
                            break;
                        } else {
                            // We found a loaded weapon of different type
                            // save it for after
                            found_weapons.push_back(pWeapon);
                        }
                    }
                }
            }
            break;
        case WeaponSelectCriteria::kCritLoadedShoot:
            for (const auto& pWeapon : weapons_) {
                if (pWeapon->canShoot() && pWeapon->ammoRemaining() > 0) {
                    found_weapons.push_back(pWeapon);
                }
            }
            break;
        default:
            break;
    }

    if (!found_weapons.empty()) {
        int best_rank = -1;
        WeaponInstance *weaponToSelect = nullptr;
        for (const auto& pWeapon : found_weapons) {
            if (criteria.use_ranks) {
                if (best_rank < pWeapon->rank()) {
                    best_rank = pWeapon->rank();
                    weaponToSelect = pWeapon;
                }
            } else {
                weaponToSelect = pWeapon;
            }
        }
        selectWeapon(weaponToSelect);
        return true;
    }
    return false;
}

/*!
 * Select a weapon for the ped if he has no weapon out.
 */
void WeaponHolder::selectShootingWeaponWithAmmo() {
    if (!isAnyWeaponSelected()) {
        // Select a loaded weapon for ped
        WeaponSelectCriteria crit;
        crit.desc = WeaponSelectCriteria::kCritLoadedShoot;
        crit.use_ranks = true;
        selectRequiredWeapon(crit);
    }
}

void WeaponHolder::selectShootingWeaponWithSameTypeFirst(WeaponInstance *pLeaderWeapon) {
    WeaponSelectCriteria criteria;
    criteria.desc = WeaponSelectCriteria::kCritPlayerSelection;
    criteria.criteria.wi = pLeaderWeapon;
    criteria.use_ranks = true;
    selectRequiredWeapon(criteria);
}

void WeaponHolder::selectMedikitOrShield(Weapon::WeaponType weaponType) {
    if (weaponType == Weapon::MediKit || weaponType == Weapon::EnergyShield) {
        WeaponSelectCriteria criteria;
        criteria.desc = WeaponSelectCriteria::kCritWeaponType;
        criteria.criteria.wpn_type = weaponType;
        criteria.use_ranks = false;
        selectRequiredWeapon(criteria);
    }
}
}
/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2013  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "weaponholder.h"
#include "core/gameevent.h"

//*************************************
// Constant definition
//*************************************
const int WeaponHolder::kNoWeaponSelected = -1;
const uint8 WeaponHolder::kMaxHoldedWeapons = 8;

WeaponHolder::WeaponHolder() {
    // -1 means no weapon is selected
    selected_weapon_ = kNoWeaponSelected;
}

WeaponHolder::~WeaponHolder() {
    destroyAllWeapons();
}

/*!
 * Adds the givent weapon to the inventory.
 * Weapon is placed at the end of the inventory.
 * \param w The weapon to add
 */
void WeaponHolder::addWeapon(WeaponInstance *w) {
    if (w != NULL && weapons_.size() < kMaxHoldedWeapons) {
        w->setDrawable(false);
        weapons_.push_back(w);
    }
}

/*!
 * Removes the weapon at the given index in the inventory.
 * Caller is responsible for freeing the returned value.
 * \param n The index. Must be less than the total number of weapons.
 * \return The removed weapon or NULL if not found.
 */
WeaponInstance *WeaponHolder::removeWeaponAtIndex(uint8 n) {
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
    // if no weapon was selected before dropping,
    // no use to update selection
    bool upd_selected = selected_weapon_ != kNoWeaponSelected;

    for (int i = 0; i < (int)weapons_.size(); ++i) {
        std::vector <WeaponInstance *>::iterator it = weapons_.begin() + i;
        if ((*it) == wi) {
            if (selectedWeapon() == wi) {
                deselectWeapon();
                // when dropping the selected weapon
                // we don't select another weapon after
                upd_selected = false;
            }

            // shift index to selected weapon if dropped weapon index
            // was before selected weapon
            if (upd_selected && selected_weapon_ > i)
                --selected_weapon_;

            weapons_.erase(it);
            wi->setOwner(NULL);

            break;
        }
    }
}

/*!
 * Removes and delete all weapons from the inventory.
 */
void WeaponHolder::destroyAllWeapons() {
    while (weapons_.size())
        delete removeWeaponAtIndex(0);
}

/*!
 * Selects the weapon at given index in the inventory.
 * Calls onWeaponDeselected() and onWeaponSelected().
 */
void WeaponHolder::selectWeapon(uint8 n) {
    assert(n < weapons_.size());
    WeaponInstance *pNewWeapon = weapons_[n];
    if (canSelectWeapon(pNewWeapon)) {
        // First deselect current weapon if any
        WeaponInstance *prevSelectedWeapon = deselectWeapon();

        selected_weapon_ = n;
        handleWeaponSelected(pNewWeapon, prevSelectedWeapon);
    }
}

/**
 * Selects the given weapon if it is in the inventory.
 * \param weapon The weapon to select
 * \return void
 *
 */
void WeaponHolder::selectWeapon(const WeaponInstance &weaponToSelect) {
    for (uint8 i = 0; i < numWeapons(); ++i) {
        if (weaponToSelect.id() == weapons_[i]->id()) {
            selectWeapon(i);
            return;
        }
    }
}

/*!
 * Deselects a selected weapon if any.
 * Calls onWeaponDeselected().
 * \return the deselected weapon.
 */
WeaponInstance * WeaponHolder::deselectWeapon() {
    WeaponInstance *wi = NULL;
    if (selected_weapon_ != kNoWeaponSelected) {
        wi = weapons_[selected_weapon_];
        selected_weapon_ = kNoWeaponSelected;
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
    WeaponInstance *wi = selectedWeapon();
    // pair <rank, indx>
    std::vector < std::pair<int, int> > found_weapons;
    uint8 sz = weapons_.size();

    bool found = false;
    switch (criteria.desc) {
        case WeaponSelectCriteria::kCritPointer:
            if (criteria.criteria.wi == wi) {
                found = true;
            } else {
                for (uint8 i = 0; i < sz; ++i) {
                    if (weapon(i) == wi) {
                        selectWeapon(i);
                        found = true;
                        break;
                    }
                }
            }
            break;
        case WeaponSelectCriteria::kCritWeaponType:
            for (uint8 i = 0; i < sz; ++i) {
                WeaponInstance *pWI = weapons_[i];
                if (pWI->isInstanceOf(criteria.criteria.wpn_type)) {
                    if (pWI->usesAmmo()) {
                        if (pWI->ammoRemaining()) {
                            found = true;
                            selectWeapon(i);
                            break;
                        }
                    } else {
                        found = true;
                        selectWeapon(i);
                        break;
                    }
                }
            }
            break;
        case WeaponSelectCriteria::kCritDamageStrict:
            for (uint8 i = 0; i < sz; ++i) {
                WeaponInstance *pWI = weapons_[i];
                if (pWI->canShoot()
                    && pWI->doesDmgStrict(criteria.criteria.dmg_type))
                {
                    if (pWI->usesAmmo()) {
                        if (pWI->ammoRemaining()) {
                            found = true;
                            found_weapons.push_back(std::make_pair(pWI->rank(),
                                i));
                        }
                    } else {
                        found = true;
                        found_weapons.push_back(std::make_pair(pWI->rank(), i));
                    }
                }
            }
            break;
        case WeaponSelectCriteria::kCritDamageNonStrict:
            for (uint8 i = 0; i < sz; ++i) {
                WeaponInstance *pWI = weapons_[i];
                if (pWI->canShoot()
                    && pWI->doesDmgNonStrict(criteria.criteria.dmg_type))
                {
                    if (pWI->usesAmmo()) {
                        if (pWI->ammoRemaining()) {
                            found = true;
                            found_weapons.push_back(std::make_pair(pWI->rank(), i));
                        }
                    } else {
                        found = true;
                        found_weapons.push_back(std::make_pair(pWI->rank(), i));
                    }
                }
            }
            break;
        case WeaponSelectCriteria::kCritPlayerSelection:
            if (criteria.criteria.wi->canShoot()) {
                // If the selected weapon was a shooting one
                // select a shooting weapon for the agent, choosing
                // first a weapon of same type then any shooting weapon
                for (uint8 i = 0; i < sz; ++i) {
                    WeaponInstance *pWI = weapons_[i];
                    if (pWI->canShoot() && pWI->ammoRemaining() > 0)
                    {
                        if (pWI->hasSameTypeAs(*(criteria.criteria.wi))) {
                            found_weapons.clear();
                            found_weapons.push_back(std::make_pair(pWI->rank(), i));
                            break;
                        } else {
                            // We found a loaded weapon of different type
                            // save it for after
                            found_weapons.push_back(std::make_pair(pWI->rank(), i));
                        }
                    }
                }
            }
            break;
        case WeaponSelectCriteria::kCritLoadedShoot:
            for (uint8 i = 0; i < sz; ++i) {
                WeaponInstance *pWI = weapons_[i];
                if (pWI->canShoot() && pWI->ammoRemaining() > 0) {
                    found_weapons.push_back(std::make_pair(pWI->rank(), i));
                }
            }
            break;
        default:
            break;
    }

    if (!found_weapons.empty()) {
        int best_rank = -1;
        int indx = found_weapons[0].second;
        if (criteria.use_ranks) {
            sz = found_weapons.size();
            for (uint8 i = 0; i < sz; ++i) {
                if (best_rank < found_weapons[i].first) {
                    best_rank = found_weapons[i].first;
                    indx = found_weapons[i].second;
                }
            }
        }
        found = true;
        selectWeapon(indx);
    }
    return found;
}

/*!
 * Select a weapon for the ped if he has no weapon out.
 */
void WeaponHolder::selectShootingWeaponWithAmmo() {
    if (selected_weapon_ == kNoWeaponSelected) {
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
    selectRequiredWeapon(criteria);
}

void WeaponHolder::selectMedikitOrShield(Weapon::WeaponType weaponType) {
    if (weaponType == Weapon::MediKit || weaponType == Weapon::EnergyShield) {
        for (uint8 i = 0; i < numWeapons(); ++i) {
            WeaponInstance *pWI = weapons_[i];
            if (pWI->isInstanceOf(weaponType) && pWI->ammoRemaining() > 0) {
                selectWeapon(i);
                break;
            }
        }
    }
}

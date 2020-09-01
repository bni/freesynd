/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2011  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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
#ifndef MODOWNER_H
#define MODOWNER_H
#include "model/mod.h"

class ModOwner {
public:
    ModOwner() {
        for (int i = 0; i < 6; i++)
            slots_[i] = NULL;
    }
    /*!
    * Returns true if the agent can be equiped with that mod version.
    */
    bool canHaveMod(Mod *pNewMod) {
        if (pNewMod == NULL) {
            return false;
        }

        Mod *pMod = slots_[pNewMod->getType()];
        if (pMod) {
            // Agent has a mod of the same type
            // Returns true if equiped version if less than new version
            return (pMod->getVersion() < pNewMod->getVersion());
        }

        // There is no mod of that type so agent can be equiped
        return true;
    }

    void addMod(Mod *pNewMod) {
        if (pNewMod) {
            slots_[pNewMod->getType()] = pNewMod;
        }
    }

    Mod *slot(int n) {
        assert(n < 6);
        return slots_[n];
    }

    /*!
     * Returns true if the owner is equiped with mod of the
     * given type and at least of given version.
     */
    bool hasMinimumVersionOfMod(Mod::EModType type, Mod::EModVersion version) {
        Mod *pMod = slots_[type];
        return (pMod && pMod->getVersion() >= version);
    }

    void clearSlots() {
        for (int i = 0; i < 6; i++)
            slots_[i] = NULL;
    }

    /*!
     * This method returns the amount of time before health
     * is restored when a ped owns the right version of Chest.
     * \return 0 if ped do not have good chest.
     */
    uint16 getHealthRegenerationPeriod() {
        Mod *pMod = slots_[Mod::MOD_CHEST];
        if (pMod) {
            // TODO : add different times depending on version of chest
            if (pMod->getVersion() >= Mod::MOD_V1) {
                return 4000;
            }
        }

        return 0;
    }

protected:
    Mod *slots_[6];
};

#endif

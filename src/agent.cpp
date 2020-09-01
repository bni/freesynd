/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
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
#include <string.h>
#include <assert.h>

#include "gfx/screen.h"
#include "agent.h"

int Agent::agentCnt = 1;

/*!
 * Constructs a new agent.
 * \param id Object id.
 * \param agent_name The agent name.
 * \param male True means the agent is a male.
 */
Agent::Agent(const char *agent_name, bool male) : name_(agent_name),
male_(male), active_(true) {
    id_ = agentCnt++;
    is_alive_ = true;
}

bool Agent::saveToFile(PortableFile &file) {
    // id
    file.write32(id_);
    // Agent name : 13 characters max, nul-padded
    file.write_string(name_, 13);
    // gender : male = 1, female = 0
    file.write8b(male_);
    // active : true = 1, false = 0
    file.write8b(active_);
    // Health : not used
    file.write32(0);
    // Mods
    int nb = 0;
    for (int i = 0; i < 6; i++) {
        if (slots_[i]) {
            nb++;
        }
    }
    file.write32(nb);
    for (int mIndex = 0; mIndex < 6; mIndex++) {
        Mod *pMod = slot(mIndex);
        if (pMod) {
            file.write32(pMod->getType());
            file.write32(pMod->getVersion());
        }
    }
    // Weapons
    nb = numWeapons();
    file.write32(nb);
    for (int wIndex = 0; wIndex < numWeapons(); wIndex++) {
        WeaponInstance *pWinst = weapon(wIndex);
        file.write32(pWinst->getClass()->getType());
        file.write32(pWinst->ammoRemaining());
    }
    return true;
}

bool Agent::loadFromFile(PortableFile &infile, const FormatVersion& v) {
    // if this instance has already been populated reset it
    clearSlots();
    destroyAllWeapons();
    // id
    id_ = infile.read32();
    // update counter
    if (agentCnt<=id_) {
        agentCnt = id_ + 1;
    }
    // name
    name_ = infile.read_string((v == 0x0100) ? 15 : 13, true);
    // gender
    male_ = infile.read8b();
    // active
    active_ = infile.read8b();
    // health : not used
    int health = infile.read32();

    return true;
}

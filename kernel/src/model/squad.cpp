/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2012, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-kernel/model/squad.h"

#include "fs-kernel/model/ped.h"

namespace fs_knl {

const size_t Squad::kMaxSlot = 4;
const size_t Squad::kSlot1 = 0;
const size_t Squad::kSlot2 = 1;
const size_t Squad::kSlot3 = 2;
const size_t Squad::kSlot4 = 3;

//! Default constructor
Squad::Squad() {
    clear();
}

void Squad::clear() {
    for (size_t s = 0; s < kMaxSlot; s++) {
        members_[s] = nullptr;
    }
    size_ = 0;
}

/*!
* Sets an agent in the squad at the given index.
* \param slotId The agent's index in the team (from 0 to 3)
* \param pPedAgent The new agent
*/
void Squad::setMember(size_t slotId, PedInstance *pPedAgent) {
    assert(slotId < kMaxSlot);
    members_[slotId] = pPedAgent;

    // recount the number of agent
    size_ = 0;
    for (const auto& agent : members_) {
        if (agent) {
            size_++;
        }
    }
};

//! Returns the agent on the given slot
PedInstance * Squad::member(size_t slotId) {
    assert(slotId < kMaxSlot);
    return members_[slotId];
}

/*!
 * Returns true if one living agent has a scanner.
 */
bool Squad::hasScanner() {
    for (const auto& pAgent : members_) {
        if (pAgent && pAgent->isAlive()) {
            if (pAgent->hasWeapon(Weapon::Scanner)) {
                return true;
            }
        }
    }

    // No agent has a scanner
    return false;
}

/*!
 * Check if all agents are dead.
 * \return true if at least one agent is alive.
 */
bool Squad::isAllDead() {
    for (const auto& pAgent : members_) {
        if (pAgent && pAgent->isAlive()) {
            return false;
        }
    }

    // No agent is alive
    return true;
}

void Squad::getPositionInSquadFormation(size_t slotId, TilePoint *pPosition) {
    //TODO: current group position is like
    // in original this can make non-tile
    // oriented
    //int sox = (i % 2) * (i - 2) * 16;
    //int soy = ((i + 1) % 2) * (i - 1) * 8;

    //this should be removed if non-tile position needed
    pPosition->ox = 63 + 128 * (static_cast<int>(slotId) % 2);
    pPosition->oy = 63 + 128 * (static_cast<int>(slotId) >> 1);
}

/*!
 * Advance the iterator if the current element does not match the criteria
 */
void Squad::SquadIterator::advanceIfNeeded() {
    while (current != end) {
        if (*current == nullptr || (checkHealth_ && !(*current)->isAlive())) {
            ++current;
        } else {
            break;
        }
    }
}

}
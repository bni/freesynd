/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
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
#include <catch2/catch_test_macros.hpp>

#include "fs-kernel/model/ped.h"

TEST_CASE( "PedAction", "[kernel][ped]" ) {
    fs_knl::PedInstance cut(1, nullptr, fs_knl::PedInstance::kPedTypeAgent, true, 128);
    cut.setStartHealth(10);
    cut.switchActionStateTo(fs_knl::PedInstance::pa_smStanding);

    SECTION( "can take hit action") {
        // Reject cause health is zero
        REQUIRE_FALSE( cut.canTakeAction(fs_knl::Action::kActTypeHit) );

        // Should accept
        cut.resetHealth();
        REQUIRE( cut.canTakeAction(fs_knl::Action::kActTypeHit) );

        // Reject cause state is already hit
        cut.switchActionStateTo(fs_knl::PedInstance::pa_smHit);
        REQUIRE_FALSE( cut.canTakeAction(fs_knl::Action::kActTypeHit) );
    }
}
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

#include "fs-kernel/model/mission.h"
#include "fs-kernel/model/ped.h"

TEST_CASE( "Mission", "[kernel][mission]" ) {
    LevelData::MapInfos mapInfos;
    fs_knl::Mission cut(mapInfos, nullptr);

    SECTION( "Add/Remove Armed peds") {
        fs_knl::PedInstance ped1(1, nullptr, true);
        fs_knl::PedInstance ped2(2, nullptr, true);

        REQUIRE( cut.numArmedPeds() == 0 );

        cut.addArmedPed(&ped1);
        cut.addArmedPed(&ped2);
        REQUIRE( cut.numArmedPeds() == 2 );

        cut.removeArmedPed(&ped2);
        REQUIRE( cut.numArmedPeds() == 1 );
        REQUIRE( cut.armedPedAtIndex(0) == &ped1 );
    }
}
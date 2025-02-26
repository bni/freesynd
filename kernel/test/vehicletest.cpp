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

#include "fs-kernel/model/vehicle.h"

TEST_CASE( "Vehicle", "[kernel][vehicle]" ) {
    fs_knl::GenericCar cut(1, fs_knl::Vehicle::kVehicleTypeRegularCar, nullptr);

    SECTION( "Should set animations with right offset") {
        // Test is not complete
        cut.setAnimations(2);
        REQUIRE( cut.regularAnimation() == 0 );
        REQUIRE( cut.burntAnimation() == 2 );
    }
}
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
#include "testcase.h"
#include "fs-engine/appcontext.h"

TEST_CASE( "Ped", "[kernel][ped]" ) {
    fs_eng::AppContext appCtx;
    ConfigFile config;
    initWeaponConfigFile(config);

    fs_knl::PedInstance cut(1, nullptr, fs_knl::PedInstance::kPedTypeAgent, true, 50);
    cut.setStartHealth(10, true);

    SECTION( "State management") {
        cut.switchActionStateTo(fs_knl::PedInstance::pa_smWalking);
        REQUIRE( cut.isState(fs_knl::PedInstance::pa_smWalking) );
    }

    SECTION("Mods") {
        SECTION ("Speed should be default with no mods and no load") {
            cut.setSpeedToMax();
            REQUIRE( cut.speed() == 50 );
        }

        SECTION ("Speed should be modified with no mods and overload") {
            fs_knl::Weapon shieldClass(fs_knl::Weapon::EnergyShield, config);
            fs_knl::Weapon gaussClass(fs_knl::Weapon::GaussGun, config);
            fs_knl::WeaponInstance energyShield(&shieldClass, 0, nullptr);
            fs_knl::WeaponInstance gauss(&gaussClass, 1, nullptr);

            // inventory is only above max weight
            cut.addWeapon(&energyShield);
            cut.setSpeedToMax();
            REQUIRE( cut.speed() == (50 / 2) );

            // Inventory is now more than double max weight
            cut.addWeapon(&gauss);
            cut.setSpeedToMax();
            REQUIRE( cut.speed() == fs_knl::PedInstance::kAgentMaxSpeedWithOverweight );
        }

        SECTION ("Speed should be higher with Leg Mod and no load") {
            fs_knl::Mod legV1("LegV1", fs_knl::Mod::MOD_LEGS, fs_knl::Mod::MOD_V1, 0, "", 0);
            fs_knl::Mod legV2("LegV2", fs_knl::Mod::MOD_LEGS, fs_knl::Mod::MOD_V2, 0, "", 0);
            fs_knl::Mod legV3("LegV3", fs_knl::Mod::MOD_LEGS, fs_knl::Mod::MOD_V3, 0, "", 0);

            cut.addMod(&legV1);
            cut.setSpeedToMax();
            REQUIRE( cut.speed() == 62 );

            cut.addMod(&legV2);
            cut.setSpeedToMax();
            REQUIRE( cut.speed() == 75 );

            cut.addMod(&legV3);
            cut.setSpeedToMax();
            REQUIRE( cut.speed() == 87 );
        }
    }
}
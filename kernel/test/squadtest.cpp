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

#include "fs-engine/appcontext.h"
#include "fs-kernel/model/squad.h"
#include "fs-kernel/model/ped.h"

#include "testcase.h"

TEST_CASE( "Squad", "[kernel][squad]" ) {
    // Component under test
    fs_knl::Squad cut;
    fs_knl::PedInstance agent1(1, nullptr, fs_knl::PedInstance::kPedTypeAgent, true, 128);
    fs_knl::PedInstance agent2(2, nullptr, fs_knl::PedInstance::kPedTypeAgent, true, 128);
    fs_knl::PedInstance agent3(3, nullptr, fs_knl::PedInstance::kPedTypeAgent, true, 128);
    fs_knl::PedInstance agent4(4, nullptr, fs_knl::PedInstance::kPedTypeAgent, true, 128);
    agent1.setHealth(10);
    agent2.setHealth(10);
    agent3.setHealth(10);
    agent4.setHealth(10);

    SECTION( "Add/Remove members") {
        REQUIRE( cut.size() == 0);
        cut.setMember(0, &agent1);
        cut.setMember(1, &agent2);
        REQUIRE( cut.size() == 2);

        cut.setMember(0, &agent3);
        REQUIRE( cut.member(0) == &agent3);
        REQUIRE( cut.size() == 2);

        cut.clear();
        REQUIRE( cut.size() == 0);
    }

    SECTION( "isAllDead") {
        cut.setMember(0, &agent1);
        cut.setMember(1, &agent2);
        cut.setMember(2, &agent3);
        cut.setMember(3, &agent4);
        REQUIRE( cut.size() == 4);

        REQUIRE_FALSE( cut.isAllDead());

        agent4.setHealth(0);
        REQUIRE_FALSE( cut.isAllDead());

        agent1.setHealth(0);
        agent2.setHealth(0);
        agent3.setHealth(0);
        REQUIRE( cut.isAllDead());
    }

    SECTION( "hasScanner") {
        fs_eng::AppContext appCtx;
        ConfigFile config;
        initWeaponConfigFile(config);
        fs_knl::Weapon scannerClass(fs_knl::Weapon::Scanner, config);
        fs_knl::WeaponInstance scanner(&scannerClass, 0, nullptr);

        cut.setMember(0, &agent1);
        cut.setMember(1, &agent2);
        cut.setMember(2, &agent3);
        cut.setMember(3, &agent4);
        REQUIRE_FALSE( cut.hasScanner() );

        agent3.addWeapon(&scanner);
        REQUIRE( cut.hasScanner() );
    }

    SECTION( "Non null Iterator") {
        cut.setMember(1, &agent2);
        cut.setMember(2, &agent4);

        int nbFound = 0;
        for (auto& agent : cut) {
            REQUIRE( agent != nullptr );
            nbFound++;
        }

        REQUIRE( nbFound == 2 );
    }

    SECTION( "Alive Iterator") {
        cut.setMember(1, &agent2);
        cut.setMember(2, &agent4);
        cut.setMember(3, &agent1);
        agent2.setHealth(0);

        size_t i = 0;
        int nbFound = 0;
        for (auto it = cut.alive_begin(); it != cut.alive_end(); ++it) {
            fs_knl::PedInstance *pAgent = *it;
            REQUIRE( pAgent != nullptr );
            REQUIRE( pAgent->isAlive() );
            REQUIRE( pAgent != &agent2 );
            nbFound++;
        }

        REQUIRE( nbFound == 2 );
    }
}
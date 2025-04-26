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

    SECTION( "Update stats") {
        // Define a squad of 2 agents
        fs_knl::PedInstance *agent = new fs_knl::PedInstance(1, nullptr, true);
        cut.addPed(agent);
        cut.getSquad()->setMember(fs_knl::Squad::kSlot1, agent);
        agent->setStartHealth(10);
        agent->resetHealth();
        agent = new fs_knl::PedInstance(2, nullptr, true);
        cut.addPed(agent);
        agent->setStartHealth(10);
        agent->resetHealth();
        cut.getSquad()->setMember(fs_knl::Squad::kSlot2, agent);

        // Enemies : 4 total : 3 killed, 1 persuaded
        fs_knl::PedInstance *ped = new fs_knl::PedInstance(3, nullptr, true);
        ped->setTypeFromValue(0x02);
        ped->setObjGroupDef(fs_knl::PedInstance::og_dmAgent);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped = new fs_knl::PedInstance(4, nullptr, true);
        cut.addPed(ped);
        ped->setTypeFromValue(0x02);
        ped->setObjGroupDef(fs_knl::PedInstance::og_dmAgent);
        ped->setStartHealth(10);
        ped->resetHealth();
        ped->handlePersuadedBy(agent);
        ped = new fs_knl::PedInstance(5, nullptr, true);
        cut.addPed(ped);
        ped->setTypeFromValue(0x02);
        ped->setObjGroupDef(fs_knl::PedInstance::og_dmAgent);
        ped->setStartHealth(10);
        ped = new fs_knl::PedInstance(6, nullptr, true);
        cut.addPed(ped);
        ped->setTypeFromValue(0x02);
        ped->setObjGroupDef(fs_knl::PedInstance::og_dmAgent);
        ped->setStartHealth(10);
    
        // Civilian : 5 total : 2 killed, 2 persuaded
        ped = new fs_knl::PedInstance(7, nullptr, true);
        ped->setTypeFromValue(0x01);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped->resetHealth();
        ped->handlePersuadedBy(agent);
        ped = new fs_knl::PedInstance(8, nullptr, true);
        ped->setTypeFromValue(0x01);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped = new fs_knl::PedInstance(9, nullptr, true);
        ped->setTypeFromValue(0x01);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped->resetHealth();
        ped->handlePersuadedBy(agent);
        ped = new fs_knl::PedInstance(10, nullptr, true);
        ped->setTypeFromValue(0x01);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped = new fs_knl::PedInstance(11, nullptr, true);
        ped->setTypeFromValue(0x01);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped->resetHealth();

        // Criminal : 2 total : 1 killed
        ped = new fs_knl::PedInstance(12, nullptr, true);
        ped->setTypeFromValue(0x10);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped = new fs_knl::PedInstance(13, nullptr, true);
        ped->setTypeFromValue(0x10);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped->resetHealth();

        // Guard : 3 total : 3 killed
        ped = new fs_knl::PedInstance(14, nullptr, true);
        ped->setTypeFromValue(0x08);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped = new fs_knl::PedInstance(15, nullptr, true);
        ped->setTypeFromValue(0x08);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped = new fs_knl::PedInstance(16, nullptr, true);
        ped->setTypeFromValue(0x08);
        cut.addPed(ped);
        ped->setStartHealth(10);

        // Police : 2 total : 1 killed
        ped = new fs_knl::PedInstance(17, nullptr, true);
        ped->setTypeFromValue(0x04);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped = new fs_knl::PedInstance(18, nullptr, true);
        ped->setTypeFromValue(0x04);
        cut.addPed(ped);
        ped->setStartHealth(10);
        ped->resetHealth();

        cut.stats()->init(2);
        cut.endWithStatus(fs_knl::Mission::kMissionStatusCompleted);

        REQUIRE( cut.stats()->enemyKilled() == 3 );
        REQUIRE( cut.stats()->civilKilled() == 2 );
        REQUIRE( cut.stats()->criminalKilled() == 1 );
        REQUIRE( cut.stats()->guardKilled() == 3 );
        REQUIRE( cut.stats()->policeKilled() == 1 );

        REQUIRE( cut.stats()->nbAgentCaptured() == 1 );
        REQUIRE( cut.stats()->convinced() == 2 );
    }
}
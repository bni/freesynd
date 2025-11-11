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
#include "fs-kernel/model/vehicle.h"

void configureMission(fs_knl::Mission &mission) {
    // Define a squad of 2 agents
    fs_knl::PedInstance *agent = new fs_knl::PedInstance(1, nullptr, fs_knl::PedInstance::kPedTypeAgent, true, 128);
    mission.addPed(agent);
    mission.getSquad()->setMember(fs_knl::Squad::kSlot1, agent);
    agent->setStartHealth(10, true);
    agent->setPosition(10, 20, 3, 5, 56, 0);
    agent = new fs_knl::PedInstance(2, nullptr, fs_knl::PedInstance::kPedTypeAgent, true, 128);
    mission.addPed(agent);
    agent->setStartHealth(10, true);
    agent->setPosition(20, 45, 1, 10, 45, 0);
    mission.getSquad()->setMember(fs_knl::Squad::kSlot2, agent);

    // Enemies : 4 total : 3 killed, 1 persuaded
    fs_knl::PedInstance *ped = new fs_knl::PedInstance(3, nullptr, fs_knl::PedInstance::kPedTypeAgent, false, 128);
    ped->setObjGroupDef(fs_knl::PedInstance::og_dmAgent);
    mission.addPed(ped);
    ped->setStartHealth(10);
    ped = new fs_knl::PedInstance(4, nullptr, fs_knl::PedInstance::kPedTypeAgent, false, 128);
    mission.addPed(ped);
    ped->setObjGroupDef(fs_knl::PedInstance::og_dmAgent);
    ped->setStartHealth(10, true);
    ped->setPosition(10, 20, 3, 56, 12, 0); //same tile as agent 1
    ped->handlePersuadedBy(agent);
    ped = new fs_knl::PedInstance(5, nullptr, fs_knl::PedInstance::kPedTypeAgent, false, 128);
    mission.addPed(ped);
    ped->setObjGroupDef(fs_knl::PedInstance::og_dmAgent);
    ped->setStartHealth(10);
    ped = new fs_knl::PedInstance(6, nullptr, fs_knl::PedInstance::kPedTypeAgent, false, 128);
    mission.addPed(ped);
    ped->setObjGroupDef(fs_knl::PedInstance::og_dmAgent);
    ped->setStartHealth(10);
    ped->setPosition(10, 20, 3, 45, 20, 0); //same tile as ped 4 but dead

    // Civilian : 5 total : 2 killed, 2 persuaded
    ped = new fs_knl::PedInstance(7, nullptr, fs_knl::PedInstance::kPedTypeCivilian, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10, true);
    ped->handlePersuadedBy(agent);
    ped->setPosition(55, 20, 3, 45, 20, 0);
    ped = new fs_knl::PedInstance(8, nullptr, fs_knl::PedInstance::kPedTypeCivilian, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10);
    ped = new fs_knl::PedInstance(9, nullptr, fs_knl::PedInstance::kPedTypeCivilian, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10, true);
    ped->handlePersuadedBy(agent);
    ped = new fs_knl::PedInstance(10, nullptr, fs_knl::PedInstance::kPedTypeCivilian, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10);
    ped = new fs_knl::PedInstance(11, nullptr, fs_knl::PedInstance::kPedTypeCivilian, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10, true);

    // Criminal : 2 total : 1 killed
    ped = new fs_knl::PedInstance(12, nullptr, fs_knl::PedInstance::kPedTypeCriminal, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10);
    ped = new fs_knl::PedInstance(13, nullptr, fs_knl::PedInstance::kPedTypeCriminal, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10, true);

    // Guard : 3 total : 3 killed
    ped = new fs_knl::PedInstance(14, nullptr, fs_knl::PedInstance::kPedTypeGuard, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10);
    ped = new fs_knl::PedInstance(15, nullptr, fs_knl::PedInstance::kPedTypeGuard, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10);
    ped = new fs_knl::PedInstance(16, nullptr, fs_knl::PedInstance::kPedTypeGuard, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10);

    // Police : 2 total : 1 killed
    ped = new fs_knl::PedInstance(17, nullptr, fs_knl::PedInstance::kPedTypePolice, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10);
    ped = new fs_knl::PedInstance(18, nullptr, fs_knl::PedInstance::kPedTypeGuard, false, 128);
    mission.addPed(ped);
    ped->setStartHealth(10, true);

    // Vehicle : 1 alive, 1 destroyed
    fs_knl::Vehicle *pVehicle = 
        new fs_knl::GenericCar(11, fs_knl::Vehicle::kVehicleTypePolice, nullptr, 100);
    mission.addVehicle(pVehicle);
    pVehicle->setPosition(12, 45, 2, 0, 5, 0);
    pVehicle->setStartHealth(10, true);

    pVehicle = 
        new fs_knl::GenericCar(12, fs_knl::Vehicle::kVehicleTypeRegularCar, nullptr, 100);
    pVehicle->setPosition(12, 45, 2, 0, 5, 0);
    pVehicle->setStartHealth(10);
    mission.addVehicle(pVehicle);
}

TEST_CASE( "Mission", "[kernel][mission]" ) {
    LevelData::MapInfos mapInfos;
    fs_knl::Mission cut(mapInfos, nullptr);
    
    configureMission(cut);

    SECTION( "Add/Remove Armed peds") {
        REQUIRE( cut.numArmedPeds() == 0 );

        cut.addArmedPed(cut.ped(0));
        cut.addArmedPed(cut.ped(1));
        REQUIRE( cut.numArmedPeds() == 2 );

        cut.removeArmedPed(cut.ped(1));
        REQUIRE( cut.numArmedPeds() == 1 );
        REQUIRE( cut.armedPedAtIndex(0) == cut.ped(0) );
    }

    SECTION( "Update stats") {
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

    SECTION( "findObjectWithNatureAtPos") {
        SECTION( "should find nothing") {
            fs_knl::MapObject::ObjectNature nature = fs_knl::MapObject::kNaturePed;
            size_t searchIndex = 0;

            fs_knl::MapObject *pObject = 
                cut.findObjectWithNatureAtPos(20, 45, 3, nature, &searchIndex);

            REQUIRE( pObject == nullptr );
            REQUIRE( nature == fs_knl::MapObject::kNaturePed );
            REQUIRE( searchIndex == 0 );
        }

        SECTION( "should find agent #2") {
            fs_knl::MapObject::ObjectNature nature = fs_knl::MapObject::kNaturePed;
            size_t searchIndex = 0;

            fs_knl::MapObject *pObject = 
                cut.findObjectWithNatureAtPos(20, 45, 1, nature, &searchIndex);

            REQUIRE( pObject->id() == 2 );
            REQUIRE( nature == fs_knl::MapObject::kNaturePed );
            REQUIRE( searchIndex == 2 );
        }

        SECTION( "should find vehicle #2") {
            fs_knl::MapObject::ObjectNature nature = fs_knl::MapObject::kNatureVehicle;
            size_t searchIndex = 1;

            fs_knl::MapObject *pObject = 
                cut.findObjectWithNatureAtPos(12, 45, 2, nature, &searchIndex);

            REQUIRE( pObject->id() == 12 );
            REQUIRE( nature == fs_knl::MapObject::kNatureVehicle );
            REQUIRE( searchIndex == 2 );
        }
    }
}
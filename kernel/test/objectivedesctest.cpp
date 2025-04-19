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
#include "fs-engine/events/event.h"
#include "fs-kernel/model/objectivedesc.h"
#include "fs-kernel/model/mission.h"
#include "fs-kernel/model/squad.h"

#include "testcase.h"

class ObjectiveDescImpl : public fs_knl::ObjectiveDesc {
public:
    ObjectiveDescImpl() {
        handleStartCalled_ = false;
        objectiveEndedCalled_ = false;
    }

    bool handleStartCalled() { return handleStartCalled_; }
    bool objectiveEndedCalled() { return objectiveEndedCalled_; }
    void onObjectiveEndedEvent(fs_knl::ObjectiveEndedEvent *pEvt) { objectiveEndedCalled_ = true; }
protected:
    void handleStart() override {
        handleStartCalled_ = true;
    }

    void doEvaluate(fs_knl::Mission *pMission) override { endObjective(true); }

private:
    bool handleStartCalled_;
    bool objectiveEndedCalled_;
};

TEST_CASE( "ObjectiveDesc", "[kernel][objectivedesc]" ) {

    fs_eng::AppContext appCtx;
    fs_knl::PedInstance agent1(3, nullptr, false);
    fs_knl::PedInstance agent2(4, nullptr, false);
    fs_knl::PedInstance agent3(5, nullptr, false);
    agent1.setHealth(10);
    agent2.setHealth(10);
    agent3.setHealth(10);
    LevelData::MapInfos infos;
    fs_knl::Mission mission(infos, nullptr);
    mission.getSquad()->setMember(0, &agent1);
    mission.getSquad()->setMember(1, &agent2);
    mission.getSquad()->setMember(2, &agent3);

    SECTION( "ObjectiveDesc basic") {
        ObjectiveDescImpl cut;

        SECTION( "Should not evaluate when not started") {
            REQUIRE( cut.status == fs_knl::kNotStarted);

            cut.evaluate(&mission);
            REQUIRE_FALSE( cut.isTerminated() );
        }

        SECTION( "Should evaluate when started") {
            EventManager::listen<fs_knl::ObjectiveEndedEvent>(&cut, &ObjectiveDescImpl::onObjectiveEndedEvent);

            cut.start();
            REQUIRE( cut.status == fs_knl::kStarted);
            REQUIRE( cut.handleStartCalled());

            cut.evaluate(&mission);
            REQUIRE( cut.isTerminated() );
            REQUIRE( cut.status == fs_knl::kCompleted );
            REQUIRE( cut.objectiveEndedCalled() );
        }

        SECTION( "Should not end in failed with forceEnd if not started") {
            cut.forceEnd();
            
            REQUIRE( cut.status == fs_knl::kNotStarted );
        }

        SECTION( "Should end in failed with forceEnd") {
            cut.start();
            cut.forceEnd();

            REQUIRE( cut.status == fs_knl::kFailed );
        }
        
        SECTION( "Should end in failed if all agents have died") {
            agent1.setHealth(0);
            agent2.setHealth(0);
            agent3.setHealth(0);

            cut.start();
            cut.evaluate(&mission);

            REQUIRE( cut.status == fs_knl::kFailed );
        }
    }

    SECTION( "ObjEvacuate") {
        fs_knl::PedInstance ped1(1, nullptr, false);
        fs_knl::PedInstance ped2(2, nullptr, false);
        std::vector<fs_knl::PedInstance *> pedsToEvacuate = {&ped1, &ped2};
        fs_knl::ObjEvacuate cut(5200, 6400, 128, pedsToEvacuate);

        ped1.setHealth(10);
        ped2.setHealth(10);
        // By default, all peds and agents are in the zone
        ped1.setPosition(20, 25, 1);
        ped2.setPosition(20, 25, 1);
        agent1.setPosition(20, 25, 1);
        agent2.setPosition(21, 24, 1);
        agent3.setPosition(20, 24, 1, 15, 110);

        SECTION( "Should fail due to ped to evacuate is dead") {
            // Ped1 is dead
            ped1.setHealth(0);
            cut.start();
            cut.evaluate(&mission);
            REQUIRE( cut.status == fs_knl::kFailed );
        }

        SECTION( "Should not be terminated due to ped not in zone") {
            // ped 2 is out of zone
            ped2.setPosition(30, 24, 1);
            cut.start();
            cut.evaluate(&mission);
            REQUIRE( cut.status == fs_knl::kStarted );
        }

        SECTION( "Should not be terminated due to agent not in zone") {
            // agent2 is out of the zone
            agent2.setPosition(31, 24, 1);
            cut.start();
            cut.evaluate(&mission);
            REQUIRE( cut.status == fs_knl::kStarted );
        }

        SECTION( "Should be completed") {
            cut.start();
            cut.evaluate(&mission);
            REQUIRE( cut.status == fs_knl::kCompleted );
        }
    }
}
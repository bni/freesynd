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

#include "fs-kernel/model/mapobject.h"
#include "fs-kernel/model/map.h"

class MapObjectTest : public fs_knl::MapObject {
public:
    MapObjectTest(uint16_t id, ObjectNature nature): fs_knl::MapObject(id, nullptr, nature) {
        doUpdateState_ = 0;
    }
    virtual ~MapObjectTest() {}
    void draw(const Point2D &screenPos) override {}

    uint32_t doUpdateStateValue() { return doUpdateState_; }
protected:
    void doUpdateState([[maybe_unused]] uint32_t elapsed) {
        doUpdateState_ = elapsed;
    }
protected:
    uint32_t doUpdateState_;
};

class ShootableMapObjectTest : public fs_knl::ShootableMapObject {
public:
    ShootableMapObjectTest(uint16_t id) :
        fs_knl::ShootableMapObject(id, nullptr, fs_knl::MapObject::kNaturePed) {}

    void draw(const Point2D &screenPos) override {}
};

TEST_CASE( "Map Object", "[kernel][mapobject]" ) {
    MapObjectTest cut{1, fs_knl::MapObject::kNaturePed};

    SECTION( "Directions for 8 sectors") {
        cut.setDirection(10);
        REQUIRE( cut.getDiscreteDirection(8) == 0);
        cut.setDirection(24);
        REQUIRE( cut.getDiscreteDirection(8) == 1);
        cut.setDirection(79);
        REQUIRE( cut.getDiscreteDirection(8) == 2);
        cut.setDirection(90);
        REQUIRE( cut.getDiscreteDirection(8) == 3);
        cut.setDirection(132);
        REQUIRE( cut.getDiscreteDirection(8) == 4);
        cut.setDirection(153);
        REQUIRE( cut.getDiscreteDirection(8) == 5);
        cut.setDirection(192);
        REQUIRE( cut.getDiscreteDirection(8) == 6);
        cut.setDirection(213);
        REQUIRE( cut.getDiscreteDirection(8) == 7);
        cut.setDirection(256);
        REQUIRE( cut.getDiscreteDirection(8) == 0);
    }

    SECTION( "Directions for 4 sectors") {
        cut.setDirection(10);
        REQUIRE( cut.getDiscreteDirection(4) == 0);
        cut.setDirection(80);
        REQUIRE( cut.getDiscreteDirection(4) == 1);
        cut.setDirection(150);
        REQUIRE( cut.getDiscreteDirection(4) == 2);
        cut.setDirection(220);
        REQUIRE( cut.getDiscreteDirection(4) == 3);
        cut.setDirection(255);
        REQUIRE( cut.getDiscreteDirection(4) == 0);
    }

    SECTION( "Animate") {
        SECTION( "should call doUpdateState()") {
            cut.animate(32);
            REQUIRE( cut.doUpdateStateValue() == 32 );
        }
    }
}

TEST_CASE( "ShootableMapObject", "[kernel][mapobject]" ) {
    ShootableMapObjectTest cut(1);
    SECTION( "set health and start health") {
        // By default health and start health are zero
        REQUIRE( cut.health() == 0 );
        REQUIRE( cut.startHealth() == 0 );

        cut.setHealth(10);
        REQUIRE( cut.health() == 10 );

        cut.setStartHealth(20);
        REQUIRE( cut.startHealth() == 20 );
    }

    SECTION( "Should not increase health more than start health") {
        cut.setHealth(10);
        cut.setStartHealth(20);

        cut.increaseHealth(5);
        REQUIRE( cut.health() == 15 );

        cut.increaseHealth(10);
        REQUIRE( cut.health() == 20 );
    }

    SECTION( "Should decrease health") {
        cut.setHealth(10);

        cut.decreaseHealth(20);
        REQUIRE( cut.isDead() );
    }

    SECTION( "Should init health with start health") {
        cut.setStartHealth(20, true);
        REQUIRE( cut.health() == 20 );
        REQUIRE( cut.health() == cut.startHealth() );
    }

    SECTION( "Should reset health") {
        cut.setHealth(10);
        cut.setStartHealth(20);

        cut.resetHealth();
        REQUIRE( cut.health() == cut.startHealth() );
    }
}
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
    MapObjectTest(uint16_t id, ObjectNature nature): fs_knl::MapObject(id, nullptr, nature) {}
    virtual ~MapObjectTest() {}
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
}
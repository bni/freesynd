/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-kernel/model/time.h"

TEST_CASE( "GameTime", "[kernel][gametime]" ) {
    fs_knl::GameTime cut;

    SECTION( "Should initialize with right value") {
        REQUIRE( cut.currentHour() == 0 );
        REQUIRE( cut.currentDay() == 1 );
        REQUIRE( cut.currentYear() == 85 );
    }

    SECTION( "Should get the good string format of time") {
        std::string timeAsString;

        cut.getTimeAsStr(timeAsString);
        REQUIRE( timeAsString.compare("00:1:85NC") == 0 );
    }

    SECTION( "Should update time with default hour duration") {
        REQUIRE( cut.updateTime(2000) == 0 );

        REQUIRE( cut.updateTime(3500) == 1 ); // remains 1500
        REQUIRE( cut.currentHour() == 1 );
        REQUIRE( cut.currentDay() == 1 );
        REQUIRE( cut.currentYear() == 85 );

        REQUIRE( cut.updateTime(2500) == 1 ); // remains 0
        REQUIRE( cut.currentHour() == 2 );
        REQUIRE( cut.currentDay() == 1 );
        REQUIRE( cut.currentYear() == 85 );

        REQUIRE( cut.updateTime(87000) == 21 ); // remains 3000
        REQUIRE( cut.currentHour() == 23 );
        REQUIRE( cut.currentDay() == 1 );
        REQUIRE( cut.currentYear() == 85 );

        REQUIRE( cut.updateTime(1500) == 1 ); // remains 500
        REQUIRE( cut.currentHour() == 0 );
        REQUIRE( cut.currentDay() == 2 );
        REQUIRE( cut.currentYear() == 85 );
    }

    SECTION( "Should update time with accelerated hour duration") {
        cut.cheatAccelerateTime();
        REQUIRE( cut.updateTime(8808000) == 8808 );
        REQUIRE( cut.currentHour() == 0 );
        REQUIRE( cut.currentDay() == 3 );
        REQUIRE( cut.currentYear() == 86 );

        cut.reset();
        REQUIRE( cut.currentDay() == 1 );
        REQUIRE( cut.currentHour() == 0 );
        REQUIRE( cut.currentYear() == 85 );
    }

    SECTION( "Should return the number of days and hours for elapsed") {
        uint32_t days, hours;
        cut.getDayHourFromPeriod(308000, days, hours);
        REQUIRE( days == 3 );
        REQUIRE( hours == 5 );
    }

    SECTION( "diffInDays()") {
        fs_knl::GameTime previousTime;
        SECTION( "Should return 0 when same time") {
            REQUIRE( cut.diffInDays(previousTime) == 0 );
        }

        SECTION( "Should return zero when same day and year") {
            cut.setTime(85, 1, 2);
            REQUIRE( cut.diffInDays(previousTime) == 0 );
        }

        SECTION( "Should return zero when other time is smaller") {
            previousTime.setTime(86, 10, 2);
            REQUIRE( cut.diffInDays(previousTime) == 0 );
        }

        SECTION( "Should return non zero when same year") {
            cut.setTime(85, 3, 3);
            REQUIRE( cut.diffInDays(previousTime) == 2 );
        }

        SECTION( "Should return non zero when year are different") {
            cut.setTime(86, 8, 10);
            REQUIRE( cut.diffInDays(previousTime) == 372 );
        }
    }
}
/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2010, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>
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
#ifndef KERNEL_MODEL_TIME_H
#define KERNEL_MODEL_TIME_H

#include <cstdint>
#include <string>

namespace fs_knl {

class GameTime {
public:
    //! Number of days in a year
    static const int kNbDaysInYear;
    /*! Number of hours in a day in the game.*/
    static const int kNbHourInDay;

    GameTime();

    //! Reset the time to default values
    void reset();
    //! Force the time with given values
    void setTime(uint32_t year, uint32_t day, uint32_t hour);

    uint32_t currentHour() { return currentHour_; }
    uint32_t currentDay() { return currentDay_; }
    uint32_t currentYear() { return currentYear_; }

    //! adds the given elapsed milliseconds to the current time
    uint32_t updateTime(uint32_t elapsed);

    //! Return the representation of the time in the given string
    void getTimeAsStr(std::string &timeStr);

    //! Set the duration of hours in milliseconds to be faster
    void cheatAccelerateTime() { hourDurationInMs_ = 1000; }

    //! Returns the number of days and hours from the given amount of time
    void getDayHourFromPeriod(uint32_t elapsed, uint32_t & days, uint32_t & hours);

    //! Return the difference in number of days with the given previous time
    uint32_t diffInDays(const GameTime &previousTime);

protected:
    /*! Number of millisecond for an hour in the game.*/
    static const int kDefaultHourDurationInMs;

    /*! Stores the current hour. */
    uint32_t currentHour_;
    /*! Stores the current day. */
    uint32_t currentDay_;
    /*! Stores the current year. */
    uint32_t currentYear_;
    /*! How long does an hour in millisecond. */
    uint32_t hourDurationInMs_;
    /*! Time in millisecond since the last time update.*/
    uint32_t timeElapsed_;
};

}; // end of namespace

#endif // KERNEL_MODEL_TIME_H
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

#include "fs-kernel/model/time.h"

#include <format>

namespace fs_knl {

const int GameTime::kDefaultHourDurationInMs = 4000;
const int GameTime::kNbHourInDay = 24;
const int GameTime::kNbDaysInYear = 365;

GameTime::GameTime() : hourDurationInMs_(kDefaultHourDurationInMs) {
    reset();
}

/*!
 * @brief 
 * @param year 
 * * @param day Must be less than 365
 * @param hour Must be less than 24
 */
void GameTime::setTime(uint32_t year, uint32_t day, uint32_t hour) {
    if (hour >= kNbHourInDay || day > kNbDaysInYear) {
        return;
    }
    currentHour_ = hour;
    currentDay_ = day;
    currentYear_ = year;
}

void GameTime::reset() {
    currentHour_ = 0;
    currentDay_ = 1;
    currentYear_ = 85;
    timeElapsed_ = 0;
}

/*!
 * Update the current time (hours, day, year) and return
 * the number of hours corresponding to the elapsed time in parameter
 * @param elapsed 
 * @return Elapsed time in millisecond since last call
 */
uint32_t GameTime::updateTime(uint32_t elapsed) {
    timeElapsed_ += elapsed;
    // Number of hours in elapsed time
    uint32_t hoursInElapsed = timeElapsed_ / hourDurationInMs_;
    // Number of days in that time
    uint32_t daysInElapsed = hoursInElapsed / kNbHourInDay;
    
    // Remaining milliseconds that will be counted next time this method is called
    timeElapsed_ %= hourDurationInMs_;

    // Update current hours
    currentHour_ += hoursInElapsed % kNbHourInDay;
    if (currentHour_ >= kNbHourInDay) {
        currentHour_ -= kNbHourInDay;
        currentDay_++;
    }
    // Update current day
    currentDay_ += daysInElapsed % kNbDaysInYear;
    if (currentDay_ >= kNbDaysInYear) {
        currentDay_ = 1;
        currentYear_++;
    }
    // Update current year
    currentYear_ += daysInElapsed / kNbDaysInYear;
    
    
    return hoursInElapsed;
}

void GameTime::getTimeAsStr(std::string &timeStr) {
    timeStr = std::format("{:02d}:{}:{}NC", currentHour_, currentDay_, currentYear_);
}

/*!
 * Returns the number of hours and days for the given period
 * in millisecond.
 * \param elapsed
 * \param days
 * \param hours
 */
void GameTime::getDayHourFromPeriod(uint32_t elapsed, uint32_t & days, uint32_t & hours) {
    // Total number of hours in the period
    uint32_t hour_elapsed = elapsed / hourDurationInMs_;

    days = hour_elapsed / 24;
    hours = hour_elapsed % 24;
}

/*!
 * Return the difference in number of days.
 * @param previousTime 
 * @return 0 if times are the same or if previous time is smaller
 */
uint32_t GameTime::diffInDays(const GameTime &previousTime) {
    uint32_t nbDays = 0;

    if (currentYear_ > previousTime.currentYear_) {
        uint32_t diffInYears = currentYear_ - previousTime.currentYear_;
        // First add the days to finish current years
        nbDays += kNbDaysInYear - previousTime.currentDay_;
        // Then the numbers of days in next years
        nbDays += diffInYears * currentDay_;
    } else if (currentYear_ == previousTime.currentYear_ && currentDay_ > previousTime.currentDay_) {
        nbDays = currentDay_ - previousTime.currentDay_;
    }

    return nbDays;
}

};
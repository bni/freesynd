/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "gamesession.h"
#include "mission.h"

// List of logo colours
extern int g_Colours[8];

Block g_Blocks[50] = {
    // name, defpop, population, mis_id, tax, addtotax, popStatus, daysToNextStatus, daysStatusElapsed, status, nextMission, color, infos, enhanced
    {"#CNTRY_17", 46000000, 46000000, 17, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},            // 0 - ALASKA
    {"#CNTRY_39", 56000000, 56000000, 39, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},            // 1 - NORTHWEST TERRITORIES
    {"#CNTRY_08", 58000000, 58000000, 8, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},             // 2 - NORTHEAST TERRITORIES
    {"#CNTRY_16", 40000000, 40000000, 16, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},            // 3 - GREENLAND
    {"#CNTRY_20", 54000000, 54000000, 20, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "10", 0, 0, 0},            // 4 - Scandinavia
    {"#CNTRY_18", 40000000, 40000000, 18, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "6:12:13", 0, 0, 0},       // 5 - Ural
    {"#CNTRY_22", 54000000, 54000000, 22, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "7:13", 0, 0, 0},          // 6 - Siberia
    {"#CNTRY_12", 56000000, 56000000, 12, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "0", 0, 0, 0},             // 7 - KAMCHATKA
    {"#CNTRY_21", 58000000, 58000000, 21, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},            // 8 - YUKON
    {"#CNTRY_01", 48000000, 48000000, 1, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "4", 0, 0, 0},              // 9 - West Eur.
    {"#CNTRY_15", 50000000, 50000000, 15, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "11", 0, 0, 0},            // 10 - Central Eur.
    {"#CNTRY_10", 52000000, 52000000, 10, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "5", 0, 0, 0},             // 11 - Eastern Eur.
    {"#CNTRY_09", 42000000, 42000000, 9, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "23:24", 0, 0, 0},          // 12 - KAZAKHSTAN
    {"#CNTRY_03", 52000000, 52000000, 3, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "14", 0, 0, 0},             // 13 - MONGOLIA
    {"#CNTRY_02", 42000000, 42000000, 2, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},             // 14 - FAR EAST
    {"#CNTRY_42", 44000000, 44000000, 42, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},            // 15 - 
    {"#CNTRY_05", 46000000, 46000000, 5, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},             // 16 - 
    {"#CNTRY_23", 56000000, 56000000, 23, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},            // 17 - 
    {"#CNTRY_34", 58000000, 58000000, 34, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},            // 18 - 
    {"#CNTRY_29", 40000000, 40000000, 29, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},            // 19 - 
    {"#CNTRY_28", 50000000, 50000000, 28, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},            // 20 - 
    {"#CNTRY_35", 40000000, 40000000, 35, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},            // 21 - 
    {"#CNTRY_06", 50000000, 50000000, 6, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},             // 22 - 
    {"#CNTRY_04", 52000000, 52000000, 4, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},             // 23 - Iran
    {"#CNTRY_50", 54000000, 54000000, 50, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},            // 24 - China
    {"#CNTRY_32", 40000000, 40000000, 32, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_24", 42000000, 42000000, 24, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_37", 44000000, 44000000, 37, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_41", 58000000, 58000000, 41, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_33", 44000000, 44000000, 33, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_38", 54000000, 54000000, 38, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_07", 56000000, 56000000, 7, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_48", 58000000, 58000000, 48, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_26", 44000000, 44000000, 26, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_44", 46000000, 46000000, 44, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_45", 48000000, 48000000, 45, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_27", 58000000, 58000000, 27, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_40", 48000000, 48000000, 40, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_14", 58000000, 58000000, 14, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_36", 40000000, 40000000, 36, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_46", 42000000, 42000000, 46, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_31", 48000000, 48000000, 31, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_13", 46000000, 46000000, 13, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_11", 48000000, 48000000, 11, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_19", 42000000, 42000000, 19, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_43", 48000000, 48000000, 43, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_49", 58000000, 58000000, 49, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_30", 40000000, 40000000, 30, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_47", 58000000, 58000000, 47, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0},
    {"#CNTRY_25", 48000000, 48000000, 25, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0}
};

int g_syndicate_color_id[7];

const int GameSession::HOUR_DELAY = 4000;
const int GameSession::NB_MISSION = 50;

GameSession::GameSession() {
    mission_ = NULL;
    enable_all_mis_ = false;
    replay_mission_ = false;
    hour_delay_ = HOUR_DELAY;
}

GameSession::~GameSession() {
    destroy();
}

void GameSession::destroy() {
    if (mission_) {
        delete mission_;
        mission_ = NULL;
    }
}

bool GameSession::reset() {
    destroy();

    // Init default colors for enemy syndicates
    g_syndicate_color_id[0] = 7;
    g_syndicate_color_id[1] = 14;
    g_syndicate_color_id[2] = 3;
    g_syndicate_color_id[3] = 11;
    g_syndicate_color_id[4] = 12;
    g_syndicate_color_id[5] = 13;
    g_syndicate_color_id[6] = 15;

    // Init default value for player
    logo_ = 0;
    logo_colour_ = 6;
    company_name_.clear();
    username_.clear();
    money_ = 30000;

    // this array contains the initial number of countries for each enemy syndicate
    int map_count [7] = {7, 7, 7, 7, 7, 7, 8};
    // init the map
    for (int i=0; i<NB_MISSION; i++) {
        g_Blocks[i].status = enable_all_mis_ ? BLK_AVAIL : BLK_UNAVAIL;
        g_Blocks[i].tax = 30;
        g_Blocks[i].addToTax = 0;
        g_Blocks[i].popStatus = STAT_VERY_HAPPY;
        g_Blocks[i].population = g_Blocks[i].defPopulation;
        g_Blocks[i].daysToNextStatus = 0;
        g_Blocks[i].daysStatusElapsed = 0;

        // Find a enemy syndicate as owner
        bool found_owner = false;
        do {
            int index = rand() % 7;
            if (map_count[index] > 0) {
                g_Blocks[i].syndicate_owner = index;
                map_count[index] -= 1;
                found_owner = true;
            }
        } while (!found_owner);

        // Reset briefing information
        g_Blocks[i].infoLevel = 0;
        g_Blocks[i].enhanceLevel = 0;
    }

    // By default, West Europe is the first playable mission
    selected_blck_ = 9;
    g_Blocks[selected_blck_].status = BLK_AVAIL;

    time_hour_ = 0;
    time_day_ = 1;
    time_year_ = 85;
    time_elapsed_ = 0;

    return researchMan_.reset();
}

Block & GameSession::getBlock(uint8 index) {
    return g_Blocks[index];
}

Block & GameSession::getSelectedBlock() {
    return g_Blocks[selected_blck_];
}

/*!
 * Returns the color of the given block depending on its owner.
 * If the block is finished then the owner is the player
 * else it's the syndicate given by Block.syndicate_owner.
 * \param blk The block to find the color.
 */
uint8 GameSession::get_owner_color(Block & blk) {
    switch (blk.status) {
    case BLK_FINISHED:
    case BLK_REBEL:
        return getLogoColour();
    default:
        return g_syndicate_color_id[blk.syndicate_owner];
    }
}

/*!
 * The player has changed color. It's already an enemy syndicate color
 * so invert colors.
 */
void GameSession::exchange_color_wt_syndicate(uint8 new_color) {
    for (int i=0; i<7; i++) {
        if (g_syndicate_color_id[i] == new_color) {
            g_syndicate_color_id[i] = getLogoColour();
            break;
        }
    }
    setLogoColour(new_color);
}

void GameSession::setMission(Mission *pMission) {
    if ((pMission == NULL && mission_) || pMission != mission_) {
        if (mission_) {
            delete mission_;
        }
        mission_ = pMission;
    }
}

/*!
 * Called when user finishes the current mission.
 * The block cannot be played again, a tax is set
 * and next missions are made available.
 */
void GameSession::mark_selected_block_completed() {
    g_Blocks[selected_blck_].status = BLK_FINISHED;
    g_Blocks[selected_blck_].popStatus = STAT_VERY_HAPPY;
    g_Blocks[selected_blck_].syndicate_owner = 0;
}

void GameSession::cheatEnableAllMission() { 
    enable_all_mis_ = true; 

    for (int i=0; i<NB_MISSION; i++) {
        if (g_Blocks[i].status == BLK_UNAVAIL) {
            g_Blocks[i].status = BLK_AVAIL;
        }
    }
}

/*!
 * Returns the number of hours and days for the given period 
 * in millisecond.
 * \param elapsed
 * \param days
 * \param hours
 */
void GameSession::getDayHourFromPeriod(int elapsed, int & days, int & hours) {
    // Total number of hours in the period
    int hour_elapsed = elapsed / hour_delay_;

    days = hour_elapsed / 24;
    hours = hour_elapsed % 24;
}

/*!
 * Updates the game time based on the given elapsed millisecond since
 * the last update.
 * For every day passed, it calls the updateCountries() method and update
 * the user's amount of money.
 * \param elapsed The number of millisecond since the last update. Set to -1
 * to reset the counter.
 * \return True if time has changed.
 */
bool GameSession::updateTime(int elapsed) {
    
    if (elapsed == -1) {
        time_elapsed_ = 0;
        return false;
    }

    time_elapsed_ += elapsed;
    if (time_elapsed_ > hour_delay_) {
        // Computes how much hours have passed
        int hour_elapsed = time_elapsed_ / hour_delay_;
        // Reset the counter
        time_elapsed_ = 0;
        // Number of days in that time
        int day_elapsed = hour_elapsed / 24;
        int hour_remain = hour_elapsed % 24;

        // Hour update
        time_hour_ += hour_remain;
        // Update research
        money_ -= researchMan_.process(hour_remain, money_);
        // Collect taxes if we reached the end of the day
        if (time_hour_ > 23) {
            time_hour_ -= 24;
            time_day_++;

            if (time_day_ > 365) {
                time_day_ = 1;
                time_year_++;
            }

            // Update money
            money_ += updateCountries();
        }

        if (day_elapsed != 0) {
            for (int i=0; i<day_elapsed; i++) {
                time_day_++;
                if (time_day_ > 365) {
                    time_day_ = 1;
                    time_year_++;
                }

                // Update research for a day
                money_ -= researchMan_.process(24, money_);
                // Update money
                money_ += updateCountries();
            }
        }

        return true;
    }

    return false;
}

/*!
 * Returns the current game time as a string.
 * \param dest Buffer that will contains the resulting string.
 */
void GameSession::getTimeAsStr(char *dest) {
    sprintf(dest, "%02d:%d:%dNC", time_hour_, time_day_, time_year_);
}

/*!
 * Returns the revenue of the given tax rate on the given population.
 * \param population The population number
 * \param rate The current tax rate (0<=rate<=100)
 * \return The resulting amount of money
 */
int GameSession::getTaxRevenue(int population, int rate) {
    // This formula is not exactly the same as in the original game
    int amount = ((population / 1000000 + 1) * 1375 * rate) / 1000;

    return amount;
}

/*!
 * Increase/decrease the current amount of tax that will be added to the tax rate
 * at the end of the day.
 * \param amount The amount to add to tax rate.
 * \return true if rate has changed.
 */
bool GameSession::addToTaxRate(int amount) {
    int newRate = g_Blocks[selected_blck_].tax + g_Blocks[selected_blck_].addToTax + amount;
    
    if (newRate <= 100 && newRate >= 0) {
        g_Blocks[selected_blck_].addToTax += amount;
        return true;
    }

    return false;
}

/*!
 * Returns the number of days before status changes.
 * \param status The current population status
 * \param tax The current tax rate
 * \return a number of days
 */
int GameSession::getDaysBeforeChange(Status_Pop status, int tax) {

    if (tax <= 30) {
        // Below 30% status will increase slowly : every 15 days
        return 15;
    } else if (tax >= 90) {
        // Above 90%, status will decrease every 1 or 2 days
        return 1 + (rand() % 2);
    }

    // Between 30% and 90%, period is a pourcentage of a base number of days
    // plus a randow variation between -1 to +1 days
    int base = 12;
    int delta = 1 - (rand() % 2);
    int days = (100 - tax) * base / 100 + delta;

    // Period cannot be zero
    if (days <= 0) {
        days = 1;
    }

    // There's a plateau between status CONTENT and UNHAPPY between 1 and 2 days
    if (status == STAT_CONTENT) {
        days = days + 1 + (rand() % 2);
    }

    return days;
}

int GameSession::getNewPopulation(const int defaultPop, int currPop) {

    // Population variation is between 100 and 300 people / day
    int add = 100 + (rand() % 200);
    // 2/5 chances that population decreases and 3/5 that it increases
    int op = (rand() % 5) > 1 ? 1 : -1;
    add *= op;

    if (((defaultPop + 2000) < (currPop + add)) || ((defaultPop - 1000) > (currPop + add))) {
        // If we're out of limit add the opposite of variation
        return currPop - add;
    } else {
        // else add variation
        return currPop + add;
    }
}

/*!
 * Updates population number and status for all countries on a daily basis.
 * \return The amount of money collected after all countries have been updated
 */
int GameSession::updateCountries() {
    int amount = 0;

    for (int i=0; i < 50; i++) {
        // Update country population number
        g_Blocks[i].population = getNewPopulation(g_Blocks[i].defPopulation, g_Blocks[i].population);

        if (g_Blocks[i].status == BLK_FINISHED) {
            // update the population status
            if (g_Blocks[i].daysStatusElapsed < g_Blocks[i].daysToNextStatus) {
                g_Blocks[i].daysStatusElapsed += 1;
                
                if (g_Blocks[i].daysStatusElapsed == g_Blocks[i].daysToNextStatus) {
                    // We have reached the limit -> Change population status
                    // reset the counter
                    g_Blocks[i].daysStatusElapsed = 0;
                    g_Blocks[i].daysToNextStatus = getDaysBeforeChange(g_Blocks[i].popStatus, g_Blocks[i].tax);

                    // Above 30% status will be down
                    bool down = g_Blocks[i].tax > 30;

                    switch (g_Blocks[i].popStatus) {
                        case STAT_VERY_HAPPY:
                            if (down) {
                                g_Blocks[i].popStatus = STAT_HAPPY;
                            }
                            break;
                        case STAT_HAPPY:
                            if (down) {
                                g_Blocks[i].popStatus = STAT_CONTENT;
                            } else {
                                g_Blocks[i].popStatus = STAT_VERY_HAPPY;
                                // We've reached the max status so no more evolution
                                g_Blocks[i].daysToNextStatus = 0;
                            }
                            break;
                        case STAT_CONTENT:
                            if (down) {
                                g_Blocks[i].popStatus = STAT_UNHAPPY;
                            } else {
                                g_Blocks[i].popStatus = STAT_HAPPY;
                            }
                            break;
                        case STAT_UNHAPPY:
                            if (down) {
                                g_Blocks[i].popStatus = STAT_DISCONTENT;
                            } else {
                                g_Blocks[i].popStatus = STAT_CONTENT;
                            }
                            break;
                        case STAT_DISCONTENT:
                            if (down) {
                                g_Blocks[i].popStatus = STAT_REBEL;
                                g_Blocks[i].status = BLK_REBEL;
                                g_Blocks[i].daysToNextStatus = 0;
                            } else {
                                g_Blocks[i].popStatus = STAT_UNHAPPY;
                            }
                            break;
                        default:
                            break;
                    } // end switch
                }
            }

            // Status update has been made : check if money can be percieved
            if (g_Blocks[i].status == BLK_FINISHED) {
                // Status has not changed so take the money
                amount += getTaxRevenue(g_Blocks[i].population, g_Blocks[i].tax);
            }

            // Adds the tax buffer to the current tax rate and computes evolution
            if (g_Blocks[i].addToTax != 0) {
                int newTax = g_Blocks[i].tax + g_Blocks[i].addToTax;

                if (newTax <= 30) {
                    if (g_Blocks[i].tax > 30) {
                        // Tax is now below 30 so stop going down and slowly begin to go up
                        g_Blocks[i].daysStatusElapsed = 0;
                        // It always take 15 days to gain one level up of status
                        g_Blocks[i].daysToNextStatus = 15;
                    }
                } else {
                    if (g_Blocks[i].tax <= 30) {
                        // Tax is now above 30 : stops going up and begin going down
                        g_Blocks[i].daysStatusElapsed = 0;
                        g_Blocks[i].daysToNextStatus = getDaysBeforeChange(g_Blocks[i].popStatus, newTax);
                    } else {
                        // Tax was already above 30
                        g_Blocks[i].daysToNextStatus = getDaysBeforeChange(g_Blocks[i].popStatus, newTax);
                        if (g_Blocks[i].daysToNextStatus <= g_Blocks[i].daysStatusElapsed) {
                            // new limit is already below current elapsed days so put
                            // elapsed day just below limit
                            g_Blocks[i].daysStatusElapsed = g_Blocks[i].daysToNextStatus - 1;
                        }
                    }
                }

                g_Blocks[i].tax = newTax;
                g_Blocks[i].addToTax = 0;
            }
        }
    }

    return amount;
}

//! Save instance to file
bool GameSession::saveToFile(PortableFile &file) {
    // Company name
    file.write_string(company_name_, 16);
    // User name
    file.write_string(username_, 16);
    // Logo
    file.write32(logo_);
    // Logo colour
    file.write32(logo_colour_);
    // Money
    file.write32(money_);
    // Time
    file.write32(time_year_);
    file.write32(time_day_);
    file.write32(time_hour_);

    // Missions
    for (int i=0; i<GameSession::NB_MISSION; i++) {
        file.write32(g_Blocks[i].population);
        file.write32(g_Blocks[i].tax);
        file.write32(g_Blocks[i].popStatus);
        file.write32(g_Blocks[i].daysToNextStatus);
        file.write32(g_Blocks[i].daysStatusElapsed);
        file.write32(g_Blocks[i].status);
        // NOTE : before 1.2 we were saving the block color
        file.write8(g_Blocks[i].syndicate_owner);
        file.write8(g_Blocks[i].infoLevel);
        // NOTE: format version 1.0 had a bug where infoLevel was written again instead of enhanceLevel.
        file.write8(g_Blocks[i].enhanceLevel);
    }

    return true;
}

//! Load instance from file
bool GameSession::loadFromFile(PortableFile &infile, const FormatVersion& v) {
    // Read company name
    company_name_ = infile.read_string((v == 0x0100) ? 17 : 16, true);
    // Read user name
    username_ = infile.read_string((v == 0x0100) ? 17 : 16, true);

    // Read logo id
    logo_ = infile.read32();
    // Read logo colour
    logo_colour_ = infile.read32();
    // Read money
    money_ = infile.read32();
    // Read time
    time_year_ = infile.read32();
    time_day_ = infile.read32();
    time_hour_ = infile.read32();

    // Missions
    for (int i=0; i<GameSession::NB_MISSION; i++) {
        g_Blocks[i].population = infile.read32();
        g_Blocks[i].tax = infile.read32();
        int ival = infile.read32();
        switch (ival) {
            case 0: g_Blocks[i].popStatus = STAT_REBEL;break;
            case 1: g_Blocks[i].popStatus = STAT_DISCONTENT;break;
            case 2: g_Blocks[i].popStatus = STAT_UNHAPPY;break;
            case 3: g_Blocks[i].popStatus = STAT_CONTENT;break;
            case 4: g_Blocks[i].popStatus = STAT_HAPPY;break;
            case 5: g_Blocks[i].popStatus = STAT_VERY_HAPPY;break;
            default: g_Blocks[i].popStatus = STAT_VERY_HAPPY;break;
        }

        g_Blocks[i].daysToNextStatus = infile.read32();
        g_Blocks[i].daysStatusElapsed = infile.read32();

        // Read status
        ival = infile.read32();
        switch (ival) {
            case 0: g_Blocks[i].status = BLK_UNAVAIL;break;
            case 1: g_Blocks[i].status = BLK_AVAIL;break;
            case 2: g_Blocks[i].status = BLK_FINISHED;break;
            case 3: g_Blocks[i].status = BLK_REBEL;break;
            default: g_Blocks[i].status = BLK_UNAVAIL;break;
        }

        // Read owner (before 1.2 it was color)
        g_Blocks[i].syndicate_owner = 0;
        uint8 value = infile.read8();
        if (v.majorVersion() == 1 && v.minorVersion() < 2) {
            if (g_Blocks[i].status != BLK_FINISHED && g_Blocks[i].status != BLK_REBEL) {
                // The block is not owned by the player
                // so assigned it to the syndicate with the read color
                for (int s_idx=0; s_idx<7; s_idx++) {
                    if (value == g_syndicate_color_id[s_idx]) {
                        g_Blocks[i].syndicate_owner = s_idx;
                    }
                }
            }
        } else {
            // We read the owner
            g_Blocks[i].syndicate_owner = value;
        }

        g_Blocks[i].infoLevel = infile.read8();
        g_Blocks[i].enhanceLevel = infile.read8();
    }

    return true;
}

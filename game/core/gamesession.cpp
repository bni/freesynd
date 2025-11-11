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

 #include "gamesession.h"

#include <stdlib.h>
#include <stdio.h>

#include "fs-engine/gfx/logomanager.h"

Block g_Blocks[50] = {
    // name,      defpop,   popul,    mis_id, tax, addtotax, popStatus, daysToNextStatus, daysStatusElapsed, status, nextMission, color, infos, enhanced
    {"#CNTRY_17", 46000000, 46000000, 17, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 3},            // 0 - ALASKA
    {"#CNTRY_39", 56000000, 56000000, 39, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 5},            // 1 - NORTHWEST TERRITORIES
    {"#CNTRY_08", 58000000, 58000000, 8, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 4},             // 2 - NORTHEAST TERRITORIES
    {"#CNTRY_16", 40000000, 40000000, 16, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 2},            // 3 - GREENLAND
    {"#CNTRY_20", 54000000, 54000000, 20, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "10", 0, 0, 0, 1},            // 4 - Scandinavia
    {"#CNTRY_18", 40000000, 40000000, 18, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "6:12:13", 0, 0, 0, 4},       // 5 - Ural
    {"#CNTRY_22", 54000000, 54000000, 22, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "7:13", 0, 0, 0, 3},          // 6 - Siberia
    {"#CNTRY_12", 56000000, 56000000, 12, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "0", 0, 0, 0, 3},             // 7 - KAMCHATKA
    {"#CNTRY_21", 58000000, 58000000, 21, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 2},            // 8 - YUKON
    {"#CNTRY_01", 48000000, 48000000, 1, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "4", 0, 0, 0, 2},              // 9 - West Eur.
    {"#CNTRY_15", 50000000, 50000000, 15, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "11", 0, 0, 0, 1},            // 10 - Central Eur.
    {"#CNTRY_10", 52000000, 52000000, 10, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "5", 0, 0, 0, 1},             // 11 - Eastern Eur.
    {"#CNTRY_09", 42000000, 42000000, 9, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "23:24", 0, 0, 0, 5},          // 12 - KAZAKHSTAN
    {"#CNTRY_03", 52000000, 52000000, 3, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, "14", 0, 0, 0, 4},             // 13 - MONGOLIA
    {"#CNTRY_02", 42000000, 42000000, 2, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 3},             // 14 - FAR EAST
    {"#CNTRY_42", 44000000, 44000000, 42, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 3},            // 15 - New found land
    {"#CNTRY_05", 46000000, 46000000, 5, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 1},          // 16 - California
    {"#CNTRY_23", 56000000, 56000000, 23, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 4},            // 17 - Rockies
    {"#CNTRY_34", 58000000, 58000000, 34, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 5},            // 18 - Midwest
    {"#CNTRY_29", 40000000, 40000000, 29, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 5},            // 19 - New England
    {"#CNTRY_28", 50000000, 50000000, 28, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 4},            // 20 - Algeria
    {"#CNTRY_35", 40000000, 40000000, 35, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 1},            // 21 - Lybia
    {"#CNTRY_06", 50000000, 50000000, 6, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 2},             // 22 - Irak
    {"#CNTRY_04", 52000000, 52000000, 4, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 5},          // 23 - Iran
    {"#CNTRY_50", 54000000, 54000000, 50, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 1},            // 24 - China
    {"#CNTRY_32", 40000000, 40000000, 32, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 3},             // 25 - Colorado
    {"#CNTRY_24", 42000000, 42000000, 24, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 5},             //26 - Southern state
    {"#CNTRY_37", 44000000, 44000000, 37, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 3},             // 27 - Atlantic Accelerator
    {"#CNTRY_41", 58000000, 58000000, 41, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 2},             // Mauritania
    {"#CNTRY_33", 44000000, 44000000, 33, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 4},             // Sudan
    {"#CNTRY_38", 54000000, 54000000, 38, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 4},             // Arabia
    {"#CNTRY_07", 56000000, 56000000, 7, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 1},          // India
    {"#CNTRY_48", 58000000, 58000000, 48, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 4},             // Pacific Rim
    {"#CNTRY_26", 44000000, 44000000, 26, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 2},             // Mexico
    {"#CNTRY_44", 46000000, 46000000, 44, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 5},             // Colombia
    {"#CNTRY_45", 48000000, 48000000, 45, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 1},             // Nigeria
    {"#CNTRY_27", 58000000, 58000000, 27, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 3},             // Zaire
    {"#CNTRY_40", 48000000, 48000000, 40, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 1},             // Kenya
    {"#CNTRY_14", 58000000, 58000000, 14, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 5},             // Peru
    {"#CNTRY_36", 40000000, 40000000, 36, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 2},             // Venezuela
    {"#CNTRY_46", 42000000, 42000000, 46, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 2},             // Brazil
    {"#CNTRY_31", 48000000, 48000000, 31, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 2},             // South Africa
    {"#CNTRY_13", 46000000, 46000000, 13, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 4},             // Mozambique
    {"#CNTRY_11", 48000000, 48000000, 11, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 2},             // Western Australia
    {"#CNTRY_19", 42000000, 42000000, 19, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 5},             // Northern Territories
    {"#CNTRY_43", 48000000, 48000000, 43, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 4},             // New South Wales
    {"#CNTRY_49", 58000000, 58000000, 49, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 5},             // Paraguay
    {"#CNTRY_30", 40000000, 40000000, 30, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 1},             // Argentina
    {"#CNTRY_47", 58000000, 58000000, 47, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 3},             // Uruguay
    {"#CNTRY_25", 48000000, 48000000, 25, 30, 0, STAT_VERY_HAPPY, 0, 0, BLK_UNAVAIL, NULL, 0, 0, 0, 1}              // Indonesia
};

/*! 
 * This array contains the index in the menu palette
 * of the possible colors for logos and countries.
 */
int g_syndicate_color_id[7];

//! The maximum number of syndicate in the game
constexpr int kMaxSyndicate { 8 };

const int GameSession::NB_MISSION = 50;
const int GameSession::kNameMaxSize = 16;

GameSession::GameSession(fs_knl::WeaponManager *pWeaponManager, fs_knl::ModManager *pModManager) : 
        researchMan_(pWeaponManager, pModManager),
        rng_(std::random_device{}()),
        syndicateDist_(0, kMaxSyndicate - 2) {
    enable_all_mis_ = false;
    replay_mission_ = false;
}

GameSession::~GameSession() {}

bool GameSession::reset() {
    // Init default colors for enemy syndicates
    g_syndicate_color_id[0] = 1;
    g_syndicate_color_id[1] = 2;
    g_syndicate_color_id[2] = 3;
    g_syndicate_color_id[3] = 4;
    g_syndicate_color_id[4] = 5;
    g_syndicate_color_id[5] = 6;
    g_syndicate_color_id[6] = 7;

    // Init default value for player
    logo_ = 0;
    logo_colour_ = 0;
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
            uint8_t index = getRandomEnemySyndicateId();
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

    currentTime_.reset();

    return researchMan_.reset();
}

uint8_t GameSession::getRandomEnemySyndicateId() {
    return static_cast<uint8_t>(syndicateDist_(rng_));
}

Block & GameSession::getBlock(int index) {
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
        return g_LogoMgr.getColorAtIndex(getLogoColour());
    default:
        return g_LogoMgr.getColorAtIndex(g_syndicate_color_id[blk.syndicate_owner]);
    }
}

void GameSession::setLogoColour(int newColour) {
    // we must check in the map if the new player color
    // has not already been assigned to an enemy syndicate
    // => if true, invert colors.
    for (int i=0; i<7; i++) {
        if (g_syndicate_color_id[i] == newColour) {
            g_syndicate_color_id[i] = logo_colour_;
            break;
        }
    }
    // change user color
    logo_colour_ = newColour;
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
uint32_t GameSession::updateCountries() {
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
bool GameSession::saveToFile(fs_utl::PortableFile &file) {
    // Company name
    file.write_string(company_name_, kNameMaxSize);
    // User name
    file.write_string(username_, kNameMaxSize);
    // Logo
    file.write32(static_cast<uint32_t>(logo_));
    // Logo colour
    file.write32(static_cast<uint32_t>(logo_colour_));
    // Money
    file.write32(money_);
    // Time
    file.write32(currentTime_.currentYear());
    file.write32(currentTime_.currentDay());
    file.write32(currentTime_.currentHour());

    // Missions
    for (int i=0; i<GameSession::NB_MISSION; i++) {
        file.write32(static_cast<uint32_t>(g_Blocks[i].population));
        file.write32(static_cast<uint32_t>(g_Blocks[i].tax));
        file.write32(g_Blocks[i].popStatus);
        file.write32(static_cast<uint32_t>(g_Blocks[i].daysToNextStatus));
        file.write32(static_cast<uint32_t>(g_Blocks[i].daysStatusElapsed));
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
bool GameSession::loadFromFile(fs_utl::PortableFile &infile, const fs_utl::FormatVersion& v) {
    // Read company name
    company_name_ = infile.read_string((v == 0x0100) ? 17 : kNameMaxSize, true);
    // Read user name
    username_ = infile.read_string((v == 0x0100) ? 17 : kNameMaxSize, true);

    // Read logo id
    logo_ = infile.reads32();
    // Read logo colour : before 1.3, we stored directly the color value
    if (v.majorVersion() == 1 && v.minorVersion() < 3) {
        int32_t color = infile.reads32();
        // So find the index of the color now
        for (int i = 0; i < g_LogoMgr.kMaxColour; i++) {
            if (g_LogoMgr.getColorAtIndex(i) == color) {
                logo_colour_ = i;
                break;
            }
        }
    } else {
        logo_colour_ = infile.reads32();
    }

    // Read money
    money_ = infile.read32();
    // Read time
    currentTime_.setTime(infile.read32(), infile.read32(), infile.read32());

    // Missions
    for (int i=0; i<GameSession::NB_MISSION; i++) {
        g_Blocks[i].population = infile.reads32();
        g_Blocks[i].tax = infile.reads32();
        int ival = infile.reads32();
        switch (ival) {
            case 0: g_Blocks[i].popStatus = STAT_REBEL;break;
            case 1: g_Blocks[i].popStatus = STAT_DISCONTENT;break;
            case 2: g_Blocks[i].popStatus = STAT_UNHAPPY;break;
            case 3: g_Blocks[i].popStatus = STAT_CONTENT;break;
            case 4: g_Blocks[i].popStatus = STAT_HAPPY;break;
            case 5: g_Blocks[i].popStatus = STAT_VERY_HAPPY;break;
            default: g_Blocks[i].popStatus = STAT_VERY_HAPPY;break;
        }

        g_Blocks[i].daysToNextStatus = infile.reads32();
        g_Blocks[i].daysStatusElapsed = infile.reads32();

        // Read status
        ival = infile.reads32();
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
                for (uint8_t s_idx=0; s_idx<7; s_idx++) {
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

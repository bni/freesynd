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

#ifndef CORE_GAME_SESSION_H
#define CORE_GAME_SESSION_H

#include <string>
#include <set>

#include "../common.h"
#include "core/researchmanager.h"
#include "utils/portablefile.h"

enum Status_Pop {
    STAT_VERY_HAPPY = 5,
    STAT_HAPPY = 4,
    STAT_CONTENT = 3,
    STAT_UNHAPPY = 2,
    STAT_DISCONTENT = 1,
    STAT_REBEL = 0
};

enum Status_Blk {
    BLK_UNAVAIL = 0,
    BLK_AVAIL = 1,
    BLK_FINISHED = 2,
    BLK_REBEL = 3
};

typedef struct Block_ {
    const char *name;
    /*! Default population number.*/
    int defPopulation;
    /*! Current population number.*/
    int population;
    int mis_id;
    int tax;
    /*!
     * A temporary buffer to holds tax update.
     * Real tax rate is updated with this buffer every day.
     */
    int addToTax;
    /*!
     * Status of the population satisfaction.
     */
    Status_Pop popStatus;
    /*!
     * Number of days before changing statuts.
     */
    int daysToNextStatus;
    /*!
     * Number of days elapsed for the current status
     */
    int daysStatusElapsed;
    /**
     * Tells whether the mission can be played or is finished.
     */
    Status_Blk status;
    /*! The list of blocks available after finishing this mission.*/
    const char *next;
    /*! This is the index in the enemy syndicate array to get the color.
     This field is used if status field is BLK_UNAVAIL or BLK_AVAIL.*/
    uint8 syndicate_owner;
    /*! Informations level for briefing. */
    unsigned char infoLevel;
    /*! Details level for briefing. */
    unsigned char enhanceLevel;
} Block;

class Mission;
class Agent;

/*!
 * A holder for player data.
 * This class stores all user data.
 */
class GameSession : public Singleton<GameSession> {
public:
    /*! Number of millisecond for an hour in the game.*/
    static const int HOUR_DELAY;
    /*! Total number of missions. */
    static const int NB_MISSION;

    GameSession();
    ~GameSession();

    /*!
     * Resets all data.
     * \returns True if reset has succeeded
     */
    bool reset();

    /*!
     * Returns the id of the user logo.
     */
    int getLogo() const {
        return logo_;
    }

    /*!
     * Sets the id of the user's logo.
     */
    void setLogo(int new_logo) {
        logo_ = new_logo;
    }

    /*!
     * Returns the logo colour.
     */
    int getLogoColour() const {
        return logo_colour_;
    }

    /*!
     * Sets the colour of the user's logo.
     */
    void setLogoColour(int colour) {
        logo_colour_ = colour;
    }

    /*!
     * Returns the user's company name.
     */
    const char *getCompanyName() const {
        return company_name_.c_str();
    }

    /*!
     * Sets the user's company name.
     */
    void setCompanyName(const char *name) {
        company_name_ = name;
    }

    /*!
     * Returns the user's name.
     */
    const char *getUserName() const {
        return username_.c_str();
    }

    /*!
     * Sets the user's name.
     */
    void setUserName(const char *name) {
        username_ = name;
    }

    /*!
     * Returns the user's money.
     */
    int getMoney() const {
        return money_;
    }

    /*!
     * Sets the user's money.
     */
    void setMoney(int m) {
        money_ = m;
    }

    void increaseMoney(int amount) {
        money_ += amount;
    }

    void decreaseMoney(int amount) {
        money_ -= amount;
        if (money_ < 0) {
            money_ = 0;
        }
    }

    bool canAfford(int amount) {
        return money_ >= amount;
    }

    ResearchManager &researchManager() {
        return researchMan_;
    }

    //! Sets the representation of the time in the given string
    void getTimeAsStr(char *dest);

    //! Returns the Block at the given index.
    Block & getBlock(uint8 index);

    /*!
     * Returns the index of the current selected region on map menu.
     */
    uint8 getSelectedBlockId() { return selected_blck_; }

    //! Convenience method to get the selected block
    Block & getSelectedBlock();

    /*!
     * Sets the index of the current selected region on map menu.
     * \param index The region index (between 0 and 49 inclusive)
     */
    void setSelectedBlockId(uint8 index) { if (index < 50) selected_blck_ = index; }

    //! Returns the block's color depending on who owns it
    uint8 get_owner_color(Block & blk);

    //! Change the player color for the enemy syndicate color.
    void exchange_color_wt_syndicate(uint8 player_color);

    /*!
     * Return the currently played mission.
     * \return Can be NULL.
     */
    Mission * getMission() { return mission_; }

    /*!
     * Sets the mission.
     * \param pMission The mission the user will play.
     */
    void setMission(Mission *pMission);

    //! Update state when finishing a mission
    void mark_selected_block_completed();

    //! Cheat method to enable all missions
    void cheatEnableAllMission();

    //! Cheat method to replay finished mission
    void cheatReplayMission() { replay_mission_ = true; }

    //! Tells if cheat mode Replay missions is on
    bool canReplayMission() { return replay_mission_; }

    void cheatAccelerateTime() { hour_delay_ = 1000; }

    //! Do all time related updates
    bool updateTime(int elapsed);

    //! Returns the number of days and hours from the given amount of time
    void getDayHourFromPeriod(int elapsed, int & days, int & hours);

    //! Adds the given amount to the selected block tax rate.
    bool addToTaxRate(int amount);

    //! Returns a revenue for a given population and rate.
    int getTaxRevenue(int population, int rate);

    //! Save instance to file
    bool saveToFile(PortableFile &file);
    //! Load instance from file
    bool loadFromFile(PortableFile &infile, const FormatVersion& v);

private:
    //! Destroy GameSession resources
    void destroy();
    int getDaysBeforeChange(Status_Pop status, int tax);
    //! Update population, status and returns money
    int updateCountries();
    //! Returns new population number
    int getNewPopulation(const int defaultPop, int currPop);

private:
    int logo_;
    int logo_colour_;
    int money_;
    std::string company_name_;
    std::string username_;
    /*! Stores the current hour. */
    int time_hour_;
    /*! Stores the current day. */
    int time_day_;
    /*! Stores the current year. */
    int time_year_;
    /*! Time in millisecond since the last time update.*/
    int time_elapsed_;
    /*! How long does an hour in millisecond. */
    int hour_delay_;
    /*!
     * Stores the index of the current selected
     * region on the mission map.
     */
    uint8 selected_blck_;
    /*!
     * Currently played mission.
     */
    Mission *mission_;
    /*! Cheat flag to tell that all missions are playable.*/
    bool enable_all_mis_;
    /*! Cheat flag to enable replay of finished missions. */
    bool replay_mission_;
    /*! Manager for researches. */
    ResearchManager researchMan_;
};

#define g_Session   GameSession::singleton()

#endif //CORE_GAME_SESSION_H

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2012  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef CORE_MISSIONBRIEFING_H_
#define CORE_MISSIONBRIEFING_H_

#include <string>

#include "common.h"
#include "model/leveldata.h"

#define MAX_INFOS   10
#define MAX_ENHT    10

class Map;
class MiniMap;

/*!
 * MissionBriefing contains descriptive elements of a mission :
 * - briefing text
 * - number of avalaible info/enhancement and their prices
 * - mission minimap
 * Mission briefing is used in the Briefing menu.
 */
class MissionBriefing {
 public:
    /*! Maximum number of informations for a mission.*/
    static const int kMaxInfos;
    /*! Maximum number of enhancements for a mission.*/
    static const int kMaxEnht;

    /*! Class constructor.*/
    MissionBriefing();
    /*! class destructor.*/
    ~MissionBriefing();

    //! Loads briefing from the given file
    bool loadBriefing(uint8 * missData, int size);
    //! Init the minimap and minimap overlay
    void init_minimap(Map *p_map, LevelData::LevelDataAll &level_data);

    //! Returns the number of available informations
    int nb_infos() { return i_nb_infos_; }

    //! Returns the number of available enhancements
    int nb_enhts() { return i_nb_enhts_; }

    /*!
     * Returns the cost of given info.
     * \param infoIdx informations index.
     * \return If index is greater than current number of information
     * returns 0.
     */
    int infoCost(int infoIdx) {
        if (infoIdx < i_nb_infos_) {
            return a_info_costs_[infoIdx];
        }
        return 0;
    }

    /*!
     * Returns the cost of given enhancement.
     * \param infoIdx enhancement index.
     * \return If index is greater than current number of enhancement
     * returns 0.
     */
    int enhanceCost(int enhIdx) {
        if (enhIdx < i_nb_enhts_) {
            return a_enhts_costs_[enhIdx];
        }
        return 0;
    }

    /*!
     * Returns the mission briefing as text for the given level.
     * \param infoLvl Value of 0 is the default information.
     * \return empty string if level is greater than available infos.
     */
    const char * briefing(uint8 infoLvl) { 
        return infoLvl <= i_nb_infos_ ?a_briefing_[infoLvl].c_str() : ""; 
    }

    //! Returns the minimap
    MiniMap * minimap() { return p_minimap_; }

    //! Tells what is on the given tile
    uint8 getMinimapOverlay(int x, int y);

 private:
    /*! Briefing text is stored in an array. Each entry is the text for
     * the current level of information.*/
    std::string a_briefing_[MAX_INFOS];
    /*! The total number of available infos.*/
    int i_nb_infos_;
    /*! An array of prices for each available informations for the mission.*/
    int a_info_costs_[MAX_INFOS];
    /*! The total number of available enhancements.*/
    int i_nb_enhts_;
    /*! An array of prices for each available enhancements for the mission.*/
    int a_enhts_costs_[MAX_ENHT];
    /*! The minimap displayed in the brief menu.*/
    MiniMap *p_minimap_;
    /*! This grid holds position of agents on the minimap.*/
    uint8 minimap_overlay_[128*128];
};

#endif  // CORE_MISSIONBRIEFING_H_

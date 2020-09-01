/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2012  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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
#include <string>

#include "core/missionbriefing.h"
#include "map.h"

const int MissionBriefing::kMaxInfos = MAX_INFOS;
const int MissionBriefing::kMaxEnht = MAX_ENHT;

MissionBriefing::MissionBriefing() {
    i_nb_infos_ = 0;
    i_nb_enhts_ = 0;
    p_minimap_ = NULL;

    for (int i = 0; i < kMaxInfos; i++) {
        a_info_costs_[i] = 0;
    }

    for (int i = 0; i < kMaxEnht; i++) {
        a_enhts_costs_[i] = 0;
    }
}

MissionBriefing::~MissionBriefing() {
    delete p_minimap_;
}

/*!
 * Loads briefing text, infos and enhancement from a mission file.
 * The file is divided in three sections : infos, enhancement and briefing.
 * Each section is separated by the '|' character.
 * Each string in a section is separated with an EOL.
 * \param missData
 * \param size
 */
bool MissionBriefing::loadBriefing(uint8 * missData, int size) {
    char *miss = reinterpret_cast<char *>(missData);
    miss[size - 1] = 0;

    // reading infos
    i_nb_infos_ = 0;
    while (*miss != '|') {
        a_info_costs_[i_nb_infos_++] = atoi(miss);
        miss = strchr(miss, '\n') + 1;
    }

    miss += 2;

    // reading enhancements
    i_nb_enhts_ = 0;
    while (*miss != '|') {
        a_enhts_costs_[i_nb_enhts_++] = atoi(miss);
        miss = strchr(miss, '\n') + 1;
    }

    miss += 2;

    // reading briefing text
    if (miss) {
        std::string tmp(miss);

        int start = 0;
        // We store the default information plus
        // the additional informations for each level on info
        for (int i = 0; i < i_nb_infos_+1; i++) {
            std::size_t idx = tmp.find_first_of('|', start);
            if (std::string::npos != idx) {
                a_briefing_[i].assign(tmp.substr(start, idx - start));
                // skipping "|\n" pair
                start = idx + 2;
            } else {
                a_briefing_[i].assign(tmp.substr(start));
            }
            // sometimes text have additional single '\n''s, we will remove them
            int16 first = -1;
            size_t sz = a_briefing_[i].size();
            std::string &str_ref = a_briefing_[i];
            for (int16 cindx = 0; cindx < (int16)sz; cindx++) {
                if (str_ref[cindx] == '\n') {
                    if (first == -1 || (first != -1 && first + 1 != cindx))
                        first = cindx;
                    else {
                        first = -1;
                    }
                } else {
                    if (first != -1 && first + 1 == cindx) {
                        str_ref[first] = ' ';
                    }
                    first = -1;
                }
            }
        }
    }

    return true;
}

/*!
 * This method creates the minimap from the given map.
 * Then it creates a minimap overlay that tells for each tile
 * whether there is an agent (our/enemy) on it or not.
 * \param p_map The big map used to create the minimap
 * \param level_data Mission infos to create the overlay
 */
void MissionBriefing::init_minimap(Map *p_map, LevelData::LevelDataAll &level_data) {
    // Create the minimap
    p_minimap_ = new MiniMap(p_map);

    // Then create the minimap overlay
    // First, put zero every where
    memset(minimap_overlay_, MiniMap::kOverlayNone, 128*128);

    // We use 2 infos to create the overlay: by ped offset or by weapon offset
    // - if weapon has owner we look into type/index of
    // owner to define our/enemy type; 
    // original map overlay is a 16384x2 array(container), only using map size 
    // we can correctly use our minimap_overlay_; 
    // our agent = 1, enemy agent = 2, tile doesn't have ped = 0
    for (uint32 i = 0; i < (128*128); i++) {
        uint32 pin = READ_LE_UINT16(level_data.map.objs + i * 2);
        if (pin >= 0x0002 && pin < 0x5C02) {  // Pointing to the Pedestrian section
            if (pin >= 0x0002 && pin < 0x02e2) {  // Pointing to one of our agents
                minimap_overlay_[i] = MiniMap::kOverlayOurAgent;
            } else {
                LevelData::People ped = level_data.people[(pin - 2) / 92];
                if (ped.type_ped == 2) { // We take only agent type
                    minimap_overlay_[i] = MiniMap::kOverlayEnemyAgent;
                }
            }
        } else if (pin >= 0x9562 && pin < 0xDD62) {  // Pointing to the Weapon section
            pin = (pin - 0x9562) / 36; // 36 = weapon data size
            LevelData::Weapons & wref = level_data.weapons[pin];
            if (wref.desc == 0x05) {
                pin = READ_LE_UINT16(wref.offset_owner);
                if (pin != 0) {
                    pin = (pin - 2) / 92; // 92 = ped data size
                    if (pin > 7) {
                        LevelData::People ped = level_data.people[(pin - 2) / 92];
                        if (ped.type_ped == 2) {
                            minimap_overlay_[i] = MiniMap::kOverlayEnemyAgent;
                        }
                    } else {
                        // from 0 to 7 are our agents
                        minimap_overlay_[i] = MiniMap::kOverlayOurAgent;
                    }
                }
            }
        }
    }
}

/*!
 * \return - not present, 1 - our agent, 2 - enemy agent
 */
uint8 MissionBriefing::getMinimapOverlay(int x, int y) {
    return minimap_overlay_[x + y * p_minimap_->max_x()];
}
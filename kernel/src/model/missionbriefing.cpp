/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> 
 *   Copyright (C) 2012, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-kernel/model/missionbriefing.h"

#include <stdlib.h>
#include <string>
#include "utf8.h"

#include "fs-engine/gfx/cp437.h"
#include "fs-kernel/model/map.h"

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
 * \param data The content of the briefing file
 * \param size
 */
bool MissionBriefing::loadBriefing(uint8 * data, int size) {
    char *cp437car = reinterpret_cast<char *>(data);
    cp437car[size - 1] = 0;

    // reading infos
    i_nb_infos_ = 0;
    while (*cp437car != '|') {
        a_info_costs_[i_nb_infos_++] = atoi(cp437car);
        cp437car = strchr(cp437car, '\n') + 1;
    }

    cp437car += 2;

    // reading enhancements
    i_nb_enhts_ = 0;
    while (*cp437car != '|') {
        a_enhts_costs_[i_nb_enhts_++] = atoi(cp437car);
        cp437car = strchr(cp437car, '\n') + 1;
    }

    cp437car += 2;

    // reading briefing text
    if (cp437car) {
        // The whole briefing text
        std::string tmp(cp437car);
        // position at the start of each briefing
        size_t start = 0;

        // We store the default information plus
        // the additional information for each level on info
        for (int i = 0; i < i_nb_infos_+1; i++) {
            std::string briefCp437;
            std::size_t idx = tmp.find_first_of('|', start);
            if (std::string::npos != idx) {
                briefCp437.assign(tmp.substr(start, idx - start));
                // skipping "|\n" pair
                start = idx + 2;
            } else {
                briefCp437.assign(tmp.substr(start));
            }

            if (i_nb_infos_ >= 1) {
                // Add a blank line between info
                utf8::append(0x000A, a_briefing_[i]);
            }
            // We transcode the string into a UTF-8 string
            // We also remove single line feeds and keep when there are 2 or more
            int nbLF = 0;
            for (size_t cindx = 0; cindx < briefCp437.size(); cindx++) {
                fs_utl::cp437char_t cp437char = briefCp437[cindx];
                utf8::utfchar32_t u8char = fs_eng::cp437ToUnicode[cp437char];
                if (u8char == 0x000A) { //current character is a line feed
                    nbLF += 1;
                } else { // another type of character
                    if (nbLF > 1) {
                        // There was 2 or more LF before this char, so add exactly 2
                        utf8::append(0x000A, a_briefing_[i]);
                        utf8::append(0x000A, a_briefing_[i]);
                    } else if (nbLF == 1) {
                        // There was only one, replace it with a space
                        utf8::append(0x0020, a_briefing_[i]);
                    }
                    // Then append the letter
                    utf8::append(u8char, a_briefing_[i]);
                    nbLF = 0;
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
    for (uint32_t i = 0; i < (128*128); i++) {
        uint32_t pin = fs_utl::READ_LE_UINT16(level_data.map.objs + i * 2);
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
                pin = fs_utl::READ_LE_UINT16(wref.offset_owner);
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

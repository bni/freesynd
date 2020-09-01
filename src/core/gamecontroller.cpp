/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include <list>

#include "core/gamecontroller.h"
#include "core/gamesession.h"
#include "mission.h"

GameController::GameController() {
    agents_.setModManager(&mods_);
    agents_.setWeaponManager(&weaponMgr_);
}

GameController::~GameController() {}

void GameController::destroy() {
    agents_.destroy();
}

bool GameController::reset() {
    // Reset default mods and weapons
    mods_.reset();
    weaponMgr_.reset();
    // TODO add reading cheatcode for onlywomen parameter
    agents_.reset();
    return true;
}

void GameController::clearAllListeners() {
    game_listeners_.clear();
    mission_listeners_.clear();
}

/*!
 * Adds a listener to the list of listeners for a stream.
 * \param pListener The listener
 * \param stream The stream
 */
void GameController::addListener(GameEventListener *pListener, GameEvent::EEventStream stream) {
    if (pListener) {
        if (stream == GameEvent::kGame) {
            // Check if listener has already subscribed
            for (std::list < GameEventListener * >::iterator it = game_listeners_.begin();
             it != game_listeners_.end(); it++) {
                 if (pListener == *it) {
                     return;
                 }
            }
            game_listeners_.push_back(pListener);
        } else if (stream == GameEvent::kMission) {
            // Check if listener has already subscribed
            for (std::list < GameEventListener * >::iterator it = mission_listeners_.begin();
             it != mission_listeners_.end(); it++) {
                 if (pListener == *it) {
                     return;
                 }
            }
            mission_listeners_.push_back(pListener);
        }
    }
}

//! Removes the listener from the given stream of events
void GameController::removeListener(GameEventListener *pListener, GameEvent::EEventStream stream) {
    if (pListener) {
        if (stream == GameEvent::kGame) {
            for (std::list < GameEventListener * >::iterator it = game_listeners_.begin();
             it != game_listeners_.end(); it++) {
                 if (pListener == *it) {
                     game_listeners_.erase(it);
                     return;
                 }
            }
        } else if (stream == GameEvent::kMission) {
            for (std::list < GameEventListener * >::iterator it = mission_listeners_.begin();
             it != mission_listeners_.end(); it++) {
                 if (pListener == *it) {
                     mission_listeners_.erase(it);
                     return;
                 }
            }
        }
    }
}
//! Sends the event to the listeners
void GameController::fireGameEvent(GameEvent & evt) {
    switch (evt.stream) {
    case GameEvent::kGame:
        for (std::list < GameEventListener * >::iterator it = game_listeners_.begin();
                        it != game_listeners_.end(); it++) {
            (*it)->handleGameEvent(evt);
        }
        break;
    case GameEvent::kMission:
        for (std::list < GameEventListener * >::iterator it = mission_listeners_.begin();
                        it != mission_listeners_.end(); it++) {
            (*it)->handleGameEvent(evt);
        }
        break;
    }
}

/*!
 * Changes the user informations.
 */
void GameController::change_user_infos(const char *company_name, const char *player_name,
                            int new_logo, int new_color) {
    g_Session.setCompanyName(company_name);
    g_Session.setUserName(player_name);
    g_Session.setLogo(new_logo);

    if (g_Session.getLogoColour() != new_color) {
        // we must check in the map if the new player color
        // has not already been assigned to an enemy syndicate
        // => if true, invert colors.
        g_Session.exchange_color_wt_syndicate(new_color);
    }
}

void GameController::handle_mission_end(Mission *p_mission) {
    int elapsed = p_mission->stats()->missionDuration();
    g_Session.updateTime(elapsed);

    if (p_mission->completed()) {
        g_Session.mark_selected_block_completed();

        // Make the next missions available
        Block &blk = g_Session.getSelectedBlock();
        if (blk.next != NULL) {
            char s[50];
            strcpy(s, blk.next);
            char *token = strtok(s, ":");
            while ( token != NULL ) {
                int id = atoi(token);
                Block &next_blk = g_Session.getBlock(id);
                if (next_blk.status == BLK_UNAVAIL) {
                    // Make available only if the mission is not already finished
                    next_blk.status = BLK_AVAIL;
                }
                token = strtok(NULL, ":");
            }
        }

        // simulate other syndicates activity
        simulate_enemy_moves();
    }
}

/*!
 * This method simulates enemy syndicates activtiy
 * by changing ownership of a random number of countries (depending on
 * the number of remaining syndicates).
 */
void GameController::simulate_enemy_moves() {
    // Total number of active syndicates ie syndicates
    // that own at least one country
    int nb_active_synds = 0;
    // Each list of the array contains the id of countries
    // owned by a given syndicate.
    std::list<int> blocks_per_synd[7];
    for (int i=0; i<GameSession::NB_MISSION; i++) {
        Block & blk = g_Session.getBlock(i);
        if (blk.status == BLK_AVAIL || blk.status == BLK_UNAVAIL) {
            if (blocks_per_synd[blk.syndicate_owner].empty()) {
                // We found a country not owned by player
                // so we can count its owner as an active syndicate
                nb_active_synds++;
            }
            blocks_per_synd[blk.syndicate_owner].push_back(i);
        }
    }

    // Computes how many movements (ie change country ownership) we will simulate
    int nb_mvt = get_nb_mvt_for_active_synds(nb_active_synds);

    // Simulate movements

    // this list stores id of missions that shifted
    // to prevent movint them multiple times
    std::list<int> used_block;
    while (nb_mvt > 0) {
        nb_mvt--;
        bool move_done = false;
        do {
            // randomly chose a syndicate
            // that will lose a country
            int synd_from_id = rand() % 7;
            if (!blocks_per_synd[synd_from_id].empty()) {
                // randomly chose one of its countries
                int m_pos = rand() % blocks_per_synd[synd_from_id].size();
                int i = 0;
                for (std::list < int >::iterator it = blocks_per_synd[synd_from_id].begin();
                        it != blocks_per_synd[synd_from_id].end(); it++) {
                    if (i == m_pos) {
                        // First check to see if mission hasn't already been shited before
                        bool is_in_list = false;
                        for (std::list < int >::iterator used_it = used_block.begin();
                                used_it != used_block.end(); used_it++) {
                            if (*it == *used_it) {
                                is_in_list = true;
                                break;
                            }
                        }
                        if (!is_in_list) {
                            int synd_to_id;
                            do {
                                // find another syndicate that owns countries.
                                // synds that do not have countries are out
                                synd_to_id = rand() % 7;
                            } while (synd_from_id == synd_to_id || blocks_per_synd[synd_to_id].empty());

                            // Gives the country to the other syndicate
                            g_Session.getBlock(*it).syndicate_owner = synd_to_id;
                            used_block.push_back(*it); // remember we used this mission
                            move_done = true;
                        }
                        // out anyway
                        break;
                    }

                    i++;
                } // end for
            } // end if
        } while (!move_done);
    }
}

/*!
 * Returns a random number of shifts given the number of
 * syndicates in activity.
 */
int GameController::get_nb_mvt_for_active_synds(int nb_active_synds) {
    switch(nb_active_synds) {
    case 7:
        return rand() % 9 + 2; // between 2 and 10 moves
    case 6:
        return rand() % 7 + 2; // between 2 and 8 moves
    case 5:
        return rand() % 6 + 2; // between 2 and 7 moves
    case 4:
        return rand() % 5 + 1; // between 1 and 5 moves
    case 3:
        return rand() % 4 + 1; // between 1 and 4 moves
    case 2:
        return rand() % 2 + 1; // between 1 and 2 moves
    default:
        return 0;
    }
}

/*!
 * This method is just a wrapper for the call of GameController.fireGameEvent().
 * \param stream The stream of the event
 * \param type The type of the event
 * \param pCtx The context of the event.
 */
void GameEvent::sendEvt(EEventStream stream, EEventType type, void *pCtx) {
    GameEvent evt;
    evt.stream = stream;
    evt.type = type;
    evt.pCtxt = pCtx;
    g_gameCtrl.fireGameEvent(evt);
}

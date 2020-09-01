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

#ifndef CORE_GAMECONTROLLER_H_
#define CORE_GAMECONTROLLER_H_

#include <cassert>
#include <list>

#include "utils/singleton.h"
#include "core/gameevent.h"
#include "agentmanager.h"
#include "weaponmanager.h"
#include "modmanager.h"
#include "missionmanager.h"

class Mission;

/*!
 * The game controller holds the game logic.
 */
class GameController : public Singleton < GameController > {
  public:
    GameController();
    virtual ~GameController();

    /*!
     * Resets controller.
     * \returns True if reset has succeeded
     */
    bool reset();

    //! Delete all ressources. Called by App:destroy()
    void destroy();

    //*************************************
    // Event management
    //*************************************
    //! Adds a listener to the given stream of events
    void addListener(GameEventListener *pListener, GameEvent::EEventStream stream);
    //! Removes the listener from the given stream of events
    void removeListener(GameEventListener *pListener, GameEvent::EEventStream stream);
    //! Sends the event to the listeners
    void fireGameEvent(GameEvent & evt);
    //! Removes all listeners from every stream
    void clearAllListeners();

    //*************************************
    // Managers
    //*************************************
    AgentManager &agents() {
        return agents_;
    }

    WeaponManager &weaponManager() {
        return weaponMgr_;
    }

    ModManager &mods() {
        return mods_;
    }

    MissionManager &missions() {
        return missions_;
    }

    //*************************************
    // Game services
    //*************************************
    //! Changes the user preferences (from the config menu)
    void change_user_infos(const char *company_name, const char *player_name,
                            int logo, int color);
    //! Checks if mission is completed and updates game state
    void handle_mission_end(Mission *p_mission);
private:
    //! Simulates syndicates fighting for countries
    void simulate_enemy_moves();
    // helper method
    int get_nb_mvt_for_active_synds(int nb_active_synds);

private:
    /*!
     * Manager of agent.
     */
    AgentManager agents_;
    /*! Manager of weapons.*/
    WeaponManager weaponMgr_;
    /*! Manager of mods.*/
    ModManager mods_;
    /*! Manager of missions.*/
    MissionManager missions_;
    /*! List of listeners for game stream events.*/
    std::list<GameEventListener *> game_listeners_;
    /*! List of listeners for mission stream events.*/
    std::list<GameEventListener *> mission_listeners_;
};

#define g_gameCtrl    GameController::singleton()

#endif  // CORE_GAMECONTROLLER_H_

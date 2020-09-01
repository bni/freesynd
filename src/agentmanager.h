/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
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
#ifndef AGENTMANAGER_H
#define AGENTMANAGER_H

#include "common.h"
#include "agent.h"
#include "utils/seqmodel.h"
#include "utils/portablefile.h"

extern const char * const g_AgentNames[];
extern const int g_NumAgentNames;

class WeaponManager;
class ModManager;

/*!
 * Agent Manager class.
 *
 * Provides methods for managing player's agents and squad.
 * The Squad is the team of agent selected for a mission. It holds up to 4 agents.
 * An agent can be selected for a squad but not active : in this case, he will not
 * participate
 */
class AgentManager {
public:
    /*! Max number of agents in cryo chamber.*/
    static const int MAX_AGENT;
    /*! Total number of slots on a squad.*/
    static const size_t kMaxSlot;
    /*! Id of slot 1.*/
    static const size_t kSlot1;
    /*! Id of slot 2.*/
    static const size_t kSlot2;
    /*! Id of slot 3.*/
    static const size_t kSlot3;
    /*! Id of slot 4.*/
    static const size_t kSlot4;

    AgentManager();
    ~AgentManager() {}

    void destroy();

    void setWeaponManager(WeaponManager *pWeaponManager) {
        pWeaponManager_ = pWeaponManager;
    }

    void setModManager(ModManager *pModManager) {
        pModManager_ = pModManager;
    }

    void loadAgents();
    void reset(bool onlyWomen = false);
    //! Destroy an agent from squad and cryo chamber
    void destroyAgentSlot(size_t squadSlot);

    Agent *agent(int n) {
        assert(n < MAX_AGENT);
        return agents_.get(n);
    }

    SequenceModel * getAgents() { return &agents_; }

    //! Empties the squad
    void clearSquad();

    //! Returns true if the squad slot holds an agent and if he's active
    bool isSquadSlotActive(size_t slotId);

    /*!
     * Sets an agent in the squad at the given index.
     * \param n The agent's index in the team (from 0 to 3)
     * \param a The new agent
     */
    void setSquadMember(size_t slotId, Agent *pAgent) {
        assert(slotId < kMaxSlot);
        a_squad_[slotId] = pAgent;
    }

    //! Returns the agent on the given squad slot
    Agent * squadMember(size_t slotId) {
        assert(slotId < kMaxSlot);
        return a_squad_[slotId];
    }

    //! Return the slot that holds the given agent or -1 if ni agent is found
    int getSquadSlotForAgent(Agent *pAgent);

    //! Save instance to file
    bool saveToFile(PortableFile &file);
    //! Load instance from file
    bool loadFromFile(PortableFile &infile, const FormatVersion& v);
    Agent *createAgent(bool onlyWomen = false);

protected:
    /*!
     * All available agents.
     */
    VectorModel<Agent *> agents_;
    /*!
     * Selected agents for the next mission. Up to 4 agents.
     */
    Agent *a_squad_[4];
    int nextName_;
    //! The weapon manager
    WeaponManager *pWeaponManager_;
    //! The mod manager
    ModManager *pModManager_;
};

#endif

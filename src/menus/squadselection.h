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

#ifndef MENUS_SQUADSELECTION_H_
#define MENUS_SQUADSELECTION_H_

#include "model/squad.h"
#include "agentmanager.h"
#include "model/position.h"

class Mission;
class Vehicle;

/*!
 * This class manages the agents selection during a gameplay session.
 * A selection always contains at least one element.
 * A selection has a leader : it the first selected agent. The leader
 * doesn't change if an agent is added to the current selection.
 */
class SquadSelection {
 public:
     /*!
      * An iterator to iterate over the selected agents.
      */
     class Iterator {
         public:
            Iterator(size_t idx, SquadSelection *pSel) {
                idx_ = idx;
                pSel_ = pSel;
            }

            ~Iterator() {
                idx_ = idx_;
            }

            // The assignment and relational operators are straightforward
            Iterator& operator=(const Iterator& other)
            {
                 idx_ = other.idx_;
                 return(*this);
            }

            bool operator==(const Iterator& other)
            {
                return(idx_ == other.idx_);
            }

            bool operator!=(const Iterator& other)
            {
                return(idx_ != other.idx_);
            }

            /*!
             *
             */
            Iterator& operator++()
            {
                while (idx_ < AgentManager::kMaxSlot)
                {
                    idx_ += 1;
                    if (idx_ < AgentManager::kMaxSlot && pSel_->isAgentSelected(idx_)) {
                        break;
                    }
                }
                return(*this);
            }

            /*!
             *
             */
            Iterator& operator++(int)
            {
                ++(*this);
                return(*this);
            }

            // Return a reference to the value in the node.  I do this instead
          // of returning by value so a caller can update the value in the
          // node directly.
          PedInstance * operator*()
          {
              return (pSel_->pSquad_->member(idx_));
          }

          // Return the address of the value referred to.
          PedInstance * operator->()
          {
              //PedInstance * pPed = *(*this); // dereference *this to get the value
              //return (&pPed); // Return the address of the referent
              return pSel_->pSquad_->member(idx_);
          }

         private:
             size_t idx_;
             SquadSelection *pSel_;
     };

    //! Default constructor
    SquadSelection();

    /*!
     * Returns the first element of the selection.
     */
    Iterator begin() {

        for (size_t idx=0; idx < AgentManager::kMaxSlot; idx++)
        {
            if (isAgentSelected(idx)) {
                return Iterator(idx, this);
            }
        }
        return end();
    }

    /*!
     * Returns the end of selection.
     */
    Iterator end() {
       return(Iterator(4, NULL));
    }

    /*!
     * Returns true if an agent is selected.
     * \param agentNo The index of the slot to test (use constant kSlotN)
     */
    bool isAgentSelected(size_t agentNo) {
        return (selected_agents_ & (1 << agentNo)) != 0;
    }

    //! Sets the squand and selects the first active agent of the squad.
    void setSquad(Squad *pSquad);
    //! Clear current selection
    void clear();
    //! Returns the number of selected agents
    size_t size();
    //! selects/deselects agent
    bool selectAgent(size_t agentNo, bool addToGroup);
    //! Force deselection of agent
    void deselectAgent(PedInstance *p_ped);
    //! Selects/deselects all agent
    void selectAllAgents(bool b_selectAll);
    //! Returns the leader's slot
    size_t getLeaderSlot() { return leader_; }
    // Returns the leader as a PedInstance
    PedInstance * leader() { return pSquad_->member(leader_);}

    //*************************************
    // Action on multiple agents
    //*************************************
    //! Deselects all selected agents weapons
    void deselectWeaponOfSameCategory(Weapon *pWeaponFromLeader);
    //! Select a weapon for the leader and for all selected agents
    void selectWeaponFromLeader(int weapon_idx, bool apply_to_all);
    //! Go and pick up weapon
    void pickupWeapon(WeaponInstance *pWeapon, bool addAction);
    //! Follow Ped
    void followPed(PedInstance *pPed, bool addAction);
    //! Enter or leave the vehicle : do the same as leader
    void enterOrLeaveVehicle(Vehicle *pVehicle, bool addAction);
    //! Move selected agents to the given point
    void moveTo(TilePoint &mapPt, bool addAction);
    //! Every selected armed agent shoot at location
    void shootAt(const WorldPoint &aimedLocW);
    //! Return true if target is in range of at least one agent
    bool isTargetInRange(Mission *pMission, ShootableMapObject *pTarget);
 private:
    /*!
     * Return true if an agent can be selected :
     * slot must be active and agent not dead.
     */
    bool isAgentSelectable(size_t agentNo);
    //! checks if leader must be changed
    void checkLeader(size_t agentNo);

 private:
    /*!
     * This field is seen as a field of bits. Each bit tells if
     * a slot is selected.
     */
    size_t selected_agents_;
    /*! Leader slot number.*/
    size_t leader_;
    /*! A reference to the current squad so we can check the agents
     * status.*/
    Squad  *pSquad_;
};

#endif  // MENUS_SQUADSELECTION_H_

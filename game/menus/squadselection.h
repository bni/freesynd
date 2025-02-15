/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
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

#ifndef MENUS_SQUADSELECTION_H_
#define MENUS_SQUADSELECTION_H_

#include "fs-kernel/model/squad.h"
#include "fs-kernel/model/position.h"
#include "fs-kernel/model/vehicle.h"
#include "fs-kernel/model/mission.h"

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
                idx_ = 0;
                pSel_ = nullptr;
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
                while (idx_ < fs_knl::Squad::kMaxSlot)
                {
                    idx_ += 1;
                    if (idx_ < fs_knl::Squad::kMaxSlot && pSel_->isAgentSelected(idx_)) {
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
          fs_knl::PedInstance * operator*()
          {
              return (pSel_->pSquad_->member(idx_));
          }

          // Return the address of the value referred to.
          fs_knl::PedInstance * operator->()
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

        for (size_t idx=0; idx < fs_knl::Squad::kMaxSlot; idx++)
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
    void setSquad(fs_knl::Squad *pSquad);
    //! Clear current selection
    void clear();
    //! Returns the number of selected agents
    size_t size();
    //! selects/deselects agent
    bool selectAgent(size_t agentNo, bool addToGroup);
    //! Force deselection of agent
    void deselectAgent(fs_knl::PedInstance *p_ped);
    //! Selects/deselects all agent
    void selectAllAgents(bool b_selectAll);
    //! Returns the leader's slot
    size_t getLeaderSlot() { return leader_; }
    // Returns the leader as a PedInstance
    fs_knl::PedInstance * leader() { return pSquad_->member(leader_);}

    //*************************************
    // Action on multiple agents
    //*************************************
    //! Deselects all selected agents weapons
    void deselectWeaponOfSameCategory(fs_knl::Weapon *pWeaponFromLeader);
    //! Select a weapon for the leader and for all selected agents
    void selectWeaponFromLeader(uint8_t weapon_idx, bool apply_to_all);
    //! Go and pick up weapon
    void pickupWeapon(fs_knl::WeaponInstance *pWeapon, bool addAction);
    //! Follow Ped
    void followPed(fs_knl::PedInstance *pPed);
    //! Enter or leave the vehicle : do the same as leader
    void enterOrLeaveVehicle(fs_knl::Vehicle *pVehicle, bool addAction);
    //! Move selected agents to the given point
    void moveTo(fs_knl::TilePoint &mapPt, bool addAction);
    //! Every selected armed agent shoot at location
    void shootAt(const fs_knl::WorldPoint &aimedLocW);
    //! Return true if target is in range of at least one agent
    bool isTargetInRange(fs_knl::Mission *pMission, fs_knl::ShootableMapObject *pTarget);
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
    fs_knl::Squad  *pSquad_;
};

#endif  // MENUS_SQUADSELECTION_H_

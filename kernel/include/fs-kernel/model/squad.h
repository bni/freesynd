/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2012, 2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef CORE_SQUAD_H_
#define CORE_SQUAD_H_

#include <array>

#include "fs-utils/common.h"
#include "fs-kernel/model/position.h"

namespace fs_knl {

class PedInstance;

/*!
 * A squad is a team of 1 to 4 agents that perform the mission.
 * A squad is composed of 4 slots that can be filled with agents or can
 * be left empty.
 * An agent on a slot must be active to participate in a mission.
 */
class Squad {
public:
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
    
    //! Default constructor
    Squad();

    //! Empties the squad
    void clear();
    //! Sets an agent in the squad at the given index.
    void setMember(size_t slotId, PedInstance *pPedAgent) ;
    //! Returns the agent on the given slot
    PedInstance * member(size_t slotId);
    //! Returns true if at least one agent has a scanner
    bool hasScanner();
    //! Returns true if at least one agent is alive
    bool isAllDead();
    //! Returns the size of the squad (with the dead agents)
    size_t size() { return size_; }

    void getPositionInSquadFormation(size_t slotId, TilePoint *pPosition);

    /*!
     * A SquadIterator is use to facilitate iterating over the squad elements.
     * The default behaviour for this iterator is to return non null elements.
     * When setting checkHealth to true, this iterator return also only alive elements.
     */
    class SquadIterator {
        public:
            
            /*!
            * Constructor for the SquadIterator
            * @param begin The beginning of the element list
            * @param anEnd The end of the element list
            * @param checkHealth Set to true to return only alive elements
            */
            explicit SquadIterator(std::array<PedInstance *, 4>::iterator begin, 
                                    std::array<PedInstance *, 4>::iterator anEnd,
                                    bool checkHealth) : 
                        current(begin), end(anEnd), checkHealth_(checkHealth) {
                // in case the first element is not ok
                advanceIfNeeded();
            }
    
            //! Return the current element
            PedInstance *& operator*() {
                return *current;
            }
    
            //! Increment to the next element
            SquadIterator& operator++() {
                ++current;
                advanceIfNeeded();

                return *this;
            }
    
            //! Equality check
            bool operator==(const SquadIterator& other) const {
                return current == other.current;
            }
    
            //! Inequality check
            bool operator!=(const SquadIterator& other) const {
                return current != other.current;
            }
        
        private:
            //! Find the next matching element
            void advanceIfNeeded();

        private:
            //! The current element of the iterator
            std::array<PedInstance *, 4>::iterator current;
            //! The end of iterator
            std::array<PedInstance *, 4>::iterator end;
            //! True means checking health
            bool checkHealth_;
        };
    
        //! Return the default begin iterator
        SquadIterator begin() {
            return SquadIterator(members_.begin(), members_.end(), false);
        }
        //! Return the default end iterator
        SquadIterator end() {
            return SquadIterator(members_.end(), members_.end(), false);
        }
        //! Return the begin iterator for alive elements
        SquadIterator alive_begin() {
            return SquadIterator(members_.begin(), members_.end(), true);
        }
        //! Return the end iterator for alive elements
        SquadIterator alive_end() {
            return SquadIterator(members_.end(), members_.end(), true);
        }
private:
    /*!
     * Selected agents for the next mission. Up to 4 agents.
     */
    std::array<PedInstance *, 4> members_;
    /*!
     * The number of active agents on the squad (dead or alive).
     */
    size_t size_;
};

}
#endif  // CORE_SQUAD_H_

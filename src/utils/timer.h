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

#ifndef UTILS_TIMER_H_
#define UTILS_TIMER_H_

#include "common.h"
namespace fs_utils {

/*!
 * A simple timer implementation. Create the timer with
 * max time and call update in each frame animation with the
 * elapsed time since the last call.
 * When the maximum is reached, update() returns true otherwise
 * it returns false.
 * Call reset to set the timer to zero.
 */
class Timer {
 public:
     /*!
      * Contructor to set the max time.
      */
     Timer(uint32 i_max) {
         reset(i_max);
     }

     /*!
      * Reset the counter.
      */
     void reset() {
         i_counter_ = 0;
     }

     /*!
      * Reset the counter and sets a new max time.
      */
     void reset(uint32 i_max) {
         i_counter_ = 0;
         i_max_ = i_max;
     }

     /*!
      * Adds time to time and returns true if max is reached.
      * If max is reached, counter is reset.
      * \return True if timer has reached max.
      */
     bool update(uint32 elapsed) {
         i_counter_ += elapsed;
         if (i_counter_ > i_max_) {
             i_counter_ = 0;
             return true;
         }
         return false;
     }

     /*!
      * Set the counter to max so next time update is called,
      * it automatically returns true.
      * Useful when you want to force the timer to pass next time update is called.
      */
     void setToMax() {
         i_counter_ = i_max_;
     }

 private:
    uint32 i_counter_;
    uint32 i_max_;
};

/*!
 * A convenient timer to store a boolean value.
 * Each time the timer reaches the max, the value is set to the opposite.
 * Call state() to get the current state.
 */
class BoolTimer : public Timer {
 public:
     /*!
      * Contructor to set the max time.
      * \param i_max Maximum value of the counter
      * \param b_state The initial state.
      */
     BoolTimer(uint32 i_max, bool b_state) : Timer(i_max) {
         b_state_ = b_state;
     }

     //! Return the current state
     bool state() { return b_state_; }

     /*!
      * Adds time to time and returns true if max is reached.
      * If max is reached, counter is reset and state value is set
      * to the opposite.
      * \return True if timer has reached max.
      */
     bool update(uint32 elapsed) {
         bool res = Timer::update(elapsed);
         if (res) {
             b_state_ = !b_state_;
         }
         return res;
     }

 private:
    bool b_state_;
};
};

#endif  // UTILS_TIMER_H_

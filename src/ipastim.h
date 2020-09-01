/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2012  Ryan Cocks <ryan@ryancocks.net>                *
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

#ifndef FreeSynd_ipastim_h
#define FreeSynd_ipastim_h

#include "utils/timer.h"

// Stores the values for the Intelligence, Perception and
// Adrenaline bars. Also calculates the multipliers to things
// like speed that these give.
//
// Each IPAStim tracks a few values and via the processTicks() method
// these values will change over time as in the original Syndicate.

class IPAStim
{
public:
    typedef enum {
        Adrenaline,
        Perception,
        Intelligence
    } IPAType;
    IPAStim(IPAType ipa_type, int amount = 50, int dependency = 50);
#ifdef _DEBUG
    const char * getName() const
    {
        return IPANames[(int)ipa_type_];
    }
#endif
    
    // Return value varies from 0.5 to 2 and returns 1
    // for 'neutral' adrenaline.
    float getMultiplier() const;
    
    // We are using percentages, the original data files are using uint8 256 ranges
    void setLevels256(int amount, int dependency, int effect)
    {
        setLevels(
            (int)((float)amount/256.0*100.0),
            (int)((float)dependency/256.0*100.0),
            (int)((float)effect/256.0*100.0)
        );
    }
    
    // Note: this method takes percentages as arguments, use
    // setLevels256 to use the values from Syndicate's data files.
    // "effect" may not currently be used.
    void setLevels(int amount, int dependency, int effect = 0);
    
    void setAmount(int percentage) { amount_ = percentage; }

    int getAmount()     const { return amount_; }
    int getDependency() const { return dependency_; }
    int getEffect()    const { return effect_; }
    IPAType getType()   const { return ipa_type_; }

    void processTicks(int elapsed);

private:
    // Used to select colors when rendering
    IPAType ipa_type_;
#ifdef _DEBUG
    // Allow pretty debug
    static const char * IPANames[3];
#endif

    // These are percentages - 50% is the neutral mid-point
    int amount_, dependency_;

    // The darker bar that grows to match 'amount'. If you watch
    // the behaviour of this in the original Syndicate it grows from
    // the dependency line to meet the currently set 'amount'. Once the
    // two are equal it disappears and the 'amount' starts
    // moving towards the dependency line.
    int effect_;

    // Given a percentage returns that % of 1 to 2
    // i.e. instead of 0 to 2
    float part_of_two(int percentage) const
    {
        return (((float)percentage)/100.0f) + 1.0f;
    }

    enum IPADir {
        IPA_boost,
        IPA_reduce
    };
    IPADir direction() const
        { return amount_ - dependency_ >= 0 ? IPA_boost : IPA_reduce; }

    int getMagnitude() const;

    // A one second timer, IPA's seem to adjust themselves
    // about once a second
    fs_utils::Timer effect_timer_; // every second
    fs_utils::Timer dependency_timer_; // once every 4.5 seconds
};

#endif

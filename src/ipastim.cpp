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

#include "ipastim.h"
#include <assert.h>

#ifdef _DEBUG
#include <stdio.h>

const char * IPAStim::IPANames[3] = {
    "Adrenaline",
    "Perception",
    "Intelligence"
};
#endif

IPAStim::IPAStim(IPAType ipa_type, int amount, int dependency)
:ipa_type_(ipa_type), effect_(50), effect_timer_(1000), dependency_timer_(4500)
{
    assert(ipa_type_ <= 3);
    setLevels(amount, dependency);
}

int IPAStim::getMagnitude() const
{
    return dependency_ < amount_ ? amount_ - dependency_: dependency_ - amount_;
}


float IPAStim::getMultiplier() const
{
    // With an agent with no mods and his adrenaline dependency in the
    // center: his speed will be halved or doubled with the adrenaline
    // bar at the respective extreme.
    
    // An agent with no adrenaline, no mods and no weaponry walks at
    // the same speed as a civilian.
    
    // This function has been implemented to assume that FULL adrenaline
    // would give a 2x increase in speed and the worst case would leave
    // you walking at a half speed. With neutral adrenaline it has no
    // effect and therefore returns 1.
    
    // Thus, the algortithm used here goes from 0.5 to 1 for 'negative'
    // adrenaline and 1 to 2 on the positive side.
    int magnitude = getMagnitude();
    
    if(direction() == IPA_boost) {
        // return value is 1 to 2 for values
        // of 'magnitude' from 0 to 100
        // If you fiddle with this equation beware of
        // values for effective which are close to 0
        float mult = part_of_two(magnitude);
        assert(mult >= 1 && mult <= 2);
        //printf("%s boost: m:%d->%fx\n", getName(), magnitude_, mult);
        return mult;
    } else {
        // < 0
        // range: 0.5 up towards 1
        float mult = 1.0/part_of_two(magnitude);
        assert(mult >= 0.5 && mult <= 1.0);
        //printf("%s reduce: m:%d->%fx\n", getName(), magnitude_, mult);
        return mult;
    }
}

void IPAStim::setLevels(int amount, int dependency, int effect)
{
    amount_ = amount;
    dependency_ = dependency;

    effect_ = dependency;

    //printf("%s: A: %d, D: %d, E: %d\n", getName(), amount, dependency, effect_);
}

void IPAStim::processTicks(int elapsed)
{
    // From observation of the original Syndicate:
    // * Effect moves about once a second.
    // * Dependency bar moves once for every 5 or 6 moves of Effect
    // * Effect is independent of amount! If you flick amount to the
    //   other side effect will stay where it was.
    // * there appear to be 50 'positions' on the bar so it looks
    //   like the levels move in notches, 1% at a time.
    
    if(effect_timer_.update(elapsed))
    {
        if(effect_ > amount_)
        {
            --effect_;
        }
        else if(effect_ < amount_)
        {
            ++effect_;
        }
        else // equal
        {
            // So once effect has 'caught up' to amount then they
            // both start moving towards the value of dependency
            // together
            if(amount_ > dependency_)
            {
                effect_ = --amount_;
            }
            else if(amount_ < dependency_)
            {
                effect_ = ++amount_;
            }
        }
        assert(effect_ >= 0 && effect_ <= 100);
    }

    // The dependency indicator always creaps towards amount
    if(dependency_timer_.update(elapsed))
    {
        if (dependency_ > amount_) {
            --dependency_;
        } else if (dependency_ < amount_) {
            ++dependency_;
        } else {
            // equal
            if (amount_ < 50) {
                ++amount_;
                ++dependency_;
            } else if (amount_ > 50) {
                --amount_;
                --dependency_;
            }
        }
        assert(dependency_ >= 0 && dependency_ <= 100);

    }
}

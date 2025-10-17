/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2012  Ryan Cocks <ryan@ryancocks.net>
 *   Copyright (C) 2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef FreeSynd_ipastim_h
#define FreeSynd_ipastim_h

#include "fs-utils/misc/timer.h"

/*!
 * Stores the values for the Intelligence, Perception and
 * Adrenaline bars. Also calculates the multipliers to things
 * like speed that these give.
 *
 * Each IPAStim tracks a few values and via the processTicks() method
 * these values will change over time as in the original Syndicate.
 */
class IPAStim
{
public:
    enum IPAType {
        Adrenaline,
        Perception,
        Intelligence
    };

    IPAStim(IPAType ipa_type, uint8_t amount = 50, uint8_t dependency = 50);

    IPAType type()   const { return ipa_type_; }
    uint8_t amount()     const { return amount_; }
    void setAmount(uint8_t percentage) { amount_ = percentage; }
    uint8_t dependency() const { return dependency_; }
    uint8_t effect()    const { return effect_; }

    //! Return value varies from 0.5 to 2 and returns 1 for 'neutral' adrenaline.
    float getMultiplier() const;

    //! We are using percentages, the original data files are using uint8 256 ranges
    void setLevels256(int amount, int dependency, int effect)
    {
        setLevels(
            (uint8_t)((float)amount/256.0*100.0),
            (uint8_t)((float)dependency/256.0*100.0),
            (uint8_t)((float)effect/256.0*100.0)
        );
    }

    void processTicks(uint32_t elapsed);

#ifdef _DEBUG
    const char * getName() const {
        return IPANames[ipa_type_];
    }
#endif

private:
    /*!
     * @brief Indicate if IPA stimulation is boosting or reducing
     */
    enum IPADir {
        kIPADirBoost,
        kIPADirReduce
    };

    /*!
     * Note: this method takes percentages as arguments, use
     * setLevels256 to use the values from Syndicate's data files.
     * "effect" may not currently be used.
     */
    void setLevels(uint8_t amount, uint8_t dependency, uint8_t effect = 0);

    /*!
     * Given a percentage returns that % of 1 to 2
     * i.e. instead of 0 to 2
     */
    float part_of_two(int percentage) const {
        return (((float)percentage)/100.0f) + 1.0f;
    }

    IPADir direction() const
        { return amount_ - dependency_ >= 0 ? kIPADirBoost : kIPADirReduce; }

    int getMagnitude() const;

private:
    //! Used to select colors when rendering
    IPAType ipa_type_;

#ifdef _DEBUG
    // Allow pretty debug
    static const char * IPANames[3];
#endif

    /*!
     * This represent the amount of drug injected for the current type.
     * On screen, it is the bright color and is set by the player.
     * It is a percentage - 50% is the neutral mid-point.
     */
    uint8_t amount_;
    /*!
     * This represent the level of consumption and the effect of given amount of drug.
     * It's the darker bar on screen. It grows from the dependency line
     * to meet the currently set 'amount'.
     * Once the two are equal, it disappears and the 'amount' and effect start
     * moving towards the dependency line.
     */
    uint8_t effect_;
    /*!
     * This represent the level of dependency for the current type.
     * It is a percentage - 50% is the neutral mid-point.
     * The higher this level is set, the less time the effect will last.
     */
    uint8_t dependency_;

    //! A timer to control the update of effect level
    fs_utl::Timer effect_timer_;
    //! A timer to control the level of dependency
    fs_utl::Timer dependency_timer_;
};

#endif

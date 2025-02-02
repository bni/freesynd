/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2010, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef SDLMIXERSOUND_H_
#define SDLMIXERSOUND_H_

#include <SDL_mixer.h>

#include "fs-utils/common.h"
#include "fs-engine/sound/sound.h"

namespace fs_eng {

//! Sound implementation using Sdl_Mixer.
/*!
 * This class is an implementation of the sound system
 * using the Sdl_Mixer library.
 */
class SdlMixerSound : public Sound {
public:
    //! Class constructor
    SdlMixerSound();
    //! Class destructor
    ~SdlMixerSound();

    //! Plays the sound a number a time
    void play(int channel, int loops) const override;
    //! Stops the sound
    void stop(int channel) const override;
    //! Sets the sample volume
    bool setVolume(int volume) override;
    //! Loads the sample from memory
    bool loadSound(uint8_t *soundData, uint32_t size) override;

protected:
    /*! The sdl structure that stores sound data.*/
    Mix_Chunk *sound_data_;
};

}

#endif  // SDLMIXERSOUND_H_

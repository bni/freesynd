/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef SDLMIXERSOUND_H_
#define SDLMIXERSOUND_H_

#include "../common.h"

#ifdef HAVE_SDL_MIXER

#include <SDL_mixer.h>

//! Sound implementation using Sdl_Mixer.
/*!
 * This class is an implementation of the sound system
 * using the Sdl_Mixer library.
 */
class SdlMixerSound {
public:
    //! Class constructor 
    SdlMixerSound();
    //! Class destructor 
    ~SdlMixerSound();

    //! Plays the sound a number a time
    void play(int loops = 0, int channel = 0) const;
    //! Stops the sound
    void stop(int channel = 0) const;
    //! Sets the sample volume
    bool setVolume(int volume);
    //! Loads the sample from memory
    bool loadSound(uint8 *soundData, uint32 size);

protected:
    /*! The sdl structure that stores sound data.*/
    Mix_Chunk *sound_data_;
};

#endif  // HAVE_SDL_MIXER

#endif  // SDLMIXERSOUND_H_

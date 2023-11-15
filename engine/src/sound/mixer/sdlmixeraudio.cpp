/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2023  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "sdlmixeraudio.h"

#include <SDL.h>
#include <SDL_mixer.h>

#include "fs-utils/log/log.h"
#include "sdlmixersound.h"
#include "sdlmixermusic.h"

SdlMixerAudio::SdlMixerAudio() {
    initialized_ = false;
}

SdlMixerAudio::~SdlMixerAudio() {}

/*!
 * Initialize the audio system using SDL_Mixer.
 * \param freq The frequency
 * \param fmt A format
 * \param chan Mono or stereo
 * \param chunksize Chunsize
 * \return True is initialization is ok.
 */
bool SdlMixerAudio::init(EFrequency freq, EFormat fmt, EChannel chan, int chunksize) {

    LOG(Log::k_FLG_SND, "SdlMixerAudio", "init", ("Initializing sound system with SDL_Mixer..."));

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        FSERR(Log::k_FLG_SND, "SdlMixerAudio", "init", ("Failed initialize SDL Audio module."));
        return false;
    }

    if (Mix_QuerySpec(0, 0, 0) > 0) {
        FSERR(Log::k_FLG_SND, "SdlMixerAudio", "init", ("SDL Mixer has already been initialized."));
        return true;
    }

    // Choosing the frequency
    int frequency;
    switch (freq) {
        case FRQ_11025:
            frequency = 11025;
            break;
        case FRQ_22050:
            frequency = 22050;
            break;
        case FRQ_44100:
            frequency = 44100;
            break;
        default:
            frequency = MIX_DEFAULT_FREQUENCY;
    }

    // Choosing the format
    Uint16 format;
    switch (fmt) {
        default:
            format = MIX_DEFAULT_FORMAT;
            break;
    }

    // Choosing the channel
    int channel;
    switch (chan) {
        case MONO :
            channel = 1;
            break;
        case STEREO:
            channel = 2;
            break;
        default:
            channel = MIX_DEFAULT_CHANNELS;
            break;
    }

    if (Mix_OpenAudio(frequency, format, channel, chunksize) < 0) {
        FSERR(Log::k_FLG_SND, "SdlMixerAudio", "init", ("Failed to initialize SDL Mixer."));
        return false;
    }

    LOG(Log::k_FLG_SND, "Audio", "init", ("Sound system initialized"));

    initialized_ = true;
    return true;
}

/*!
 * Destroy the audio system.
 * \return True if ok.
 */
bool SdlMixerAudio::quit(void) {
    if (Mix_QuerySpec(0, 0, 0) > 0) {
        Mix_HaltMusic();
        Mix_HaltChannel(-1);
        Mix_CloseAudio();
        initialized_ = false;
        return true;
    }
    return false;
}

/*!
 * Sets the music volume.
 * \param volume The volume level. If greater than the maximum
 * volume, then the maximum is set.
 */
void SdlMixerAudio::setMusicVolume(int volume)
{
    if (initialized_) {
        Mix_VolumeMusic(volume);
    }
}

/*!
 * Returns the music volume.
 * \return -1 if the system has not been initialized.
 */
int SdlMixerAudio::getMusicVolume() {
    if (initialized_) {
        // Using -1 as argument does not change
        // the volume but returns its level
        return Mix_VolumeMusic(-1);
    } else {
        return -1;
    }
}

/*!
 * Sets the music volume.
 * \param volume The volume level. If greater than the maximum
 * volume, then the maximum is set.
 * \param channel The channel on which the volume is set.
 */
void SdlMixerAudio::setSoundVolume(int volume, int channel)
{
    if (initialized_) {
        Mix_Volume(channel, volume);
    }
}

/*!
 * Returns the music volume.
 * \return -1 if the system has not been initialized.
 */
int SdlMixerAudio::getSoundVolume(int channel) {
    if (initialized_) {
        return Mix_Volume(channel, -1);
    } else {
        return -1;
    }
}

/*!
 * Returns the maximum volume possible
 * \return The maximum for the underneath system.
 */
int SdlMixerAudio::getMaxVolume() {
    return MIX_MAX_VOLUME;
}

/*!
 * Returns an instance of Sound for this implementation of Audio
 * \return Returns an instance of SdlMixerSound.
 */
std::unique_ptr<Sound> SdlMixerAudio::createSound() {
    return std::make_unique<SdlMixerSound>();
}

/*!
 * Returns an instance of Music for this implementation of Audio
 * \return Returns an instance of SdlMixerMusic.
 */
std::unique_ptr<Music> SdlMixerAudio::createMusic() {
    return std::make_unique<SdlMixerMusic>();
}


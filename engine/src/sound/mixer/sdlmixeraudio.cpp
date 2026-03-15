/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2023-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "sdlmixeraudio.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include "adlmidi.h"

#include "fs-utils/log/log.h"
#include "sdlmixersound.h"



/*!
 * This flag is used to control the execution of the callback
 */
static Uint32 is_playing = 0;
//! The final format for audio
static struct ADLMIDI_AudioFormat s_audioFormat;
/*!
 * It's the midi player who is in charge of playing all music.
 * This midi player allows us to use AIL 2.0 banks which were used originally in Syndicate
 * The bank corresponding to Syndicate was 19.
 */
static struct ADL_MIDIPlayer    *pMidiPlayer = NULL;

/**
 * This is the audio callback function that SDL_Mixer will call when playing the music.
 * Here you have to copy the data of your audio buffer into the
 * requesting audio buffer (stream)
 * you should only copy as much as the requested length (len)
 * See https://github.com/Wohlstand/libADLMIDI/blob/master/examples/sdl2_mixer/sdl2_mixer_example.c
 */
void fs_audio_callback(void *midi_player, Uint8 *stream, int len)
{
    if (!is_playing)
      return;

    struct ADL_MIDIPlayer* p = (struct ADL_MIDIPlayer*)midi_player;

    /* Convert bytes length into total count of samples in all channels */
    int samples_count = len / s_audioFormat.containerSize;

    /* Take some samples from the ADLMIDI */
    samples_count = adl_playFormat(p, samples_count,
                                   stream,
                                   stream + s_audioFormat.containerSize,
                                   &s_audioFormat);

    if(samples_count <= 0)
    {
        is_playing = 0;
        SDL_memset(stream, 0, len);
    }
}

namespace fs_eng {

SdlMixerAudio::SdlMixerAudio() {
    initialized_ = false;
    pMidiPlayer = nullptr;
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

    if (initialized_) {
        FSERR(Log::k_FLG_SND, "SdlMixerAudio", "init", ("SDL Mixer has already been initialized."));
        return true;
    }

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        FSERR(Log::k_FLG_SND, "SdlMixerAudio", "init", ("Failed initialize SDL Audio module."));
        return false;
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
    Uint16 format = MIX_DEFAULT_FORMAT;

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

    // initialize the player
    pMidiPlayer = adl_init(frequency);
    if (!pMidiPlayer) {
        FSERR(Log::k_FLG_SND, "SdlMixerAudio", "init", ("Couldn't initialize ADLMIDI: %s\n", adl_errorString()));
        return false;
    }

    adl_switchEmulator(pMidiPlayer, ADLMIDI_EMU_NUKED);

    if (Mix_OpenAudio(frequency, format, channel, chunksize) < 0) {
        FSERR(Log::k_FLG_SND, "SdlMixerAudio", "init", ("Failed to initialize SDL Mixer."));
        return false;
    }

    Mix_AllocateChannels(16);

    // We set a hook so that we don't use SDL_Mixer built-in music playback but libADLMidi
    Mix_HookMusic(fs_audio_callback, pMidiPlayer);

    initObtainedAudioFormat();
    adl_setNumChips(pMidiPlayer, 1);
    // We are using the Bank 19 which is the one for syndicate
    adl_setBank(pMidiPlayer, 19);

    LOG(Log::k_FLG_SND, "Audio", "init", ("Sound system initialized"));

    initialized_ = true;
    return true;
}

/*!
 * This initializes the final audioFormat for libAdlMidi with
 * the real format that SDL Mixer has been able to confirm.
 */
void SdlMixerAudio::initObtainedAudioFormat() {
    static Uint16 obtainedFormat; /* Format of the audio device */

    Mix_QuerySpec(NULL, &obtainedFormat, NULL);

    switch(obtainedFormat)
    {
    case AUDIO_S8:
        s_audioFormat.type = ADLMIDI_SampleType_S8;
        s_audioFormat.containerSize = sizeof(int8_t);
        s_audioFormat.sampleOffset = sizeof(int8_t) * 2;
        break;
    case AUDIO_U8:
        s_audioFormat.type = ADLMIDI_SampleType_U8;
        s_audioFormat.containerSize = sizeof(uint8_t);
        s_audioFormat.sampleOffset = sizeof(uint8_t) * 2;
        break;
    case AUDIO_S16:
        s_audioFormat.type = ADLMIDI_SampleType_S16;
        s_audioFormat.containerSize = sizeof(int16_t);
        s_audioFormat.sampleOffset = sizeof(int16_t) * 2;
        break;
    case AUDIO_U16:
        s_audioFormat.type = ADLMIDI_SampleType_U16;
        s_audioFormat.containerSize = sizeof(uint16_t);
        s_audioFormat.sampleOffset = sizeof(uint16_t) * 2;
        break;
    case AUDIO_S32:
        s_audioFormat.type = ADLMIDI_SampleType_S32;
        s_audioFormat.containerSize = sizeof(int32_t);
        s_audioFormat.sampleOffset = sizeof(int32_t) * 2;
        break;
    case AUDIO_F32:
        s_audioFormat.type = ADLMIDI_SampleType_F32;
        s_audioFormat.containerSize = sizeof(float);
        s_audioFormat.sampleOffset = sizeof(float) * 2;
        break;
    }
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
        adl_close(pMidiPlayer);
        initialized_ = false;
        pMidiPlayer = nullptr;
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
 * \param channel The channel on which the volume is set. if -1, then it's all channels
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

void SdlMixerAudio::stopSound(int channel) {
    if (initialized_){
        Mix_HaltChannel(channel);
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

bool SdlMixerAudio::openFile(std::string path) {
    if (adl_openFile(pMidiPlayer, path.c_str()) < 0) {
        FSERR(Log::k_FLG_SND, "SdlMixerAudio", "openFile", ("Couldn't open music file: %s\n", adl_errorInfo(pMidiPlayer)));
        SDL_CloseAudio();
        adl_close(pMidiPlayer);
        return false;
    }
    //Mix_PauseMusic();
    is_playing = 0;

    return true;
}

/*!
 * A XMI file used in Syndicate can contain multiple songs.
 * @param songNb The song to play
 * @param loop True to play the sing indefinitly
 */
void SdlMixerAudio::playMusic(int songNb, bool loop) {
    pauseMusic();
    // We use the loop marker that are in the file
    adl_setLoopEnabled(pMidiPlayer, loop ? 1 : 0);
    adl_selectSongNum(pMidiPlayer, songNb);
    resumeMusic();
}

/*!
 * @brief 
 */
void SdlMixerAudio::pauseMusic() {
    is_playing = 0;
    Mix_PauseMusic();
}

/*!
 * @brief 
 */
void SdlMixerAudio::resumeMusic() {
    is_playing = 1;
    Mix_ResumeMusic();
}

/*!
 * @brief 
 */
void SdlMixerAudio::stopMusic() {
    is_playing = 0;
    //Mix_HaltMusic();
}

}

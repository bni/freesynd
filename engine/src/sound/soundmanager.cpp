/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
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

#include "fs-engine/sound/soundmanager.h"

#include <memory>

#include "fs-engine/sound/audio.h"
#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"

SoundManager::SoundManager():tabentry_startoffset_(58), tabentry_offset_(32)
{
    volumeBeforeMute_ = -1;
    disabled_ = false;
}

SoundManager::~SoundManager() {}

Sound *SoundManager::soundFromInGame(InGameSample sample)
{
    if (sample == NO_SOUND)
        return NULL;
    return sounds_.at(sample).get();
}

Sound *SoundManager::soundFromIntro(IntroSample sample)
{
    if (sample == kNoSound)
        return NULL;
    return introSounds_.at(sample).get();
}

void SoundManager::initialize(Audio* audio, bool disabled, bool doLoadIntroSounds) {
    audio_ = audio;
    disabled_ = disabled;

    LOG(Log::k_FLG_INFO, "SoundManager", "initialize", ("Loading sounds..."))

    if (disabled_) {
        FSINFO(Log::k_FLG_SND, "SoundManager", "initialize", ("Sound will be disabled\n"))
    } else {
        // Always load ingame sounds
        loadSounds(SAMPLES_GAME);
        if (doLoadIntroSounds) {
            loadSounds(SAMPLES_INTRO);
        }
    }
}

void SoundManager::loadSounds(SampleSet set)
{
    switch (set) {
    case SAMPLES_INTRO:
        {
            LOG(Log::k_FLG_SND, "SoundManager", "loadSounds", ("Loading intro sounds 1/2"))
            loadSounds("ISNDS-0.TAB", "ISNDS-0.DAT", introSounds_);

            LOG(Log::k_FLG_SND, "SoundManager", "loadSounds", ("Loading intro sounds 2/2"))
            loadSounds("ISNDS-1.TAB", "ISNDS-1.DAT", introSounds_);
        }
        break;
    case SAMPLES_GAME:
        {
            LOG(Log::k_FLG_SND, "SoundManager", "loadSounds", ("Loading game sounds 1/2"))
            loadSounds("SOUND-0.TAB", "SOUND-0.DAT", sounds_);

            LOG(Log::k_FLG_SND, "SoundManager", "loadSounds", ("Loading game sounds 2/2"))
            loadSounds("SOUND-1.TAB", "SOUND-1.DAT", sounds_);
        }
        break;
    default:
        break;
    }
}

void SoundManager::loadSounds(const std::string &tabFile, const std::string &datFile, std::vector<std::unique_ptr<Sound>> &sounds)
{
    size_t tabSize, datSize;

    uint8 *tabDataOri = File::loadOriginalFile(tabFile, tabSize);
    uint8 *soundDataOri = File::loadOriginalFile(datFile, datSize);

    uint8 *tabData = tabDataOri;
    uint8 *soundData = soundDataOri;

    tabData += tabentry_startoffset_;
    uint32 offset = 0;

    for (int i = 0; i < tabSize - tabentry_offset_;
        i += tabentry_offset_)
    {
        uint32 soundsize = READ_LE_UINT32(tabData);

        // Samples with size < 144 are bogus
        if (soundsize > 144) {
            sounds.push_back(audio_->createSound());
            uint8 *sample = new uint8[soundsize];
            memcpy(sample, soundData, soundsize);
            // patching wrong sample rate
            if (sounds.size() == 13)
                sample[0x1e] = 0x9c;
            else if (sounds.size() == 24)
                sample[0x1e] = 0x9c;
            else if (sounds.size() == 25)
                sample[0x1e] = 0x38;

            sounds.back()->loadSound(sample, soundsize);
            LOG(Log::k_FLG_SND, "SoundManager", "loadSounds", ("Sound loaded at offset %d", i))
            delete []sample;
        }
        soundData += soundsize;
        offset += soundsize;

        tabData += tabentry_offset_;
    }

    delete[] tabDataOri;
    delete[] soundDataOri;
}

bool SoundManager::canUseAudio() {
    return !disabled_ &&
            audio_ != NULL &&
            audio_->isInitialized();
}


/** \brief Play a sound from in game library
 *
 * \param sample InGameSample
 * \param channel int
 * \param loops int
 * \return void
 *
 */
void SoundManager::play(InGameSample sample, int channel, int loops) {
    if (canUseAudio()) {
        Sound *pSound = soundFromInGame(sample);

        if (pSound) {
            // Sound is played on first available channel (value -1)
            pSound->play(loops, -1);
        }
    }
}

/** \brief Play a sound from in intro library
 *
 * \param sample IntroSample
 * \param channel int
 * \param loops int
 * \return void
 *
 */
void SoundManager::playIntro(IntroSample sample, int channel, int loops) {
    if (canUseAudio()) {
        Sound *pSound = soundFromIntro(sample);

        if (pSound) {
            // Sound is played on first available channel (value -1)
            pSound->play(loops, -1);
        }
    }
}


/** \brief
 *
 * \param sample InGameSample
 * \return void
 *
 */
void SoundManager::stop(InGameSample sample) {
    if (canUseAudio()) {
        Sound *pSound = soundFromInGame(sample);

        if (pSound) {
            pSound->stop(sample >= MENU_UP ? 1 : 0);
        }
    }
}

/** \brief
 *
 * \param sample IntroSample
 * \return void
 *
 */
void SoundManager::stopIntro(IntroSample sample) {
    if (canUseAudio()) {
        Sound *pSound = soundFromIntro(sample);

        if (pSound) {
            // TODO (benblan): understand what channel to use
            pSound->stop(0);
        }
    }
}

void SoundManager::setVolume(int volume) {
    if (canUseAudio()) {
        audio_->setSoundVolume(volume);
    }
}

int SoundManager::getVolume() {
    if (canUseAudio()) {
        return audio_->getSoundVolume();
    }
    return 0;
}

/*!
 * Turns the music on/off.
 * The method watches for the value of field volumeBeforeMute_ :
 *  - if it's value is -1 : then music volume is on. So the method
 *    saves the level into volumeBeforeMute_ and sets the volume to zero
 *  - else it means volume is mute so the methods restores the original
 *    volume level.
 */
void SoundManager::toggleSound() {
    if (canUseAudio()) {
        if (volumeBeforeMute_ == -1) {
            volumeBeforeMute_ = audio_->getSoundVolume();
            LOG(Log::k_FLG_SND, "SoundManager", "toggleSound", ("Turning sound off : %d", volumeBeforeMute_))
            audio_->setSoundVolume(0);
        } else {
            LOG(Log::k_FLG_SND, "SoundManager", "toggleSound", ("Turning sound on : %d", volumeBeforeMute_))
            audio_->setSoundVolume(volumeBeforeMute_);
            volumeBeforeMute_ = -1;
        }
    }
}

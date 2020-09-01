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

#include "soundmanager.h"
#include "config.h"
#include "audio.h"
#include "utils/file.h"
#include "utils/log.h"

SoundManager::SoundManager(bool disabled):tabentry_startoffset_(58), tabentry_offset_(32), disabled_(disabled)
{
    volumeBeforeMute_ = -1;
}

SoundManager::~SoundManager()
{
    for(std::vector< Sound * >::iterator it = sounds_.begin(); it != sounds_.end(); ++it)
    {
        delete *it;
    }
}

Sound *SoundManager::sound(snd::InGameSample sample)
{
    if (sample == snd::NO_SOUND)
        return NULL;
    return sounds_.at(sample);
}


bool SoundManager::loadSounds(SampleSet set)
{
    int tabSize, size;
    uint8 *tabData, *data;
    switch (set) {
    case SAMPLES_INTRO:
        {
            tabData = File::loadOriginalFile("ISNDS-0.TAB", tabSize);
            data = File::loadOriginalFile("ISNDS-0.DAT", size);
            bool loaded = loadSounds(tabData, tabSize, data);
            delete[] tabData;
            delete[] data;
            if (!loaded) {
                printf("Error : Could not load sounds from file ISNDS-0.DAT\n");
                return false;
            }
            tabData = File::loadOriginalFile("ISNDS-1.TAB", tabSize);
            data = File::loadOriginalFile("ISNDS-1.DAT", size);
            loadSounds(tabData, tabSize, data);
            delete[] tabData;
            delete[] data;
            if (!loaded) {
                printf("Error : Could not load sounds from file ISNDS-1.DAT\n");
                return false;
            }
        }
        break;
    case SAMPLES_GAME:
        {
            tabData = File::loadOriginalFile("SOUND-0.TAB", tabSize);
            data = File::loadOriginalFile("SOUND-0.DAT", size);
            loadSounds(tabData, tabSize, data);
            delete[] tabData;
            delete[] data;
            tabData = File::loadOriginalFile("SOUND-1.TAB", tabSize);
            data = File::loadOriginalFile("SOUND-1.DAT", size);
            loadSounds(tabData, tabSize, data);
            delete[] tabData;
            delete[] data;
        }
        break;
    default:
        break;
    }

    return true;
}

bool SoundManager::loadSounds(uint8 * tabData, int tabSize,
                              uint8 * soundData)
{
    if (!tabData || !soundData) {
        return false;
    }
    
    tabData += tabentry_startoffset_;
    uint32 offset = 0;

    for (int i = 0; i < tabSize - tabentry_offset_;
        i += tabentry_offset_)
    {
        uint32 soundsize = READ_LE_UINT32(tabData);

        // Samples with size < 144 are bogus
        if (soundsize > 144) {
            sounds_.push_back(new Sound);
            uint8 *sample = new uint8[soundsize];
            memcpy(sample, soundData, soundsize);
            //printf("sample rate %x\n", sample[0x1e]);
            // patching wrong sample rate
            if (sounds_.size() == 13)
                sample[0x1e] = 0x9c;
            else if (sounds_.size() == 24)
                sample[0x1e] = 0x9c;
            else if (sounds_.size() == 25)
                sample[0x1e] = 0x38;
            sounds_.back()->loadSound(sample, soundsize);
            delete []sample;
        }
        soundData += soundsize;
        offset += soundsize;

        tabData += tabentry_offset_;
    }
    return true;
}

/*!
 *
 */
void SoundManager::play(snd::InGameSample sample, int channel, int loops) {
    if (disabled_) return;
    Sound *pSound = sound(sample);

    if (pSound) {
        // Sound is played on first available channel (value -1)
        pSound->play(loops, -1);
    }
}

/*!
 *
 */
void SoundManager::stop(snd::InGameSample sample) {
    if (disabled_) return;
    Sound *pSound = sound(sample);

    if (pSound) {
        pSound->stop(sample >= snd::MENU_UP ? 1 : 0);
    }
}

void SoundManager::setVolume(int volume) {
    Audio::setSoundVolume(volume);
}

int SoundManager::getVolume() {
    return Audio::getSoundVolume();
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
    if (volumeBeforeMute_ == -1) {
        volumeBeforeMute_ = Audio::getSoundVolume();
        LOG(Log::k_FLG_SND, "SoundManager", "toggleSound", ("Turning sound off : %d", volumeBeforeMute_))
        Audio::setSoundVolume(0);
    } else {
        LOG(Log::k_FLG_SND, "SoundManager", "toggleSound", ("Turning sound on : %d", volumeBeforeMute_))
        Audio::setSoundVolume(volumeBeforeMute_);
        volumeBeforeMute_ = -1;
    }
}

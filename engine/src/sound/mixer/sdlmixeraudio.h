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

#ifndef FS_SDLMIXER_AUDIO_H
#define FS_SDLMIXER_AUDIO_H

#include "fs-engine/sound/audio.h"

class SdlMixerAudio : public Audio {
public:
    SdlMixerAudio();

    ~SdlMixerAudio();

    //! Initialize the audio underneath implementation
    bool init(EFrequency frequency = FRQ_DEFAULT,
                     EFormat format = FMT_DEFAULT,
                     EChannel channels = CHN_DEFAULT,
                     int chunksize = 1024);

    //! Returns true if the audio system has been initialized with success
    bool isInitialized() { return initialized_; }
    //! Terminates the audio system
    bool quit();

    //! Returns the maximum volume possible
    int getMaxVolume();

    //! Sets the music volume to the given level
    void setMusicVolume(int volume);
    //! Returns the music volume
    int getMusicVolume();

    //! Sets the sound volume on the given channel to the given level
    void setSoundVolume(int volume, int channel = 0);
    //! Returns the sound volume of the given channel
    int getSoundVolume(int channel = 0);

    std::unique_ptr<Sound> createSound();

    std::unique_ptr<Music> createMusic();

private:
    /*! True if the audio system has been initialized with success.*/
    bool initialized_;

};

#endif //FS_SDLMIXER_AUDIO_H

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

#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include "common.h"
#include "sound.h"

#include <vector>

/*!
 * Sound manager class.
 */
class SoundManager {
public:
    enum SampleSet {
        SAMPLES_INTRO,
        SAMPLES_GAME
    };

    SoundManager(bool disabled);
    ~SoundManager();

    bool loadSounds(SampleSet set);

    //! Plays the sound a number a time on the given channel
    void play(snd::InGameSample sample, int channel = 0, int loops = 0);
    //! Stops the sound
    void stop(snd::InGameSample sample);

    //! Sets the music volume to the given level
    void setVolume(int volume);
    //! Returns the current volume
    int getVolume();
    //! Mute / unmute the music
    void toggleSound();

protected:
    Sound *sound(snd::InGameSample sample);
    bool loadSounds(uint8 *tabData, int tabSize, uint8 *soundData);

    const int tabentry_startoffset_;
    const int tabentry_offset_;
    std::vector<Sound *> sounds_;
    /*! 
     * Saves the volume level before a mute so
     * we can restore it after a unmute.
     */
    int volumeBeforeMute_;
    bool disabled_;
};

#endif

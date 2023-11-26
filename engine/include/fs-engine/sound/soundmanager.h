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

#include <vector>
#include <memory>

#include "fs-utils/common.h"
#include "fs-utils/misc/singleton.h"
#include "fs-engine/sound/sound.h"
#include "fs-engine/sound/audio.h"

/*!
 * Sound manager class.
 */
class SoundManager : public Singleton < SoundManager > {
public:
    enum SampleSet {
        SAMPLES_INTRO,
        SAMPLES_GAME
    };

    SoundManager();
    ~SoundManager();

    //! Loads the sounds for the game and possibly for the intro
    void initialize(Audio* audio, bool disabled, bool doLoadIntroSounds);

    //! Plays the sound a number a time on the given channel
    void play(InGameSample sample, int channel = 0, int loops = 0);
    //! Stops the sound
    void stop(InGameSample sample);
    //! Plays the sound a number a time on the given channel
    void playIntro(IntroSample sample, int channel = 0, int loops = 0);
    //! Stops the sound
    void stopIntro(IntroSample sample);

    //! Sets the music volume to the given level
    void setVolume(int volume);
    //! Returns the current volume
    int getVolume();
    //! Mute / unmute the music
    void toggleSound();

protected:
    Sound *soundFromInGame(InGameSample sample);
    Sound *soundFromIntro(IntroSample sample);

    void loadSounds(SampleSet set);

    void loadSounds(const std::string &tabFile, const std::string &datFile, std::vector<std::unique_ptr<Sound>> &sounds);
    bool canUseAudio();

protected:
    const int tabentry_startoffset_;
    const int tabentry_offset_;
    //!Store the sounds that are used during the game
    std::vector<std::unique_ptr<Sound>> sounds_;
    //! Store the sounds used only for the intro
    std::vector<std::unique_ptr<Sound>> introSounds_;
    /*!
     * Saves the volume level before a mute so
     * we can restore it after a unmute.
     */
    int volumeBeforeMute_;
    bool disabled_;
    Audio *audio_;
};

#define g_SoundMgr   SoundManager::singleton()

#endif

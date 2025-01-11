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

#include "fs-engine/sound/musicmanager.h"

#include "fs-engine/sound/audio.h"
#include "fs-engine/sound/xmidi.h"
#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"

namespace fs_eng {

MusicManager::MusicManager()
{
    // -1 means music is not mute
    // other value stores music volume before mute
    volumeBeforeMute_ = -1;
    audio_ = NULL;
    disabled_ = false;
    currentFile_ = kMusicFileNoFile;
}

MusicManager::~MusicManager()
{
}

bool MusicManager::isAudioInitialized() {
    return audio_ != NULL && audio_->isInitialized();
}

void MusicManager::initialize(bool disabled, Audio* audio)
{
    LOG(Log::k_FLG_SND, "MusicManager", "initialize", ("Initialize MusicManager..."))
    audio_ = audio;
    disabled_ = disabled;
    if (disabled_) {
        FSINFO(Log::k_FLG_SND, "MusicManager", "initialize", ("Music will be disabled\n"))
    }
    // If audio has not been initialized -> do nothing
    if (!isAudioInitialized()) {
        return;
    }

    LOG(Log::k_FLG_SND, "MusicManager", "initialize", ("Music initialized"))
}

/*!
 * Play the given track.
 * @param song 
 * @param loops True to loop indefinetly
 */
void MusicManager::playSong(MusicSong song, bool loopForEver) {
    if (!disabled_) {
        int songNb;

        MusicFile fileToOpen = (song == kMusicSongIntro) ? kMusicFileIntro : kMusicFileGame;
        std::string fullFilename = fs_utl::File::getOriginalDataFullPath(
                                                    (song == kMusicSongIntro) ? "INTRO.XMI" : "SYNGAME.XMI", 
                                                    true);

        if (currentFile_ != fileToOpen && !audio_->openFile(fullFilename)) {
            FSERR(Log::k_FLG_SND, "MusicManager", "playSong", ("Cannot open file %s\n", fullFilename.c_str()))
            return;
        }
        
        switch (song)
        {
        case kMusicSongIntro:
            songNb = 0;
            break;
        case kMusicSongAssassinate:
            songNb = 0;
            break;
        case kMusicSongDanger:
            songNb = 1;
            break;
        case kMusicSongGameCompleted:
            songNb = 2;
            break;
        case kMusicSongMissionCompleted:
            songNb = 3;
            break;
        case kMusicSongMissionFailed:
            songNb = 4;
            break;
        default:
            songNb = 0;
            break;
        }
        
        audio_->playMusic(songNb, loopForEver);
    }
}

/*!
 * Stop the track that is playing
 */
void MusicManager::stopCurrentSong() {
    audio_->stopMusic();
}

/*!
 * @brief 
 */
void MusicManager::pause() {
    audio_->pauseMusic();
}

/*!
 * @brief 
 */
void MusicManager::resume() {
    audio_->resumeMusic();
}

void MusicManager::setVolume(int volume) {
    if (isAudioInitialized()) {
        audio_->setMusicVolume(volume);
    }
}

int MusicManager::getVolume() {
    if (isAudioInitialized()) {
        return audio_->getMusicVolume();
    } else {
        return 0;
    }
}

/*!
* Turns the music on/off.
* The method watches for the value of field volumeBeforeMute_ :
*  - if it's value is -1 : then music volume is on. So the method
*    saves the level into volumeBeforeMute_ and sets the volume to zero
*  - else it means volume is mute so the methods restores the original
*    volume level.
*/
void MusicManager::toggleMusic() {
    if (isAudioInitialized()) {
        if (volumeBeforeMute_ == -1) {
            volumeBeforeMute_ = audio_->getMusicVolume();
            LOG(Log::k_FLG_SND, "MusicManager", "toggleMusic", ("Turning music off : %d", volumeBeforeMute_))
            audio_->setMusicVolume(0);
        } else {
            LOG(Log::k_FLG_SND, "MusicManager", "toggleMusic", ("Turning music on : %d", volumeBeforeMute_))
            audio_->setMusicVolume(volumeBeforeMute_);
            volumeBeforeMute_ = -1;
        }
    }
}
}
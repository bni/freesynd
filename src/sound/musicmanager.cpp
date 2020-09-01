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

#include "config.h"
#include "audio.h"
#include "musicmanager.h"
#include "xmidi.h"
#include "utils/file.h"
#include "utils/log.h"

MusicManager::MusicManager(bool disabled):is_playing_(false), disabled_(disabled)
{
    // -1 means music is not mute
    // other value stores music volume before mute
    volumeBeforeMute_ = -1;
}

MusicManager::~MusicManager()
{
    for (unsigned int i = 0; i < tracks_.size(); ++i) {
        delete tracks_[i];
    }
    tracks_.clear();
}

void MusicManager::loadMusic()
{
    // If audio has not been initialized -> do nothing
    if (!Audio::isInitialized()) {
        return;
    }

    XMidi xmidi;
    std::vector < XMidi::Midi > tracks;
    int size;
    uint8 *data;

#if USE_INTRO_OGG
    tracks_.push_back(new Music);
    tracks_.back()->loadMusicFile("music/intro.ogg");
#else
    data = File::loadOriginalFile("INTRO.XMI", size);
    tracks = xmidi.convertXMidi(data, size);
    for (unsigned int i = 0; i < tracks.size(); ++i) {
        tracks_.push_back(new Music);
        tracks_.back()->loadMusic(tracks[i].data_, tracks[i].size_);
    }
    delete[] data;
#endif

    data = File::loadOriginalFile("SYNGAME.XMI", size);
    tracks = xmidi.convertXMidi(data, size);
    for (unsigned int i = 0; i < tracks.size(); ++i) {
        if (i == 0) {
#if USE_ASSASSINATE_OGG
            tracks_.push_back(new Music);
            tracks_.back()->loadMusicFile("music/assassinate.ogg");
#else
            tracks_.push_back(new Music);
            tracks_.back()->loadMusic(tracks[i].data_, tracks[i].size_);
#endif
        } else {
            tracks_.push_back(new Music);
            tracks_.back()->loadMusic(tracks[i].data_, tracks[i].size_);
        }
    }
    delete[] data;
}

void MusicManager::playTrack(msc::MusicTrack track, int loops)
{
    if (disabled_) return;
    if (Audio::isInitialized()) {
        if (is_playing_) {
            tracks_.at(current_track_)->stopFadeOut();
        }
        tracks_.at(track)->play(loops);
        current_track_ = track;
        is_playing_ = true;
    }
}

void MusicManager::stopPlayback()
{
    if (disabled_) return;
    if (Audio::isInitialized() && is_playing_) {
        tracks_.at(current_track_)->stop();
        is_playing_ = false;
    }
}

void MusicManager::setVolume(int volume) {
    Audio::setMusicVolume(volume);
}

int MusicManager::getVolume() {
    return Audio::getMusicVolume();
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
    if (volumeBeforeMute_ == -1) {
        volumeBeforeMute_ = Audio::getMusicVolume();
        LOG(Log::k_FLG_SND, "MusicManager", "toggleMusic", ("Turning music off : %d", volumeBeforeMute_))
        Audio::setMusicVolume(0);
    } else {
        LOG(Log::k_FLG_SND, "MusicManager", "toggleMusic", ("Turning music on : %d", volumeBeforeMute_))
        Audio::setMusicVolume(volumeBeforeMute_);
        volumeBeforeMute_ = -1;
    }
}

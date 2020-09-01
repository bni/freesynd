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

#ifndef MUSICMANAGER_H
#define MUSICMANAGER_H

#include "common.h"
#include "music.h"

#include <vector>

/*!
 * Music manager class.
 */
class MusicManager {
public:
    MusicManager(bool disabled);
    ~MusicManager();

    void loadMusic();
    void playTrack(msc::MusicTrack track, int loops = -1);
    void stopPlayback();
    //! Sets the music volume to the given level
    void setVolume(int volume);
    //! Returns the current volume
    int getVolume();
    //! Mute / unmute the music
    void toggleMusic();

protected:
    std::vector<Music *> tracks_;
    msc::MusicTrack current_track_;
    bool is_playing_;
    /*! 
     * Saves the volume level before a mute so
     * we can restore it after a unmute.
     */
    int volumeBeforeMute_;
    bool disabled_;
};

#endif

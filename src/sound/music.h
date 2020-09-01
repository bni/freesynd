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

#ifndef MUSIC_H
#define MUSIC_H

#include "config.h"
#include "common.h"

namespace msc {
    enum MusicTrack {
        TRACK_INTRO,
        TRACK_ASSASSINATE,
        TRACK_DANGER,
        TRACK_GAME_COMPLETED,
        TRACK_MISSION_FAILED,
        TRACK_MISSION_COMPLETED,
        NO_TRACK = -1
    };
};

#ifdef HAVE_SDL_MIXER

// Load the SDL_Mixer implementation
#include "sdlmixermusic.h"

// This macro is used to hide the implementation class
#define Music SdlMixerMusic

#else

/*!
 * Dummy implementation of music.
 */
class DefaultMusic {
  public:
    //! Play the music
    /*! 
     * \param loops = -1 means play forever
     */
      void play(int loops = -1) const {;}
      //! Plays the music with a fade in.
    /*!
     * \param loops
     * \param ms
     */
      void playFadeIn(int loops = -1, int ms = 200) const {;}
      //! Stops the music
    /*!
     *
     */
      void stop() const {;}
      //! Stops the music with a fade out.
    /*!
     * \param ms
     */
      void stopFadeOut(int ms = 200) const {;}
      //! Loads the music from the given data.
    /*!
     * \param musicData
     * \param size
     */
      bool loadMusic(uint8 *musicData, int size) { return true; }
      //! Loads the music from the given file.
    /*!
     * \param fname
     */
      bool loadMusicFile(const char *fname) { return true;}
};

#define Music DefaultMusic

#endif  // HAVE_SDL_MIXER

#endif  // MUSIC_H

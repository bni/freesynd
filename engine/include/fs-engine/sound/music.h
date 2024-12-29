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

#include "fs-utils/common.h"

namespace fs_eng {

class Music {
  public:
    enum MusicTrack {
        TRACK_INTRO,
        TRACK_ASSASSINATE,
        TRACK_DANGER,
        TRACK_GAME_COMPLETED,
        TRACK_MISSION_FAILED,
        TRACK_MISSION_COMPLETED,
        NO_TRACK = -1
    };

    virtual ~Music() {}

    //! Play the music
    /*!
     * \param loops = -1 means play forever
     */
     virtual void play(int loops = -1) const = 0;
      //! Plays the music with a fade in.
    /*!
     * \param loops
     * \param ms
     */
    virtual  void playFadeIn(int loops = -1, int ms = 200) const = 0;
      //! Stops the music
    /*!
     *
     */
    virtual void stop() const = 0;
      //! Stops the music with a fade out.
    /*!
     * \param ms
     */
    virtual void stopFadeOut(int ms = 200) const = 0;
      //! Loads the music from the given data.
    /*!
     * \param musicData
     * \param size
     */
    virtual bool loadMusic(uint8 *musicData, size_t size) = 0;
      //! Loads the music from the given file.
    /*!
     * \param fname
     */
    virtual bool loadMusicFile(const char *fname) = 0;
};

#if !defined(HAVE_SDL_MIXER) || HAVE_SDL_MIXER == 0

/*!
 * Dummy implementation of music.
 */
class DefaultMusic : public Music {
  public:
    //! Play the music
    /*!
     * \param loops = -1 means play forever
     */
      void play([[maybe_unused]] int loops = -1) const {;}
      //! Plays the music with a fade in.
    /*!
     * \param loops
     * \param ms
     */
      void playFadeIn([[maybe_unused]] int loops = -1, [[maybe_unused]] int ms = 200) const {;}
      //! Stops the music
    /*!
     *
     */
      void stop() const {;}
      //! Stops the music with a fade out.
    /*!
     * \param ms
     */
      void stopFadeOut([[maybe_unused]] int ms = 200) const {;}
      //! Loads the music from the given data.
    /*!
     * \param musicData
     * \param size
     */
      bool loadMusic([[maybe_unused]] uint8 *musicData, [[maybe_unused]] size_t size) { return true; }
      //! Loads the music from the given file.
    /*!
     * \param fname
     */
      bool loadMusicFile([[maybe_unused]] const char *fname) { return true;}
};

#endif  // HAVE_SDL_MIXER

}

#endif  // MUSIC_H

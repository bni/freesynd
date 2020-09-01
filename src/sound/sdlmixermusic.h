/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
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

#ifndef SDLMIXERMUSIC_H
#define SDLMIXERMUSIC_H

#ifdef HAVE_SDL_MIXER

#include <SDL_mixer.h>

/*!
 * Implementation of music using SDL_Mixer.
 */
class SdlMixerMusic {
  public:
    SdlMixerMusic();
    ~SdlMixerMusic();

    void play(int loops = -1) const;
    void playFadeIn(int loops = -1, int ms = 200) const;
    void stop() const;
    void stopFadeOut(int ms = 200) const;
    bool loadMusic(uint8 *musicData, int size);
    bool loadMusicFile(const char *fname);

  protected:
    Mix_Music *music_data_;
    SDL_RWops *rw_;
};

#endif  // HAVE_SDL_MIXER

#endif  //SDLMIXERMUSIC_H

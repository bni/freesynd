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

#ifdef HAVE_SDL_MIXER

#include "utils/file.h"
#include "sdlmixermusic.h"

#include <SDL.h>
#include <SDL_mixer.h>

/*!
 * Class constructor.
 */
SdlMixerMusic::SdlMixerMusic():music_data_(NULL),rw_(NULL)
{
}

/*!
 * Class destructor.
 * Free the music data if it was allocated.
 */
SdlMixerMusic::~SdlMixerMusic()
{
    if (music_data_) {
        Mix_FreeMusic(music_data_);
    }

    if (rw_)
        SDL_FreeRW(rw_);
}

/*!
 * Plays the music a number of times.
 * \param loops The number of times music is played.
 */
void SdlMixerMusic::play(int loops) const
{
    Mix_PlayMusic(music_data_, loops);
}

/*!
 * Plays the music with a fade in.
 * \param loops The number of times music is played.
 * \param ms The length in milliseconds of the fade in.
 */
void SdlMixerMusic::playFadeIn(int loops, int ms) const
{
    Mix_FadeInMusic(music_data_, loops, ms);
}

/*!
 * Stops the music from playing.
 */
void SdlMixerMusic::stop() const
{
    Mix_HaltMusic();
    Mix_RewindMusic();
}

/*!
 * Stops the music with a fadeout.
 * \param ms The length in milliseconds of the fade out.
 */
void SdlMixerMusic::stopFadeOut(int ms) const
{
    while (!Mix_FadeOutMusic(ms) && Mix_PlayingMusic()) {
        SDL_Delay(100);
    }
    Mix_RewindMusic();
}

/*!
 * Loads the music from the given data.
 * \param musicdata The data from original resource
 * \param size The size of the data.
 * \return true if the music was loaded.
 */
bool SdlMixerMusic::loadMusic(uint8 * musicData, int size)
{
    SDL_RWops *rw = SDL_RWFromMem(musicData, size);
    if (!rw) {
        Audio::error("SdlMixerMusic", "loadMusic", "Failed creating SDL_RW buffer from memory");
        return false;
    }
    Mix_Music *newmusic = Mix_LoadMUS_RW(rw);

    if (!newmusic) {
        Audio::error("SdlMixerMusic", "loadMusic", "Failed loading music from SDL_RW buffer");
        SDL_FreeRW(rw);
        return false;
    }
    if (music_data_)
        Mix_FreeMusic(music_data_);

    if (rw_) {
        SDL_FreeRW(rw_);
        rw_ = NULL;
    }

    music_data_ = newmusic;
    rw_ = rw;

    return true;
}

/*!
 * Loads the music from the given file.
 * \param musicdata The name of the file.
 * \return true if the music was loaded.
 */
bool SdlMixerMusic::loadMusicFile(const char *fname)
{
    Mix_Music *newmusic = Mix_LoadMUS(File::dataFullPath(fname).c_str());

    if (!newmusic) {
        Audio::error("SdlMixerMusic", "loadMusicFile", "Failed loading music from file");
        return false;
    }
    if (music_data_)
        Mix_FreeMusic(music_data_);

    if (rw_) {
        SDL_FreeRW(rw_);
        rw_ = NULL;
    }

    music_data_ = newmusic;

    return true;
}


#endif                          // HAVE_SDL_MIXER

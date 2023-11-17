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

#include "fs-engine/sound/audio.h"

#include <iostream>
#include <SDL.h>

#include "fs-utils/log/log.h"

//Audio::~Audio() {}

#ifndef HAVE_SDL_MIXER

DefaultAudio::DefaultAudio() {}

DefaultAudio::~DefaultAudio() {}

bool DefaultAudio::init(EFrequency freq, EFormat fmt, EChannel chan, int chunksize) {

    LOG(Log::k_FLG_SND, "Audio", "init", ("Initializing Dummy sound system..."));

    LOG(Log::k_FLG_SND, "Audio", "init", ("Sound system initialized"));

    return true;
}

bool DefaultAudio::isInitialized(){
    return true;
}

bool DefaultAudio::quit(void) {
    return true;
}

void DefaultAudio::setMusicVolume(int volume)
{
}

int DefaultAudio::getMusicVolume() {
    return 0;
}

/*!
 * Sets the music volume.
 * \param volume The volume level. If greater than the maximum
 * volume, then the maximum is set.
 * \param channel The channel on which the volume is set.
 */
void DefaultAudio::setSoundVolume(int volume, int channel)
{
}

/*!
 * Returns the music volume.
 * \return -1 if the system has not been initialized.
 */
int DefaultAudio::getSoundVolume(int channel) {
    return 0;
}

int DefaultAudio::getMaxVolume() {
    return 0;
}

std::unique_ptr<Sound> DefaultAudio::createSound() {
    return std::make_unique<DefaultSound>();
}

std::unique_ptr<Music> DefaultAudio::createMusic() {
    return std::make_unique<DefaultMusic>();
}

#endif // HAVE_SDL_MIXER

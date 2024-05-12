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

#ifndef SOUND_H
#define SOUND_H

#include "fs-utils/common.h"

enum IntroSample {
    kSound1 = 0,
    kNoSound = -1
};
/*!
 * These enum values match the indices in the vector containing the samples
 * so don't mess up the order in which they are in.
 */
enum InGameSample {
    SHOTGUN = 0,
    PISTOL,
    LASER,
    FLAME,
    FLAMING_DEATH,
    GLASS_BREAKING,
    EXPLOSION,
    UZI,
    LONGRANGE,
    MINIGUN,
    PERSUADE,
    TRACKING_PONG,
    SPEECH_SELECTED,
    GAUSSGUN,
    SPEECH_MISSION_COMPLETED,
    SPEECH_MISSION_FAILED,
    DOOR,
    TIMEBOMB,
    DOOR_2,
    PUTDOWN_WEAPON,
    MENU_UP = 20,
    MENU_CHANGE,
    FIREWORKS,
    SPEECH_NO,
    // mission failed, lamp-monitor impact
    MONITOR_IMPACT,
    // mission failed, lamp breaks
    GLASS_BREAKING_2,
    APPLAUSE,
    APPLAUSE_ZOOM,
    FIREWORKS_APPLAUSE,
    EXPLOSION_BIG,
    MENU_AFTER_MISSION,
    FALLING_COMMAND_SHIP,
    // mission failed, pressed button on chair
    PRESS_BUTTON,
    NO_SOUND = -1
};

/*!
 * This is an abstraction of sound. It's a pure virtual class.
 */
class Sound {
public:
    virtual ~Sound() {}
    virtual void play(int loops = 0, int channel = 0) const = 0;
    virtual void stop(int channel = 0) const  = 0;
    virtual bool setVolume(int volume)  = 0;
    virtual bool loadSound(uint8 *soundData, uint32 size)  = 0;
};

#if !defined(HAVE_SDL_MIXER) || HAVE_SDL_MIXER == 0

//! Default implementation for the sound.
/*!
 * This class is a dummy implementation. It does nothing.
 */
class DefaultSound : public Sound {
public:
    void play([[maybe_unused]] int loops = 0, [[maybe_unused]] int channel = 0) const {;}
    void stop([[maybe_unused]] int channel = 0) const {;}
    bool setVolume([[maybe_unused]] int volume) { return true; }
    bool loadSound([[maybe_unused]] uint8 *soundData, [[maybe_unused]] uint32 size) { return true; }
};

#endif  // HAVE_SDL_MIXER

#endif  // SOUND_H

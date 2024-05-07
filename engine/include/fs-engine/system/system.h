/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#ifndef SYSTEM_H
#define SYSTEM_H

#include <memory>

#include "fs-utils/common.h"
#include "fs-utils/misc/singleton.h"
#include "fs-engine/io/keys.h"
#include "fs-engine/sound/audio.h"

class Sprite;

enum FS_EventType {
    EVT_NONE = 0,        // No event in the queue
    EVT_QUIT = 1,        // Quit event
    EVT_MSE_MOTION = 2,    // Mouse motion
    EVT_MSE_UP = 3,
    EVT_MSE_DOWN = 4,
    EVT_KEY_DOWN = 5
};

/** The "quit requested" event */
struct FS_QuitEvent {
    FS_EventType type;
};

struct FS_MouseMotionEvent {
    FS_EventType type;    /**< SDL_MOUSEMOTION */
    uint32 state;    /**< The current button state */
    int32 x, y;    /**< The X/Y coordinates of the mouse */
    int keyMods;
};

struct FS_MouseButtonEvent {
    FS_EventType type;    /**< SDL_MOUSEMOTION */
    uint8 button;    /**< The current button state */
    int32 x, y;    /**< The X/Y coordinates of the mouse */
    int keyMods;
};

struct FS_KeyEvent {
    FS_EventType type;    /**< EVT_KEY_DOWN */
    FS_Key key;
    int keyMods;
};

/** General event structure */
union FS_Event {
    FS_EventType type;

    FS_QuitEvent quit;
    FS_MouseMotionEvent motion;
    FS_MouseButtonEvent button;
    FS_KeyEvent key;
};

/*!
 * Abstract interface that all systems/ports should implement.
 */
class System : public Singleton<System> {
public:
    static std::unique_ptr<System> createSystem();

    virtual ~System() {}
    virtual bool initialize(bool fullscreen) = 0;
    virtual void updateScreen() = 0;
    //! Pumps an event from the event queue
    virtual bool pumpEvents(FS_Event &evtOut) = 0;
    virtual void delay(uint32 msec) = 0;
    virtual uint32 getTicks() = 0;

    virtual bool setPalette6b3(const uint8 *pal, int cols = 256) = 0;
    virtual bool setPalette8b3(const uint8 *pal, int cols = 256) = 0;
    virtual void setColor(uint8 index, uint8 r, uint8 g, uint8 b) = 0;

    //! Returns the mouse pointer coordinates
    virtual uint32 getMousePos(int *x, int *y) = 0;
    //! Hides the mouse cursor.
    virtual void hideCursor() = 0;
    //! Shows the mouse cursor.
    virtual void showCursor() = 0;
    //! Use this cursor for menu screens
    virtual void useMenuCursor() = 0;
    virtual void usePointerCursor() = 0;
    virtual void usePointerYellowCursor() = 0;
    virtual void useTargetCursor() = 0;
    virtual void useTargetRedCursor() = 0;
    virtual void usePickupCursor() = 0;

    virtual int getKeyModState() = 0;

    virtual bool isKeyModStatePressed(const int keyMod) = 0;

    //! Call this method to activate the text event
    virtual void startReceiveText() = 0;
    //! Call this method to deactivate the text event
    virtual void stopReceiveText() = 0;

    //! Return a pointer to the Audio system instance
    Audio* getAudio() {return audio_.get();}

protected:
    std::unique_ptr<Audio> audio_;
};

#define g_System    System::singleton()

#endif

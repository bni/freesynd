/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
 *
 *   This program is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as 
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>. 
 * 
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#include <memory>

#include "fs-utils/common.h"
#include "fs-utils/misc/singleton.h"
#include "fs-engine/enginecommon.h"
#include "fs-engine/io/keys.h"
#include "fs-engine/sound/audio.h"
#include "fs-engine/gfx/fstexture.h"

class Sprite;

namespace fs_eng {

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
    uint32_t state;    /**< The current button state */
    int32_t x, y;    /**< The X/Y coordinates of the mouse */
    int keyMods;
};

struct FS_MouseButtonEvent {
    FS_EventType type;    /**< SDL_MOUSEMOTION */
    uint8 button;    /**< The current button state */
    int32_t x, y;    /**< The X/Y coordinates of the mouse */
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

    virtual void initialize(bool fullscreen) = 0;

    // Display an error message in a visible way for the user
    virtual void showError(const char *errorMsg) = 0;

    virtual bool clearScreen() = 0;
    virtual void updateScreen() = 0;
    // Set the render target to be the default one
    virtual bool resetRenderTarget() = 0;
    //! Set the viewport for the drawing area
    virtual bool setViewport(int x, int y, int width, int height) = 0;
    // Reset to default viewport
    virtual bool resetViewport() = 0;
    //! Switch between 4:3 stretched (menus) and native 8:5 (gameplay) scaling
    virtual void setNativeAspectRatio(bool native) = 0;
    //! Returns the current game texture height (400 in menu mode, dynamic in gameplay)
    virtual int getGameHeight() const = 0;
    //! Returns the current game texture width (640 in menu mode, dynamic in gameplay)
    virtual int getGameWidth() const = 0;

    //! Pumps an event from the event queue
    virtual bool pumpEvents(FS_Event &evtOut) = 0;
    virtual void delay(uint32_t msec) = 0;
    virtual uint32_t getTicks() = 0;

    //! Draw a point on the screen at given position and color
    virtual void drawPoint (Point2D start, fs_eng::FSColor color) = 0;
    /**
     * Draw a vertical line with the given color
     */
    virtual void drawVLine(Point2D start, int length, fs_eng::FSColor color) = 0;
    //! Draw a vertical dashed line with the given color
    virtual void drawDashedVLine(Point2D start, int length, int dashLength, int dashOffset, fs_eng::FSColor color) = 0;
    /**
     * Draw a horizontal line with the given color
     */
    virtual void drawHLine(Point2D start, int length, fs_eng::FSColor color) = 0;
    //! Draw a horizontal dashed line with the given color
    virtual void drawDashedHLine(Point2D start, int length, int dashLength, int dashOffset, fs_eng::FSColor color) = 0;
    /**
     * Draw a line with the given color
     */
    virtual void drawLine(Point2D start, Point2D end, fs_eng::FSColor color) = 0;
    /**
     * Draw a rectangle with the given color
     */
    virtual void drawRect(Point2D pos, int width, int height, fs_eng::FSColor color) = 0;
    /*!
     * @brief Draw a rect filled with the given color
     * @param pos Upper left corner position of the rect
     * @param width Width of the rectangle
     * @param height Height of the rectangle
     * @param color Color to fill the rectangle
     */
    virtual void drawFillRect(Point2D pos, int width, int height, fs_eng::FSColor color) = 0;

    //! Returns the mouse pointer coordinates
    virtual uint32_t getMousePos(Point2D &point) = 0;
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

    //! Returns true if the given key is currently held down (polled, not event-based)
    virtual bool isKeyDown(fs_eng::FS_KeyCode key) const = 0;

    //! Call this method to activate the text event
    virtual void startReceiveText() = 0;
    //! Call this method to deactivate the text event
    virtual void stopReceiveText() = 0;

    //! Return a pointer to the Audio system instance
    fs_eng::Audio* getAudio() {return audio_.get();}

    //! Create a FSTexture with the current system
    virtual std::unique_ptr<FSTexture> createTexture() = 0;

    /*!
     * Return the language based on the system preferences.
     * @return English is the default
     */
    virtual fs_eng::FS_Lang getLanguageFromSystem() = 0;

protected:
    std::unique_ptr<fs_eng::Audio> audio_;
};
}

#define g_System    fs_eng::System::singleton()

#endif

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2010-2024                                            *
 *      Benoit Blancard <benblan@users.sourceforge.net>                 *
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

#ifndef SYSTEM_SDL_H
#define SYSTEM_SDL_H

#include <SDL.h>

#include "fs-engine/system/system.h"
#include "fs-engine/io/keys.h"

/*! \brief Implementation of the System interface for SDL.
 *
 * This class implements the System interface based on the SDL2 library.
 *  - Display
 *    Every object in the game is drawn on a simple uint8 array in the Screen
 *    class. Then in SystemSDL::updateScreen(), we copy this array into a
 *    Uint32 array that matches the display format and this array is then copied
 *    to an SDL Texture. This texture is then copied on the back buffer before
 *    presenting the scree.
 *  - Mouse Cursor
 *    In order to display colorfull cursors, the SDL cursor display is disabled
 *    and is manually managed by this class with a SDL_Texture.
 *    A surface is loaded with a collection of sprites, and every time the
 *    mouse moves, the corresponding sprite is blit on screen at the
 *    mouse coordinates.\n
 *    If the SDLSystem fails to load the cursor surface, the default SDL cursor
 *    will be used, but not change will affect the cursor, except hide/show.
 */
class SystemSDL : public System {
public:
    SystemSDL();
    ~SystemSDL();

    //! Initialize the SDL resources
    bool initialize(bool fullscreen) override;
    //! Render back buffer to the screen
    void updateScreen() override;
    //! Pumps an event from the event queue
    bool pumpEvents(FS_Event &evtOut) override;

    void delay(uint32 msec) override;
    uint32 getTicks() override;

    bool setPalette6b3(const uint8 *pal, int cols = 256) override;
    bool setPalette8b3(const uint8 *pal, int cols = 256) override;
    void setColor(uint8 index, uint8 r, uint8 g, uint8 b) override;

    //! Returns the mouse pointer coordinates
    uint32 getMousePos(int *x, int *y) override;

    void hideCursor() override;
    void showCursor() override;
    void useMenuCursor() override;
    void usePointerCursor() override;
    void usePointerYellowCursor() override;
    void useTargetCursor() override;
    void useTargetRedCursor() override;
    void usePickupCursor() override;
    int getKeyModState() override {
        return keyModState_;
    }

    bool isKeyModStatePressed(const int keyMod) override {
        return (keyModState_ & keyMod) != 0;
    }

    //! Call this method to activate the text event
    void startReceiveText() override;
    //! Call this method to deactivate the text event
    void stopReceiveText() override;


protected:
    //! Creates a SDL window either for fullscreen or not
    SDL_Window * createWindow(bool fullscreen);
    //! Loads the graphic file that contains the cursor sprites.
    bool loadCursorSprites();

    //! Sets the key arguments with some key codes
    void fillKeyEvent(SDL_Keysym sym, FS_Event &evtOut);

protected:
    /*! A constant that holds the cursor icon width and height.*/
    static const int kCursorWidth;
    /*! Cursor visibility.*/
    bool cursor_visible_;
    /*! Cursor screen coordinates. */
    int32 cursor_x_;
    /*! Cursor screen coordinates. */
    int32 cursor_y_;
    /*! Current cursor hotspot.*/
    int cursor_hs_x_;
    /*! Current cursor hotspot.*/
    int cursor_hs_y_;

    //! The main window
    SDL_Window *pWindow_;
    //! The renderer is necessary to manipulate SDL_Texture and use graphic acceleration
    SDL_Renderer *pRenderer_;

    /*!
     * This array is used to transform the Screen::pixels that are uint8 into Uint32 pixels.
     */
    Uint32 *pixels_;
    /*!
     * This surface is only used to store the current palette. It should replace completely
     * the pixels_ array.
     * NOTE : There is currently a bug when using directly the surface to copy pixels : getting
     * an error message "double free or corruption (!prev)" when exiting the application.
     */
    SDL_Surface *pScreenSurface_;
    /*!
     * A texture to render the screen using hardware acceleration.
     */
    SDL_Texture *pScreenTexture_;

    /*!
     * A texture that holds all cursors images.
     */
    SDL_Texture *pCursorTexture_;
    /*! A rect that identify the part of
     * the cursor surface for the current cursor.*/
    SDL_Rect cursor_rect_;
    /*! A flag that tells that cursor must be updated because
     the mouse has moved or the cursor has changed.*/
    bool update_cursor_;
    /*!
     * This field is a bit buffer storing the state of modifier buttons.
     * When a bit is set, that means a button is pressed.
     * See KeyMod enumeration to know all modifier buttons.
     */
    int keyModState_;
};

#endif

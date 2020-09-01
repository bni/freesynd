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

#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include "common.h"
#include "menu.h"
#include "gfx/dirtylist.h"
#include <string>
#include <map>

class SpriteManager;
class ConfigFile;
class MenuManager;
class SoundManager;

/*!
 * This abstract class is responsible for instanciating menus from a given id.
 */
class MenuFactory {
public:
    virtual ~MenuFactory() {}

    virtual Menu * createMenu(const int menuId) = 0;
    void setMenuManager(MenuManager *pManager) { pManager_ = pManager; }

protected:
    MenuManager *pManager_;
};

/*!
 * Menu manager class.
 */
class MenuManager {
public:
    MenuManager(MenuFactory *pFactory, SoundManager *pGameSounds);
    ~MenuManager();

    bool initialize(bool loadIntroFont);

    //! Destroy all menus and resources
    void destroy();

    //! Return the menu sprites manager
    SpriteManager &menuSprites() {
        return menuSprites_;
    }

    FontManager &fonts() {
        return fonts_;
    }

    /*! Reads events from the event queue and dispatches them.*/
    void handleEvents();

    void handleTick(int elapsed) {
        if (current_)
            current_->handleTick(elapsed);

        if (nextMenuId_ != -1) {
            changeCurrentMenu();
        }
    }

    // Change the menu
    void gotoMenu(int menuId);

    //! Take a snapshot of the screen.
    void saveBackground();
    //! Blit a portion of the background to the current screen
    void blitFromBackground(int x, int y, int width, int height);
    /*!
     * Adds a new dirty rectangle
     */
    void addRect(int x, int y, int width, int height) {
        dirtyList_.addRect(x, y, width, height);
    }

    //! Sets the default palette
    void setDefaultPalette();
    //! Sets the palette for the given mission id
    void setPaletteForMission(int i_id);
    void setPalette(const char *fname, bool sixbit = true);

    //! Displays the current menu
    void renderMenu();

    //! Returns true if a menu is being displayed
    bool showingMenu() { return current_ != NULL; }

    void resetSinceMouseDown() {
        since_mouse_down_ = 0;
        mouseup_was_ = false;
    }
    bool simpleMouseDown();
    bool isMouseDragged() { return !(simpleMouseDown() && mouseup_was_ ); }
    void updtSinceMouseDown(int32 elapsed) {
        if (!mouseup_was_)
            since_mouse_down_ += elapsed;
    }

protected:
    //! Returns a menu with the given id
    Menu * getMenu(int menuId);
    //! Shows the menu opening animation
    void showMenu(Menu *pMenu);
    //! Shows the menu closing animation
    void leaveMenu(Menu *pMenu);
    //! Switch from menu and plays the transition animation.
    void changeCurrentMenu();

protected:
    /** The menu factory.*/
    MenuFactory *pFactory_;
    /** The list of currently loaded menus.*/
    std::map<int, Menu *> menus_;
    /** The current menu being displayed.*/
    Menu *current_;
    /** The id of the next menu. -1 by default.*/
    int nextMenuId_;
    /** This flag prevents the input events from being processed.*/
    bool drop_events_;
    /** This array stores a copy of the back buffer to reset the screen.*/
    uint8 *background_;
    /*! This flag tells whether current menu needs a background or not.*/
    bool needBackground_;
    /*! Dirty rects list. */
    DirtyList   dirtyList_;

    /*! Sprite manager for menu sprites.*/
    SpriteManager menuSprites_;
    /*! Sprite manager for intro font. */
    SpriteManager *pIntroFontSprites_;
    /*! Font manager.*/
    FontManager fonts_;
    SoundManager *pGameSounds_;

    /*! Time since last mouse down event without mouseup*/
    int32 since_mouse_down_;
    //! Will not update since_mouse_down_ after this event
    bool mouseup_was_;
};

#endif

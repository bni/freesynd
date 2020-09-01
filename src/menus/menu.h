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

#ifndef MENU_H
#define MENU_H

#include <string>
#include <list>
#include <map>
#include "keys.h"
#include "common.h"
#include "menus/widget.h"
#include "gfx/dirtylist.h"

class FontManager;
class Menu;
class MenuManager;


/*!
 * Menu class.
 */
class Menu {
public:
    static const int MENU_NO_MENU;
    static const int kMenuIdLogout;
    /*! Id of the mouse left button.*/
    static const int kMouseLeftButton;
    /*! Id of the mouse right button.*/
    static const int kMouseRightButton;

    /*!
        * Menu constructor.
        */
    Menu(MenuManager *menuManager, int id, int parentId, 
        const char *showAnim = "", const char *leaveAnim = "");
    virtual ~Menu();

    int getId() { return id_; }

    /*! Returns true if an animation has been defined when opening the menu.*/
    bool hasShowAnim() { return showAnim_.size() != 0; }
    /*! Returns the opening animation file name. */
    const char * getShowAnimName() { return showAnim_.c_str(); }
    /*! Returns true if an animation has been defined when leaving the menu.*/
    bool hasLeaveAnim() { return leaveAnim_.size() != 0; }
    /*! Returns the leaving animation file name. */
    const char * getLeaveAnimName() { return leaveAnim_.c_str(); }
    //! Returns true is Menu can be put in cache
    bool isCachable() { return isCachable_; }

    //! Returns the sprites used in menus
    SpriteManager &menuSprites();
    
    //! Creates a new text label and returns its id
    int addStatic(int x, int y, const char *text, FontManager::EFontSize size, bool highlighted);
    //! Creates a new text label with a fixed size and returns its id
    int addStatic(int x, int y, int width, const char *text, FontManager::EFontSize size, bool highlighted);
    //! Creates a new button and returns its id
    int addOption(int x, int y, int width, int height, const char *text, FontManager::EFontSize size,
            int to = -1, bool visible = true, bool centered = true, int dark_widget = 0, int light_widget = 0);
    //! Creates a new button that has no text but an image
    int addImageOption(int x, int y, int dark_widget, int light_widget, bool visible = true);
    //! Creates a new toggle button and returns its id
    int addToggleAction(int x, int y, int width, int height, const char *text, FontManager::EFontSize size, bool selected);
    //! Creates a new list box and returns a pointer on it
    ListBox * addListBox(int x, int y, int width, int height, bool visible = true);
    //! Creates a specific list box for team selection and returns a pointer on it
    TeamListBox * addTeamListBox(int x, int y, int width, int height, bool visible = true);
    //! Creates a new textfield widget and returns a pointer on it
    TextField * addTextField(int x, int y, int width, int height, FontManager::EFontSize size, int maxSize, bool displayEmpty = false, bool visible = false);

    //! Returns the MenuText with given id
    MenuText * getStatic(int staticId);
    //! Returns the ActionWidget with the given id
    ActionWidget * getActionWidget(int buttonId);
    //! Returns the Option with the given id
    Option * getOption(int buttonId);
    //! Adds a mapping between a Function Key and an Option
    void registerHotKey(KeyFunc key, int optId);
    //! Adds a mapping between a Printable Key and an Option
    void registerHotKey(uint16 unicode, int optId);

    //! Does common actions before leaving
    void leave();

    //! Callback function : Childs can reimplement
    /*! 
     * Called just after the opening animation is played (if one has
     * been defined) and before the menu is rendered for the first time.
     */
    virtual void handleShow() {}
    
    //! Main render function
    void render(DirtyList &dirtyList);

    //! Callback function : Childs can reimplement
    /*! 
     * Called just before the closing animation is played (if one has
     * been defined) and the menu closed.
     */
    virtual void handleLeave() {}

    //! Handles key pressed
    void keyEvent(Key key, const int modKeys);
    //! Handles mouse moved
    void mouseMotionEvent(int x, int y, int state, const int modKeys);
    //! Handles mouse button pressed
    void mouseDownEvent(int x, int y, int button, const int modKeys);
    //! Handles mouse button released
    void mouseUpEvent(int x, int y, int button, const int modKeys);

    virtual void handleTick(int elapsed) {}

    //! Callback function : Childs can reimplement
    /*! 
     * Called when an action widget has been activated.
     * \param actionId The id of the actionWidget that was activated.
     * \param ctx A pointer to a context specific to the action
     * \param modKeys The state of all modifiers buttons
     */
    virtual void handleAction(const int actionId, void *ctx, const int modKeys) {}

    void selectToggleAction(int id) { group_.selectButton(id); }

    void captureInputBy(TextField *pTextfield);
    bool isPaused() { return paused_; }

    MenuManager *getMenuManager() { return menu_manager_; }
    void addDirtyRect(int x, int y, int width, int height);

protected:

    //! Callback function : Childs can reimplement
    /*! 
        * Called each time a menu is rendered.
        */
    virtual void handleRender(DirtyList &dirtyList) {}

    //! Handle mouse down event.
    /*!
        * \return true if the menu has processed the event, and it must not be processed anymore.
        */
    virtual bool handleMouseDown(int x, int y, int button, const int modKeys) { return false; }
    virtual void handleMouseUp(int x, int y, int button, const int modKeys) {}
    virtual void handleMouseMotion(int x, int y, int state, const int modKeys) {}
    virtual bool handleUnknownKey(Key key, const int modKeys) { return false;}

    void needRendering();

    //! Convenient method to return the menu font with the given size
    MenuFont * getMenuFont(FontManager::EFontSize size);
    //! Convenient method to return the game font
    GameFont *gameFont();
    //! Convenient method to get messages
    std::string getMessage(const std::string & id);
    //! Convenient method to get messages
    void getMessage(const std::string & id, std::string & msg);

protected:
    class HotKey {
    public:
        HotKey(KeyFunc code, uint16 unicode, Option *pOpt) {
            key.keyFunc = code;
            key.unicode = unicode;
            pOption = pOpt;
        }
        Key key;
        Option *pOption; 
    };

    MenuManager *menu_manager_;
    /*! A unique id to identify this menu.*/
    int id_;
    /*! Parent menu when leaving this menu.*/
    int parentId_;
    std::string showAnim_, leaveAnim_;
    /*! The list of all static widgets (MenuText).*/
    std::list<MenuText> statics_;
    /*! The list of all dynamic widgets (Option).*/
    std::list<ActionWidget *> actions_;
    /*! An association between key and option for hotkeys.*/
    std::list<HotKey> hotKeys_;
    /*! A group of mutual exclusive ToggleAction.*/
    Group group_;
    /*! The id of the widget that currently has focus.*/
    int focusedWgId_;
    /*! The current textfield that holds the cursor and so capture all key events.*/
    TextField *pCaptureInput_;
    /*! Tells if the menu is kept in cache even after leaving it or destroy it.*/
    bool isCachable_;
    /*! Used only in gameplay menu, pauses game*/
    bool paused_;
};

#endif

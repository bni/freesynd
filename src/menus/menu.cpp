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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "menus/menu.h"
#include "menus/menumanager.h"
#include "appcontext.h"
#include "gfx/fliplayer.h"
#include "gfx/screen.h"

const int Menu::MENU_NO_MENU = -1;
const int Menu::kMenuIdLogout = 6;
const int Menu::kMouseLeftButton = 1;
const int Menu::kMouseRightButton = 3;

Menu::Menu(MenuManager * menuManager, int id, int parentId, 
    const char *showAnim, const char *leaveAnim) :
    showAnim_(showAnim), leaveAnim_(leaveAnim)
{
    id_ = id;
    parentId_ = parentId;
    menu_manager_ = menuManager;
    focusedWgId_ = -1;
    pCaptureInput_ = NULL;
    isCachable_ = true;
    paused_ = false;
}

Menu::~Menu() {
    for (std::list < ActionWidget * >::iterator it = actions_.begin();
         it != actions_.end(); it++) {
        delete (*it);
    }

    actions_.clear();
}

SpriteManager & Menu::menuSprites() {
    return menu_manager_->menuSprites();
}

/*!
 * Adds a dirty rect the size of the screen.
 */
void Menu::needRendering() {
    menu_manager_->addRect(0, 0, g_Screen.gameScreenWidth(), g_Screen.gameScreenHeight());
}

/*!
 * Adds a dirty rect.
 * \param x
 * \param y
 * \param width
 * \param height
 */
void Menu::addDirtyRect(int x, int y, int width, int height) {
    menu_manager_->addRect(x, y, width, height);
}

void Menu::render(DirtyList &dirtyList)
{

    for (std::list < MenuText >::iterator it = statics_.begin();
        it != statics_.end(); it++) {
        MenuText & m = *it;
        if ( m.isVisible() && dirtyList.intersectsList(m.getX(), m.getY(), m.getWidth(), m.getHeight()) ) {
            m.draw();
        }
    }

    for (std::list < ActionWidget * >::iterator it = actions_.begin();
        it != actions_.end(); it++) {
            ActionWidget * a = *it;
            if ( a->isVisible() && dirtyList.intersectsList(a->getX(), a->getY(), a->getWidth(), a->getHeight())) {
                a->draw();
            }
    }
    handleRender(dirtyList);
}

/*!
 * This method does some common actions before given handle to the 
 * current menu instance via handleLeave().
 */
void Menu::leave() {
    // Reset focus if an action widget had one.
    if (focusedWgId_ != -1) {
        ActionWidget *pAction = getActionWidget(focusedWgId_);

        pAction->handleFocusLost();
        focusedWgId_ = -1;
    }

    // Reset capture
    if (pCaptureInput_ != NULL) {
        pCaptureInput_->handleCaptureLost();
        pCaptureInput_ = NULL;
    }

    // Give control to menu instance
    handleLeave();
}

/*!
 * Creates and adds a label to the menu.
 * \param x X coordinate
 * \param y Y coordinate
 * \param text If text starts with a '#' then
 * text is a property in the current language file and it is
 * replaced by its value.
 * \param size Font size
 * \param highlighted True means text is highlighted
 * \returns The newly created widget id.
 */
int Menu::addStatic(int x, int y, const char *text, FontManager::EFontSize size, bool highlighted) {

    MenuText m(this, x, y, text, getMenuFont(size), highlighted, true);
    statics_.push_back(m);
    return m.getId();
}

/*!
 * Creates and adds a label with a fixed width to the menu.
 * Height will be equals to text height in the given font.
 * \param x X coordinate
 * \param y Y coordinate
 * \param width Width of the widget
 * \param text If text starts with a '#' then
 * text is a property in the current language file and it is
 * replaced by its value.
 * \param size Font size
 * \param highlighted True means text is highlighted
 * \returns The newly created widget id.
 */
int Menu::addStatic(int x, int y, int width, const char *text, FontManager::EFontSize size, bool highlighted) {
    MenuText m(this, x, y, width, text, getMenuFont(size), highlighted, true);
    statics_.push_back(m);
    return m.getId();
}

/*!
 * Creates and adds a button to the menu.
 * \param x X coordinate
 * \param y Y coordinate
 * \param width Button width
 * \param height Button height
 * \param text Button label. If text starts with a '#' then
 * text is a property in the current language file and it is
 * replaced by its value.
 * \param size Font size
 * \param to Id of the next menu when button is clicked
 * \param visible True if button is visible on screen
 * \param centered True if text must centered regarding button width
 * \param dark_widget Widget drawn in front of the button when it's not highlighted
 * \param light_widget Widget drawn in front of the button when it's highlighted
 */
int Menu::addOption(int x, int y, int width, int height, const char *text, FontManager::EFontSize size,
            int to, bool visible, bool centered, int dark_widget, int light_widget) {
    
    Option *pOption = new Option(this, x, y, width, height, text, getMenuFont(size), to, visible, centered, dark_widget, light_widget);
    actions_.push_back(pOption);

    if (pOption->getHotKey().keyFunc != KFC_UNKNOWN || pOption->getHotKey().unicode != 0) {
        // The option already has an acceleration key
        registerHotKey(pOption->getHotKey().unicode, pOption->getId());
    }

    return pOption->getId();
}

/*! 
 * Creates a new button that has no text but an image.
 * Widget's size will be the same as the image used. Dark image
 * and light image should be the same size.
 * \param x X coordinate
 * \param y Y coordinate
 * \param dark_widget Widget drawn in front of the button when it's not highlighted
 * \param light_widget Widget drawn in front of the button when it's highlighted
 * \param visible True if button is visible on screen
 * \returns The newly created widget id.
 */
int Menu::addImageOption(int x, int y, int dark_widget, int light_widget, bool visible) {

    Sprite *spr = menu_manager_->menuSprites().sprite(dark_widget);

    Option *m = new Option(this, x, y, spr->width() * 2, spr->height() * 2, "", 
        getMenuFont(FontManager::SIZE_1), MENU_NO_MENU, visible, true, dark_widget, light_widget);
    actions_.push_back(m);

    return m->getId();
}

int Menu::addToggleAction(int x, int y, int width, int height, const char *text,
    FontManager::EFontSize size, bool selected)
{
    ToggleAction *a = new ToggleAction(this, x, y, width, height, text,
        getMenuFont(size), selected, &group_);
    group_.addButton(a);
    actions_.push_back(a);

    return a->getId();
}

/*!
 * Creates and adds a list box to the menu.
 * \param x X coordinate
 * \param y Y coordinate
 * \param width widget width
 * \param height widget height
 * \param visible True if widget is visible on screen
 * \return A pointer on the widget.
 */
ListBox * Menu::addListBox(int x, int y, int width, int height, bool visible) {
    ListBox *pBox = new ListBox(this, x, y, width, height, getMenuFont(FontManager::SIZE_1), visible);
    actions_.push_back(pBox);

    return pBox;
}

/*!
 * Creates and adds a team list box to the menu.
 * \param x X coordinate
 * \param y Y coordinate
 * \param width widget width
 * \param height widget height
 * \param visible True if widget is visible on screen
 * \return A pointer on the widget.
 */
TeamListBox * Menu::addTeamListBox(int x, int y, int width, int height, bool visible) {
    TeamListBox *pBox = new TeamListBox(this, x, y, width, height, getMenuFont(FontManager::SIZE_1), visible);
    actions_.push_back(pBox);

    return pBox;
}

TextField * Menu::addTextField(int x, int y, int width, int height, FontManager::EFontSize size, int maxSize, bool displayEmpty, bool visible) {
    TextField *pTextField = new TextField(this, x, y, width, height, getMenuFont(size), maxSize, displayEmpty, visible);
    actions_.push_back(pTextField);

    return pTextField;
}

/*!
 * Returns the MenuText widget with the given id.
 * \return NULL if no widget is found
 */
MenuText * Menu::getStatic(int staticId) {
    for (std::list < MenuText >::iterator it = statics_.begin();
         it != statics_.end(); it++) {
        MenuText & m = *it;
        
        if (m.getId() == staticId) {
            return &m;
        }
    }

    return NULL;
}

/*!
 * Returns a pointer to the action widget with the given id.
 * \param id The id of the widget.
 * \return NULL if no widget has that id.
 */
ActionWidget * Menu::getActionWidget(int id) {
    for (std::list < ActionWidget * >::iterator it = actions_.begin();
         it != actions_.end(); it++) {

        if (id == (*it)->getId()) {
            return *it;
        }
    }

    return NULL;
}

Option * Menu::getOption(int optionId) {
    ActionWidget *pAction = getActionWidget(optionId);

    if (pAction) {
        return dynamic_cast<Option *> (pAction);
    }

    return NULL;
}

/*!
 * Adds an acceleration key to the given option so it can be activated by that key.
 * If id is not an existing option id, then nothing is done.
 * \param code The hot key
 * \param optId The option id
 */
void  Menu::registerHotKey(KeyFunc code, int optId) {
    Option *pOption = getOption(optId);
    if (pOption) {
        HotKey hc(code, 0, pOption);
        hotKeys_.push_back(hc);
    }
}

/*!
 * Adds an acceleration key to the given option so it can be activated by that key.
 * If id is not an option id, then nothing is done.
 * \param unicode The hot key
 * \param optId The option id
 */
void  Menu::registerHotKey(uint16 unicode, int optId) {
    Option *pOption = getOption(optId);
    if (pOption) {
        HotKey hc(KFC_UNKNOWN, unicode, pOption);
        hotKeys_.push_back(hc);
    }
}

void Menu::captureInputBy(TextField *pTextfield) {
    if (pTextfield != pCaptureInput_ && pCaptureInput_ != NULL) {
        pCaptureInput_->handleCaptureLost();
    }

    pCaptureInput_ = pTextfield;

    if (pCaptureInput_ != NULL) {
        pCaptureInput_->handleCaptureGained();
    }
}

/*!
 * Handles the key pressed event.
 * \param key The key that was pressed
 * \param modKeys State of all modifier keys
 */
void Menu::keyEvent(Key key, const int modKeys)
{
    // first pass the event to the textfield that has the cursor
    if (pCaptureInput_ != NULL) {
        if (pCaptureInput_->handleKey(key, modKeys)) {
            return;
        }
    }

    if (!paused_) {
        // Then look for a mapped key to execute an action
        for (std::list < HotKey >::iterator it = hotKeys_.begin();
                it != hotKeys_.end(); it++) {
                    uint16 c = key.unicode;
                    // Hotkey can only be character from 'A' to 'Z'
                    if (c >= 'a' && c <= 'z') {
                        // so uppercase it
                        c -= 32;
                    }
                    if ((*it).key.keyFunc == key.keyFunc && (*it).key.unicode == c) {
                    Option *opt = (*it).pOption;
                    if (opt->isVisible() && opt->isWidgetEnabled()) {
                        opt->executeAction(modKeys);
                    }
                    return;
                }
        }
    }

    // Finally pass the event to the menu instance
    if (!handleUnknownKey(key, modKeys)) {
        // Menu has not consummed key event :
        // Pressing Escape changes the current menu to its parent(like a back)
        if (key.keyFunc == KFC_ESCAPE) {
            menu_manager_->gotoMenu(parentId_);
            return;
        }
    }
}

/*!
 * Handles the mouse motion event.
 * \param x X screen coordinate
 * \param y Y screen coordinate
 * \param state If button is pressed during mouse motion.
 * \param modKeys State of all modifier keys
 */
void Menu::mouseMotionEvent(int x, int y, int state, const int modKeys)
{
    handleMouseMotion(x, y, state, modKeys);

    // Check focus is lost for currently focused widget
    if (focusedWgId_ != -1) {
        ActionWidget *pAction = getActionWidget(focusedWgId_);
        
        if (!pAction->isMouseOver(x, y) || !pAction->isVisible()) {
            pAction->handleFocusLost();
            focusedWgId_ = -1;
        }
    }

    // See if the mouse is hovering an action widget
    for (std::list < ActionWidget * >::iterator it = actions_.begin();
         it != actions_.end(); it++) {
        ActionWidget *m = *it;

        if (!m->isVisible() || !m->isWidgetEnabled()) {
            // action is not visible or not enabled so it doesn't count
            continue;
        }

        // Mouse is over a widget
        if (m->isMouseOver(x, y)) {
            if (m->getId() != focusedWgId_) {
                // Widget has now the focus : handle the event
                m->handleFocusGained();
                focusedWgId_ = m->getId();
            }

            // Pass the event to the widget
            m->handleMouseMotion(x, y, state, modKeys);

            return;
        }
    }
}

/*!
 * Handles the mouse down event.
 * \param x X screen coordinate
 * \param y Y screen coordinate
 * \param button What button was pressed
 * \param modKeys State of all modifier keys
 */
void Menu::mouseDownEvent(int x, int y, int button, const int modKeys)
{
    // First pass the event to the menu
    if (handleMouseDown(x, y, button, modKeys)) {
        // Menu has processed the event, so don't pass it to widget
        return;
    }

    // The event was not processed by the menu, so give a chance to a widget
    for (std::list < ActionWidget * >::iterator it = actions_.begin();
         it != actions_.end(); it++) {
        ActionWidget *m = *it;

        if (!m->isVisible() || !m->isWidgetEnabled()) {
            // Widget is not visible or enabled so it doesn't count
            continue;
        }

        if (m->isMouseOver(x, y)) {
            m->handleMouseDown(x, y, button, modKeys);
            return;
        }
    }
}

/*!
 * Handles the mouse up event.
 * \param x X screen coordinate
 * \param y Y screen coordinate
 * \param button What button was released
 * \param modKeys State of all modifier keys
 */
void Menu::mouseUpEvent(int x, int y, int button, const int modKeys)
{
    handleMouseUp(x, y, button, modKeys);
}

MenuFont * Menu::getMenuFont(FontManager::EFontSize size) { 
    return menu_manager_->fonts().getMenuFont(size); 
}

GameFont *Menu::gameFont() {
    return menu_manager_->fonts().gameFont();
}

std::string Menu::getMessage(const std::string & id) {
    return g_Ctx.getMessage(id);
}

void Menu::getMessage(const std::string & id, std::string & msg) {
    g_Ctx.getMessage(id, msg);
}
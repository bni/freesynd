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

#include "fs-engine/menus/menumanager.h"

#include <cassert>

#include "fs-utils/io/configfile.h"
#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "fs-engine/appcontext.h"
#include "fs-engine/menus/fliplayer.h"
#include "fs-engine/gfx/screen.h"
#include "fs-engine/sound/soundmanager.h"
#include "fs-engine/menus/logoutmenu.h"
#include "fs-engine/menus/flimenu.h"

//! This defines the list of events for the transition show animation
const FrameEvent transition_show_event[] = {
    {(uint16)-1, msc::NO_TRACK, NO_SOUND, 0x0, NULL }
};

//! This defines the list of events for the transition leave animation
const FrameEvent transition_leave_event[] = {
    { 1, msc::NO_TRACK, MENU_CHANGE,   0x1, NULL },
    {(uint16)-1, msc::NO_TRACK, NO_SOUND, 0x0, NULL }
};

/*!
 * First instanciate a default menu. If menuID is not a default menu,
 * then asked subclass to instanciate via createCustomMenu().
 * @param menuId Id of the menu
 * @return 
 */
Menu * MenuFactory::createMenu(const int menuId) {
    Menu *pMenu = nullptr;

    if (menuId == Menu::kMenuIdLogout) {
        pMenu = new LogoutMenu(pManager_);
    } else if (menuId == Menu::kMenuIdFliTransition) {
        pMenu = new FliMenu(pManager_, Menu::kMenuIdFliTransition, 0);
    } else {
        pMenu = createCustomMenu(menuId);
    }

    return pMenu;
}

/*!
 * Return true if there is an animation for the given menu.
 * @param menuId 
 * @return 
 */
bool MenuFactory::hasLeaveAnimation(int menuId) {
    std::string anim(getLeaveAnimation(menuId));

    return anim.length() != 0;
}

/*!
 * Return true if there is an animation for the given menu.
 * @param menuId 
 * @return 
 */
bool MenuFactory::hasShowAnimation(int menuId) {
    std::string anim(getShowAnimation(menuId));

    return anim.length() != 0;
}

/*!
 * Return the animation for the given menu.
 * For the menus under the default MenuFactory, there are no animations.
 * @param menuId 
 * @return 
 */
const char* MenuFactory::getShowAnimation(int menuId) {
    return "";
}

/*!
 * Return the animation for the given menu.
 * For the menus under the default MenuFactory, there are no animations.
 * @param menuId 
 * @return 
 */
const char* MenuFactory::getLeaveAnimation(int menuId) {
    return "";
}

/*!
 * @brief 
 * @param pFactory 
 * @param pGameSounds 
 */
MenuManager::MenuManager(MenuFactory *pFactory, SoundManager *pGameSounds)
        : dirtyList_(g_Screen.gameScreenWidth(), g_Screen.gameScreenHeight()),
          menuSprites_(), fonts_(), logoManager_() {
    pFactory_ = pFactory;
    pFactory_->setMenuManager(this);
    pGameSounds_ = pGameSounds;
    drop_events_ = false;

    current_ = NULL;
    nextMenuId_ = -1;

    pIntroFontSprites_ = NULL;

    since_mouse_down_ = 0;
    mouseup_was_ = false;
}

MenuManager::~MenuManager()
{
    delete pFactory_;
}

/*!
 * Initialize the menu manager.
 * \param loadIntroFont If true loads the intro sprites and font
 */
bool MenuManager::initialize(bool loadIntroFont) {
    LOG(Log::k_FLG_INFO, "MenuManager", "initialize", ("initializing menus..."))

    // Loads menu sprites
    LOG(Log::k_FLG_GFX, "MenuManager", "initialize", ("Loading menu sprites ..."))
    if (!menuSprites_.loadSprites("mspr-0.tab", "mspr-0.dat", true)) {
        return false;
    }

    // loads intro sprites
    if (loadIntroFont) {
        LOG(Log::k_FLG_GFX, "MenuManager", "initialize", ("Loading intro sprites ..."))

        pIntroFontSprites_ = new SpriteManager();
        if (!pIntroFontSprites_->loadSprites("mfnt-0.tab", "mfnt-0.dat", true)) {
            return false;
        }
    }

    // Loads fonts
    LOG(Log::k_FLG_GFX, "MenuManager", "initialize", ("Loading fonts ..."))
    return fonts_.loadFonts(&menuSprites_, pIntroFontSprites_);
}

/*!
 * Destroy all menus and resources.
 */
void MenuManager::destroy() {
    LOG(Log::k_FLG_MEM, "MenuManager", "~MenuManager", ("Destruction..."))

    if (pIntroFontSprites_) {
        delete pIntroFontSprites_;
        pIntroFontSprites_ = NULL;
    }

    // NOTE: this code was before in destructor, but it should be
    // here to avoid corruption, that will happen because
    // agentmanager is destroyed before menumanager and
    // removeModelListener will fail with critical error,
    // for unknown reason this happens only in WindowsOS
    // NOTE: it might be better to create our deallocator
    // of resource in App class to have correct sequence of destructors
    if (current_) {
        if (menus_.find(current_->getId()) == menus_.end()) {
            delete current_;
            current_ = NULL;
        }
    }

    for (std::map<int, Menu*>::iterator it = menus_.begin();
        it != menus_.end(); it++)
        delete it->second;
}

void MenuManager::setDefaultPalette() {
    setPalette("mselect.pal", true);
}

void MenuManager::setPaletteForMission(int i_id) {
    // I'm not sure of the way we get the palette
    char spal[20];
    sprintf(spal,"hpal0%i.dat", i_id % 5 + 1);
    setPalette(spal);
}

void MenuManager::setPalette(const char *fname, bool sixbit) {
    LOG(Log::k_FLG_GFX, "MenuManager", "setPalette", ("Setting palette : %s", fname))
    size_t size;
    uint8 *data = File::loadOriginalFile(fname, size);

    if (data) {
        if (sixbit)
            g_System.setPalette6b3(data);
        else
            g_System.setPalette8b3(data);

        delete[] data;
    }
}

Menu * MenuManager::getMenu(int menuId) {
    // look in the cache
    if (menus_.find(menuId) != menus_.end()) {
        return menus_[menuId];
    }

    // menu is not in cache so create it
    // some menus are not saved in cache as they are not accessed many times
    Menu *pMenu = pFactory_->createMenu(menuId);

    if (pMenu && pMenu->isCachable()) {
        menus_[menuId] = pMenu;
    }

    return pMenu;
}

/*!
 * Method to search for the FliMenu used for transitions.
 * If no menu is found in cache, the menu is created and added to cache.
 * @return 
 */
FliMenu *MenuManager::getFliTransitionMenu() {
    Menu *pMenu = getMenu(Menu::kMenuIdFliTransition);
    return dynamic_cast<FliMenu *>(pMenu);
}

/*!
 * Change the current menu with the one with the given name.
 * Plays the transition animations between the two menus.
 */
void MenuManager::changeCurrentMenu()
{
    leaveCurrentMenu();

    showNextMenu();
}

void MenuManager::gotoMenu(int menuId) {
    nextMenuId_ = menuId;
    // stop listening for events until window changed
    drop_events_ = true;
}

/*!
 * Open the menu with the id defined in nextMenuId field.
 * Calls handleShow() on this instance.
 */
void MenuManager::showNextMenu() {
    // Get the next menu
    current_ = getMenu(nextMenuId_);
    if (current_ == nullptr) {
        return;
    }

    nextMenuId_ = -1;

    // Make a snapshot of background is menu needs it
    if (current_->doNeedBackground()) {
        g_Screen.saveBackground();
    }

    dirtyList_.flush();
    current_->handleShow();

    // After showing the window, check to see if a cursor must be display
    if (current_->cursorWhenShown() == Menu::kNoCursor) {
        g_System.hideCursor();
    } else {
        if (current_->cursorWhenShown() == Menu::kGameplayCursor) {
            g_System.usePointerCursor();
        } else {
            g_System.useMenuCursor();
        }
        g_System.showCursor();

        // then plot the mouse to draw the button
        // that could be highlighted because the mouse is upon it
        Point2D point;
        uint32_t state = g_System.getMousePos(point);
        current_->mouseMotionEvent(point, state);
    }

    // Adds a dirty rect to force menu rendering
    addRect(0, 0, g_Screen.gameScreenWidth(), g_Screen.gameScreenHeight());

    // reopen the event processing
    drop_events_ = false;
}

/*!
 * Close the current menu if it exists.
 * Calls handleLeave() on this instance.
 * Adds transitions between this menu and the next one by
 * inserting an instance of FliMenu with one or two animations.
 * If current is not in cache, destroys it.
 */
void MenuManager::leaveCurrentMenu() {
    if (current_) {
        bool currentIsNotTransitionFli = (current_->getId() != Menu::kMenuIdFliTransition);
        int currentId = current_->getId();
        // hide cursor to display transition
        g_System.hideCursor();
        // Give the possibility to the old menu
        // to clean before leaving
        current_->leave();

        if (currentIsNotTransitionFli && (pFactory_->hasLeaveAnimation(currentId) || pFactory_->hasShowAnimation(nextMenuId_))) {
            FliMenu *pTransitionFliMenu = getFliTransitionMenu();
            // save next menu for after transition
            pTransitionFliMenu->setNextMenu(nextMenuId_);
            pTransitionFliMenu->clearFliDescList();

            if (pFactory_->hasLeaveAnimation(currentId)) {
                pTransitionFliMenu->addFliDesc(pFactory_->getLeaveAnimation(currentId), 66, false, false, transition_leave_event);
            }

            if (pFactory_->hasShowAnimation(nextMenuId_)) {
                pTransitionFliMenu->addFliDesc(pFactory_->getShowAnimation(nextMenuId_), 66, false, false, transition_show_event);
            }
            // set next menu to be the transition menu
            nextMenuId_ = Menu::kMenuIdFliTransition;
        }

        // If menu is not in cache, it means it must be destroyed
        if (!current_->isCachable()) {
            delete current_;
            current_ = nullptr;
        }
    }
}

/*!
 * Renders the current menu if there is one
 * and if it needs to be refreshed.
 */
void MenuManager::renderMenu() {
    if (current_ && !dirtyList_.isEmpty()) {
        if (current_->doNeedBackground()) {
            for (int i=0; i < dirtyList_.getSize(); i++) {
                DirtyRect *rect = dirtyList_.getRectAt(i);
                g_Screen.blitFromBackground(rect->x, rect->y, rect->width, rect->height);
            }
        }
        current_->render(dirtyList_);
        // flush dirty list
        dirtyList_.flush();
    }
}

void MenuManager::handleEvent(const FS_Event& evt) {

    switch(evt.type) {
    case EVT_QUIT:
        gotoMenu(Menu::kMenuIdLogout);
        break;
    case EVT_MSE_MOTION:
        if (current_ && !drop_events_)
            current_->mouseMotionEvent({evt.motion.x, evt.motion.y}, evt.motion.state);
        break;
    case EVT_MSE_DOWN:
        since_mouse_down_ = 0;
        mouseup_was_ = false;
#ifdef _DEBUG
            // Display mouse coordinate
            if (evt.button.keyMods & KMD_SHIFT) {
                printf("Mouse is at %d, %d\n", evt.motion.x, evt.motion.y);
            }
#endif

        if (current_ && !drop_events_) {
            current_->mouseDownEvent({evt.button.x, evt.button.y}, evt.button.button);
        }
        break;
    case EVT_MSE_UP:
        mouseup_was_ = true;
        if (current_ && !drop_events_) {
            current_->mouseUpEvent({evt.button.x, evt.button.y}, evt.button.button);
        }
        break;
    case EVT_KEY_DOWN:
        if (current_ && !drop_events_) {
            current_->keyEvent(evt.key.key);
        }
        break;
    case EVT_NONE:
        break;
    }
}

bool MenuManager::simpleMouseDown() {
    return g_Ctx.getTimeForClick() < since_mouse_down_;
}

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

#include "config.h"
#include "gfx/screen.h"
#include "system.h"
#include "sound/audio.h"
#include "utils/file.h"
#include "utils/log.h"

#include <SDL2/SDL_image.h>

const int SystemSDL::CURSOR_WIDTH = 24;

SystemSDL::SystemSDL(int depth) {
    depth_ = depth;
    keyModState_ = 0;
    screen_surf_ = NULL;
    temp_surf_ = NULL;
    cursor_surf_ = NULL;

    pixels_ = new Uint32[GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT];
}

SystemSDL::~SystemSDL() {
    if (temp_surf_) {
        SDL_FreeSurface(temp_surf_);
    }

    if (cursor_surf_) {
        SDL_FreeSurface(cursor_surf_);
    }

#ifdef HAVE_SDL_MIXER
    Audio::quit();
#endif

    // Destroy SDL_Image Lib
    IMG_Quit();

    SDL_Quit();
}

bool SystemSDL::initialize(bool fullscreen) {
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

    if (SDL_Init(SDL_INIT_VIDEO
        ) < 0) {
        printf("Critical error, SDL could not be initialized!");
        return false;
    }

    // Audio initialisation
    if (!Audio::init()) {
        LOG(Log::k_FLG_SND, "SystemSDL", "Init", ("Couldn't initialize Sound System : no sound will be played."))
    }

    SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &screen_surf_, &renderer_);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_RenderSetIntegerScale(renderer_, SDL_TRUE);
    SDL_RenderSetLogicalSize(renderer_, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);

    temp_surf_ = SDL_CreateRGBSurface(0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT, 8, 0, 0, 0, 0);

    texture_ = SDL_CreateTextureFromSurface(renderer_, temp_surf_);

    cursor_surf_ = NULL;
    // Init SDL_Image library
    int sdl_img_flags = IMG_INIT_PNG;
    int initted = IMG_Init(sdl_img_flags);
    if ( (initted & sdl_img_flags) != sdl_img_flags ) {
        printf("Failed to init SDL_Image : %s\n", IMG_GetError());
    } else {
        // Load the cursor sprites
        if (loadCursorSprites()) {
            // Cursor movement is managed by the application
            SDL_ShowCursor(SDL_DISABLE);
        }
        // At first the cursor is hidden
        hideCursor();
        useMenuCursor();
    }

    return true;
}

void SystemSDL::updateScreen() {
    const uint8 *pixeldata = g_Screen.pixels();

    // We do manual blitting to convert from 8bpp palette indexed values to 32bpp RGB for each pixel
    uint8 r, g, b;
    for (int i = 0; i < GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT; i++) {
        uint8 index = pixeldata[i];

        r = temp_surf_->format->palette->colors[index].r;
        g = temp_surf_->format->palette->colors[index].g;
        b = temp_surf_->format->palette->colors[index].b;

        Uint32 c = ((r << 16) | (g << 8) | (b << 0)) | (255 << 24);

        pixels_[i] = c;
    }

    memcpy(temp_surf_->pixels, pixels_, GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT * sizeof (Uint32));

    SDL_RenderClear(renderer_);

    SDL_UpdateTexture(texture_, NULL, temp_surf_->pixels, GAME_SCREEN_WIDTH * sizeof (Uint32));

    if (cursor_visible_) {
        SDL_Rect dst;

        dst.x = cursor_x_ - cursor_hs_x_;
        dst.y = cursor_y_ - cursor_hs_y_;
        dst.w = CURSOR_WIDTH;
        dst.h = CURSOR_WIDTH;

        SDL_UpdateTexture(texture_, &dst, cursor_surf_->pixels, cursor_surf_->pitch);

        update_cursor_ = false;
    }

    SDL_RenderCopy(renderer_, texture_, NULL, NULL);

    SDL_RenderPresent(renderer_);
}

/*!
 * Using the keysym parameter, verify if the given key is a function key (ie
 * a not printable key) returns the corresponding entry in the KeyFunc enumeration.
 * \returns If key code is not a function key, returns KEY_UNKNOWN.
 */
void SystemSDL::checkKeyCodes(SDL_Keysym keysym, Key &key) {
    key.keyFunc = KFC_UNKNOWN;
    key.keyVirt = KVT_UNKNOWN;
    switch(keysym.sym) {
        case SDLK_ESCAPE: key.keyFunc = KFC_ESCAPE; break;
        case SDLK_BACKSPACE: key.keyFunc = KFC_BACKSPACE; break;
        case SDLK_RETURN: key.keyFunc = KFC_RETURN; break;
        case SDLK_DELETE: key.keyFunc = KFC_DELETE; break;
        case SDLK_UP:
        case SDLK_DOWN:
        case SDLK_RIGHT:
        case SDLK_LEFT:
        case SDLK_INSERT:
        case SDLK_HOME:
        case SDLK_END:
        case SDLK_PAGEUP:
        case SDLK_PAGEDOWN:
            key.keyFunc = static_cast < KeyFunc > (KFC_UP + (keysym.sym - SDLK_UP));
            break;
        case SDLK_F1:
        case SDLK_F2:
        case SDLK_F3:
        case SDLK_F4:
        case SDLK_F5:
        case SDLK_F6:
        case SDLK_F7:
        case SDLK_F8:
        case SDLK_F9:
        case SDLK_F10:
        case SDLK_F11:
        case SDLK_F12:
            key.keyFunc = static_cast < KeyFunc > (KFC_F1 + (keysym.sym - SDLK_F1));
            break;
        case SDLK_0:case SDLK_1:case SDLK_2:case SDLK_3:case SDLK_4:
        case SDLK_5:case SDLK_6:case SDLK_7:case SDLK_8:case SDLK_9:
            key.keyVirt = static_cast < KeyVirtual > (KVT_NUMPAD0 + (keysym.sym - SDLK_0));
            break;
        default:
            // unused key
            break;
    }
}

//! Pumps an event from the event queue
/*!
 * Watch the event queue and dispatch events.
 * - keyboard events : when a modifier key is pressed,
 * the system does not dispatch the event to the application:
 * it stores the key state and then passes the complete state
 * when a regular key is pressed. So that the application knows
 * if multiple modifier keys are pressed at the same time (ie Ctrl/Shift)
 */
bool SystemSDL::pumpEvents(FS_Event *pEvtOut) {
    SDL_Event evtIn;

    pEvtOut->type = EVT_NONE;

    if (SDL_PollEvent(&evtIn)) {
        switch (evtIn.type) {
        case SDL_QUIT:
            pEvtOut->quit.type = EVT_QUIT;
            break;
        case SDL_KEYDOWN:
            {
            // Check if key pressed is a modifier
            switch(evtIn.key.keysym.sym) {
                case SDLK_RSHIFT:
                    keyModState_ = keyModState_ | KMD_RSHIFT; 
                    break;
                case SDLK_LSHIFT:
                    keyModState_ = keyModState_ | KMD_LSHIFT; 
                    break;
                case SDLK_RCTRL:
                    keyModState_ = keyModState_ | KMD_RCTRL;
                    // NOTICE Hide cursor
                    g_System.hideCursor();
                    break;
                case SDLK_LCTRL:
                    keyModState_ = keyModState_ | KMD_LCTRL;
                    // NOTICE Hide cursor
                    g_System.hideCursor();
                    break;
                case SDLK_RALT:
                    keyModState_ = keyModState_ | KMD_RALT; 
                    break;
                case SDLK_LALT:
                    keyModState_ = keyModState_ | KMD_LALT; 
                    break;
                default:
                    // We pass the event only if it's not a allowed modifier key
                    // Plus, the application receives event only when key is pressed
                    // not released.
                    pEvtOut->type = EVT_KEY_DOWN;
                    Key key;
                    key.unicode = 0;
                    checkKeyCodes(evtIn.key.keysym, key);
                    pEvtOut->key.key = key;
                    pEvtOut->key.keyMods = keyModState_;
                    break;
                } // end switch
            } // end case SDL_KEYDOWN
            break;
        case SDL_KEYUP:
            {
            switch(evtIn.key.keysym.sym) {
                case SDLK_RSHIFT:
                    keyModState_ = keyModState_ & !KMD_RSHIFT;
                    break;
                case SDLK_LSHIFT:
                    keyModState_ = keyModState_ & !KMD_LSHIFT;
                    break;
                case SDLK_RCTRL:
                    keyModState_ = keyModState_ & !KMD_RCTRL;
                    // NOTICE Show cursor again
                    g_System.showCursor();
                    break;
                case SDLK_LCTRL:
                    keyModState_ = keyModState_ & !KMD_LCTRL;
                    // NOTICE Show cursor again
                    g_System.showCursor();
                    break;
                case SDLK_RALT:
                    keyModState_ = keyModState_ & !KMD_RALT;
                    break;
                case SDLK_LALT:
                    keyModState_ = keyModState_ & !KMD_LALT;
                    break;
                default:
                    break;
            }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            pEvtOut->button.type = EVT_MSE_UP;
            pEvtOut->button.x = evtIn.button.x;
            pEvtOut->button.y = cursor_y_ = evtIn.button.y;
            pEvtOut->button.button = evtIn.button.button;
            pEvtOut->button.keyMods = keyModState_;
            break;
        case SDL_MOUSEBUTTONDOWN:
            pEvtOut->button.type = EVT_MSE_DOWN;
            pEvtOut->button.x = evtIn.button.x;
            pEvtOut->button.y = cursor_y_ = evtIn.button.y;
            pEvtOut->button.button = evtIn.button.button;
            pEvtOut->button.keyMods = keyModState_;
            break;
        case SDL_MOUSEMOTION:
            update_cursor_ = true;
            pEvtOut->motion.type = EVT_MSE_MOTION;
            pEvtOut->motion.x = cursor_x_ = evtIn.motion.x;
            pEvtOut->motion.y = cursor_y_ = evtIn.motion.y;
            pEvtOut->motion.state = evtIn.motion.state;
            pEvtOut->motion.keyMods = keyModState_;
            break;
        default:
            break;
        }
    }

    return pEvtOut->type != EVT_NONE;
}

void SystemSDL::delay(int msec) {
    SDL_Delay(msec);
}

int SystemSDL::getTicks() {
    return SDL_GetTicks();
}

bool like(int a, int b) {
    return a == b || a == b - 1 || a == b + 1;
}

void SystemSDL::setPalette6b3(const uint8 * pal, int cols) {
    static SDL_Color palette[256];

    for (int i = 0; i < cols; ++i) {
        uint8 r = pal[i * 3 + 0];
        uint8 g = pal[i * 3 + 1];
        uint8 b = pal[i * 3 + 2];

        // multiply by 255 divide by 63 isn't good enough?
        palette[i].r = (r << 2) | (r >> 4);
        palette[i].g = (g << 2) | (g >> 4);
        palette[i].b = (b << 2) | (b >> 4);

#if 0
        if (like(palette[i].r, 28) && like(palette[i].g, 144)
            && like(palette[i].b, 0))
            printf("col %i = %i, %i, %i\n", i, palette[i].r, palette[i].g,
                   palette[i].b);
#endif
    }

    SDL_SetPaletteColors(temp_surf_->format->palette, palette, 0, cols);
}

void SystemSDL::setPalette8b3(const uint8 * pal, int cols) {
    static SDL_Color palette[256];

    for (int i = 0; i < cols; ++i) {
        palette[i].r = pal[i * 3 + 0];
        palette[i].g = pal[i * 3 + 1];
        palette[i].b = pal[i * 3 + 2];
    }

    SDL_SetPaletteColors(temp_surf_->format->palette, palette, 0, cols);
}

void SystemSDL::setColor(uint8 index, uint8 r, uint8 g, uint8 b) {
    static SDL_Color color;

    color.r = r;
    color.g = g;
    color.b = b;

    SDL_SetPaletteColors(temp_surf_->format->palette, &color, index, 1);
}

/*!
 * This method uses the SDL_Image library to load a file called
 * cursors/cursors.png under the root path.
 * The file is loaded into the cursor surface.
 * \return False if the loading has failed. If it's the case, 
 * cursor_surf_ will be NULL.
 */
bool SystemSDL::loadCursorSprites() {
    cursor_rect_.w = cursor_rect_.h = CURSOR_WIDTH;

    cursor_surf_ = IMG_Load(File::dataFullPath("cursors/cursors.png").c_str());

    if (!cursor_surf_) {
        printf("Cannot load cursors image: %s\n", IMG_GetError());
        return false;
    }

    return true;
}

/*! 
 * Returns the mouse pointer coordinates using SDL_GetMouseState. 
 * \param x The x coordinate.
 * \param y The y coordinate.
 * \return See SDL_GetMouseState.
 */
int SystemSDL::getMousePos(int *x, int *y) {
    return SDL_GetMouseState(x, y);
}

void SystemSDL::hideCursor() {
    if (cursor_surf_ != NULL) {
        cursor_visible_ = false;
    } else {
        // Custom cursor surface doesn't
        // exists so use the default SDL Cursor
        SDL_ShowCursor(SDL_DISABLE);
    }
}

void SystemSDL::showCursor() {
    if (cursor_surf_ != NULL) {
        cursor_visible_ = true;
    } else {
        // Custom cursor surface doesn't
        // exists so use the default SDL Cursor
        SDL_ShowCursor(SDL_ENABLE);
    }
}

void SystemSDL::useMenuCursor() {
    update_cursor_ = true;
    cursor_rect_.x = cursor_rect_.y = 0;
    cursor_hs_x_ = cursor_hs_y_ = 0;
}

void SystemSDL::usePointerCursor() {
    update_cursor_ = true;
    cursor_rect_.x = 24;
    cursor_rect_.y = 0;
    cursor_hs_x_ = cursor_hs_y_ = 0;
}

void SystemSDL::usePointerYellowCursor() {
    update_cursor_ = true;
    cursor_rect_.x = 24;
    cursor_rect_.y = 24;
    cursor_hs_x_ = cursor_hs_y_ = 0;
}

void SystemSDL::useTargetCursor() {
    update_cursor_ = true;
    cursor_rect_.x = 48;
    cursor_rect_.y = 0;
    cursor_hs_x_ = cursor_hs_y_ = 10;
}

void SystemSDL::useTargetRedCursor() {
    update_cursor_ = true;
    cursor_rect_.x = 72;
    cursor_rect_.y = 0;
    cursor_hs_x_ = cursor_hs_y_ = 10;
}

void SystemSDL::usePickupCursor() {
    update_cursor_ = true;
    cursor_rect_.x = 0;
    cursor_rect_.y = 24;
    cursor_hs_x_ = cursor_hs_y_ = 2;
}

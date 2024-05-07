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

#include "system_sdl.h"

#ifdef HAVE_SDL_MIXER

#include "mixer/sdlmixeraudio.h"

#endif // HAVE_SDL_MIXER

#include <SDL_image.h>
#include <algorithm>
#include "utf8.h"

#include "fs-engine/config.h"
#include "fs-engine/gfx/screen.h"
#include "fs-engine/sound/audio.h"
#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"

SDL_Joystick *joy = NULL;

const int SystemSDL::kCursorWidth = 24;

SystemSDL::SystemSDL() {
    keyModState_ = 0;

    pCursorTexture_ = nullptr;
    pWindow_ = nullptr;
    pRenderer_ = nullptr;
    pScreenSurface_ = nullptr;
    pScreenTexture_ = nullptr;

    pixels_ = new Uint32[Screen::kScreenWidth * Screen::kScreenHeight];
}

SystemSDL::~SystemSDL() {
    delete[] pixels_;
    pixels_ = nullptr;

    if (pScreenSurface_) {
        SDL_FreeSurface(pScreenSurface_);
        pScreenSurface_ = nullptr;
    }

    if (pScreenTexture_) {
        SDL_DestroyTexture(pScreenTexture_);
        pScreenTexture_ = nullptr;
    }

    if (pCursorTexture_) {
        SDL_DestroyTexture(pCursorTexture_);
        pCursorTexture_ = nullptr;
    }

    if (audio_) {
        audio_->quit();
    }

    // Destroy SDL_Image Lib
    IMG_Quit();

    if (pRenderer_) {
        SDL_DestroyRenderer(pRenderer_);
        pRenderer_ = nullptr;
    }

    if (pWindow_) {
        SDL_DestroyWindow( pWindow_ );
        pWindow_ = nullptr;
    }

    SDL_Quit();
}

bool SystemSDL::initialize(bool fullscreen) {
    LOG(Log::k_FLG_INFO, "SystemSDL", "initialize", ("initializing System SDL"))

    if (SDL_Init( SDL_INIT_VIDEO ) < 0) {
        FSERR(Log::k_FLG_GFX, "SystemSDL", "initialize", ("Critical error, SDL could not be initialized! SDL Error : %s", SDL_GetError()))
        return false;
    }

    pWindow_ = createWindow(fullscreen);

    if (pWindow_ == nullptr) {
        FSERR(Log::k_FLG_GAME, "SystemSDL", "initialize", ("Critical error, SDL could not be initialized! SDL Error : %s", SDL_GetError()))
        return false;
    }

    pRenderer_ = SDL_CreateRenderer(pWindow_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (pRenderer_ == nullptr) {
        FSERR(Log::k_FLG_GAME, "SystemSDL", "initialize", ("Critical error, SDL could not be initialized! SDL Error : %s", SDL_GetError()))
        return false;
    }

    // initialize the screen to black
    SDL_SetRenderDrawColor(pRenderer_, 0, 0, 0, 255);
    SDL_RenderClear(pRenderer_);
    // in case we are fullscreen and did not specify dimensions
    SDL_RenderSetLogicalSize(pRenderer_, Screen::kScreenWidth, Screen::kScreenHeight);

    //Get window surface
    pScreenSurface_ = SDL_CreateRGBSurface(0, Screen::kScreenWidth, Screen::kScreenHeight, 8, 0, 0, 0, 0);
    if (pScreenSurface_ == nullptr) {
        FSERR(Log::k_FLG_GAME, "SystemSDL", "initialize", ("Critical error, Screen surface could not be created! SDL Error : %s", SDL_GetError()))
        return false;
    }

    pScreenTexture_ = SDL_CreateTexture(pRenderer_,
                                            SDL_PIXELFORMAT_ARGB8888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            Screen::kScreenWidth, Screen::kScreenHeight);
    //pScreenTexture_ = SDL_CreateTextureFromSurface(pRenderer_, pScreenSurface_);

    if (pScreenTexture_ == nullptr) {
        FSERR(Log::k_FLG_GAME, "SystemSDL", "initialize", ("Critical error, Screen texture could not be created! SDL Error : %s", SDL_GetError()))
        return false;
    }

    // Init SDL_Image library
    int sdlImgFlags = IMG_INIT_PNG;
    int initted = IMG_Init(sdlImgFlags);
    if ( (initted & sdlImgFlags) != sdlImgFlags ) {
        FSERR(Log::k_FLG_GFX, "SystemSDL", "initialize", ("Critical error, SDL_Image could not be initialized! SDL Error : %s", IMG_GetError()))
        return false;
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

    // Audio initialization
#ifdef HAVE_SDL_MIXER
    audio_ = std::make_unique<SdlMixerAudio>();
#else
    audio_ = std::make_unique<DefaultAudio>();
#endif

    if (!audio_->init()) {
        FSINFO(Log::k_FLG_SND, "SystemSDL", "Init", ("Couldn't initialize Sound System : no sound will be played."))
    }

    // Be sure that we don't receive unusefull text events
    SDL_StopTextInput();

    return true;
}

SDL_Window * SystemSDL::createWindow(bool fullscreen) {
    SDL_Window *sdlWindow = nullptr;
    if (fullscreen) {
        sdlWindow = SDL_CreateWindow("Freesynd",
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             0, 0,
                             SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        sdlWindow = SDL_CreateWindow("Freesynd",
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          Screen::kScreenWidth, Screen::kScreenHeight,
                          SDL_WINDOW_SHOWN
                          );
    }

    return sdlWindow;
}

void SystemSDL::updateScreen() {
    if (g_Screen.dirty()|| (cursor_visible_ && update_cursor_)) {
        // Clear screen buffer
        SDL_RenderClear(pRenderer_);

        SDL_LockSurface(pScreenSurface_);

        const uint8 *srcPixels = g_Screen.pixels();

        // We do manual blitting to convert from 8bpp palette indexed values to 32bpp RGB for each pixel
        // thanks to bni (https://github.com/bni/freesynd)
        uint8 r, g, b;
        for (int i = 0; i < Screen::kScreenWidth * Screen::kScreenHeight; i++) {
            uint8 index = srcPixels[i];

            r = pScreenSurface_->format->palette->colors[index].r;
            g = pScreenSurface_->format->palette->colors[index].g;
            b = pScreenSurface_->format->palette->colors[index].b;

            Uint32 c = ((r << 16) | (g << 8) | (b << 0)) | (255 << 24);

            // TODO : try to use pScreenSurface directly
            pixels_[i] = c;
        }
        //memcpy (pScreenSurface_->pixels, srcPixels, Screen::kScreenWidth * Screen::kScreenHeight);

        SDL_UnlockSurface(pScreenSurface_);

        // Copy the pixel to the texture
        //SDL_UpdateTexture(pScreenTexture_, NULL, pScreenSurface_->pixels, pScreenSurface_->pitch);
        SDL_UpdateTexture(pScreenTexture_, NULL, pixels_, Screen::kScreenWidth * sizeof(Uint32));

        // Copy texture to the screen buffer
        SDL_RenderCopy(pRenderer_, pScreenTexture_, NULL, NULL);

        if (cursor_visible_) {
            SDL_Rect dst;

            dst.x = cursor_x_ - cursor_hs_x_;
            dst.y = cursor_y_ - cursor_hs_y_;
            dst.w = dst.h = kCursorWidth;

            SDL_RenderCopy( pRenderer_, pCursorTexture_, &cursor_rect_, &dst );
            update_cursor_ = false;
        }

        // Flip screen
        SDL_RenderPresent( pRenderer_ );
    }
}

/*!
 * Using the keysym parameter, verify if the given key is a function key (ie
 * a not printable key) returns the corresponding entry in the KeyFunc enumeration.
 * \returns If key code is not a function key, returns KEY_UNKNOWN.
 */
void SystemSDL::fillKeyEvent(SDL_Keysym keysym, FS_Event &evtOut) {
    evtOut.type = EVT_KEY_DOWN;
    FS_Key key;
    key.keyCode = kKeyCode_Unknown;
    switch(keysym.sym) {
        case SDLK_ESCAPE: key.keyCode = kKeyCode_Escape; break;
        case SDLK_BACKSPACE: key.keyCode = kKeyCode_Backspace;break;
        case SDLK_SPACE: key.keyCode = kKeyCode_Space;break;
        case SDLK_RETURN: key.keyCode = kKeyCode_Return; break;
        case SDLK_DELETE: key.keyCode = kKeyCode_Delete;break;
        case SDLK_UP: key.keyCode = kKeyCode_Up;break;
        case SDLK_DOWN: key.keyCode = kKeyCode_Down;break;
        case SDLK_RIGHT: key.keyCode = kKeyCode_Right;break;
        case SDLK_LEFT: key.keyCode = kKeyCode_Left;break;
        case SDLK_INSERT: key.keyCode = kKeyCode_Insert;break;
        case SDLK_HOME: key.keyCode = kKeyCode_Home;break;
        case SDLK_END: key.keyCode = kKeyCode_End;break;
        case SDLK_PAGEUP: key.keyCode = kKeyCode_PageUp;break;
        case SDLK_PAGEDOWN: key.keyCode = kKeyCode_PageDown;break;
        case SDLK_F1: case SDLK_F2: case SDLK_F3: case SDLK_F4:
        case SDLK_F5: case SDLK_F6: case SDLK_F7: case SDLK_F8:
        case SDLK_F9: case SDLK_F10: case SDLK_F11: case SDLK_F12:
            key.keyCode = static_cast < FS_KeyCode > (kKeyCode_F1 + (keysym.sym - SDLK_F1));
            break;
        case SDLK_0:case SDLK_1:case SDLK_2:case SDLK_3:case SDLK_4:
            key.keyCode = static_cast < FS_KeyCode > (kKeyCode_0 + (keysym.sym - SDLK_0));
            break;
        case SDLK_a: case SDLK_b: case SDLK_c: case SDLK_d: case SDLK_e:
        case SDLK_f: case SDLK_g: case SDLK_h: case SDLK_i: case SDLK_j:
        case SDLK_k: case SDLK_l: case SDLK_m: case SDLK_n: case SDLK_o:
        case SDLK_p: case SDLK_q: case SDLK_r: case SDLK_s: case SDLK_t:
        case SDLK_u: case SDLK_v: case SDLK_w: case SDLK_x: case SDLK_y:
        case SDLK_z:
            key.keyCode = static_cast < FS_KeyCode > (kKeyCode_A + (keysym.sym - SDLK_a));
            break;
        default:
            // unused key
            break;
    }

    evtOut.key.key = key;
    evtOut.key.keyMods = keyModState_;
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
bool SystemSDL::pumpEvents(FS_Event &evtOut) {
    SDL_Event evtIn;

    evtOut.type = EVT_NONE;

    if (SDL_PollEvent(&evtIn)) {
        switch (evtIn.type) {
        case SDL_QUIT:
            evtOut.quit.type = EVT_QUIT;
            break;
        case SDL_TEXTINPUT:
            {
            evtOut.key.type = EVT_KEY_DOWN;
            evtOut.key.key.keyCode = kKeyCode_Text;
            // ensure we have only one character in the event as SDL_TEXTINPUT can have a long text
            // which should not be the case in our game
            char* w = evtIn.text.text;
            evtOut.key.key.codePoint = utf8::next(w, evtIn.text.text + SDL_TEXTINPUTEVENT_TEXT_SIZE);
            evtOut.key.keyMods = keyModState_;
            }
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
                    break;
                case SDLK_LCTRL:
                    keyModState_ = keyModState_ | KMD_LCTRL;
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
                    fillKeyEvent(evtIn.key.keysym, evtOut);
                    break;
                } // end switch
            } // end case SDL_KEYDOWN
            break;
        case SDL_KEYUP:
            {
            switch(evtIn.key.keysym.sym) {
                case SDLK_RSHIFT:
                    keyModState_ = keyModState_ & ~KMD_RSHIFT;
                    break;
                case SDLK_LSHIFT:
                    keyModState_ = keyModState_ & ~KMD_LSHIFT;
                    break;
                case SDLK_RCTRL:
                    keyModState_ = keyModState_ & ~KMD_RCTRL;
                    break;
                case SDLK_LCTRL:
                    keyModState_ = keyModState_ & ~KMD_LCTRL;
                    break;
                case SDLK_RALT:
                    keyModState_ = keyModState_ & ~KMD_RALT;
                    break;
                case SDLK_LALT:
                    keyModState_ = keyModState_ & ~KMD_LALT;
                    break;
                default:
                    break;
            }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            evtOut.button.type = EVT_MSE_UP;
            evtOut.button.x = evtIn.button.x;
            evtOut.button.y = cursor_y_ = evtIn.button.y;
            evtOut.button.button = evtIn.button.button;
            evtOut.button.keyMods = keyModState_;
            break;
        case SDL_MOUSEBUTTONDOWN:
            evtOut.button.type = EVT_MSE_DOWN;
            evtOut.button.x = evtIn.button.x;
            evtOut.button.y = cursor_y_ = evtIn.button.y;
            evtOut.button.button = evtIn.button.button;
            evtOut.button.keyMods = keyModState_;
            break;
        case SDL_MOUSEMOTION:
            update_cursor_ = true;
            evtOut.motion.type = EVT_MSE_MOTION;
            evtOut.motion.x = cursor_x_ = evtIn.motion.x;
            evtOut.motion.y = cursor_y_ = evtIn.motion.y;
            evtOut.motion.state = evtIn.motion.state;
            evtOut.motion.keyMods = keyModState_;
            break;
        default:
            break;
        }
    }

    return evtOut.type != EVT_NONE;
}

void SystemSDL::delay(uint32 msec) {
    SDL_Delay(msec);
}

uint32 SystemSDL::getTicks() {
    return SDL_GetTicks();
}

bool like(int a, int b) {
    return a == b || a == b - 1 || a == b + 1;
}

bool SystemSDL::setPalette6b3(const uint8 * pal, int cols) {
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

    if (SDL_SetPaletteColors(pScreenSurface_->format->palette, palette, 0, cols)) {
        FSERR(Log::k_FLG_GFX, "SystemSDL", "setPalette6b3", ("Could not set palette6b3 with %i colors! SDL Error : %s", cols, SDL_GetError()))
        return false;
    }
    return true;
}

bool SystemSDL::setPalette8b3(const uint8 * pal, int cols) {
    static SDL_Color palette[256];

    for (int i = 0; i < cols; ++i) {
        palette[i].r = pal[i * 3 + 0];
        palette[i].g = pal[i * 3 + 1];
        palette[i].b = pal[i * 3 + 2];
    }

    if (SDL_SetPaletteColors(pScreenSurface_->format->palette, palette, 0, cols)) {
        FSERR(Log::k_FLG_GFX, "SystemSDL", "setPalette6b3", ("Could not set palette8b3 with %i colors! SDL Error : %s", cols, SDL_GetError()))
        return false;
    }
    return true;
}

void SystemSDL::setColor(uint8 index, uint8 r, uint8 g, uint8 b) {
    static SDL_Color color;

    color.r = r;
    color.g = g;
    color.b = b;

    SDL_SetPaletteColors(pScreenSurface_->format->palette, &color, index, 1);
}

/*!
 * This method uses the SDL_Image library to load a file called
 * cursors/cursors.png under the root path.
 * The file is loaded into the cursor surface.
 * \return False if the loading has failed. If it's the case,
 * pCursorSurface_ will be NULL.
 */
bool SystemSDL::loadCursorSprites() {
    LOG(Log::k_FLG_GFX, "SystemSDL", "loadCursorSprites", ("loading cursor sprites from file cursors/cursors.png"))
    cursor_rect_.w = cursor_rect_.h = kCursorWidth;

    SDL_Surface* pLoadedSurface = IMG_Load(File::getFreesyndDataFullPath("cursors/cursors.png").c_str());

    if (!pLoadedSurface) {
        FSERR(Log::k_FLG_GFX, "SystemSDL", "loadCursorSprites", ("Cannot load cursors image : %s", IMG_GetError()))
        return false;
    }

    pCursorTexture_ = SDL_CreateTextureFromSurface( pRenderer_, pLoadedSurface );

    //Get rid of old loaded surface
    SDL_FreeSurface( pLoadedSurface );

    if( pCursorTexture_ == nullptr ) {
        FSERR(Log::k_FLG_GFX, "SystemSDL", "loadCursorSprites", ("Cannot create cursor texture : %s", SDL_GetError()))
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
uint32 SystemSDL::getMousePos(int *x, int *y) {
    return SDL_GetMouseState(x, y);
}

void SystemSDL::hideCursor() {
    if (pCursorTexture_!= NULL) {
        cursor_visible_ = false;
    } else {
        // Custom cursor surface doesn't
        // exists so use the default SDL Cursor
        SDL_ShowCursor(SDL_DISABLE);
    }
}

void SystemSDL::showCursor() {
    if (pCursorTexture_ != NULL) {
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

/*!
 * Calls SDL_StartTextInput() will tell SDL2 to send SDL_TEXTINPUT events.
 * This method is called when a textfield widget receives focus.
 */
void SystemSDL::startReceiveText() {
    SDL_StartTextInput();
}

/*!
 * Calls SDL_StartTextInput() will tell SDL2 to stop sendeing SDL_TEXTINPUT events.
 * This method is called when a textfield widget loses focus.
 */
void SystemSDL::stopReceiveText() {
    SDL_StopTextInput();
}

/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
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

#include "system_sdl.h"

#ifdef HAVE_SDL_MIXER

#include "mixer/sdlmixeraudio.h"

#endif // HAVE_SDL_MIXER

#include <SDL_image.h>
#include <algorithm>
#include <format>
#include "utf8.h"

#include "fs-engine/appcontext.h"
#include "fs-engine/config.h"
#include "fs-engine/sound/audio.h"
#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "fstexture_sdl.h"

namespace fs_eng {

SDL_Joystick *joy = NULL;

const int SystemSDL::kCursorWidth = 24;

SystemSDL::SystemSDL() {
    keyModState_ = 0;

    pCursorTexture_ = nullptr;
    pWindow_ = nullptr;
    pRenderer_ = nullptr;
    pGameTexture_ = nullptr;
    viewportRect_ = {0, 0, 0, 0};
    nativeAspectRatio_ = false;
    gameHeight_ = fs_eng::kScreenHeight;
    gameWidth_ = fs_eng::kScreenWidth;
}

SystemSDL::~SystemSDL() {
    if (pCursorTexture_) {
        SDL_DestroyTexture(pCursorTexture_);
        pCursorTexture_ = nullptr;
    }

    if (audio_) {
        audio_->quit();
    }

    // Destroy SDL_Image Lib
    IMG_Quit();

    if (pGameTexture_) {
        SDL_DestroyTexture(pGameTexture_);
        pGameTexture_ = nullptr;
    }

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

void SystemSDL::initialize(bool fullscreen) {
    LOG(Log::k_FLG_INFO, "SystemSDL", "initialize", ("initializing System SDL"))

    if (SDL_Init( SDL_INIT_VIDEO ) < 0) {
        throw InitializationFailedException(std::format("Critical error, SDL could not be initialized! SDL Error :  {}", SDL_GetError()));
    }

    pWindow_ = createWindow(fullscreen);

    if (pWindow_ == nullptr) {
        throw InitializationFailedException(std::format("Critical error, SDL could not be initialized! SDL Error :  {}", SDL_GetError()));
    }

    pRenderer_ = SDL_CreateRenderer(pWindow_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (pRenderer_ == nullptr) {
        throw InitializationFailedException(std::format("Critical error, SDL could not be initialized! SDL Error :  {}", SDL_GetError()));
    }

    // initialize the screen to black
    SDL_SetRenderDrawColor(pRenderer_, 0, 0, 0, 255);
    SDL_RenderClear(pRenderer_);

    // Create offscreen game texture and calculate viewport
    calculateGameViewport();
    recreateGameTexture();

    // Init SDL_Image library
    int sdlImgFlags = IMG_INIT_PNG;
    int initted = IMG_Init(sdlImgFlags);
    if ( (initted & sdlImgFlags) != sdlImgFlags ) {
        throw InitializationFailedException(std::format("Critical error, SDL_Image could not be initialized! SDL Error :  {}", IMG_GetError()));
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
    // TextInput are activated when user clicks in a textfield
    SDL_StopTextInput();
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
                          fs_eng::kScreenWidth * 2, fs_eng::kScreenWidth * 3 / 2,
                          SDL_WINDOW_SHOWN
                          );
    }

    return sdlWindow;
}

void SystemSDL::showError(const char *errorMsg) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                "Error in Freesynd",
                                errorMsg,
                                pWindow_);
    FSERR(Log::k_FLG_IO, "SystemSDL", "showError", (errorMsg));
}

/*!
 * Clear the screen with the black color.
 * @return True if ok.
 */
bool SystemSDL::clearScreen() {
    SDL_SetRenderDrawColor(pRenderer_, 0x00, 0x00, 0x00, 0xFF);
    return SDL_RenderClear(pRenderer_);
}

void SystemSDL::updateScreen() {
    // Draw cursor into the game texture (in game coordinates)
    if (cursor_visible_) {
        SDL_Rect dst;
        dst.x = cursor_x_ - cursor_hs_x_;
        dst.y = cursor_y_ - cursor_hs_y_;
        dst.w = dst.h = kCursorWidth;
        SDL_RenderCopy(pRenderer_, pCursorTexture_, &cursor_rect_, &dst);
        update_cursor_ = false;
    }

    // Switch to the screen, clear to black (fills letterbox borders), then
    // blit the game texture stretched to the 4:3 integer-scaled viewport
    SDL_SetRenderTarget(pRenderer_, nullptr);
    SDL_SetRenderDrawColor(pRenderer_, 0, 0, 0, 255);
    SDL_RenderClear(pRenderer_);
    SDL_RenderCopy(pRenderer_, pGameTexture_, nullptr, &viewportRect_);

    SDL_RenderPresent(pRenderer_);

    // Restore game texture as render target for the next frame
    SDL_SetRenderTarget(pRenderer_, pGameTexture_);
}

bool SystemSDL::resetRenderTarget() {
    // Restore to the game texture (not the screen), so all rendering
    // continues into the offscreen buffer
    if (SDL_SetRenderTarget(pRenderer_, pGameTexture_)) {
        FSERR(Log::k_FLG_GFX, "SystemSDL", "resetRenderTarget", ("Critical error, Could not reset target texture! SDL Error : %s", SDL_GetError()))
        return false;
    }

    return true;
}

void SystemSDL::calculateGameViewport() {
    int winW, winH;
    SDL_GetWindowSize(pWindow_, &winW, &winH);

    if (nativeAspectRatio_) {
        // Gameplay: use the largest integer scale N where N*640 <= winW and N*400 <= winH.
        // Expand the game texture to fill the full window in both directions:
        // gameWidth = winW / N, gameHeight = winH / N (both >= their respective kScreen* minimums).
        int n = g_Ctx.getScaleFactor();
        if (n <= 0) {
            bool isFullscreen = (SDL_GetWindowFlags(pWindow_) &
                (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
            if (isFullscreen) {
                // Default to ~2x at 1440p: use 1.5x native resolution as base unit
                // (960×600), yielding n=1 at 720p/1080p, n=2 at 1440p, n=3 at 4K.
                n = std::min(winW / (fs_eng::kScreenWidth * 3 / 2),
                             winH / (fs_eng::kScreenHeight * 3 / 2));
            } else {
                n = 2; // Default 2x scale in windowed mode
            }
        }
        if (n < 1) n = 1;
        gameWidth_  = winW / n;
        gameHeight_ = winH / n;
        viewportRect_ = {0, 0, n * gameWidth_, n * gameHeight_};
    } else {
        // Menus: original 320x200 was displayed on a 4:3 CRT, equivalent to
        // 640x480 in our doubled coordinate space. Stretch 400 source rows to
        // 480 display rows to replicate the CRT pixel aspect ratio.
        gameWidth_  = fs_eng::kScreenWidth;
        gameHeight_ = fs_eng::kScreenHeight;
        int targetH = fs_eng::kScreenWidth * 3 / 4; // 480
        int n = std::min(winW / fs_eng::kScreenWidth, winH / targetH);
        if (n < 1) n = 1;
        int scaledW = n * fs_eng::kScreenWidth;
        int scaledH = n * targetH;
        viewportRect_ = {(winW - scaledW) / 2, (winH - scaledH) / 2, scaledW, scaledH};
    }
}

void SystemSDL::recreateGameTexture() {
    if (pGameTexture_) {
        SDL_DestroyTexture(pGameTexture_);
        pGameTexture_ = nullptr;
    }
    pGameTexture_ = SDL_CreateTexture(pRenderer_, SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_TARGET,
                                      gameWidth_, gameHeight_);
    if (pGameTexture_ == nullptr) {
        throw InitializationFailedException(std::format("Critical error, could not create game texture! SDL Error: {}", SDL_GetError()));
    }
    SDL_SetRenderTarget(pRenderer_, pGameTexture_);
}

void SystemSDL::setNativeAspectRatio(bool native) {
    int oldGameWidth  = gameWidth_;
    int oldGameHeight = gameHeight_;
    nativeAspectRatio_ = native;
    calculateGameViewport();
    if (gameWidth_ != oldGameWidth || gameHeight_ != oldGameHeight) {
        recreateGameTexture();
    }
}

Point2D SystemSDL::windowToGame(int x, int y) const {
    if (viewportRect_.w == 0 || viewportRect_.h == 0) {
        return {x, y};
    }
    int gx = (x - viewportRect_.x) * gameWidth_  / viewportRect_.w;
    int gy = (y - viewportRect_.y) * gameHeight_ / viewportRect_.h;
    gx = std::clamp(gx, 0, gameWidth_  - 1);
    gy = std::clamp(gy, 0, gameHeight_ - 1);
    return {gx, gy};
}

/*!
 * Set the viewport
 * @param x Origin of the viewport on the screen
 * @param y Origin of the viewport on the screen
 * @param width Width of the viewport
 * @param height Height of the viewport
 * @return True if setting the viewport is ok
 */
bool SystemSDL::setViewport(int x, int y, int width, int height) {
    SDL_Rect rect {x, y, width, height};
    if (SDL_RenderSetViewport(pRenderer_, &rect)) {
        FSERR(Log::k_FLG_GFX, "SystemSDL", "setViewport", ("Critical error, Could not set viewport! SDL Error : %s", SDL_GetError()))
        return false;
    }

    return true;
}
    
/*!
 * Reset the viewport to the default one
 * @return True if ok
 */
bool SystemSDL::resetViewport() {
    if (SDL_RenderSetViewport(pRenderer_, nullptr)) {
        FSERR(Log::k_FLG_GFX, "SystemSDL", "resetViewport", ("Critical error, Could not reset viewport! SDL Error : %s", SDL_GetError()))
        return false;
    }

    return true;
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
        case SDLK_TAB: key.keyCode = kKeyCode_Tab; break;
        case SDLK_KP_PLUS: key.keyCode = kKeyCode_Plus; break;
        case SDLK_KP_MINUS: key.keyCode = kKeyCode_Minus; break;
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
                    if (!(keyModState_ & KMD_CTRL)) {
                        evtOut.key.type = EVT_KEY_UP;
                        evtOut.key.key.keyCode = kKeyCode_Ctrl;
                        evtOut.key.keyMods = keyModState_;
                    }
                    break;
                case SDLK_LCTRL:
                    keyModState_ = keyModState_ & ~KMD_LCTRL;
                    if (!(keyModState_ & KMD_CTRL)) {
                        evtOut.key.type = EVT_KEY_UP;
                        evtOut.key.key.keyCode = kKeyCode_Ctrl;
                        evtOut.key.keyMods = keyModState_;
                    }
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
            {
            Point2D p = windowToGame(evtIn.button.x, evtIn.button.y);
            evtOut.button.type = EVT_MSE_UP;
            evtOut.button.x = p.x;
            evtOut.button.y = cursor_y_ = p.y;
            evtOut.button.button = evtIn.button.button;
            evtOut.button.keyMods = keyModState_;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            {
            Point2D p = windowToGame(evtIn.button.x, evtIn.button.y);
            evtOut.button.type = EVT_MSE_DOWN;
            evtOut.button.x = p.x;
            evtOut.button.y = cursor_y_ = p.y;
            evtOut.button.button = evtIn.button.button;
            evtOut.button.keyMods = keyModState_;
            }
            break;
        case SDL_MOUSEMOTION:
            {
            update_cursor_ = true;
            evtOut.motion.type = EVT_MSE_MOTION;
            evtOut.motion.state = evtIn.motion.state;
            evtOut.motion.keyMods = keyModState_;
            if (SDL_GetRelativeMouseMode()) {
                // In relative mode: report a fake absolute position = centre +
                // scaled delta.  The game code computes delta as (point -
                // last_motion_), and last_motion_ is reset to centre after each
                // panning step, so this transparently delivers the raw delta
                // without any screen-edge or Dock friction.
                // Store the untruncated float deltas so the game can use them
                // with a sub-pixel accumulator for perfectly smooth scrolling.
                float scaleX = viewportRect_.w > 0
                    ? (float)gameWidth_  / (float)viewportRect_.w : 1.0f;
                float scaleY = viewportRect_.h > 0
                    ? (float)gameHeight_ / (float)viewportRect_.h : 1.0f;
                lastRelX_ = evtIn.motion.xrel * scaleX;
                lastRelY_ = evtIn.motion.yrel * scaleY;
                evtOut.motion.x = gameWidth_  / 2 + (int)lastRelX_;
                evtOut.motion.y = gameHeight_ / 2 + (int)lastRelY_;
            } else {
                lastRelX_ = 0.0f;
                lastRelY_ = 0.0f;
                Point2D p = windowToGame(evtIn.motion.x, evtIn.motion.y);
                evtOut.motion.x = cursor_x_ = p.x;
                evtOut.motion.y = cursor_y_ = p.y;
            }
            }
            break;
        default:
            break;
        }
    }

    return evtOut.type != EVT_NONE;
}

void SystemSDL::delay(uint32_t msec) {
    SDL_Delay(msec);
}

uint32_t SystemSDL::getTicks() {
    return SDL_GetTicks();
}

bool like(int a, int b) {
    return a == b || a == b - 1 || a == b + 1;
}

/*!
 * Draw a point on the screen at given position
 * @param position 
 * @param color 
 */
void SystemSDL::drawPoint (Point2D position, fs_eng::FSColor color) {
    SDL_SetRenderDrawColor( pRenderer_, color.r, color.g, color.b, color.a );
    SDL_RenderDrawPoint(pRenderer_, position.x, position.y);
}

/*!
 * Draw a vertical line with given length and color
 * @param start The starting point of the line
 * @param length The length of the line
 * @param color Color of the line
 */
void SystemSDL::drawVLine(Point2D start, int length, fs_eng::FSColor color) {
    SDL_SetRenderDrawColor( pRenderer_, color.r, color.g, color.b, color.a );
    SDL_RenderDrawLine( pRenderer_, start.x, start.y, start.x, start.y + length );
}

/*!
 * Draw a horizontal line with given length and color
 * @param start The starting point of the line
 * @param length The length of the line
 * @param color Color of the line
 */
void SystemSDL::drawHLine(Point2D start, int length, fs_eng::FSColor color) {
    SDL_SetRenderDrawColor( pRenderer_, color.r, color.g, color.b, color.a );
    SDL_RenderDrawLine( pRenderer_, start.x, start.y, start.x + length, start.y );
}

/*!
 * Draw a vertical line of the given length which is composed of segments of dashLength and space of same size.
 * @param start The starting point of the line
 * @param length The length of the line
 * @param dashLength This is the length of a segment. Drawn segments and not drawn segment have the same size
 * @param dashOffset This value is used to animate the dashline. Its maximum is twice of dashLength.
 * @param color Color of the line
 */
void SystemSDL::drawDashedVLine(Point2D start, int length, int dashLength, int dashOffset, fs_eng::FSColor color) {
    // dashOffset cannot be longer than 2 times the length of a segment
    // knowing that drawn and non drawn segments are equals
    dashOffset %= 2 * dashLength;
    Point2D end = start.add(0, length);

    SDL_SetRenderDrawColor( pRenderer_, color.r, color.g, color.b, color.a );
   
    Point2D segmentStart = start;
    Point2D segmentEnd = start;

    if (dashOffset <= dashLength) {
        segmentEnd.y += dashOffset;
    } else {
        segmentStart.y += (dashOffset - dashLength);
        segmentEnd.y = (segmentStart.y + dashLength);
    }
    SDL_RenderDrawLine(pRenderer_, segmentStart.x, segmentStart.y, segmentEnd.x, segmentEnd.y);
    // distance done is the accumulation of draw segment and non drawn segment
    int distanceDone = dashOffset;

    int i=0;
    while (distanceDone < length && i < 100) {
        if (segmentEnd.y + dashLength > end.y) {
            break;
        }
        segmentStart.y = segmentEnd.y + dashLength;
        segmentEnd.y = (segmentStart.y + dashLength > end.y) ? end.y : segmentStart.y + dashLength;
        SDL_RenderDrawLine(pRenderer_, segmentStart.x, segmentStart.y, segmentEnd.x, segmentEnd.y);
        distanceDone += dashLength + (segmentEnd.y - segmentStart.y);
        i++;
    }
}

/*!
 * Draw a horizontal line of the given length which is composed of segments of dashLength and space of same size.
 * @param start The starting point of the line
 * @param length The length of the line
 * @param dashLength This is the length of a segment. Drawn segments and not drawn segment have the same size
 * @param dashOffset This value is used to animate the dashline. Its maximum is twice of dashLength.
 * @param color Color of the line
 */
void SystemSDL::drawDashedHLine(Point2D start, int length, int dashLength, int dashOffset, fs_eng::FSColor color) {
    // dashOffset cannot be longer than 2 times the length of a segment
    // knowing that drawn and non drawn segments are equals
    dashOffset %= 2 * dashLength;
    Point2D end = start.add(length, 0);

    SDL_SetRenderDrawColor( pRenderer_, color.r, color.g, color.b, color.a );
   
    Point2D segmentStart = start;
    Point2D segmentEnd = start;

    if (dashOffset <= dashLength) {
        segmentEnd.x += dashOffset;
    } else {
        segmentStart.x += (dashOffset - dashLength);
        segmentEnd.x = (segmentStart.x + dashLength);
    }
    SDL_RenderDrawLine(pRenderer_, segmentStart.x, segmentStart.y, segmentEnd.x, segmentEnd.y);
    // distance done is the accumulation of draw segment and non drawn segment
    int distanceDone = dashOffset;

    int i=0;
    while (distanceDone < length && i < 100) {
        if (segmentEnd.x + dashLength > end.x) {
            break;
        }
        segmentStart.x = segmentEnd.x + dashLength;
        segmentEnd.x = (segmentStart.x + dashLength > end.x) ? end.x : segmentStart.x + dashLength;
        SDL_RenderDrawLine(pRenderer_, segmentStart.x, segmentStart.y, segmentEnd.x, segmentEnd.y);
        distanceDone += dashLength + (segmentEnd.x - segmentStart.x);
        i++;
    }
}

/*!
 * Draw a line from start to end points with given color.
 * @param start 
 * @param end 
 * @param color 
 */
void SystemSDL::drawLine(Point2D start, Point2D end, fs_eng::FSColor color) {
    SDL_SetRenderDrawColor( pRenderer_, color.r, color.g, color.b, color.a );
    SDL_RenderDrawLine( pRenderer_, start.x, start.y, end.x, end.y );
}

/*!
 * Draw a rect with given color
 * @param pos
 * @param width 
 * @param height 
 * @param color 
 */
void SystemSDL::drawRect(Point2D pos, int width, int height, fs_eng::FSColor color) {
    SDL_Rect outlineRect = { pos.x, pos.y, width, height};
    SDL_SetRenderDrawColor( pRenderer_, color.r, color.g, color.b, color.a );
    SDL_RenderDrawRect( pRenderer_, &outlineRect );
}

/*!
 * Draw a rect filled with given color
 * @param pos 
 * @param width 
 * @param height 
 * @param color 
 */
void SystemSDL::drawFillRect(Point2D pos, int width, int height, fs_eng::FSColor color) {
    SDL_Rect outlineRect = { pos.x, pos.y, width, height};
    SDL_SetRenderDrawColor( pRenderer_, color.r, color.g, color.b, color.a );
    SDL_RenderFillRect( pRenderer_, &outlineRect );
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

    SDL_Surface* pLoadedSurface = IMG_Load(fs_utl::File::getFreesyndDataFullPath("cursors/cursors.png").c_str());

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
 * \param point The mouse coordinate.
 * \return See SDL_GetMouseState.
 */
uint32_t SystemSDL::getMousePos(Point2D &point) {
    int x, y;
    uint32_t buttons = SDL_GetMouseState(&x, &y);
    point = windowToGame(x, y);
    return buttons;
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

void SystemSDL::warpMouseToCenter() {
    int cx = viewportRect_.x + viewportRect_.w / 2;
    int cy = viewportRect_.y + viewportRect_.h / 2;
    SDL_WarpMouseInWindow(pWindow_, cx, cy);
    // Discard the synthetic SDL_MOUSEMOTION event that the warp generates
    // so it doesn't corrupt the panning delta on the next frame.
    SDL_FlushEvents(SDL_MOUSEMOTION, SDL_MOUSEMOTION);
}

void SystemSDL::setRelativeMouseMode(bool enabled) {
    SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE);
}

void SystemSDL::getLastMouseRelDelta(float &xrel, float &yrel) const {
    xrel = lastRelX_;
    yrel = lastRelY_;
}

void SystemSDL::warpMouseTo(int gameX, int gameY) {
    if (viewportRect_.w == 0 || viewportRect_.h == 0) return;
    int wx = gameX * viewportRect_.w / gameWidth_  + viewportRect_.x;
    int wy = gameY * viewportRect_.h / gameHeight_ + viewportRect_.y;
    SDL_WarpMouseInWindow(pWindow_, wx, wy);
    SDL_FlushEvents(SDL_MOUSEMOTION, SDL_MOUSEMOTION);
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
/*!
 * Returns true if the given key is currently held down.
 * Uses SDL_GetKeyboardState for polling (not event-based), so it works
 * correctly for keys held across multiple frames (e.g. WASD panning).
 */
bool SystemSDL::isKeyDown(fs_eng::FS_KeyCode key) const {
    const Uint8 *state = SDL_GetKeyboardState(nullptr);
    if (key >= fs_eng::kKeyCode_A && key <= fs_eng::kKeyCode_Z) {
        int letter = key - fs_eng::kKeyCode_A;  // 0 = 'a', 25 = 'z'
        return state[SDL_SCANCODE_A + letter];
    }
    return false;
}

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

std::unique_ptr<FSTexture> SystemSDL::createTexture() {
    return std::make_unique<FSTextureSDL>(pRenderer_);
}

/*!
 * Call SDL_GetPreferredLocales() to get the locales for the system.
 * If there is an error, return English silently.
 * @return If no language matches the system locales, return English
 */
fs_eng::FS_Lang SystemSDL::getLanguageFromSystem() {
    fs_eng::FS_Lang resLang = fs_eng::ENGLISH;
    SDL_Locale * locales = SDL_GetPreferredLocales();
    if (locales != NULL) {
        SDL_Locale *pLocale = locales;
        while (pLocale->language != NULL) {
            std::string lang(pLocale->language);
            if (lang.compare("en") == 0) {
                resLang = fs_eng::ENGLISH;
                break;
            } else if (lang.compare("fr") == 0) {
                resLang = fs_eng::FRENCH;
                break;
            } else if (lang.compare("it") == 0) {
                resLang = fs_eng::ITALIAN;
                break;
            } if (lang.compare("de") == 0) {
                resLang = fs_eng::GERMAN;
                break;
            }
            pLocale++;
        }
    }

    SDL_free(locales);
    return resLang;
}

}

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *  Copyright (C) 2011  Benoit Blancard <benblan@users.sourceforge.net> *
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

#include "app.h"
#include "menus/flimenu.h"
#include "menus/gamemenuid.h"
#include "utils/file.h"
#include "gfx/screen.h"

const FrameEvent intro[] = {
    { 1,  msc::TRACK_INTRO, snd::NO_SOUND, 0x0, NULL }, // Play track
    { 15, msc::NO_TRACK, snd::NO_SOUND,        0x0, "INTRO_0" }, 
    { 39, msc::NO_TRACK, snd::NO_SOUND,        0x0, NULL }, // clear subtitle
    { 44, msc::NO_TRACK, snd::NO_SOUND,        0x0, "INTRO_1" },
    { 62, msc::NO_TRACK, snd::NO_SOUND,        0x0, NULL }, // clear subtitle
    { 67, msc::NO_TRACK, snd::NO_SOUND,        0x0, "INTRO_2" },
    { 85, msc::NO_TRACK, snd::NO_SOUND,        0x0, NULL }, // clear
    { 90, msc::NO_TRACK, snd::NO_SOUND,        0x0, "INTRO_3" },
    { 117, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL }, // clear
    { 121, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_4" },
    { 135, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL }, // clear
    { 138, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_5" },
    { 153, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_6" },
    { 165, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_7" },
    { 178, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL }, // Clear
    { 180, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_8" },
    { 200, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL }, // clear
    { 234, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_9" },
    { 271, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL },//clear
    { 291, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_10" },
    { 333, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL }, // Clear
    { 359, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_11" },
    { 406, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL }, // clear
    { 442, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_12" },
    { 467, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL }, // clear
    { 473, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_13" },
    { 502, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL }, // clear
    { 548, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_14" },
    { 562, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL },
    { 569, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_15" },
    { 610, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL },
    { 639, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_16" },
    { 673, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL },
    { 848, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_17" },
    { 892, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL },
    { 904, msc::NO_TRACK, snd::NO_SOUND,    0x0, "INTRO_18" },
    { 960, msc::NO_TRACK, snd::NO_SOUND,    0x0, NULL },
    {(uint16)-1, msc::NO_TRACK, snd::NO_SOUND, 0x0, NULL }
};

const FrameEvent mission_win[] = {
    { 1, msc::NO_TRACK, snd::APPLAUSE,              0x1, NULL },
    { 1, msc::NO_TRACK, snd::APPLAUSE_ZOOM,         0x0, NULL },
    { 10, msc::NO_TRACK, snd::FIREWORKS,            0x1, NULL },
    { 30, msc::NO_TRACK, snd::FIREWORKS_APPLAUSE,   0x1, NULL },
    {(uint16)-1, msc::NO_TRACK, snd::NO_SOUND,              0x0, NULL }
};

const FrameEvent mission_failed[] = {
    { 1, msc::NO_TRACK, snd::NO_SOUND, 0x0, NULL },
    { 35, msc::NO_TRACK, snd::PRESS_BUTTON, 0x0, NULL },
    { 52, msc::NO_TRACK, snd::MENU_AFTER_MISSION, 0x0, NULL },
    { 80, msc::NO_TRACK, snd::SPEECH_NO, 0x0, NULL },
    { 108, msc::NO_TRACK, snd::MONITOR_IMPACT, 0x0, NULL },
    { 118, msc::NO_TRACK, snd::GLASS_BREAKING_2, 0x0, NULL },
    {(uint16)-1, msc::NO_TRACK, snd::NO_SOUND, 0x0, NULL }
};

const FrameEvent game_lost[] = {
    { 1, msc::NO_TRACK, snd::NO_SOUND, 0x0, NULL },
    { 2, msc::NO_TRACK, snd::FALLING_COMMAND_SHIP, 0x0, NULL },
    { 34, msc::NO_TRACK, snd::EXPLOSION_BIG, 0x0, NULL },
    {(uint16)-1, msc::NO_TRACK, snd::NO_SOUND, 0x0, NULL }
};

const FrameEvent menu_up[] = {
    { 1, msc::NO_TRACK, snd::MENU_UP,   0x1, NULL },
    {(uint16)-1, msc::NO_TRACK, snd::NO_SOUND,  0x0, NULL }
};

const FrameEvent no_event[] = {
    {(uint16)-1, msc::NO_TRACK, snd::NO_SOUND, 0x0, NULL }
};

FliMenu::FliMenu(MenuManager *m, int menuId) : Menu(m, menuId, fs_game_menus::kMenuIdMain), fliPlayer_(m)
{
    fliIndex_ = 0;
    pData_ = NULL;
    playingFli_ = false;
    isCachable_ = false;
    currSubTitle_ = "";

    if (menuId == fs_game_menus::kMenuIdFliSuccess || menuId == fs_game_menus::kMenuIdFliFailedMission) {
        if (menuId == fs_game_menus::kMenuIdFliSuccess) {
            addFliDesc("mgamewin.dat", 66, false, true, mission_win);
        } else if (menuId == fs_game_menus::kMenuIdFliFailedMission) {
            addFliDesc("mendlose.dat", 66, false, true, mission_failed);
        } else if (menuId == fs_game_menus::kMenuIdFliGameLost) {
            // All availiable agents are dead
            // TODO: use it
            addFliDesc("mlosegam.dat", 66, false, true, game_lost);
        }
        // common fli to prepare next menu show
        addFliDesc("mscrenup.dat", 50, false, false, menu_up);
        nextMenu_ = fs_game_menus::kMenuIdDebrief;
    } else if (menuId == fs_game_menus::kMenuIdFliTitle || menuId == fs_game_menus::kMenuIdFliIntro) {
        if ( menuId == fs_game_menus::kMenuIdFliIntro) {
            // Display the intro animation
            addFliDesc("intro.dat", 66, false, true, intro);
        }
        // Display the splash screen
        addFliDesc("mtitle.dat", 66, true, true, no_event);
        // common fli to prepare next menu show
        addFliDesc("mscrenup.dat", 50, false, false, menu_up);
        nextMenu_ = fs_game_menus::kMenuIdMain;
    }

}

FliMenu::~FliMenu()
{
    if (pData_) {
        delete[] pData_;
        pData_ = NULL;
    }
}

/*!
 * Adds a new description.
 * \param anim animation file name.
 * \param frameDelay animation speed.
 * \param waitKey True to wait for the user input
 * \param skipable True means user can skip animation
 * \param music
 * \param sound
 */
void FliMenu::addFliDesc(const char *anim, uint8 frameDelay, bool waitKey, bool skipable, const FrameEvent *events) {
    FliDesc desc;
    desc.name = anim;
    desc.frameDelay = frameDelay;
    desc.waitKeyPressed = waitKey;
    desc.skipable = skipable;
    desc.evtList = events;

    fliList_.push_back(desc);
}

/*!
 * Loads the next animation if there is one still in the list.
 * \return True if an animation has been loaded.
 */
bool FliMenu::loadNextFli() {
    // Stop all music if one was being played
    g_App.music().stopPlayback();
    playingFli_ = false;
    // loads Fli
    if ( fliIndex_ < fliList_.size()) {
        int size = 0;

        if (pData_) {
            delete[] pData_;
            pData_ = NULL;
        }
        // Gets the fli description
        FliDesc desc = fliList_.at(fliIndex_);
        // Loads data from file
        pData_ = File::loadOriginalFile(desc.name, size);
        if (pData_) {
            fliPlayer_.loadFliData(pData_);
            if (fliPlayer_.hasFrames()) {
                g_Screen.clear(0);
                // init frame delay counter with max value so first frame is
                // drawn in the first pass
                frameDelay_ = desc.frameDelay;
                frameIndex_ = 0;
                currSubTitle_.erase();
                fliIndex_++;
                return true;
            }
        }
    }

    // No animation was loaded
    return false;
}

void FliMenu::handleShow() 
{
    // Loads the first Fli
    if (!fliList_.empty()) {
        loadNextFli();
    }
}

void FliMenu::handleTick(int elapsed) 
{
    if (fliPlayer_.hasFrames()) {
        FliDesc desc = fliList_.at(fliIndex_ - 1);
        // There is a frame to display
        frameDelay_ += elapsed;
        if (frameDelay_ > desc.frameDelay) {
            // read frame
            if (!fliPlayer_.decodeFrame()) {
                // Frame is not good -> quit
                menu_manager_->gotoMenu(nextMenu_);
                return;
            }

            fliPlayer_.copyCurrentFrameToScreen();
            // Add a dirty rect just to start the render routine
            addDirtyRect(0, 0, 1, 1);
            // Reset delay between frames
            frameDelay_ = 0;

            // handle events
            for (uint16 i = 0; desc.evtList[i].frame != (uint16)-1; i++) {
                if (desc.evtList[i].frame > frameIndex_)
                    break;
                else if (desc.evtList[i].frame == frameIndex_) {
                    // Play music
                    if (desc.evtList[i].music != msc::NO_TRACK) {
                        g_App.music().playTrack(desc.evtList[i].music);
                    }
                    // Play sound
                    if (desc.evtList[i].sound != snd::NO_SOUND) {
                        g_App.gameSounds().play(desc.evtList[i].sound, desc.evtList[i].sndChan);
                    }
                    // Draw subtitle
                    if (desc.evtList[i].subtitle != NULL) {
                        getMessage(desc.evtList[i].subtitle, currSubTitle_);
                    } else {
                        currSubTitle_ = "";
                    }
                }
            }
            frameIndex_++;
        }

        playingFli_ = true;
    } else if (playingFli_ ) {
        // A fli was being played but it has ended
        // so if user doesn't have to click end animation
        // trying to load next fli.
        playingFli_ = false;
        FliDesc desc = fliList_.at(fliIndex_ - 1);
        if (!desc.waitKeyPressed && !loadNextFli()) {
            // no more animation so quit
            menu_manager_->gotoMenu(nextMenu_);
        }
    }
}

void FliMenu::handleRender(DirtyList &dirtyList)
{
    if (currSubTitle_.size() != 0) {
        menu_manager_->fonts().introFont()->drawText(10, 360, currSubTitle_.c_str(), false);;
    }
}

/*!
 * Ends the animation.
 */
bool FliMenu::handleMouseDown(int x, int y, int button, const int modKeys)
{
    if (fliIndex_ > 0) {
        FliDesc desc = fliList_.at(fliIndex_ - 1);
        if ((playingFli_ && desc.skipable) || (!playingFli_ && desc.waitKeyPressed)) {
            // Fli can be ended by pressing a key
            if (!loadNextFli()) {
                // no fli next so quit
                menu_manager_->gotoMenu(nextMenu_);
            }
        }
    }

    return true;
}

/*!
 * Ends the animation.
 */
bool FliMenu::handleUnknownKey(Key key, const int modKeys) {
    if (fliIndex_ > 0) {
        FliDesc desc = fliList_.at(fliIndex_ - 1);
        if ((playingFli_ && desc.skipable) || (!playingFli_ && desc.waitKeyPressed)) {
            // Fli can be ended by pressing a key
            if (!loadNextFli()) {
                // no fli next so quit
                menu_manager_->gotoMenu(nextMenu_);
            }
        }
    }

    return true;
}
    
void FliMenu::handleLeave() 
{
    if (pData_) {
        delete[] pData_;
        pData_ = NULL;
    }

    fliList_.clear();
    fliIndex_ = 0;
}


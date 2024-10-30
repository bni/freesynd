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

#include "fs-engine/menus/flimenu.h"

#include "fs-utils/io/file.h"
#include "fs-engine/gfx/screen.h"
#include "fs-engine/sound/musicmanager.h"


/*!
 * Constructor
 * @param m 
 * @param menuId 
 * @param parentId 
 */
FliMenu::FliMenu(MenuManager *m, int menuId, int parentId) : Menu(m, menuId, parentId), fliPlayer_(m)
{
    fliIndex_ = 0;
    pData_ = NULL;
    playingFli_ = false;
    isCachable_ = (menuId == Menu::kMenuIdFliTransition);
    currSubTitle_ = "";
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
 * Remove all entries in the FliDesc list.
 */
void FliMenu::clearFliDescList() {
    fliList_.clear();
}

/*!
 * Loads the next animation if there is one still in the list.
 * \return True if an animation has been loaded.
 */
bool FliMenu::loadNextFli() {
    // Stop all music if one was being played
    g_MusicMgr.stopPlayback();
    playingFli_ = false;
    // loads Fli
    if ( fliIndex_ < fliList_.size()) {
        size_t size = 0;

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

void FliMenu::handleTick(uint32_t elapsed)
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
                        g_MusicMgr.playTrack(desc.evtList[i].music);
                    }
                    // Play sound
                    if (desc.evtList[i].sound != NO_SOUND) {
                        g_SoundMgr.play(desc.evtList[i].sound, desc.evtList[i].sndChan);
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
        menu_manager_->fonts().introFont()->drawText(10, 360, currSubTitle_, true);
    }
}

/*!
 * Ends the animation.
 */
bool FliMenu::handleMouseDown(Point2D point, int button)
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
bool FliMenu::handleUnMappedKey(const FS_Key key) {
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


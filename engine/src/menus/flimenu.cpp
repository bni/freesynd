/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2011, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-engine/menus/flimenu.h"

#include "fs-engine/sound/musicmanager.h"
#include "fs-engine/sound/soundmanager.h"

namespace fs_eng {

/*!
 * Constructor
 * @param m 
 * @param menuId 
 * @param parentId 
 */
FliMenu::FliMenu(MenuManager *m, int menuId, int parentId) : Menu(m, menuId, parentId), fliPlayer_(), frameTimer_(0)
{
    fliIndex_ = 0;
    playingFli_ = false;
    isCachable_ = (menuId == Menu::kMenuIdFliTransition);
    currSubTitle_ = "";
    isFrameLoaded_ = false;
}

FliMenu::~FliMenu()
{}

/*!
 * Adds a new description for the FLI animation.
 * \param anim animation file name.
 * \param frameDelay animation speed.
 * \param waitKey True to wait for the user input
 * \param skipable True means user can skip animation
 * \param usePalette True will reload the sprite texture for intro font with the palette in the fli
 * \param events The events to handle during the animation play
 * \param type Which bank of sound to use
 */
void FliMenu::addFliDesc(const char *anim,
                            uint32_t frameDelay,
                            bool waitKey,
                            bool skipable,
                            bool usePalette,
                            const FrameEvent *events,
                            SampleType type,
                            MusicManager::MusicSong song) {
    FliDesc desc;
    desc.type = kSampleGame;
    desc.name = anim;
    desc.frameDelay = frameDelay;
    desc.waitKeyPressed = waitKey;
    desc.skipable = skipable;
    desc.usePaletteForFont = usePalette;
    desc.evtList = events;
    desc.type = type;
    desc.song = song;

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
    g_MusicMgr.stopCurrentSong();
    
    playingFli_ = false;
    isFrameLoaded_ = false;
    // loads Fli
    if ( fliIndex_ < fliList_.size()) {
        // Gets the fli description
        FliDesc desc = fliList_.at(fliIndex_);
        // Loads data from file
        if (fliPlayer_.loadFliData(desc.name)) {
            if (fliPlayer_.hasFrames()) {
                // init frame delay counter with max value so first frame is
                // drawn in the first pass
                frameTimer_.reset(desc.frameDelay);
                frameTimer_.setToMax();
                currSubTitle_.erase();
                fliIndex_++;
                return true;
            }
        }
    }

    // No animation was loaded
    return false;
}

bool FliMenu::handleBeforeShow()
{
    // Loads the first Fli
    if (!fliList_.empty()) {
        // There's supposed to be a fli to play
        // so if loadNextFli return false, it's an error
        return loadNextFli();
    }

    return true;
}

bool FliMenu::handleTick(uint32_t elapsed)
{
    if (fliPlayer_.hasFrames()) {
        FliDesc desc = fliList_.at(fliIndex_ - 1);
        // There is a frame to display
        if (frameTimer_.update(elapsed)) {
            int nbColor = 0;
            int frameIndex;
            // time to change frame -> read frame
            if ((frameIndex = fliPlayer_.decodeFrame(nbColor)) == -1) {
                // Frame is not good -> quit
                menu_manager_->gotoMenu(nextMenu_);
                return true;
            }
            isFrameLoaded_ = true;

            // number of color is different from zero
            // so that means we load a palette
            if (nbColor != 0 && desc.usePaletteForFont) {
                menu_manager_->fonts().introFont()->setPalette(fliPlayer_.getPalette());
            }

            // If we're at the first frame, then play music if defined
            if (frameIndex == 1 && desc.song != MusicManager::kMusicSongNoSong) {
                g_MusicMgr.playSong(desc.song);
            }

            // handle events
            for (uint16_t i = 0; desc.evtList[i].frame != -1; i++) {
                if (desc.evtList[i].frame > frameIndex)
                    break;
                else if (desc.evtList[i].frame == frameIndex) {
                    // Play sound
                    if (desc.evtList[i].sound != kNoSound) {
                        if (desc.type == kSampleGame) {
                            g_SoundMgr.play(desc.evtList[i].sound, desc.evtList[i].sndChan);
                        } else {
                            g_SoundMgr.playIntro(desc.evtList[i].sound, desc.evtList[i].sndChan);
                        }
                    }
                    // Draw subtitle
                    if (desc.evtList[i].subtitle != NULL) {
                        if (strlen(desc.evtList[i].subtitle)) {
                            getMessage(desc.evtList[i].subtitle, currSubTitle_);
                        } else {
                            currSubTitle_ = "";
                        }   
                    }
                }
            }
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

    return true;
}

void FliMenu::handleRender()
{
    if (isFrameLoaded_) {
        fliPlayer_.renderFrame();
    }

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
            // Stop all sounds only for skipable FLIs
            g_SoundMgr.stop();
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
bool FliMenu::handleUnMappedKey([[maybe_unused]] const FS_Key key) {
    if (fliIndex_ > 0) {
        FliDesc desc = fliList_.at(fliIndex_ - 1);
        if ((playingFli_ && desc.skipable) || (!playingFli_ && desc.waitKeyPressed)) {
            // Stop all sounds only for skipable FLIs
            g_SoundMgr.stop();
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
    // If this is the animation between two screen
    // we save the last frame that will serve as the background 
    // for the menu
    if (id_ == Menu::kMenuIdFliTransition) {
        menu_manager_->preSaveBackground();
        fliPlayer_.renderFrame();
        menu_manager_->postSaveBackground();
    }

    fliPlayer_.resetPlayer();

    fliList_.clear();
    fliIndex_ = 0;
}

}

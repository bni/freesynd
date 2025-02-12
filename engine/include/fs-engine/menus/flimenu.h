/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2011, 2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef FLIMENU_H
#define FLIMENU_H

#include "fs-engine/menus/menumanager.h"
#include "fs-engine/menus/fliplayer.h"
#include "fs-engine/sound/sound.h"
#include "fs-engine/sound/musicmanager.h"
#include "fs-utils/misc/timer.h"

namespace fs_eng {

struct FrameEvent {
    /*! frame Id.*/
    int frame;
    /*! sound to play.*/
    size_t    sound;
    //! channel for the sound
    uint8_t    sndChan;
    /*! Operation on subtitle. If NULL, do nothing.*/
    const char* subtitle;
};

/*!
 * A description of the fli animation to be played.
 */
struct FliDesc {
    //! Is it a sound for intro or
    SampleType type;
    /*! Name of the file containing the animation.*/
    std::string name;
    /*! Speed of animation : delay between 2 frames.*/
    uint32_t frameDelay;
    /*! True means user has to press key or mouse to go to
     * next animation or next menu after the end of animation.*/
    bool waitKeyPressed;
    /*! True means animation can be skipped by pressing key or mouse.*/
    bool skipable;
    //! True means the intro font uses the palette from FLI
    bool usePaletteForFont;
    const FrameEvent *evtList;
    //! Song to play during the fli animation
    MusicManager::MusicSong song;
};

/*!
 * A Flimenu can display a set of fli animations before redirecting towards another menu.
 */
class FliMenu : public Menu {
public:
    FliMenu(MenuManager *m, int menuId, int parentId);
    ~FliMenu();

    //! Set the menu to be display at the end of the last animation
    void setNextMenu(int menuId) { nextMenu_ = menuId; }

    bool handleTick(uint32_t elapsed) override;
    bool handleBeforeShow() override;

    void handleLeave() override;

    //! Append a animation to play with its caracteristics
    void addFliDesc(const char *anim, 
                    uint32_t frameDelay,
                    bool waitKey,
                    bool skipable,
                    bool usePalette,
                    const FrameEvent *events,
                    SampleType type=kSampleGame,
                    MusicManager::MusicSong song = MusicManager::kMusicSongNoSong);
    //! Remove all FliDesc
    void clearFliDescList();

protected:
    void handleRender() override;

    bool handleMouseDown(Point2D point, int button) override;

    bool handleUnMappedKey(const FS_Key key) override;

    bool loadNextFli();

protected:
    /*! The Fli player.*/
    FliPlayer fliPlayer_;
    /*! The list of animation to play.*/
    std::vector<FliDesc> fliList_;
    /*! Index of the next animation to play.*/
    size_t fliIndex_;
    /*! Timer to control animation speed.*/
    fs_utl::Timer frameTimer_;
    /*! A flag telling if an animation is being played.*/
    bool playingFli_;
    /*! Id of the menu to go after all animations have been played.*/
    int nextMenu_;
    /*! Content of a subtitle to draw during animation. Used in intro.*/
    std::string currSubTitle_;
    bool isFrameLoaded_;
};

}

#endif

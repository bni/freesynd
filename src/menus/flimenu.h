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

#ifndef FLIMENU_H
#define FLIMENU_H

#include "menus/menumanager.h"
#include "gfx/fliplayer.h"
#include "sound/sound.h"
#include "sound/music.h"

struct FrameEvent {
    /*! frame Id.*/
    uint16 frame;
    /*! start music.*/
    msc::MusicTrack    music;
    /*! Game sound to play.*/
    snd::InGameSample    sound;
    uint8    sndChan;
    /*! Operation on subtitle.*/
    const char* subtitle;
};

/*!
 * A description of the fli animation to be played.
 */
struct FliDesc {
    /*! Name of the file containing the animation.*/
    std::string name;
    /*! Speed of animation : delay between 2 frames.*/
    uint8 frameDelay;
    /*! True means user has to press key or mouse to go to 
     * next animation or next menu after the end of animation.*/
    bool waitKeyPressed;
    /*! True means animation can be skipped by pressing key or mouse.*/
    bool skipable;
    const FrameEvent *evtList;
};

/*!
 * A Flimenu can display a set of fli animations before redirecting towards another menu.
 */
class FliMenu : public Menu {
public:
    FliMenu(MenuManager *m, int menuId);
    ~FliMenu();

    void handleTick(int elapsed);
    void handleShow();
    
    void handleLeave();
    
protected:
    void handleRender(DirtyList &dirtyList);

    bool handleMouseDown(int x, int y, int button, const int modKeys);

    bool handleUnknownKey(Key key, const int modKeys);

    bool loadNextFli();

    void addFliDesc(const char *anim, uint8 frameDelay, bool waitKey, bool skipable, const FrameEvent *events);

protected:
    /*! The Fli player.*/
    FliPlayer fliPlayer_;
    /*! The list of animation to play.*/
    std::vector<FliDesc> fliList_;
    /*! Index of the next animation to play.*/
    size_t fliIndex_;
    uint16 frameIndex_;
    /*! Timer to control animation speed.*/
    int frameDelay_;
    /*! Content of current animation.*/
    uint8 *pData_;
    /*! A flag telling if an animation is being played.*/
    bool playingFli_;
    /*! Id of the menu to go after all animations have been played.*/
    int nextMenu_;
    /*! Content of a subtitle to draw during animation. Used in intro.*/
    std::string currSubTitle_;
};

#endif

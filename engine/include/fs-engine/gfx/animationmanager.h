/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2024  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef GFX_ANIMATIONMANAGER_H
#define GFX_ANIMATIONMANAGER_H

#include "fs-engine/gfx/spritemanager.h"

/*!
 * Game sprite frame class.
 */
class GameSpriteFrame {
public:
    GameSpriteFrame() : first_element_(0), width_(0), height_(0), flags_(0),
            next_frame_(0) {}
    size_t first_element_;
    int width_, height_;
    int flags_;
    size_t next_frame_;
};

/*!
 * Game sprite frame element class.
 */
class GameSpriteFrameElement {
public:
    GameSpriteFrameElement() : sprite_(0), off_x_(0), off_y_(0),
            flipped_(false), next_element_(0) {}
    int sprite_;
    int off_x_, off_y_;
    bool flipped_;
    size_t next_element_;
};

/*!
 * Game sprite class.
 */
class GameSpriteManager : public SpriteManager, public Singleton < GameSpriteManager > {
public:
    GameSpriteManager();
    virtual ~GameSpriteManager();

    //! Loads the sprites from original files
    bool load();

    int numAnims() { return (int) index_.size(); }

    bool drawFrame(unsigned int animNum, int frameNum, const Point2D &screenPos);
    bool lastFrame(unsigned int animNum, int frameNum);
    int lastFrame(unsigned int animNum);
    int getFrameFromFrameIndx(unsigned int frameIndx);
    int getFrameNum(unsigned int animNum);

protected:
    bool loadElementsFromCustomFiles();
    bool loadElementsFromOriginalFiles();
protected:
    std::vector<size_t> index_;
    std::vector<GameSpriteFrame> frames_;
    std::vector<GameSpriteFrameElement> elements_;
};

#define g_SpriteMgr   GameSpriteManager::singleton()

#endif //GFX_ANIMATIONMANAGER_H
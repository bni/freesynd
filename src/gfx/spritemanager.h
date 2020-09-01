/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H

#include "sprite.h"
#include <vector>

/*!
 * Sprite manager class.
 */
class SpriteManager {
public:
    SpriteManager();
    virtual ~SpriteManager();

    void clear();

    bool loaded() { return sprite_count_ != 0; }
    int spriteCount() { return sprite_count_; }

    bool loadSprites(uint8 * tabData, int tabSize, uint8 *spriteData,
            bool rle = false);
    Sprite *sprite(int spriteNum);
    bool drawSpriteXYZ(int spriteNum, int x, int y, int z, bool flipped = false,
            bool x2 = false);

protected:
    Sprite *sprites_;
    int sprite_count_;
};

/*!
 * Game sprite frame class.
 */
class GameSpriteFrame {
public:
    GameSpriteFrame() : first_element_(0), width_(0), height_(0), flags_(0),
            next_frame_(0) {}
    int first_element_;
    int width_, height_;
    int flags_;
    int next_frame_;
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
    int next_element_;
};

/*!
 * Game sprite class.
 */
class GameSpriteManager : public SpriteManager {
public:
    GameSpriteManager();
    virtual ~GameSpriteManager();

    void load();

    int numAnims() { return (int) index_.size(); }

    bool drawFrame(int animNum, int frameNum, int x, int y);
    bool lastFrame(int animNum, int frameNum);
    int lastFrame(int animNum);
    int getFrameFromFrameIndx(int frameIndx);
    int getFrameNum(int animNum);

protected:
    std::vector<int> index_;
    std::vector<GameSpriteFrame> frames_;
    std::vector<GameSpriteFrameElement> elements_;
};

#endif

/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef GFX_ANIMATIONMANAGER_H
#define GFX_ANIMATIONMANAGER_H

#include "fs-engine/gfx/spritemanager.h"

namespace fs_eng {
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
class AnimationManager : public Singleton < AnimationManager > {
public:
    AnimationManager();
    virtual ~AnimationManager();

    //! Loads the sprites from original files
    void load();

    bool setPalette(const fs_eng::Palette &missionPalette);

    int numAnims() { return (int) index_.size(); }
    //! Draw a single sprite without animation
    void drawSprite(int spriteId, const Point2D &screenPos);
    //! Draw a frame for a given animation
    bool drawFrame(uint32_t animId, int frameId, const Point2D &screenPos);
    //! Return true if the frameId is the last frame for given animation
    bool lastFrame(uint32_t animId, int frameId);
    //! Return the id of the last frame of given animation
    int lastFrame(uint32_t animId);
    int getFrameFromFrameIndx(unsigned int frameIndx);
    int getFrameNum(uint32_t animNum);

protected:
    bool loadElementsFromCustomFiles();
    void loadElementsFromOriginalFiles();
protected:
    //! Size for the texture for storing animation sprites
    static const size_t kAnimSpritesTextureWidth;

    SpriteManager spritesManager_;
    std::vector<size_t> index_;
    std::vector<GameSpriteFrame> frames_;
    std::vector<GameSpriteFrameElement> elements_;
};

}

#define g_SpriteMgr   fs_eng::AnimationManager::singleton()

#endif //GFX_ANIMATIONMANAGER_H
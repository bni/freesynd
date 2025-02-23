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
 * An animation is composed of several frames called GameSpriteFrame.
 * Each frame has a widht and height and points to the next frame.
 * A frame is composed of several elements called GameSpriteElement.
 */
struct GameSpriteFrame {
    GameSpriteFrame() : firstElement(0), width(0), height(0), flags(0),
            nextFrame(0) {}
    //! The first element that composes the frame
    size_t firstElement;
    //! Width of the frame
    int width;
    //! Height of the frame
    int height;
    //! Seems to be 0x0100 when it is the first frame of an animation.
    int flags;
    // Index of the next frame in the animation. The last frame points to the first frame.
    size_t nextFrame;
};

/*!
 * Each frame is composed of a set of GameSpriteFrameElement.
 */
struct GameSpriteFrameElement {
    GameSpriteFrameElement() : sprite(0), off_x(0), off_y(0),
            flipped(false), nextElement(0) {}
    //! An element is a sprite
    int sprite;
    //! An offset to use when drawing the sprite on screen
    int off_x;
    //! An offset to use when drawing the sprite on screen
    int off_y;
    //! True means the element is flipped when drawn
    bool flipped;
    //! Index of the next element
    size_t nextElement;
};

/*!
 * AnimationManager stores all animations read from files.
 * Files formats are given in libsyndicate document.
 */
class AnimationManager : public Singleton < AnimationManager > {
public:
    AnimationManager();
    virtual ~AnimationManager();

    //! Adds a new element to the list
    void addFrameElement(GameSpriteFrameElement element);
    //! Adds a new frame to the list
    void addFrame(GameSpriteFrame frame);
    //! Adds a new animation pointing to a frame
    void addAnimation(uint16_t firstFrameIndex);
    //! Check that resources have been loaded well
    void checkLoadIsOk();

    //! Sets the palette used for a mission
    bool setPalette(const fs_eng::Palette &missionPalette);
    //! Returns the number of loaded animations
    size_t getNumAnims() { return firstFramePerAnim_.size(); }
    //! Draw a single sprite without animation
    void drawSprite(int spriteId, const Point2D &screenPos);
    //! Draw a frame for a given animation
    bool drawFrame(uint16_t animId, int frameId, const Point2D &screenPos);
    //! Return true if the frameId is the last frame for given animation
    bool lastFrame(uint16_t animId, int frameId);
    //! Return the id of the last frame of given animation
    uint8_t lastFrame(uint16_t animId);
    int getFrameFromFrameIndx(uint16_t frameIndx);

protected:
    //! Size for the texture for storing animation sprites
    static const size_t kAnimSpritesTextureWidth;

    SpriteManager spritesManager_;
    /*!
     * Each animation id is an entry in the vector and
     * the corresponding element is the index of the first frame
     * store in frames_ vector.
     */
    std::vector<uint16_t> firstFramePerAnim_;
    //! A list of all the frames
    std::vector<GameSpriteFrame> frames_;
    //! A list of all elements
    std::vector<GameSpriteFrameElement> elements_;
};

}

#define g_SpriteMgr   fs_eng::AnimationManager::singleton()
#define g_AnimMgr   fs_eng::AnimationManager::singleton()

#endif //GFX_ANIMATIONMANAGER_H
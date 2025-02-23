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

#include "fs-engine/gfx/animationmanager.h"

#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "fs-engine/enginecommon.h"

namespace fs_eng {

const size_t AnimationManager::kAnimSpritesTextureWidth = 1024;

AnimationManager::AnimationManager() : spritesManager_(false, kAnimSpritesTextureWidth)
{
}

AnimationManager::~AnimationManager()
{
}

/**
 * Loads game sprites.
 * \return bool return true if everything is ok.
 *
 */
void AnimationManager::checkLoadIsOk() {
    if (!spritesManager_.loaded()) {
        LOG(Log::k_FLG_GFX, "AnimationManager", "load", ("Loading game sprites ..."))
        fs_eng::Palette emptyPalette;
        spritesManager_.loadSprites("hspr-0.tab", "hspr-0.dat", emptyPalette);
    }

    // Check consistency of resources
    for (auto &element : elements_) {
        if (element.next_element_ >= elements_.size()) {
            throw InitializationFailedException(
                "GameSpriteFrameElement references an element out of limits");
        }
    }

    for (auto &frame : frames_) {
        if (frame.next_frame_ >= frames_.size()) {
            throw InitializationFailedException(
                "GameSpriteFrame references a frame out of limits");
        }

        if (frame.first_element_ >= elements_.size()) {
            throw InitializationFailedException(
                "GameSpriteFrame references an element out of limits");
        }
    }

    for (auto &index : index_) {
        if (index >= frames_.size()) {
            throw InitializationFailedException(
                "Animation references a frame out of limits");
        }
    }

    LOG(Log::k_FLG_SND, "AnimationManager", "load", ("loaded %i frame elements", elements_.size()))
    LOG(Log::k_FLG_SND, "AnimationManager", "load", ("loaded %i frames", frames_.size()))
    LOG(Log::k_FLG_SND, "AnimationManager", "load", ("index contains %i animations", index_.size()))
}

void AnimationManager::addFrameElement(GameSpriteFrameElement element) {
    elements_.push_back(element);
}

/*!
 * All elements must have been added first
 * @param frame GameSpriteFrame to add
 */
void AnimationManager::addFrame(GameSpriteFrame frame) {
    assert(frame.first_element_ < elements_.size());
    frames_.push_back(frame);
}
void AnimationManager::addAnimation(size_t index) {
    assert(index < frames_.size());
    index_.push_back(index);
}

bool AnimationManager::setPalette(const fs_eng::Palette &missionPalette) {
    return spritesManager_.setPalette(missionPalette);
}

/*!
 * Convienient method to draw a single sprite 
 * @param spriteId Id of the sprite
 * @param screenPos Position on the screen
 */
void AnimationManager::drawSprite(int spriteId, const Point2D &screenPos) {
    spritesManager_.drawSprite(spriteId, screenPos.x, screenPos.y);
}

/*!
 * Draw a given frame that's within the given animation at given screen position.
 * @param animId The animation id
 * @param frameId The frame id in the animation 
 * @param screenPos The position on the screen
 * @return 
 */
bool AnimationManager::drawFrame(uint16_t animId, int frameId, const Point2D &screenPos)
{
    assert(animId < index_.size());

    GameSpriteFrame *f = &frames_[index_[animId]];
    if (f == NULL)
        return false;

    while (frameId) {
        f = &frames_[f->next_frame_];
        frameId--;
    }

    GameSpriteFrameElement *e = &elements_[f->first_element_];
    while (1) {
        spritesManager_.drawSprite(e->sprite_, screenPos.x + e->off_x_, screenPos.y + e->off_y_, e->flipped_);
        if (e->next_element_ == 0)
            break;
        e = &elements_[e->next_element_];
    }

    return f->next_frame_ == index_[animId];
}

/*!
 * Tells if the frame is the last in the given animation
 * @param animId The animation id
 * @param frameId The id of the frame
 * @return True if the frame is the last one.
 */
bool AnimationManager::lastFrame(uint16_t animId, int frameId)
{
    assert(animId < index_.size());

    GameSpriteFrame *f = &frames_[index_[animId]];
    while (frameId) {
        f = &frames_[f->next_frame_];
        frameId--;
    }

    return f->next_frame_ == index_[animId];
}

int AnimationManager::lastFrame(uint16_t animId)
{
    int frameId = 0;
    assert(animId < index_.size());

    GameSpriteFrame *f = &frames_[index_[animId]];
    while (f->next_frame_ != index_[animId]) {
        f = &frames_[f->next_frame_];
        frameId++;
    }
    return frameId;
}

int AnimationManager::getFrameFromFrameIndx(uint16_t frameIndx)
{
    int frameId = 0;

    GameSpriteFrame *f = &frames_[frameIndx];
    while (1) {
        f = &frames_[f->next_frame_];
        if (f->flags_ == 0x0100)
            break;
    }

    while (1) {
        f = &frames_[f->next_frame_];
        if (f->next_frame_ == frameIndx)
            return frameId;
        frameId++;
    }
}

/*!
 * @brief 
 * @param animId 
 * @return 
 */
int AnimationManager::getFrameNum(uint16_t animId)
{
    assert(animId < index_.size());
    int frameId = 1;

    GameSpriteFrame *f = &frames_[index_[animId]];
    bool passedStart = false;
    while (1) {
        if (f->flags_ == 0x0100) {
            if (passedStart)
                return frameId;
            else
                passedStart = true;
        }
        f = &frames_[f->next_frame_];
        frameId++;
    }
}

}

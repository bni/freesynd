/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
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

#include "fs-engine/gfx/animationplayer.h"

#include "fs-engine/gfx/animationmanager.h"

namespace fs_eng {

AnimationPlayer::AnimationPlayer() : spentTime_(0) {
    // add a default animation to say
    addAnimation({
        .spriteAnimationBase = 0,
        .framePerSec = 0,
        .mode = kAnimationModeNoAnimation,
        .maxPlayTime = 0
    });
    reset();
}

AnimationPlayer::~AnimationPlayer() {}

/*!
 * @brief 
 * @param animation 
 */
void AnimationPlayer::addAnimation(MapObjectAnimation animation) {
    animations_.push_back(animation);
}

/*!
 * Play a given animation. By defaut animation starts at frame 0,
 * but we can specify the starting frame.
 * @param mapObjectAnimationId Id of registered animation
 * @param startFrame Starting frame
 * @return True if animation of given id has been found
 */
bool AnimationPlayer::play(const uint16_t mapObjectAnimationId, const uint8_t startFrame) {
    if (mapObjectAnimationId < animations_.size()) {
        currentAnimation_ = animations_[mapObjectAnimationId];
        frame_ = startFrame;
        lastFrame_ = g_AnimMgr.lastFrame(currentAnimation_.spriteAnimationBase);
        isPlaying_ = true;
        if (currentAnimation_.maxPlayTime) {
            spentTime_.reset(currentAnimation_.maxPlayTime);
        }
        return true;
    } else {
        // We use the default animation
        reset();
        return false;
    }
}

/*!
 * Updates the animation
 * @param elapsed Time in milliseconds since the last execution
 * @return True if current animation has finished
 */
bool AnimationPlayer::handleTick(uint32_t elapsed) {
    if (isPlaying_) {
        bool maxTimeReached = (currentAnimation_.maxPlayTime != 0) ?
                                spentTime_.update(elapsed) : false;
        // This is the time to display one frame in milliseconds
        int frameTics = 1000 / currentAnimation_.framePerSec;
        // Adding carry and elapsed to get total elapsed time
        int totalElapsed = elapsed + elapsedCarry_;
        // Get new value for elapsedCarry to store remaining time to 
        // get to next frame.
        elapsedCarry_ = totalElapsed % frameTics;
        
        // La valeur calculée est ajoutée à la frame actuelle.
        // Compute the new frame based on the number of periods
        // of frameTics contained in totalElapsed
        // Number of frames is added to current frame rank
        frame_ += (totalElapsed / frameTics);

        // Check if we reached end of animation
        if (frame_ > lastFrame_ || maxTimeReached) {
            if (currentAnimation_.mode == kAnimationModeLoop) {
                if (maxTimeReached) {
                    reset();
                    return true;
                } else {
                    // just looping through animation
                    frame_ %= (lastFrame_ + 1u);
                }
            } else if (currentAnimation_.mode == kAnimationModeSingle) {
                reset();
                return true;
            } else if (currentAnimation_.mode == kAnimationModeSingleNoEnd) {
                // do nothing
                frame_ = lastFrame_;
                isPlaying_ = false;
            }
        }
    }
    // We have not reached the end of animation
    return false;
}

void AnimationPlayer::reset() {
    lastFrame_ = 0;
    frame_ = 0;
    elapsedCarry_ = 0;
    currentAnimation_ = animations_[0];
    isPlaying_ = false;
}
}
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

#ifndef GFX_ANIMATIONPLAYER_H
#define GFX_ANIMATIONPLAYER_H

#include <vector>

#include "fs-utils/common.h"
#include "fs-utils/misc/timer.h"
#include "fs-engine/enginecommon.h"

namespace fs_eng {

/*!
 * Tells how to manage the end of the animation.
 */
enum AnimationMode {
    //! Animation is ended when the last frame is played
    kAnimationModeSingle,
    //! Animation loops to first frame
    kAnimationModeLoop,
};

/*!
 * A MapObjectAnimation describes how to play an animation.
 * It can be a single execution or a loop. It also specifies
 * the speed to play animation and if the animation has a 
 * maximum duration no matter the mode used.
 * A MapObjectAnimation points to an animation store in AnimationManager.
 * This can be a base animation and subclasses can change this if there
 * are animations that change based on some attributes (direction or weapon)
 */
struct MapObjectAnimation {
    /*!
     * The index of an animation in AnimationManager.
     */
    uint16_t framesAnimation;
    //! set the animation speed
    uint8_t framePerSec;
    //! How to play the animation
    AnimationMode mode;
    /*!
     * When non zero, give the maximum an animation 
     * can be played in any given mode. Expressed in milliseconds.
     */
    uint32_t maxPlayTime;
};

/*!
 * The AnimationPlayer is in charge of storing all MapObjectAnimation
 * used for a object and playing the current animation.
 */
class AnimationPlayer {
public:
    AnimationPlayer();
    virtual ~AnimationPlayer();

    //! Adds a animation description in the player
    uint16_t addAnimation(MapObjectAnimation animation);
    uint16_t addAnimation(uint16_t spriteAnimationBase, 
                        AnimationMode mode = kAnimationModeSingle,
                        uint8_t framePerSec = 8,
                        uint32_t maxPlayTime = 0 );
    //! Plays a given animation
    bool play(uint16_t mapObjectAnimationId, uint8_t startFrame = 0, uint32_t newMaxPlayTime = 0);
    //! Updates the current animation
    bool handleTick(uint32_t elapsed);

    /*!
     * Return true if the current animation id is the same as given in param
     * @param mapObjectAnimationId id to test
     * @return true if it's the same
     */
    bool isCurrentAnimation(const uint16_t mapObjectAnimationId) {
        return mapObjectAnimationId == currentAnimationId_;
    }

    uint16_t currentAnimationId() { return currentAnimationId_; }

    // Draw the current animation at given position
    void draw(const Point2D &screenPos, uint16_t animOffset);

    //! Keep current animation loaded but resets it to first frame and stops playing
    void resetAnimation();

protected:
    //! loads the animation to play. Subclass can alter the loaded animation
    virtual void loadAnimation(const uint16_t mapObjectAnimationId, uint32_t newMaxPlayTime);
    //! Reset the player
    void reset();
protected:
    //! List of registered animations
    std::vector<MapObjectAnimation> animations_;
    //! Flag to track is animation should be updated
    bool isPlaying_;
    //! Save the current animation id
    uint16_t currentAnimationId_;
    //! The current animation being played
    MapObjectAnimation currentAnimation_;
    //! The current frame. It's an increment from zero (not an id)
    uint8_t frame_;
    /*! 
     * The order of the last frame
     */
    uint8_t lastFrame_;
    
    //! Contains the non used remaining time from the previous iteration.
    uint32_t elapsedCarry_;
    /*!
     * When MapObjectAnimation::maxPlayTime is set, this field
     * is used to count time used to display the animation.
     * When max time is reached, animation is stopped.
     */
    fs_utl::Timer spentTime_;
};

}
#endif // GFX_ANIMATIONPLAYER_H
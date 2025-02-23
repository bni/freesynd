/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2013, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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
#include <catch2/catch_test_macros.hpp>

#include "fs-engine/gfx/animationplayer.h"
#include "fs-engine/gfx/animationmanager.h"

class TestAnimationPlayer : public fs_eng::AnimationPlayer {
public:
    TestAnimationPlayer() : AnimationPlayer() {}
    ~TestAnimationPlayer() {}

    size_t getNumberOfAnimations() { return animations_.size(); }
    uint16_t getSpriteAnimationId() { return currentAnimation_.spriteAnimationBase; }
    uint16_t getCurrentFrameId() { return frame_; }
    bool isCurrentAnimationSetToNoAnimation() { 
        return currentAnimation_.mode == fs_eng::kAnimationModeNoAnimation;
    }
protected:
    void loadAnimation(const uint16_t mapObjectAnimationId) override {
        AnimationPlayer::loadAnimation(mapObjectAnimationId);
        if (mapObjectAnimationId == 2) {
            currentAnimation_.spriteAnimationBase += 1;
        }
    }
};

void addFrameElement(fs_eng::AnimationManager &animMgr, int sprite, int next) {
    fs_eng::GameSpriteFrameElement element;
    element.sprite = sprite;
    element.nextElement = next;
    animMgr.addFrameElement(element);
}

void addFrame(fs_eng::AnimationManager &animMgr, bool isFirst, size_t nextFrame) {
    fs_eng::GameSpriteFrame frame;
    frame.flags = isFirst ? 0x0100 : 0;
    frame.firstElement = 0;
    frame.nextFrame = nextFrame;
    animMgr.addFrame(frame);
}

void initAnimationManager(fs_eng::AnimationManager &animMgr) {
    addFrameElement(animMgr, 0, 0);
    // Add frames
    addFrame(animMgr, true, 0);  // First frame is pointing to itself
    addFrame(animMgr, true, 2);  // Frame 1 -> anim 1
    addFrame(animMgr, false, 3); // Frame 2
    addFrame(animMgr, false, 1); // Frame 3
    addFrame(animMgr, true, 5);  // Frame 4 -> anim 2
    addFrame(animMgr, false, 4); // Frame 5
    addFrame(animMgr, true, 7);  // Frame 6 -> anim 3
    addFrame(animMgr, false, 6); // Frame 7
    addFrame(animMgr, true, 9);  // Frame 8 -> anim 4
    addFrame(animMgr, false, 10);// Frame 9
    addFrame(animMgr, false, 8); // Frame 10
    addFrame(animMgr, true, 12);  // Frame 11 -> anim 5
    addFrame(animMgr, false, 13);// Frame 12
    addFrame(animMgr, false, 11); // Frame 13
    // Add animations
    animMgr.addAnimation(0);  // First animation does serve
    animMgr.addAnimation(1);  // Anim 1
    animMgr.addAnimation(4);  // Anim 2
    animMgr.addAnimation(6);  // Anim 3
    animMgr.addAnimation(8);  // Anim 4
    animMgr.addAnimation(11);  // Anim 5
}

void initAnimationPlayer(fs_eng::AnimationPlayer &animPlayer) {
    animPlayer.addAnimation(
        { 
            .spriteAnimationBase = 1,
            .framePerSec = 3,
            .mode = fs_eng::kAnimationModeSingle
        });

    animPlayer.addAnimation(
        {
            .spriteAnimationBase = 4,
            .framePerSec = 3,
            .mode = fs_eng::kAnimationModeSingleNoEnd
        });

    animPlayer.addAnimation(
        {
        .spriteAnimationBase = 3,
        .framePerSec = 5,
        .mode = fs_eng::kAnimationModeLoop
    });

    animPlayer.addAnimation(
        {
            .spriteAnimationBase = 2,
            .framePerSec = 5,
            .mode = fs_eng::kAnimationModeLoop,
            .maxPlayTime = 500
    });
}

TEST_CASE( "AnimationPlayer", "[engine][animationplayer]" ) {
    SECTION( "Should play animation") {
        TestAnimationPlayer cut;
        fs_eng::AnimationManager animMgr;

        initAnimationManager(animMgr);
        initAnimationPlayer(cut);

        REQUIRE( cut.getNumberOfAnimations() == 5 );

        SECTION( "Should play single animation from given frame") {
            cut.play(1, 1);
            // Animation 1 should have 3 frames
            REQUIRE( cut.getSpriteAnimationId() == 1 );
            REQUIRE( cut.getCurrentFrameId() == 1 );

            REQUIRE_FALSE( cut.handleTick(400) );
            REQUIRE( cut.getCurrentFrameId() == 2 );
            // Last frame -> animation should end
            REQUIRE( cut.handleTick(400) );
            REQUIRE( cut.isCurrentAnimationSetToNoAnimation() );
        }

        SECTION( "Should play single no end animation") {
            cut.play(2);
            // Animation 3 must have 2 frames
            REQUIRE( cut.getSpriteAnimationId() == 5 );
            REQUIRE( cut.getCurrentFrameId() == 0 );

            // Not enough time to change frame
            REQUIRE_FALSE( cut.handleTick(200) );
            REQUIRE( cut.getCurrentFrameId() == 0 );

            REQUIRE_FALSE( cut.handleTick(200) );
            REQUIRE( cut.getCurrentFrameId() == 1 );

            REQUIRE_FALSE( cut.handleTick(400) );
            REQUIRE( cut.getCurrentFrameId() == 2 );

            // We hit last frame, does change anymore
            REQUIRE_FALSE( cut.handleTick(400) );
            REQUIRE( cut.getSpriteAnimationId() == 5 );
            REQUIRE( cut.getCurrentFrameId() == 2 );
        }

        SECTION( "Should play loop animation") {
            cut.play(3);
            // Animation 3 must have 2 frames
            REQUIRE( cut.getSpriteAnimationId() == 3 );
            REQUIRE( cut.getCurrentFrameId() == 0 );

            REQUIRE_FALSE( cut.handleTick(210) );
            REQUIRE( cut.getCurrentFrameId() == 1 );

            REQUIRE_FALSE( cut.handleTick(200) );
            REQUIRE( cut.getCurrentFrameId() == 0 );

            REQUIRE_FALSE( cut.handleTick(300) );
            REQUIRE( cut.getCurrentFrameId() == 1 );
        }

        SECTION( "Should play loop animation with time") {
            cut.play(4);
            // Animation 2 must have 2 frames
            REQUIRE( cut.getSpriteAnimationId() == 2 );

            REQUIRE_FALSE( cut.handleTick(210) );
            REQUIRE( cut.getCurrentFrameId() == 1 );

            REQUIRE_FALSE( cut.handleTick(200) );
            REQUIRE( cut.getCurrentFrameId() == 0 );

            // Time is exceeded so it's the end of animation
            REQUIRE( cut.handleTick(200) );
            REQUIRE( cut.isCurrentAnimationSetToNoAnimation() );
        }
    }
}
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
    uint16_t getSpriteAnimationId() { return currentAnimation_.framesAnimation; }
    uint16_t getCurrentFrameId() { return frame_; }
    bool isPlaying() { 
        return isPlaying_;
    }

    uint16_t animation1;
    uint16_t animation2;
    uint16_t animation3;
    uint16_t animation4;

protected:
    void loadAnimation(const uint16_t mapObjectAnimationId) override {
        AnimationPlayer::loadAnimation(mapObjectAnimationId);
        if (mapObjectAnimationId == animation2) {
            currentAnimation_.framesAnimation += 1;
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
    animMgr.addFramesAnimation(0);  // First animation does serve
    animMgr.addFramesAnimation(1);  // Anim 1
    animMgr.addFramesAnimation(4);  // Anim 2
    animMgr.addFramesAnimation(6);  // Anim 3
    animMgr.addFramesAnimation(8);  // Anim 4
    animMgr.addFramesAnimation(11);  // Anim 5
}

void initAnimationPlayer(TestAnimationPlayer &animPlayer) {
    animPlayer.animation1 = animPlayer.addAnimation(1, fs_eng::kAnimationModeSingle, 3);
    
    animPlayer.animation2 = animPlayer.addAnimation(
        {
            .framesAnimation = 4,
            .framePerSec = 3,
            .mode = fs_eng::kAnimationModeSingle,
            // play animation for 1500ms
            .maxPlayTime = 1500
        });

    animPlayer.animation3 = animPlayer.addAnimation(
        {
        .framesAnimation = 3,
        .framePerSec = 5,
        .mode = fs_eng::kAnimationModeLoop
    });

    animPlayer.animation4 = animPlayer.addAnimation(
        {
            .framesAnimation = 2,
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

        REQUIRE( cut.getNumberOfAnimations() == 4 );

        SECTION( "Should play single animation from given frame") {
            cut.play(cut.animation1, 1);
            // Animation 1 should have 3 frames
            REQUIRE( cut.getSpriteAnimationId() == 1 );
            REQUIRE( cut.getCurrentFrameId() == 1 );
            REQUIRE( cut.isCurrentAnimation(cut.animation1) );

            REQUIRE_FALSE( cut.handleTick(400) );
            REQUIRE( cut.getCurrentFrameId() == 2 );
            // Last frame -> animation should end
            REQUIRE( cut.handleTick(400) );
            REQUIRE_FALSE( cut.isPlaying() );
            REQUIRE( cut.getCurrentFrameId() == 2 );
        }

        SECTION( "Should play single with time") {
            cut.play(cut.animation2);
            // Animation 2 must have 3 frames
            // and frameAnimation is 5 because we added 1 in loadAnimation()
            REQUIRE( cut.getSpriteAnimationId() == 5 );
            REQUIRE( cut.getCurrentFrameId() == 0 );
            REQUIRE( cut.isCurrentAnimation(cut.animation2) );

            // Not enough time to change frame : Frame 1 arrives at 333ms
            REQUIRE_FALSE( cut.handleTick(200) ); // Total elapsed : 200ms
            REQUIRE( cut.getCurrentFrameId() == 0 );

            // Change to frame 1
            REQUIRE_FALSE( cut.handleTick(200) ); // Total elapsed : 400ms
            REQUIRE( cut.getCurrentFrameId() == 1 );

            // Change to frame 2: Frame 2 arrives at 666ms
            REQUIRE_FALSE( cut.handleTick(400) ); // Total elapsed : 800ms
            REQUIRE( cut.getCurrentFrameId() == 2 );

            // We pass over last frame (999ms) but still under maxPlayTime (1500ms)
            REQUIRE_FALSE( cut.handleTick(400) ); // Total elapsed : 1200ms
            REQUIRE( cut.isPlaying() ); // still playing
            REQUIRE( cut.getCurrentFrameId() == 2 );

            // We pass over max time -> should end
            REQUIRE( cut.handleTick(400) ); // Total elapsed : 1600ms
            REQUIRE_FALSE( cut.isPlaying() ); // still playing
            REQUIRE( cut.getCurrentFrameId() == 2 );
        }

        SECTION( "Should play loop animation") {
            cut.play(cut.animation3);
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
            cut.play(cut.animation4);
            // Animation 2 must have 2 frames
            REQUIRE( cut.getSpriteAnimationId() == 2 );

            REQUIRE_FALSE( cut.handleTick(210) );
            REQUIRE( cut.getCurrentFrameId() == 1 );

            REQUIRE_FALSE( cut.handleTick(200) );
            REQUIRE( cut.getCurrentFrameId() == 0 );

            // Time is exceeded so it's the end of animation
            REQUIRE( cut.handleTick(200) );
            REQUIRE_FALSE( cut.isPlaying() );
        }
    }
}
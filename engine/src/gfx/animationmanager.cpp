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

#include "fs-engine/gfx/animationmanager.h"

#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "fs-engine/enginecommon.h"

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
bool AnimationManager::load()
{
    LOG(Log::k_FLG_GFX, "AnimationManager", "load", ("Loading game sprites ..."))
    fs_eng::Palette emptyPalette;
    if (!spritesManager_.loadSprites("hspr-0.tab", "hspr-0.dat", emptyPalette)) {
        return false;
    }

    if (!loadElementsFromCustomFiles()) {
        // If we could not load custom file
        // we try loading from original files
        return loadElementsFromOriginalFiles();
    }

    return true;
}

/*!
 * @brief Load animations from files made by us
 * @return true if loading is ok
 */
bool AnimationManager::loadElementsFromCustomFiles() {
    FILE *fp = File::openOriginalFile("HELE-0.TXT");
    if (!fp) {
        return false;
    }

    char line[1024];
    while (fgets(line, 1024, fp)) {
        GameSpriteFrameElement e;
        char flipped;
        if (*line == '#')
            continue;
        sscanf(line, "%i %i %i %c %lu", &e.sprite_, &e.off_x_, &e.off_y_,
                &flipped, &e.next_element_);
        e.flipped_ = (flipped == 'f');
        elements_.push_back(e);
    }
    for (unsigned int i = 0; i < elements_.size(); i++)
        assert(elements_[i].next_element_ < elements_.size());
    fclose(fp);

    for (unsigned int i = 0; i < elements_.size(); i++) {
        int esprite = elements_[i].sprite_;
        if (esprite) {
            char tmp[1024];
            sprintf(tmp, "sprites/%i.png", esprite);
            spritesManager_.sprite(esprite)->loadSpriteFromPNG(tmp);
        }
    }

    fp = File::openOriginalFile("HFRA-0.TXT");
    if (fp) {
        while (fgets(line, 1024, fp)) {
            GameSpriteFrame f;
            if (*line == '#')
                continue;
            sscanf(line, "%lu %i %i %i %lu", &f.first_element_, &f.width_,
                    &f.height_, &f.flags_, &f.next_frame_);
            assert(f.first_element_ < elements_.size());
            frames_.push_back(f);
        }
        for (unsigned int i = 0; i < frames_.size(); i++)
            assert(frames_[i].next_frame_ < frames_.size());
        fclose(fp);
    }

    fp = File::openOriginalFile("HSTA-0.TXT");
    if (fp) {
        while (fgets(line, 1024, fp)) {
            size_t index;
            if (*line == '#')
                continue;
            sscanf(line, "%lu", &index);
            assert(index < frames_.size());
            index_.push_back(index);
        }
        fclose(fp);
    }

    return true;
}

/*!
 * @brief Load animations from original game files
 * @return true if loading is ok
 */
bool AnimationManager::loadElementsFromOriginalFiles() {
    size_t size;
    uint8 *data;

    // Load Sprite Frame Element
    data = File::loadOriginalFile("HELE-0.ANI", size);
    assert(size % 10 == 0);
    for (unsigned int i = 0; i < size / 10; i++) {
        GameSpriteFrameElement e;
        e.sprite_ = data[i * 10] | (data[i * 10 + 1] << 8);
        assert(e.sprite_ % 6 == 0);
        e.sprite_ /= 6;
        e.off_x_ = data[i * 10 + 2] | (data[i * 10 + 3] << 8);
        e.off_y_ = data[i * 10 + 4] | (data[i * 10 + 5] << 8);
        e.flipped_ =
            (data[i * 10 + 6] | (data[i * 10 + 7] << 8)) !=
            0 ? true : false;
        e.next_element_ = data[i * 10 + 8] | (data[i * 10 + 9] << 8);
        if (e.off_x_ & (1 << 15))
            e.off_x_ = -(65536 - e.off_x_);
        if (e.off_y_ & (1 << 15))
            e.off_y_ = -(65536 - e.off_y_);
        assert(e.next_element_ < size / 10);
        elements_.push_back(e);
    }
    delete[] data;

    LOG(Log::k_FLG_SND, "AnimationManager", "load", ("loaded %i frame elements", (int)elements_.size()))

    // Load sprite frame
    data = File::loadOriginalFile("HFRA-0.ANI", size);
    assert(size % 8 == 0);
    for (unsigned int i = 0; i < size / 8; i++) {
        GameSpriteFrame f;
        f.first_element_ = data[i * 8] | (data[i * 8 + 1] << 8);
        assert(f.first_element_ < elements_.size());
        f.width_ = data[i * 8 + 2];
        f.height_ = data[i * 8 + 3];
        f.flags_ = data[i * 8 + 4] | (data[i * 8 + 5] << 8);
        f.next_frame_ = data[i * 8 + 6] | (data[i * 8 + 7] << 8);
        assert(f.next_frame_ < size / 8);
        frames_.push_back(f);
    }
    delete[] data;

    LOG(Log::k_FLG_SND, "AnimationManager", "load", ("loaded %i frames", (int)frames_.size()))

    // Load index
    data = File::loadOriginalFile("HSTA-0.ANI", size);
    assert(size % 2 == 0);
    for (unsigned int i = 0; i < size / 2; i++) {
        index_.push_back(data[i * 2] | (data[i * 2 + 1] << 8));
        assert(index_[i] < frames_.size());
    }
    delete[] data;

    LOG(Log::k_FLG_SND, "AnimationManager", "load", ("index contains %i animations", (int)index_.size()))

    return true;
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
 * @brief 
 * @param animNum 
 * @param frameNum 
 * @param screenPos 
 * @return 
 */
bool AnimationManager::drawFrame(unsigned int animNum, int frameNum, const Point2D &screenPos)
{
    assert(animNum < index_.size());

    GameSpriteFrame *f = &frames_[index_[animNum]];
    if (f == NULL)
        return false;

    while (frameNum) {
        f = &frames_[f->next_frame_];
        frameNum--;
    }

    GameSpriteFrameElement *e = &elements_[f->first_element_];
    while (1) {
        spritesManager_.drawSprite(e->sprite_, screenPos.x + e->off_x_, screenPos.y + e->off_y_, e->flipped_);
        if (e->next_element_ == 0)
            break;
        e = &elements_[e->next_element_];
    }

    return f->next_frame_ == index_[animNum];
}

bool AnimationManager::lastFrame(unsigned int animNum, int frameNum)
{
    assert(animNum < index_.size());

    GameSpriteFrame *f = &frames_[index_[animNum]];
    while (frameNum) {
        f = &frames_[f->next_frame_];
        frameNum--;
    }

    return f->next_frame_ == index_[animNum];
}

int AnimationManager::lastFrame(unsigned int animNum)
{
    int frameNum = 0;
    assert(animNum < index_.size());

    GameSpriteFrame *f = &frames_[index_[animNum]];
    while (f->next_frame_ != index_[animNum]) {
        f = &frames_[f->next_frame_];
        frameNum++;
    }
    return frameNum;
}

int AnimationManager::getFrameFromFrameIndx(unsigned int frameIndx)
{
    int frameNum = 0;

    GameSpriteFrame *f = &frames_[frameIndx];
    while (1) {
        f = &frames_[f->next_frame_];
        if (f->flags_ == 0x0100)
            break;
    }

    while (1) {
        f = &frames_[f->next_frame_];
        if (f->next_frame_ == frameIndx)
            return frameNum;
        frameNum++;
    }
}

int AnimationManager::getFrameNum(unsigned int animNum)
{
    assert(animNum < index_.size());
    int frameNum = 1;

    GameSpriteFrame *f = &frames_[index_[animNum]];
    bool passedStart = false;
    while (1) {
        if (f->flags_ == 0x0100) {
            if (passedStart)
                return frameNum;
            else
                passedStart = true;
        }
        f = &frames_[f->next_frame_];
        frameNum++;
    }
}

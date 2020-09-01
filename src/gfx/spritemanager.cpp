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

#include <stdio.h>
#include <assert.h>

#include "gfx/spritemanager.h"
#include "utils/file.h"

SpriteManager::SpriteManager():sprites_(NULL), sprite_count_(0)
{
}

SpriteManager::~SpriteManager()
{
    clear();
}

void SpriteManager::clear()
{
    if (sprites_)
        delete[] sprites_;

    sprites_ = NULL;
    sprite_count_ = 0;
}

bool SpriteManager::loadSprites(uint8 * tabData, int tabSize,
                                uint8 * spriteData, bool rle)
{
    assert(tabData);
    assert(spriteData);

    sprite_count_ = tabSize / TABENTRY_SIZE;
    sprites_ = new Sprite[sprite_count_];
    assert(sprites_);

    for (int i = 0; i < sprite_count_; ++i) {
        if (!sprites_[i].loadSprite(tabData, spriteData, i, rle)) {
            printf("Failed to load sprite: %d\n", i);
        }
    }

    return true;
}

Sprite *SpriteManager::sprite(int spriteNum)
{
    if (spriteNum >= sprite_count_) {
        printf("SpriteManager::drawSpriteXYZ: sprite %d not loaded!\n",
               spriteNum);
        return NULL;
    }
    return &sprites_[spriteNum];
}


bool SpriteManager::drawSpriteXYZ(int spriteNum, int x, int y, int z,
                                  bool flipped, bool x2)
{
    if (spriteNum >= sprite_count_) {
        printf("SpriteManager::drawSpriteXYZ: sprite %d not loaded!\n",
               spriteNum);
        return false;
    }

    sprites_[spriteNum].draw(x, y, z, flipped, x2);

    return true;
}


GameSpriteManager::GameSpriteManager()
{
}

GameSpriteManager::~GameSpriteManager()
{
}

void GameSpriteManager::load()
{
    int tabSize, size;
    uint8 *tabData, *data;
#if 1
    tabData = File::loadOriginalFile("hspr-0.tab", tabSize);
    data = File::loadOriginalFile("hspr-0.dat", size);
    printf("Loaded %d sprites from hspr-0.dat\n", tabSize / 6);
#else
    tabData = File::loadOriginalFile("hspr-0-d.tab", tabSize);
    data = File::loadOriginalFile("hspr-0-d.dat", size);
    printf("Loading %d sprites from hspr-0-d.dat\n", tabSize / 6);
#endif
    loadSprites(tabData, tabSize, data);
    delete[] tabData;
    delete[] data;

    FILE *fp = File::openOriginalFile("HELE-0.TXT");
    if (fp) {
        char line[1024];
        while (fgets(line, 1024, fp)) {
            GameSpriteFrameElement e;
            char flipped;
            if (*line == '#')
                continue;
            sscanf(line, "%i %i %i %c %i", &e.sprite_, &e.off_x_, &e.off_y_,
                   &flipped, &e.next_element_);
            e.flipped_ = (flipped == 'f');
            elements_.push_back(e);
        }
        for (unsigned int i = 0; i < elements_.size(); i++)
            assert(elements_[i].next_element_ < (int)elements_.size());
        fclose(fp);
    } else {
        // try original data file
        data = File::loadOriginalFile("HELE-0.ANI", size);
        assert(size % 10 == 0);
        for (int i = 0; i < size / 10; i++) {
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
    }

    printf("loaded %i frame elements\n", (int)elements_.size());

    for (unsigned int i = 0; i < elements_.size(); i++) {
        int esprite = elements_[i].sprite_;
        if (esprite) {
            char tmp[1024];
            sprintf(tmp, "sprites/%i.png", esprite);
            sprites_[esprite].loadSpriteFromPNG(tmp);        
        }
    } 

    fp = File::openOriginalFile("HFRA-0.TXT");
    if (fp) {
        char line[1024];
        while (fgets(line, 1024, fp)) {
            GameSpriteFrame f;
            if (*line == '#')
                continue;
            sscanf(line, "%i %i %i %i %i", &f.first_element_, &f.width_, 
                    &f.height_, &f.flags_, &f.next_frame_);
            assert(f.first_element_ < (int) elements_.size());
            frames_.push_back(f);
        }
        for (unsigned int i = 0; i < frames_.size(); i++)
            assert(frames_[i].next_frame_ < (int)frames_.size());
        fclose(fp);
    } else {
        // try original data file
        data = File::loadOriginalFile("HFRA-0.ANI", size);
        assert(size % 8 == 0);
        for (int i = 0; i < size / 8; i++) {
            GameSpriteFrame f;
            f.first_element_ = data[i * 8] | (data[i * 8 + 1] << 8);
            assert(f.first_element_ < (int) elements_.size());
            f.width_ = data[i * 8 + 2];
            f.height_ = data[i * 8 + 3];
            f.flags_ = data[i * 8 + 4] | (data[i * 8 + 5] << 8);
            f.next_frame_ = data[i * 8 + 6] | (data[i * 8 + 7] << 8);
            assert(f.next_frame_ < size / 8);
            frames_.push_back(f);
        }
        delete[] data;
    }

    printf("loaded %i frames\n", (int)frames_.size());

    fp = File::openOriginalFile("HSTA-0.TXT");
    if (fp) {
        char line[1024];
        while (fgets(line, 1024, fp)) {
            int index;
            if (*line == '#')
                continue;
            sscanf(line, "%i", &index);
            assert(index < (int) frames_.size());
            index_.push_back(index);
        }
        fclose(fp);
    } else {
        // try original data file
        data = File::loadOriginalFile("HSTA-0.ANI", size);
        assert(size % 2 == 0);
        for (int i = 0; i < size / 2; i++) {
            index_.push_back(data[i * 2] | (data[i * 2 + 1] << 8));
            assert(index_[i] < (int) frames_.size());
        }
        delete[] data;
    }

    printf("index contains %i animations\n", (int)index_.size());
}

bool GameSpriteManager::drawFrame(int animNum, int frameNum, int x, int y)
{
    assert(animNum < (int) index_.size());

    GameSpriteFrame *f = &frames_[index_[animNum]];
    if (f == NULL)
        return false;

    while (frameNum) {
        f = &frames_[f->next_frame_];
        frameNum--;
    }

    GameSpriteFrameElement *e = &elements_[f->first_element_];
    while (1) {
        sprites_[e->sprite_].draw(x + e->off_x_, y + e->off_y_, 0,
                                  e->flipped_);
        if (e->next_element_ == 0)
            break;
        e = &elements_[e->next_element_];
    }

    return f->next_frame_ == index_[animNum];
}

bool GameSpriteManager::lastFrame(int animNum, int frameNum)
{
    assert(animNum < (int) index_.size());

    GameSpriteFrame *f = &frames_[index_[animNum]];
    while (frameNum) {
        f = &frames_[f->next_frame_];
        frameNum--;
    }

    return f->next_frame_ == index_[animNum];
}

int GameSpriteManager::lastFrame(int animNum)
{
    int frameNum = 0;
    assert(animNum < (int) index_.size());

    GameSpriteFrame *f = &frames_[index_[animNum]];
    while (f->next_frame_ != index_[animNum]) {
        f = &frames_[f->next_frame_];
        frameNum++;
    }
    return frameNum;
}

int GameSpriteManager::getFrameFromFrameIndx(int frameIndx)
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

int GameSpriteManager::getFrameNum(int animNum)
{
    assert(animNum < (int) index_.size());
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

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

#include "fs-engine/gfx/spritemanager.h"

#include <cassert>
#include <list>

#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "fs-engine/system/system.h"

void unpackBlocks1b(const uint8_t * data, uint8_t * pixels)
{
    for (int i = 0; i < 8; ++i) {
        if (bitSet(data[0], 7 - i)) {
            pixels[i] = 255;    // transparent
        } else {
            pixels[i] =
                static_cast < uint8_t >
                ((bitValue(data[1], 7 - i) << 0) & 0xff)
                | static_cast < uint8_t >
                ((bitValue(data[2], 7 - i) << 1) & 0xff)
                | static_cast < uint8_t >
                ((bitValue(data[3], 7 - i) << 2) & 0xff)
                | static_cast < uint8_t >
                ((bitValue(data[4], 7 - i) << 3) & 0xff);
        }
    }
}

const int SpriteManager::kTextureWidth = 512;

SpriteManager::SpriteManager(bool rle):sprites_(nullptr), spriteCount_(0), sprites2_(nullptr), isRle_(rle)
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

    if (sprites2_)
        delete[] sprites2_;

    sprites_ = NULL;
    spriteCount_ = 0;
}

/**
 * Loads data from the couple tab/dat files
 * \param tabFile const std::string& name of tabfile
 * \param datFile const std::string& name of data file
 * \param rle bool
 * \return bool Return true if loading is ok
 *
 */
bool SpriteManager::loadSprites(const std::string &tabFile, const std::string &datFile, const uint8_t * paletteColors, int nbColors) {
    size_t size = 0, tabSize = 0;
    uint8_t *data, *tabData;

    // First load tab file
    LOG(Log::k_FLG_GFX, "SpriteManager", "loadSprites", ("Loading sprites from files %s", tabFile.c_str()))
    tabData = File::loadOriginalFile(tabFile, tabSize);
    if (!tabData) {
        FSERR(Log::k_FLG_UI, "SpriteManager", "loadSprites", ("Failed reading file %s", tabFile.c_str()));
        return false;
    }
    data = File::loadOriginalFile(datFile, size);
    if (!data) {
        FSERR(Log::k_FLG_UI, "SpriteManager", "loadSprites", ("Failed reading file %s", datFile.c_str()));
        delete[] tabData;
        return false;
    }

    spriteCount_ = int(tabSize) / Sprite::kTabEntrySize;

    if (isRle_)
        loadSprites2(tabData, data, paletteColors, nbColors);

    bool res = loadSprites(tabData, tabSize, data, paletteColors, nbColors);
    delete[] tabData;
    delete[] data;

    if (res) {
        LOG(Log::k_FLG_GFX, "SpriteManager", "loadSprites", ("%d sprites loaded", tabSize / 6))
    } else {
        FSERR(Log::k_FLG_UI, "SpriteManager", "loadSprites", ("Failed loading menu sprites"));
    }

    return res;
}

/**
 * Loads data from memory
 * \param tabData
 * \param tabSize
 * \param spriteData
 * \param rle
 * \return
 *
 */
bool SpriteManager::loadSprites(uint8 * tabData, size_t tabSize,
                                uint8 * spriteData, const uint8_t * paletteColors, int nbColors)
{
    assert(tabData);
    assert(spriteData);

    sprites_ = new Sprite[spriteCount_];
    assert(sprites_);

    for (int i = 0; i < spriteCount_; ++i) {
        if (!sprites_[i].loadSprite(tabData, spriteData, i, isRle_)) {
            FSERR(Log::k_FLG_IO, "SpriteManager", "loadSprites", ("Failed to load sprite: %d\n", i))
        }
    }

    return true;
}

bool SpriteManager::loadSprites2(const uint8_t * tabData, const uint8_t * spriteData, const uint8_t * paletteColors, int nbColors) {
    assert(tabData);
    assert(spriteData);

    // sort sprites by size (big first)
    std::list<SpriteTabEntry> spriteList;
    readAndSortTabEntries(tabData, spriteList);

    sprites2_ = new Sprite[spriteCount_];
    uint8_t *spriteBuffer = new uint8_t[kTextureWidth * kTextureWidth];
    
    // This stack is used to track sprites inserted in a line
    std::stack<SpriteInsert> spriteStack;

    // load all sprites in a buffer and add them to the buffer ordered by size
    for (auto entry : spriteList) {
        if (entry.spriteId < spriteCount_) {
            sprites2_[entry.spriteId] = readSpriteDataAndCopyToBuffer(spriteData, entry, spriteStack, spriteBuffer);
        }
    }

    // Then init texture with the buffer
    spritesetTexture_ = g_System.createTexture();
    bool res = spritesetTexture_->createSurfaceFromData(spriteBuffer, 
                                            kTextureWidth, 
                                            kTextureWidth,
                                            255);

    if (res) {
        res = spritesetTexture_->setPalette6b3(paletteColors, nbColors);

        if (res) {
            // Finally create texture
            res = spritesetTexture_->loadTextureFromSurface();
        }
    }

    delete[] spriteBuffer;

    return res;
}

/*!
 * Read the tab file and sort the different entries by size : bigger height first and then width
 * @param tabData 
 * @param spriteList The sorted list of sprite tab entries
 */
void SpriteManager::readAndSortTabEntries(const uint8_t * tabData, std::list<SpriteTabEntry>& spriteList) {
    std::list<SpriteTabEntry>::iterator it;
    for (size_t i = 0; i < spriteCount_; ++i) {
        size_t spriteTabOffset = i * Sprite::kTabEntrySize;
        
        SpriteTabEntry entry {
            i,  // sprite id 
            READ_LE_UINT32(tabData + spriteTabOffset),  // offset in the data file
            *(tabData + spriteTabOffset + 4),   // width
            *(tabData + spriteTabOffset + 5),   // height
        };

        // Look for the first entry that either has a lesser height
        // or if height is equal, has a bigger width (smaller first)
        for (it = spriteList.begin(); it != spriteList.end(); ++it) {
            if ((*it).height < entry.height || 
                ((*it).height == entry.height && (*it).width > entry.width)) {
                break;
            }
        }
        if (it == spriteList.end()) {
            spriteList.push_back(entry);
        } else {
            spriteList.insert(it, entry);
        }
    }
}

/*!
 * Load the sprite pixels from spritesData and copy the data
 * in the temp buffer
 * @param spritesData All the sprites data
 * @param entry The entry for the current sprite
 * @param spriteStack The stack used to track where to put the sprite
 * @param spriteBuffer The resulting buffer
 * @return A temporary Sprite object to initialize the Sprite array
 */
Sprite SpriteManager::readSpriteDataAndCopyToBuffer(const uint8_t *spritesData, SpriteTabEntry entry, std::stack<SpriteInsert> &spriteStack, uint8_t *spriteBuffer) {
    Sprite sprite(entry);

    getInsertPoint(sprite, spriteStack);
    uint8_t * spritePixels = sprite.loadSprite(spritesData, entry, isRle_);
    if (spritePixels != nullptr) {
        SpriteInsert insert;
        insert.insertedAt = sprite.textureLocation();
        insert.height = sprite.height();
        insert.width = sprite.width();
        spriteStack.push(insert);
    }

    sprite.copyToBuffer(spritePixels, spriteBuffer, kTextureWidth, kTextureWidth);

    delete[] spritePixels;

    return sprite;
}

/*!
 * Find where to put the sprite in the spriteset texture.
 * Sprites are sorted by size, placing bigger sprite first and then
 * placing smaller sprites after.
 * The stack is used to track sprite on a given line.
 * @param sprite The sprite to place
 * @param spriteStack 
 */
void SpriteManager::getInsertPoint(Sprite &sprite, std::stack<SpriteInsert> &spriteStack) {
    Point2D insertAt {0, 0};
    if (!spriteStack.empty()) {
        // Look at the last inserted sprite
        SpriteInsert topSprite = spriteStack.top();
        insertAt.x = topSprite.insertedAt.x + topSprite.width;
        insertAt.y = topSprite.insertedAt.y;
        // and check if we can append the new sprite on the same line
        if (insertAt.x + sprite.width() >= kTextureWidth) {
            // sprite is too large so fine a place below starting at the first
            // sprite that is high enough or it can be a new line
            while (!spriteStack.empty()) {
                int currentHeight = topSprite.height;
                insertAt.y = topSprite.insertedAt.y + currentHeight;
                do {
                    topSprite = spriteStack.top();
                    insertAt.x = topSprite.insertedAt.x;
                    spriteStack.pop();
                } while (!spriteStack.empty() && spriteStack.top().height == currentHeight);
                
                if (spriteStack.empty()) {
                    // stack is empty so we change line
                    break;
                } else {
                    topSprite = spriteStack.top();
                    if (topSprite.insertedAt.y + topSprite.height > insertAt.y + sprite.height()) {
                        break;
                    }
                }
            }
        }
    }

    sprite.setTextureLocation(insertAt);
}

Sprite *SpriteManager::sprite(int spriteNum)
{
    if (spriteNum >= spriteCount_) {
        FSERR(Log::k_FLG_IO, "SpriteManager", "sprite", ("spriteNum %d is out of bound!\n", spriteNum))
        return NULL;
    }
    return &sprites_[spriteNum];
}

Sprite *SpriteManager::sprite2(int spriteNum)
{
    if (spriteNum >= spriteCount_) {
        FSERR(Log::k_FLG_IO, "SpriteManager", "sprite", ("spriteNum %d is out of bound!\n", spriteNum))
        return NULL;
    }
    return &sprites2_[spriteNum];
}


bool SpriteManager::drawSpriteXYZ(int spriteNum, int x, int y, int z,
                                  bool flipped, bool x2)
{
    if (spriteNum >= spriteCount_) {
        FSERR(Log::k_FLG_IO, "SpriteManager", "drawSpriteXYZ", ("spriteNum %d is out of bound!", spriteNum))
        return false;
    }

    sprites_[spriteNum].draw(x, y, z, flipped, x2);

    return true;
}


/*!
 * TODO : Implement flipped parameter
 * @param spriteNum Id of the sprite
 * @param x Position on the screen
 * @param y Position on the screen
 * @param flipped Do we flip the sprite before drawing it
 * @param x2 do we multiply the size by two before drawing it
 * @return True if drawing is ok
 */
bool SpriteManager::drawSprite(int spriteNum, int x, int y, bool flipped, bool x2) {
    Sprite *pSprite = sprite2(spriteNum);
    if (pSprite) {
        if (x2) {
            spritesetTexture_->renderStretch(pSprite->textureLocation(), {x, y}, pSprite->width(), pSprite->height(), 2);
        } else {
            spritesetTexture_->render(pSprite->textureLocation(), {x, y}, pSprite->width(), pSprite->height());
        }
    }

    return true;
}


GameSpriteManager::GameSpriteManager() : SpriteManager(false)
{
}

GameSpriteManager::~GameSpriteManager()
{
}

/**
 * Loads game sprites.
 * \return bool return true if everything is ok.
 *
 */
bool GameSpriteManager::load()
{
    size_t size;
    uint8 *data;

    LOG(Log::k_FLG_GFX, "GameSpriteManager", "load", ("Loading game sprites ..."))
    if (!loadSprites("hspr-0.tab", "hspr-0.dat", nullptr, 0)) {
        return false;
    }

    FILE *fp = File::openOriginalFile("HELE-0.TXT");
    if (fp) {
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
    } else {
        // try original data file
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
    }

    LOG(Log::k_FLG_SND, "GameSpriteManager", "load", ("loaded %i frame elements", (int)elements_.size()))

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
            sscanf(line, "%lu %i %i %i %lu", &f.first_element_, &f.width_,
                    &f.height_, &f.flags_, &f.next_frame_);
            assert(f.first_element_ < elements_.size());
            frames_.push_back(f);
        }
        for (unsigned int i = 0; i < frames_.size(); i++)
            assert(frames_[i].next_frame_ < frames_.size());
        fclose(fp);
    } else {
        // try original data file
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
    }

    LOG(Log::k_FLG_SND, "GameSpriteManager", "load", ("loaded %i frames", (int)frames_.size()))

    fp = File::openOriginalFile("HSTA-0.TXT");
    if (fp) {
        char line[1024];
        while (fgets(line, 1024, fp)) {
            size_t index;
            if (*line == '#')
                continue;
            sscanf(line, "%lu", &index);
            assert(index < frames_.size());
            index_.push_back(index);
        }
        fclose(fp);
    } else {
        // try original data file
        data = File::loadOriginalFile("HSTA-0.ANI", size);
        assert(size % 2 == 0);
        for (unsigned int i = 0; i < size / 2; i++) {
            index_.push_back(data[i * 2] | (data[i * 2 + 1] << 8));
            assert(index_[i] < frames_.size());
        }
        delete[] data;
    }

    LOG(Log::k_FLG_SND, "GameSpriteManager", "load", ("index contains %i animations", (int)index_.size()))

    return true;
}

bool GameSpriteManager::drawFrame(unsigned int animNum, int frameNum, const Point2D &screenPos)
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
        sprites_[e->sprite_].draw(screenPos.x + e->off_x_, screenPos.y + e->off_y_, 0,
                                  e->flipped_);
        if (e->next_element_ == 0)
            break;
        e = &elements_[e->next_element_];
    }

    return f->next_frame_ == index_[animNum];
}

bool GameSpriteManager::lastFrame(unsigned int animNum, int frameNum)
{
    assert(animNum < index_.size());

    GameSpriteFrame *f = &frames_[index_[animNum]];
    while (frameNum) {
        f = &frames_[f->next_frame_];
        frameNum--;
    }

    return f->next_frame_ == index_[animNum];
}

int GameSpriteManager::lastFrame(unsigned int animNum)
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

int GameSpriteManager::getFrameFromFrameIndx(unsigned int frameIndx)
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

int GameSpriteManager::getFrameNum(unsigned int animNum)
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

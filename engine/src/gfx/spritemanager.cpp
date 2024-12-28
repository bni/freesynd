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
        if (fs_cmn::bitSet(data[0], 7 - i)) {
            pixels[i] = 255;    // transparent
        } else {
            pixels[i] =
                static_cast < uint8_t >
                ((fs_cmn::bitValue(data[1], 7 - i) << 0) & 0xff)
                | static_cast < uint8_t >
                ((fs_cmn::bitValue(data[2], 7 - i) << 1) & 0xff)
                | static_cast < uint8_t >
                ((fs_cmn::bitValue(data[3], 7 - i) << 2) & 0xff)
                | static_cast < uint8_t >
                ((fs_cmn::bitValue(data[4], 7 - i) << 3) & 0xff);
        }
    }
}

const size_t SpriteManager::kMenuSpritesTextureWidth = 512;

SpriteManager::SpriteManager(bool rle, size_t textureWidth):
    sprites_(nullptr), spriteCount_(0), isRle_(rle), textureWidth_(textureWidth)
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
    spriteCount_ = 0;
}

/**
 * Loads data from the couple tab/dat files
 * \param tabFile const std::string& name of tabfile
 * \param datFile const std::string& name of data file
 * \param palette
 * \return bool Return true if loading is ok
 *
 */
bool SpriteManager::loadSprites(const std::string &tabFile, const std::string &datFile, const fs_eng::Palette &palette) {
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

    bool res = loadSprites(tabData, data, palette);
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
 * \param spriteData
 * \param palette
 * \return true if sprites have been loaded
 *
 */
bool SpriteManager::loadSprites(const uint8_t * tabData, const uint8_t * spriteData, const fs_eng::Palette &palette) {
    assert(tabData);
    assert(spriteData);

    // sort sprites by size (big first)
    std::list<SpriteTabEntry> spriteList;
    readAndSortTabEntries(tabData, spriteList);

    sprites_ = new Sprite[spriteCount_];
    uint8_t *spriteBuffer = new uint8_t[textureWidth_ * textureWidth_];
    
    // This stack is used to track sprites inserted in a line
    std::stack<SpriteInsert> spriteStack;

    // load all sprites in a buffer and add them to the buffer ordered by size
    for (auto entry : spriteList) {
        if (entry.spriteId < spriteCount_) {
            sprites_[entry.spriteId] = readSpriteDataAndCopyToBuffer(spriteData, entry, spriteStack, spriteBuffer);
        }
    }

    // Then init texture with the buffer
    spritesetTexture_ = g_System.createTexture();
    bool res = spritesetTexture_->create8bitsSurfaceFromData(spriteBuffer, 
                                            textureWidth_, 
                                            textureWidth_,
                                            255);

    if (res) {
        res = spritesetTexture_->setPalette(palette);

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
            fs_cmn::READ_LE_UINT32(tabData + spriteTabOffset),  // offset in the data file
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

    sprite.copyToBuffer(spritePixels, spriteBuffer, textureWidth_, textureWidth_);

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
        if (insertAt.x + sprite.stride() >= textureWidth_) {
            // sprite is too large so find a place below starting at the first
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

/*!
 * Change the palette of the sprite texture and reload it.
 * @param newPalette The new palette to set
 * @return 
 */
bool SpriteManager::setPalette(const fs_eng::Palette &newPalette) {
    if (spritesetTexture_->setPalette(newPalette)) {
        return spritesetTexture_->loadTextureFromSurface();
    }
    return false;
}

Sprite *SpriteManager::sprite(int spriteNum)
{
    if (spriteNum >= spriteCount_) {
        FSERR(Log::k_FLG_IO, "SpriteManager", "sprite", ("spriteNum %d is out of bound!\n", spriteNum))
        return NULL;
    }
    return &sprites_[spriteNum];
}


/*!
 * Draw the sprite with given Id at given position
 * @param spriteNum Id of the sprite
 * @param x Position on the screen
 * @param y Position on the screen
 * @param flipped Do we flip the sprite before drawing it
 * @param x2 do we multiply the size by two before drawing it
 * @return True if drawing is ok
 */
bool SpriteManager::drawSprite(int spriteNum, int x, int y, bool flipped, bool x2) {
    Sprite *pSprite = sprite(spriteNum);
    int ratio = x2 ? 2 : 1;
    if (pSprite) {
        spritesetTexture_->renderExtended(pSprite->textureLocation(), {x, y}, pSprite->width(), pSprite->height(), ratio, flipped);
    }

    return true;
}

/*!
 * This method is provided for debugging purpose to look inside the texture
 * without worrying at sprite locations.
 * @param src Where to take the origin of the texture
 * @param dst Where to copy the texture on the target
 * @param width Width of the portion of texture to copy
 * @param height Height of the portion of texture to copy
 * @return 
 */
bool SpriteManager::drawTexture(Point2D src, Point2D dst, int width, int height) {
    spritesetTexture_->render(src, dst, width, height);
    
    return true;
}

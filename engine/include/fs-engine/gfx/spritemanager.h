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

#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H

#include <list>
#include <vector>
#include <memory>
#include <stack>

#include "sprite.h"
#include "fs-utils/misc/singleton.h"
#include "fs-engine/gfx/fstexture.h"

/*!
 * This class is used to track the position of a Sprite in
 * the Spriteset texture.
 */
class SpriteInsert {
public:
    //! Position where the sprite is located
    Point2D insertedAt;
    //! Width of the sprite
    int width;
    //! Height of the sprite
    int height;
};

/*!
 * Sprite manager class.
 */
class SpriteManager {
public:
    /*!
     * This is the width for the texture that holds the sprites.
     * It has be chosen to use 512x512 because it is a power of 2
     * enough to hold all sprites.
     */
    static const size_t kMenuSpritesTextureWidth;

    SpriteManager(bool rle, size_t textureWidth);
    virtual ~SpriteManager();
    //! clear all loaded sprites
    void clear();

    //! Loads sprites from the given files
    bool loadSprites(const std::string &tabFile, const std::string &datFile, const fs_eng::Palette &palette);
    //! Returns true if this manager has loaded files
    bool loaded() { return spriteCount_ != 0; }
    //! Returns the number of loaded sprites
    int spriteCount() { return spriteCount_; }

    //! Return the sprite at given index
    Sprite *sprite(int spriteNum);

    bool drawSpriteXYZ(int spriteNum, int x, int y, int z, bool flipped = false,
            bool x2 = false);
    //! Draw a sprite with given Id at given position on screen. Sprite can be flipped or stretched
    bool drawSprite(int spriteNum, int x, int y, bool flipped = false, bool x2 = false);

    bool drawTexture(Point2D src, Point2D dst, int width, int height);

protected:
    
    //! Load sprites from the given files
    bool loadSprites(const uint8_t * tabData, const uint8_t * spriteData, const fs_eng::Palette &palette);
    //! Sort tab entries from the tab file by height then width
    void readAndSortTabEntries(const uint8_t * tabData, std::list<SpriteTabEntry>& spriteList);
    //! Read a sprite pixels and copy those pixels at the right location in the sprite set buffer
    Sprite readSpriteDataAndCopyToBuffer(const uint8_t *spriteData, SpriteTabEntry entry, std::stack<SpriteInsert> &spriteStack, uint8_t *spriteBuffer);
    //! Compute the location where to place the sprite in the buffer
    void getInsertPoint(Sprite &sprite, std::stack<SpriteInsert> &spriteStack);

protected:
    //! The list of sprites
    Sprite *sprites_;
    //! Total number of sprites
    int spriteCount_;
    //! True means the sprites are stored using RLE in the original file
    bool isRle_;
    //! A texture that store all the sprites with an optimization of placements
    std::unique_ptr<FSTexture> spritesetTexture_;
    //! Size of the texture that holds the spriteset
    size_t textureWidth_;
};

#endif

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
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

#ifndef ENGINE_SPRITE_H
#define ENGINE_SPRITE_H

#include "fs-utils/common.h"

/*!
 * This represent a record in the sprite Tab file
 */
struct SpriteTabEntry {
    //! the id is equals to the order of appearance in the file
    size_t spriteId;
    //! The offset in the data file
    uint32_t spriteOffset;
    //! Width of the sprite
    int width;
    //! Height of the sprite
    int height;
};

/*!
 * Sprite class.
 */
class Sprite {
public:
    /*!
     * A Block is a group of pixels stored in original files.
     * It stores partial information for 8 pixels.
     */
    static const int kPixelPerBlock;
    /*!
     * A complete pixel information is 1 bit for transparency and 4 bits for color.
     * It needs 5 blocks to complete a pixel.
     */
    static const int kBlockLength;
    //! Size of a sprite record in the tab file
    static const int kTabEntrySize;
    /*! Id of sprite agent selector 1 in the menu sprite list.*/
    static const int MSPR_SELECT_1;
    /*! Id of sprite agent selector 2 in the menu sprite list.*/
    static const int MSPR_SELECT_2;
    /*! Id of sprite agent selector 3 in the menu sprite list.*/
    static const int MSPR_SELECT_3;
    /*! Id of sprite agent selector 4 in the menu sprite list.*/
    static const int MSPR_SELECT_4;
    /*! Id of sprite left arrow dark in the menu sprite list.*/
    static const int MSPR_LEFT_ARROW_D;
    /*! Id of sprite left arrow light in the menu sprite list.*/
    static const int MSPR_LEFT_ARROW_L;
    /*! Id of sprite right arrow dark in the menu sprite list.*/
    static const int MSPR_RIGHT_ARROW_D;
    /*! Id of sprite right arrow light in the menu sprite list.*/
    static const int MSPR_RIGHT_ARROW_L;
    /*! Id of sprite bullet dark in the menu sprite list.*/
    static const int MSPR_BULLET_D;
    /*! Id of sprite bullet light in the menu sprite list.*/
    static const int MSPR_BULLET_L;
    /*! Id of sprite left arrow for tax in the menu sprite list.*/
    static const int MSPR_TAX_DECR;
    /*! Id of sprite right arrow for tax in the menu sprite list.*/
    static const int MSPR_TAX_INCR;
    /*! Id of sprite left arrow dark in the menu sprite list.*/
    static const int MSPR_LEFT_ARROW2_D;
    /*! Id of sprite left arrow light in the menu sprite list.*/
    static const int MSPR_LEFT_ARROW2_L;
    /*! Id of sprite right arrow dark in the menu sprite list.*/
    static const int MSPR_RIGHT_ARROW2_D;
    /*! Id of sprite right arrow light in the menu sprite list.*/
    static const int MSPR_RIGHT_ARROW2_L;

    enum Format {
        FMT_BLOCKS,
        FMT_RLE
    };

    Sprite();
    //! Initialize the sprite with data from the tab
    Sprite(SpriteTabEntry entry);
    virtual ~Sprite();

    //! Return the id of the sprite
    size_t id() { return id_; }

    void loadSpriteFromPNG(const char *filename);
    bool loadSprite(uint8_t *tabData, uint8_t *spriteData, int offset,
            bool rle = false);
    uint8_t * loadSprite(const uint8_t * spritesData, SpriteTabEntry entry, bool rle);
    //! Copy the sprite data to the spritebuffer
    void copyToBuffer(const uint8_t * spritePixels, uint8_t * spriteBuffer, int bufferWidth, int bufferHeight);

    int width() const { return width_; }
    int height() const { return height_; }
    int stride() const { return stride_; }

    /*!
     * Return the location of the sprite in the Spriteset texture
     * @return a Point2D
     */
    Point2D textureLocation() { return location_; }
    /*!
     * Set the sprite location in the Spriteset texture.
     * @param insertAt The location of the sprite
     */
    void setTextureLocation(Point2D &insertAt) { location_.x = insertAt.x; location_.y = insertAt.y; }

    void data(uint8_t *spr_data) const;

private:
    //! Id of the sprite. This is the order in the tab file
    size_t id_;
    //! Width of the sprite
    int width_;
    //! Height of the sprite
    int height_;
    /*!
     * sprite width_, but adjusted to be divisible by 8 without remainder
     * (boundary of 8)
     */
    int stride_;
    //! Position of the sprite in the texture
    Point2D location_;
    // TODO : remove
    uint8_t *sprite_data_;
};

#endif

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

#ifndef FLIPLAYER_H
#define FLIPLAYER_H

#include <memory>

#include "fs-utils/common.h"
#include "fs-engine/enginecommon.h"
#include "fs-engine/gfx/fstexture.h"

typedef struct FliHeader {
    uint32_t size;
    uint16_t type;                //0xAF12
    uint16_t numFrames;
    uint16_t width;
    uint16_t height;
} FliHeader;

typedef struct ChunkHeader {
    uint32_t size;
    uint16_t type;
} ChunkHeader;

typedef struct FrameTypeChunkHeader {
    ChunkHeader header;
    uint16_t numChunks;
    uint16_t delay;
    uint16_t reserved;            // always zero
    uint16_t widthOverride;
    uint16_t heightOverride;
} FrameTypeChunkHeader;

class Font;

/*!
 * A player for fli animation.
 */
class FliPlayer {
public:
    FliPlayer();
    virtual ~FliPlayer();

    void resetPlayer();

    bool loadFliData(const std::string &filename);

    int width() const { return fliData_ ? fli_info_.width : 0; }
    int height() const { return fliData_ ? fli_info_.height : 0; }

    bool decodeFrame();

    /*!
     * Return true if this animation contains frames to display
     * @return 
     */
    bool hasFrames() const {
        return fliData_ ? fli_info_.numFrames > 0 : false;
    }

    void renderFrame();

protected:
    bool isValidChunk(uint16_t type);
    ChunkHeader readChunkHeader(uint8_t *mem);
    FrameTypeChunkHeader readFrameTypeChunkHeader(ChunkHeader chunkHead,
            uint8_t *&mem);
    void decodeByteRun(uint8_t *data);
    void decodeDeltaFLC(uint8_t *data);
    void setPalette(uint8_t *mem);
    void copyCurrentFrameToScreen();

private:
    //! The content data
    uint8_t *fliData_;
    //! A pointer the frame to load in the fliData
    uint8_t *pCurrentFrameOffset_;
    //! this is the content of the frame
    const uint8_t *offscreen_;
    //! The palette for the fli using FSColor
    FSColor colorPalette_[fs_eng::kPaletteMaxColor];
    //! Info on the Fli to play
    FliHeader fli_info_;
    //! FSTexture to display a frame
    std::unique_ptr<FSTexture> texture_;
};

#endif

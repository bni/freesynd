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

#include "common.h"
#include "system.h"

typedef struct FliHeader {
    uint32 size;
    uint16 type;                //0xAF12
    uint16 numFrames;
    uint16 width;
    uint16 height;
} FliHeader;

typedef struct ChunkHeader {
    uint32 size;
    uint16 type;
} ChunkHeader;

typedef struct FrameTypeChunkHeader {
    ChunkHeader header;
    uint16 numChunks;
    uint16 delay;
    uint16 reserved;            // always zero
    uint16 widthOverride;
    uint16 heightOverride;
} FrameTypeChunkHeader;

class Font;
class MenuManager;

/*!
 * A player for fli animation.
 */
class FliPlayer {
public:
    FliPlayer(MenuManager *pManager) : fli_data_(0), offscreen_(0) {pManager_ = pManager;}
    virtual ~FliPlayer();

    //! Play an entire animation without interruption
    bool play(bool intro = false, Font *pIntroFont = NULL);
    void loadFliData(uint8 *buf);
    bool decodeFrame();
    void copyCurrentFrameToScreen();

    int width() const { return fli_data_ ? fli_info_.width : 0; }
    int height() const { return fli_data_ ? fli_info_.height : 0; }

    bool hasFrames() const {
        return fli_data_ ? fli_info_.numFrames > 0 : false;
    }

    const uint8 *offscreen() const { return offscreen_; }

protected:
    bool isValidChunk(uint16 type);
    ChunkHeader readChunkHeader(uint8 *mem);
    FrameTypeChunkHeader readFrameTypeChunkHeader(ChunkHeader chunkHead,
            uint8 *&mem);
    void decodeByteRun(uint8 *data);
    void decodeDeltaFLC(uint8 *data);
    void setPalette(uint8 *mem);

    uint8 *fli_data_;
    const uint8 *offscreen_;
    uint8 palette_[256 * 3];
    FliHeader fli_info_;
    MenuManager *pManager_;
};

#endif

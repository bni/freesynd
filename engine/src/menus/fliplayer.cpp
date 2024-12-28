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

#include "fs-engine/menus/fliplayer.h"

#include <cstdio>

#include "fs-utils/log/log.h"
#include "fs-utils/io/file.h"
#include "fs-engine/system/system.h"

#if 0 // TMN: Data for playing samples during intro video. Hardcoded from intro.exe.

//#include <pshpack1.h>
struct sample_timings {
    int16 m0;
    uint8 m1, m2, m3, m4, m5, m6;
};
//#include <poppack.h>

const sample_timings g_rg_sample_offsets_and_timings[] = {
    {   0, 0x4C, 0x12, 0x00, 0x00, 0x00 }, // read data from FLC (0x12     (18)     bytes)
    {   1, 0x4C, 0xB2, 0xF2, 0x08, 0x00 }, // read data from FLC (0x08f2b2 (586418) bytes)
    {   1, 0x45, 0x00, 0x00, 0x00, 0x00 }, // load "data/isnds-0.dat"
    {   1, 0x41, 0x01, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (1 = 10ms)
    {   1, 0x53, 0x12, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x12
    {  15, 0x54, 0x01, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    {  19, 0x41, 0x06, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (6 = 60ms)
    {  19, 0x4D, 0x00, 0x00, 0x00, 0x00 }, // start new sequence (sequence # 0)
    {  34, 0x53, 0x01, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x01
    {  39, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    {  44, 0x54, 0x02, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    {  60, 0x53, 0x02, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x02
    {  62, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    {  67, 0x54, 0x03, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    {  85, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    {  90, 0x54, 0x04, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    {  91, 0x41, 0x06, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (6 = 60ms)
    { 117, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 119, 0x4C, 0xFA, 0xD6, 0x04, 0x00 }, // read data from FLC (0x04d6fa (317178) bytes)
    { 119, 0x41, 0x06, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (6 = 60ms)
    { 121, 0x54, 0x05, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 135, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 138, 0x54, 0x06, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 153, 0x54, 0x07, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 165, 0x54, 0x08, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 169, 0x53, 0x0B, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0B
    { 178, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 180, 0x54, 0x09, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 190, 0x53, 0x0A, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0A
    { 200, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 217, 0x4C, 0x16, 0x6E, 0x02, 0x00 }, // read data from FLC (0x026e16 (159254) bytes)
    { 217, 0x41, 0x06, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (6 = 60ms)
    { 218, 0x53, 0x03, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x03
    { 231, 0x54, 0x0A, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 267, 0x4C, 0xC0, 0x52, 0x04, 0x00 }, // read data from FLC (0x0452c0 (283328) bytes)
    { 267, 0x41, 0x0C, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (12 = 120ms)
    { 268, 0x53, 0x0C, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0C
    { 271, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 291, 0x54, 0x0B, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 305, 0x41, 0x06, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (6 = 60ms)
    { 316, 0x4C, 0x84, 0x7C, 0x0A, 0x00 }, // read data from FLC (0x0a7c84 (687236) bytes)
    { 317, 0x41, 0x0C, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (12 = 120ms)
    { 318, 0x53, 0x0D, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0D
    { 333, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 333, 0x53, 0x0D, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0D
    { 343, 0x53, 0x0D, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0D
    { 354, 0x53, 0x0D, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0D
    { 359, 0x54, 0x0C, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 367, 0x41, 0x07, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (7 = 70ms)
    { 406, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 414, 0x4C, 0x78, 0x2D, 0x02, 0x00 }, // read data from FLC (0x22d78 (142712) bytes)
    { 415, 0x41, 0x06, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (6 = 60ms)
    { 427, 0x53, 0x05, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x05
    { 439, 0x4C, 0x0A, 0x29, 0x01, 0x00 }, // read data from FLC (0x01290a (76042) bytes)
    { 439, 0x41, 0x78, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (120 = 1200ms)
    { 440, 0x41, 0x0C, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (12 = 120ms)
    { 442, 0x54, 0x0D, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 449, 0x53, 0x11, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x11
    { 454, 0x53, 0x11, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x11
    { 459, 0x53, 0x0F, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0F
    { 467, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 470, 0x53, 0x10, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x10
    { 472, 0x4C, 0x42, 0xE2, 0x07, 0x00 }, // read data from FLC (0x07e242 (516674) bytes)
    { 473, 0x54, 0x0E, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 473, 0x41, 0x0A, 0x00, 0x00, 0x00 }, // wait 1/100 seconds
    { 489, 0x53, 0x07, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x07
    { 502, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 504, 0x41, 0x08, 0x00, 0x00, 0x00 }, // wait 1/100 seconds
    { 520, 0x53, 0x08, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x08
    { 528, 0x41, 0x0C, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (12 = 120ms)
    { 532, 0x53, 0x09, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x09
    { 541, 0x41, 0x08, 0x00, 0x00, 0x00 }, // wait 1/100 seconds
    { 542, 0x53, 0x08, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x08
    { 548, 0x54, 0x0F, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 548, 0x53, 0x07, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x07
    { 562, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 565, 0x41, 0x0C, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (12 = 120ms)
    { 568, 0x53, 0x06, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x06
    { 569, 0x54, 0x10, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 577, 0x53, 0x07, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x07
    { 580, 0x53, 0x04, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x04
    { 583, 0x53, 0x07, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x07
    { 589, 0x53, 0x04, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x04
    { 592, 0x53, 0x07, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x07
    { 602, 0x53, 0x04, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x04
    { 606, 0x53, 0x07, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x07
    { 610, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 617, 0x53, 0x04, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x04
    { 620, 0x53, 0x07, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x07
    { 627, 0x4C, 0xAC, 0x5D, 0x00, 0x00 }, // read data from FLC (0x5dac (23980) bytes)
    { 628, 0x41, 0x28, 0x00, 0x00, 0x00 }, // wait 1/100 seconds
    { 629, 0x53, 0x0E, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0E
    { 635, 0x4C, 0x5C, 0x31, 0x01, 0x00 }, // read data from FLC (0x01315c (78172) bytes)
    { 635, 0x41, 0x01, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (1 = 10ms)
    { 636, 0x53, 0x00, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x00
    { 637, 0x45, 0x01, 0x00, 0x00, 0x00 }, // load "data/isnds-1.dat"
    { 638, 0x41, 0x06, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (6 = 60ms)
    { 639, 0x54, 0x11, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 641, 0x53, 0x0B, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0B
    { 646, 0x53, 0x07, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x07
    { 654, 0x53, 0x05, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x05
    { 662, 0x53, 0x06, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x06
    { 673, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 674, 0x53, 0x04, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x04
    { 679, 0x53, 0x08, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x08
    { 691, 0x53, 0x09, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x09
    { 704, 0x53, 0x0A, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0A
    { 709, 0x53, 0x0C, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0C
    { 719, 0x53, 0x0A, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0A
    { 725, 0x53, 0x0A, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0A
    { 727, 0x53, 0x0C, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0C
    { 733, 0x53, 0x0A, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0A
    { 738, 0x53, 0x0A, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0A
    { 741, 0x53, 0x0C, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0C
    { 744, 0x53, 0x0A, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0A
    { 749, 0x53, 0x0A, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0A
    { 755, 0x53, 0x0C, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0C
    { 761, 0x53, 0x0D, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0D
    { 786, 0x4C, 0xCE, 0x90, 0x04, 0x00 }, // read data from FLC (0x0490ce (299214) bytes)
    { 787, 0x41, 0x06, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (6 = 60ms)
    { 788, 0x53, 0x01, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x01
    { 807, 0x53, 0x02, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x02
    { 837, 0x4C, 0xF8, 0x03, 0x01, 0x00 }, // read data from FLC (0x0103f8 (66552) bytes)
    { 848, 0x54, 0x12, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 869, 0x4C, 0x78, 0x92, 0x00, 0x00 }, // read data from FLC (0x9278 (37496) bytes)
    { 870, 0x41, 0x0C, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (12 = 120ms)
    { 871, 0x53, 0x03, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x03
    { 892, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 902, 0x4C, 0x10, 0x17, 0x05, 0x00 }, // read data from FLC, 0x051710 (333584) bytes
    { 903, 0x41, 0x0A, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (10 = 100ms)
    { 903, 0x53, 0x0E, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0E
    { 904, 0x54, 0x13, 0x00, 0x00, 0x00 }, // draw subtitle string, index
    { 917, 0x53, 0x07, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x07
    { 920, 0x41, 0x0B, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (11 = 110ms)
    { 929, 0x58, 0x00, 0x00, 0x00, 0x00 }, // stop current sequence
    { 931, 0x53, 0x0F, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x0F
    { 935, 0x53, 0x10, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x10
    { 938, 0x53, 0x10, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x10
    { 941, 0x53, 0x10, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x10
    { 944, 0x53, 0x10, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x10
    { 947, 0x53, 0x10, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x10
    { 950, 0x53, 0x10, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x10
    { 953, 0x53, 0x10, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x10
    { 956, 0x53, 0x10, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x10
    { 959, 0x53, 0x10, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x10
    { 960, 0x54, 0x00, 0x00, 0x00, 0x00 }, // clear subtitle string area
    { 963, 0x53, 0x11, 0x00, 0x00, 0x00 }, // maybe play VOC index 0x11
    { 997, 0x41, 0x78, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (120 = 1200ms)
    { 998, 0x41, 0x78, 0x00, 0x00, 0x00 }, // wait 1/100 seconds (120 = 1200ms)
    {  -1, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

#endif

FliPlayer::FliPlayer(): 
    fliData_(nullptr), 
    offscreen_(nullptr),
    texture_(g_System.createTexture()) {}

FliPlayer::~FliPlayer() {
    resetPlayer();
}

void FliPlayer::resetPlayer() {
    if (offscreen_) {
        delete[] offscreen_;
        offscreen_ = nullptr;
    }

    if (fliData_) {
        delete[] fliData_;
        fliData_ = nullptr;
        pCurrentFrameOffset_ = nullptr;
    }

    fli_info_.numFrames = 0;
    fli_info_.width = fli_info_.height = 0;
    currentFrameIndex_ = -1;
}

/*!
 * 
 * @param filename 
 * @return 
 */
bool FliPlayer::loadFliData(const std::string &filename) {
    LOG(Log::k_FLG_GFX, "FliPlayer", "loadFliData", ("Loading data from file %s", filename.c_str()))
    size_t size = 0;

    resetPlayer();
    fliData_ = File::loadOriginalFile(filename, size);
    uint8_t *pData = fliData_;

    if (fliData_ == nullptr) {
        FSERR(Log::k_FLG_GFX, "FliPlayer", "loadFliData()", ("Unable to load FLI file %s\n", filename.c_str()));
        return false;
    }

    // Read the fli Header
    fli_info_.size = fs_cmn::READ_LE_UINT32(pData);
    pData += 4;
    fli_info_.type = fs_cmn::READ_LE_UINT16(pData);
    pData += 2;
    fli_info_.numFrames = fs_cmn::READ_LE_UINT16(pData);
    pData += 2;
    fli_info_.width = fs_cmn::READ_LE_UINT16(pData);
    pData += 2;
    fli_info_.height = fs_cmn::READ_LE_UINT16(pData);
    // Move to the first frame
    pCurrentFrameOffset_ = pData + 2;

    if (fli_info_.type != 0xAF12) {     //simple check to verify it is indeed a (Bullfrog) FLI
        FSERR(Log::k_FLG_GFX, "FliPlayer", "loadFliData()", ("Attempted to load non-FLI data (type = 0x%04X)\n", fli_info_.type));
        resetPlayer();

        return false;
    }

    assert(fli_info_.width == 320 && fli_info_.height == 200);
    offscreen_ = new uint8_t[fli_info_.width * fli_info_.height];

    return texture_->createStreamingTexture(fli_info_.width, fli_info_.height);
}

bool FliPlayer::isValidChunk(uint16_t type) {
    //Even though it may be a valid chunk type, only return true if we know how to deal with it
    switch (type) {
    case 4:                    //COLOR_256
    case 7:                    //DELTA_FLC (FLI_SS2)
    case 15:                   //BYTE_RUN
    case 0xF1FA:               //FRAME_TYPE
        return true;

    default:
        FSERR(Log::k_FLG_GFX, "FliPlayer", "isValidChunk()", ("ERROR: isValidChunk(0x%04X) is NOT a valid chunk\n", type));
        return false;
    }
}

ChunkHeader FliPlayer::readChunkHeader(uint8_t * mem) {
    ChunkHeader head;
    head.size = fs_cmn::READ_LE_UINT32(mem + 0);
    head.type = fs_cmn::READ_LE_UINT16(mem + 4);
    return head;
}

FrameTypeChunkHeader FliPlayer::readFrameTypeChunkHeader(ChunkHeader chunkHead,
        uint8_t *&mem) {
    FrameTypeChunkHeader head;

    head.header = chunkHead;
    mem += 6;
    head.numChunks = fs_cmn::READ_LE_UINT16(mem);
    mem += 2;
    head.delay = fs_cmn::READ_LE_UINT16(mem);
    mem += 2;
    head.reserved = fs_cmn::READ_LE_UINT16(mem);
    mem += 2;
    head.widthOverride = fs_cmn::READ_LE_UINT16(mem);
    mem += 2;
    head.heightOverride = fs_cmn::READ_LE_UINT16(mem);
    mem += 2;

    return head;
}

void FliPlayer::decodeByteRun(uint8_t *data) {
    uint8_t *ptr = (uint8_t *) offscreen_;
    while ((ptr - offscreen_) < (fli_info_.width * fli_info_.height)) {
        uint8_t chunks = *data++;
        while (chunks--) {
            int8 count = *data++;
            if (count > 0) {
                while (count--) {
                    *ptr++ = *data;
                }
                data++;
            } else {
                uint8_t copyBytes = -count;
                memcpy(ptr, data, copyBytes);
                ptr += copyBytes;
                data += copyBytes;
            }
        }
    }
}

#define OP_PACKETCOUNT      0
#define OP_UNDEFINED        1
#define OP_LASTPIXEL        2
#define OP_LINESKIPCOUNT    3

void FliPlayer::decodeDeltaFLC(uint8_t *data) {
    uint16_t linesInChunk = fs_cmn::READ_LE_UINT16(data);
    data += 2;
    uint16_t currentLine = 0;
    uint16_t packetCount = 0;

    while (linesInChunk--) {
        uint16_t opcode;

        // First process all the opcodes.
        do {
            opcode = fs_cmn::READ_LE_UINT16(data);
            data += 2;

            switch ((opcode >> 14) & 3) {
            case OP_PACKETCOUNT:
                packetCount = opcode;
                break;
            case OP_UNDEFINED:
                break;
            case OP_LASTPIXEL:
                *(uint8_t *) (offscreen_ + (currentLine * fli_info_.width) +
                            (fli_info_.width - 1)) = (opcode & 0xFF);
                break;
            case OP_LINESKIPCOUNT:
                currentLine += -(int16) opcode;
                break;
            }
        } while (((opcode >> 14) & 3) != OP_PACKETCOUNT);

        uint16_t column = 0;

        //Now interpret the RLE data
        while (packetCount--) {
            column += *data++;
            int8 rleCount = (int8) * data++;

            if (rleCount > 0) {
                memcpy((void *) (offscreen_ +
                                 (currentLine * fli_info_.width) + column),
                       data, rleCount * 2);
                data += rleCount * 2;
                column += rleCount * 2;
            }
            else if (rleCount < 0) {
                uint16_t dataWord = *(uint16_t *) data;
                data += 2;
                for (int i = 0; i < -(int16) rleCount; ++i) {
                    *(uint16_t *) (offscreen_ +
                                 (currentLine * fli_info_.width) + column +
                                 (i * 2)) = dataWord;
                }

                column += (-(int16) rleCount) * 2;
            }
            else {            // End of cutscene ?
                return;
            }
        }

        currentLine++;
    }
}


#define FRAME_TYPE  0xF1FA
#define CHUNK_HEADER_SIZE 6

/*!
 * Read the current frame content from loaded FLI file.
 * @param nbColor Set to non zero only for the first frame where palette is loaded
 * @return True if frame has been read
 */
int FliPlayer::decodeFrame(int &nbColor) {
    FrameTypeChunkHeader frameHeader;
    ChunkHeader cHeader = readChunkHeader(pCurrentFrameOffset_);
    currentFrameIndex_++;
    nbColor = 0;
    do {
        switch (cHeader.type) {
        case 4:
            setPalette(pCurrentFrameOffset_ + CHUNK_HEADER_SIZE, nbColor);
            //g_System.setPalette8b3(palette_);
            break;
        case 7:
            decodeDeltaFLC(pCurrentFrameOffset_ + CHUNK_HEADER_SIZE);
            break;
        case 15:
            decodeByteRun(pCurrentFrameOffset_ + CHUNK_HEADER_SIZE);
            break;
        case FRAME_TYPE:
            frameHeader = readFrameTypeChunkHeader(cHeader, pCurrentFrameOffset_);
            fli_info_.numFrames--;
            //printf("Frames Remaining: %d\n", fli_info_.numFrames);
            break;
        default:
            break;
        }

        if (cHeader.type != FRAME_TYPE)
            pCurrentFrameOffset_ += cHeader.size;

        cHeader = readChunkHeader(pCurrentFrameOffset_);

    } while (isValidChunk(cHeader.type) && cHeader.type != FRAME_TYPE);

    if (isValidChunk(cHeader.type)) {
        copyCurrentFrameToScreen();
    } else {
        currentFrameIndex_ = -1;
    }

    return currentFrameIndex_;
}

void FliPlayer::setPalette(uint8_t *mem, int &nbColor) {
    // The number of packets to define the palette changes
    uint16_t numPackets = fs_cmn::READ_LE_UINT16(mem);
    mem += 2;

    if (0 == fs_cmn::READ_LE_UINT16(mem)) {     //set the whole palette
        nbColor = 256;
        mem += 2;
        for (int i = 0; i < 256; ++i) {
            // store colors as FSColor
            colorPalette_[i] = {
                (uint8_t) ((mem[i * 3] << 2) | (mem[i * 3] & 3)),   // red
                (uint8_t) ((mem[i * 3 + 1] << 2) | (mem[i * 3 + 1] & 3)),   // green
                (uint8_t) ((mem[i * 3 + 2] << 2) | (mem[i * 3 + 2] & 3)),   // blue
                0xFF    // always opaque
            };
        }
    }
    else { // Only update some colors
        // Used to keep track of the next index to update in the palette
        // we start at index 0
        uint8_t palPos = 0;

        while (numPackets--) {
            //Each packet is composed of a first byte that indicate how many
            // index to skip
            palPos += *mem++;
            // second byte is how many contiguous index to change
            uint8_t nbColorToUpdate = *mem++;
            // and then the list of new colors for those index
            for (int i = 0; i < nbColorToUpdate; ++i) {
                // store colors as FSColor
                colorPalette_[palPos + i] = {
                    (uint8_t) ((mem[i * 3] << 2) | (mem[i * 3] & 3)),   // red
                    (uint8_t) ((mem[i * 3 + 1] << 2) | (mem[i * 3 + 1] & 3)),   // green
                    (uint8_t) ((mem[i * 3 + 2] << 2) | (mem[i * 3 + 2] & 3)),   // blue
                    0xFF    // always opaque
                };
            }
            palPos += nbColorToUpdate;
            mem += (nbColorToUpdate * 3);
        }
    }
}

/*!
 * Update the texture with the current frame.
 */
void FliPlayer::copyCurrentFrameToScreen() {
    texture_->updateStreamingTexture(offscreen_, colorPalette_);
}

/*!
 * Render the current frame which is in the texture.
 */
void FliPlayer::renderFrame() {
    texture_->renderStretch({0, 0}, {0, 0}, fli_info_.width, fli_info_.height, 2);
}


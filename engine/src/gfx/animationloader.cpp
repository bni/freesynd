/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-engine/gfx/animationloader.h"

#include "fs-utils/log/log.h"
#include "fs-utils/io/file.h"
#include "fs-engine/enginecommon.h"
#include "fs-engine/gfx/animationmanager.h"

namespace fs_eng {

void OriginalFilesAnimationLoader::loadAnimations(AnimationManager &pAnimMgr) {
    size_t size;
    uint8_t *data;
    const int8_t kNbOf8bitsblockInElementStruct = 10;
    const int8_t kNbOf8bitsblockInFrameStruct = 8;
    const int8_t kNbOf8bitsblockInIndexStruct = 2;

    // Load Sprite Frame Element
    data = fs_utl::File::loadOriginalFile("HELE-0.ANI", size);
    if (data == nullptr || size % kNbOf8bitsblockInElementStruct != 0) {
        throw InitializationFailedException("Error in reading file HELE-0.ANI");
    }
    for (unsigned int i = 0; i < size / kNbOf8bitsblockInElementStruct; i++) {
        GameSpriteFrameElement e;
        uint8_t *elementStart = data + i * kNbOf8bitsblockInElementStruct;
        e.sprite = fs_utl::READ_LE_UINT16(elementStart);
        assert(e.sprite % 6 == 0);
        e.sprite /= 6;
        e.off_x = fs_utl::READ_LE_INT16(elementStart + 2);
        e.off_y = fs_utl::READ_LE_INT16(elementStart + 4);
        e.flipped =
            (fs_utl::READ_LE_UINT16(elementStart + 6)) != 0 ? true : false;
        e.nextElement = fs_utl::READ_LE_UINT16(elementStart + 8);
        assert(e.nextElement < size / kNbOf8bitsblockInElementStruct);
        pAnimMgr.addFrameElement(e);
    }
    delete[] data;

    // Load sprite frame
    data = fs_utl::File::loadOriginalFile("HFRA-0.ANI", size);
    if (data == nullptr || size % 8 != 0) {
        throw InitializationFailedException("Error in reading file HFRA-0.ANI");
    }
    for (size_t i = 0; i < size / kNbOf8bitsblockInFrameStruct; i++) {
        uint8_t *elementStart = data + i * kNbOf8bitsblockInFrameStruct;
        GameSpriteFrame frame;
        frame.firstElement = fs_utl::READ_LE_UINT16(elementStart);
        frame.width = *(elementStart + 2);
        frame.height = *(elementStart + 3);
        frame.flags = fs_utl::READ_LE_UINT16(elementStart + 4);
        frame.nextFrame = fs_utl::READ_LE_UINT16(elementStart + 6);
        assert(frame.nextFrame < size / kNbOf8bitsblockInFrameStruct);
        pAnimMgr.addFrame(frame);
    }
    delete[] data;

    // Load index
    data = fs_utl::File::loadOriginalFile("HSTA-0.ANI", size);
    if (data == nullptr || size % 2 != 0) {
        throw InitializationFailedException("Error in reading file HSTA-0.ANI");
    }
    for (size_t i = 0; i < size / kNbOf8bitsblockInIndexStruct; i++) {
        pAnimMgr.addFramesAnimation(fs_utl::READ_LE_UINT16(data + i*kNbOf8bitsblockInIndexStruct));
    }
    delete[] data;
}
    
/*!
 * Custom files must be in the same directory as original files.
 * @param pAnimMgr 
 */
void CustomFilesAnimationLoader::loadAnimations(AnimationManager &pAnimMgr) {
    FILE *fp = fs_utl::File::openOriginalFile("HELE-0.TXT");
    if (!fp) {
        throw InitializationFailedException("Cannot read file HELE-0.TXT");
    }

    char line[1024];
    while (fgets(line, 1024, fp)) {
        GameSpriteFrameElement element;
        char flipped;
        if (*line == '#')
            continue;
        sscanf(line, "%i %i %i %c %lu", &element.sprite, &element.off_x,
            &element.off_y, &flipped, &element.nextElement);
        element.flipped = (flipped == 'f');
        pAnimMgr.addFrameElement(element);
    }
    
    fclose(fp);

    // TODO : add this feature
    /* for (unsigned int i = 0; i < elements_.size(); i++) {
        int esprite = elements_[i].sprite_;
        if (esprite) {
            char tmp[1024];
            sprintf(tmp, "sprites/%i.png", esprite);
            spritesManager_.sprite(esprite)->loadSpriteFromPNG(tmp);
        }
    } */

    fp = fs_utl::File::openOriginalFile("HFRA-0.TXT");
    if (!fp) {
        throw InitializationFailedException("Cannot read file HFRA-0.TXT");
    }

    while (fgets(line, 1024, fp)) {
        GameSpriteFrame frame;
        if (*line == '#')
            continue;
        sscanf(line, "%lu %i %i %i %lu", &frame.firstElement,
            &frame.width, &frame.height, &frame.flags, &frame.nextFrame);
        
        pAnimMgr.addFrame(frame);
    }
    
    fclose(fp);

    fp = fs_utl::File::openOriginalFile("HSTA-0.TXT");
    if (!fp) {
        throw InitializationFailedException("Cannot read file HSTA-0.TXT");
    }

    while (fgets(line, 1024, fp)) {
        uint16_t index;
        if (*line == '#')
            continue;
        sscanf(line, "%hu", &index);
        
        pAnimMgr.addFramesAnimation(index);
    }
    fclose(fp);
}

}
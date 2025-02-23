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

#ifndef GFX_ANIMATIONLOADER_H
#define GFX_ANIMATIONLOADER_H
 
namespace fs_eng {

class AnimationManager;

/*!
 * The AnimationLoader is in charge of loading all animations
 * and put everything in the AnimationManager.
 */
class AnimationLoader {
public:
    virtual ~AnimationLoader() {}

    /*!
     * Loads animations into the AnimationManager.
     * @param pAnimMgr The AnimationManager to initialize
     * @throws InitializationFailedException in case of file problems
     */
    virtual void loadAnimations(AnimationManager &pAnimMgr) = 0;
};

/*!
 * This implementation loads from the original game files.
 */
class OriginalFilesAnimationLoader : public AnimationLoader {
public:
    void loadAnimations(AnimationManager &pAnimMgr) override;
};

/*!
 * This implementation loads aninmation from custom files
 * ie with a open format.
 */
class CustomFilesAnimationLoader : public AnimationLoader {
public:
    void loadAnimations(AnimationManager &pAnimMgr) override;
};

}
#endif // GFX_ANIMATIONLOADER_H
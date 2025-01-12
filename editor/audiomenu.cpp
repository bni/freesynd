/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2025  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "audiomenu.h"

#include <format>

#include "fs-engine/menus/menumanager.h"
#include "fs-engine/system/system.h"
#include "fs-engine/sound/soundmanager.h"

#include "editormenuid.h"

using fs_eng::MenuManager;
using fs_eng::Menu;
using fs_eng::FontManager;


AudioMenu::AudioMenu(MenuManager * m):
    Menu(m, fs_edit_menus::kMenuIdAudio, fs_edit_menus::kMenuIdMain)
{
    isCachable_ = false;
    cursorOnShow_ = kMenuCursor;

    addStatic(0, 40, fs_eng::kScreenWidth, "AUDIO", FontManager::SIZE_4, false);

    addStatic(80, 130, 100, "SOUNDS", FontManager::SIZE_3, false);
    addStatic(80, 160, 100, "INTRO:", FontManager::SIZE_2, false);
    idStaticSoundIntroLabel_ = addStatic(195, 160, 70, "00/30", FontManager::SIZE_2, true);

    idButtonSoundIntroDown_ = addImageOption(180, 160, fs_eng::Sprite::MSPR_TAX_DECR, fs_eng::Sprite::MSPR_TAX_DECR, true);
    idButtonSoundIntroUp_ = addImageOption(270, 160, fs_eng::Sprite::MSPR_TAX_INCR, fs_eng::Sprite::MSPR_TAX_INCR, true);

    idButtonSoundIntroPlay_ = addImageOption(290, 158, fs_eng::Sprite::MSPR_RIGHT_ARROW_D, fs_eng::Sprite::MSPR_RIGHT_ARROW_L, true);
    

    //idButtonFonts_ = addOption(510, 185, 120, 25, "FONTS", FontManager::SIZE_2, -1, true, true);
    //idButtonSpriteset_ = addOption(510, 225, 120, 25, "SPRSET", FontManager::SIZE_2, -1, true, true);

    addStatic(80, 190, 100, "GAME:", FontManager::SIZE_2, false);
    idStaticSoundGameLabel_ = addStatic(195, 190, 70, "00/30", FontManager::SIZE_2, true);

    idButtonSoundGameDown_ = addImageOption(180, 190, fs_eng::Sprite::MSPR_TAX_DECR, fs_eng::Sprite::MSPR_TAX_DECR, true);
    idButtonSoundGameUp_ = addImageOption(270, 190, fs_eng::Sprite::MSPR_TAX_INCR, fs_eng::Sprite::MSPR_TAX_INCR, true);

    idButtonSoundGamePlay_ = addImageOption(290, 188, fs_eng::Sprite::MSPR_RIGHT_ARROW_D, fs_eng::Sprite::MSPR_RIGHT_ARROW_L, true);

    /* idButtonSprites_ = addOption(510, 155, 120, 25, "SPRITES", FontManager::SIZE_2, -1, true, true);
    idButtonFonts_ = addOption(510, 185, 120, 25, "FONTS", FontManager::SIZE_2, -1, true, true);
    idButtonSpriteset_ = addOption(510, 225, 120, 25, "SPRSET", FontManager::SIZE_2, -1, true, true);
    addStatic(0, 375, fs_eng::kScreenWidth, "MORE", FontManager::SIZE_1, false);*/

    addStatic(400, 130, 100, "MUSIC", FontManager::SIZE_3, false);
}

void AudioMenu::handleShow() {
    maxNbSoundIntro_ = g_SoundMgr.getNumberOfIntroSounds();
    currentSoundIntro_ = 0;
    updateSoundIntroLabel();
    getOption(idButtonSoundIntroPlay_)->setWidgetEnabled(maxNbSoundIntro_ != 0);

    maxNbSoundGame_ = g_SoundMgr.getNumberOfGameSounds();
    currentSoundGame_ = 0;
    updateSoundGameLabel();
    getOption(idButtonSoundGamePlay_)->setWidgetEnabled(maxNbSoundGame_ != 0);
}

void AudioMenu::handleAction(const int actionId, [[maybe_unused]] void *ctx) {
    if ( actionId == idButtonSoundGameDown_ ) {
        if (currentSoundGame_ > 0) {
            --currentSoundGame_;
            updateSoundGameLabel();
        }
    } else if (actionId == idButtonSoundGameUp_) {
        if (currentSoundGame_ < (maxNbSoundGame_ - 1)) {
            ++currentSoundGame_;
            updateSoundGameLabel();
        }
    } else if ( actionId == idButtonSoundIntroDown_ ) {
        if (currentSoundIntro_ > 0) {
            --currentSoundIntro_;
            updateSoundIntroLabel();
        }
    } else if (actionId == idButtonSoundIntroUp_) {
        if (currentSoundIntro_ < (maxNbSoundGame_ - 1)) {
            ++currentSoundIntro_;
            updateSoundIntroLabel();
        }
    } else if (actionId == idButtonSoundIntroPlay_) {
        g_SoundMgr.playIntro(currentSoundIntro_);
    } else if (actionId == idButtonSoundGamePlay_) {
        g_SoundMgr.play(currentSoundGame_);
    }
}

void AudioMenu::updateSoundIntroLabel() {
    std::string label = std::format("{:02d}/{}", currentSoundIntro_+1, maxNbSoundIntro_);
    
    getStatic(idStaticSoundIntroLabel_)->setText(label.c_str(), false);
}

void AudioMenu::updateSoundGameLabel() {
    std::string label = std::format("{:02d}/{}", currentSoundGame_+1, maxNbSoundGame_);
    getStatic(idStaticSoundGameLabel_)->setText(label.c_str(), false);
}

/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef EDITOR_AUDIOMENU_H_
#define EDITOR_AUDIOMENU_H_

#include "fs-engine/menus/menu.h"

/*!
 * Audio menu to play all sounds and music.
 */
class AudioMenu : public fs_eng::Menu {
public:
    AudioMenu(fs_eng::MenuManager *m);

    bool handleBeforeShow() override;

    void handleAction([[maybe_unused]] const int actionId, [[maybe_unused]] void *ctx) override;

private:
    void updateSoundIntroLabel();
    void updateSoundGameLabel();

private:
    int idStaticSoundIntroLabel_;
    int idStaticSoundGameLabel_;
    int idButtonSoundIntroUp_;
    int idButtonSoundIntroDown_;
    int idButtonSoundGameUp_;
    int idButtonSoundGameDown_;
    int idButtonSoundIntroPlay_;
    int idButtonSoundGamePlay_;
    size_t maxNbSoundIntro_;
    size_t currentSoundIntro_;
    size_t currentSoundGame_;
    size_t maxNbSoundGame_;
};

#endif // EDITOR_AUDIOMENU_H_

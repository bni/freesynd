/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2023  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "editorcontroller.h"

#include <list>

#include "fs-utils/log/log.h"
#include "fs-utils/io/file.h"
#include "fs-kernel/model/squad.h"
#include "fs-kernel/model/ped.h"

EditorController::EditorController(MapManager *pMapManager) :
        missions_(pMapManager) {
    agents_.setModManager(&mods_);
    agents_.setWeaponManager(&weaponMgr_);
    LOG(Log::k_FLG_INFO, "EditorController", "EditorController", ("EditorController constructor"))
}

EditorController::~EditorController() {
    LOG(Log::k_FLG_INFO, "EditorController", "~EditorController", ("EditorController destructor"))
}

bool EditorController::reset() {
    g_missionCtrl.destroyMission();
    // Reset default mods and weapons
    mods_.reset();
    weaponMgr_.reset();
    // TODO add reading cheatcode for onlywomen parameter
    agents_.reset();

    return true;
}

void EditorController::destroy() {
    LOG(Log::k_FLG_INFO, "EditorController", "destroy", ("destruction"))
    agents_.destroy();
}


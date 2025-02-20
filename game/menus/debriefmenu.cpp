/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2010 2024-2025 Benoit Blancard <benblan@users.sourceforge.net>
 *   Copyright (C) 2012  Roland Haeder <quixy@users.sourceforge.net>   *
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

#include "debriefmenu.h"

#include "fs-engine/menus/menumanager.h"
#include "fs-kernel/mgr/missionmanager.h"
#include "fs-kernel/model/squad.h"
#include "fs-kernel/model/ped.h"
#include "fs-engine/system/system.h"
#include "fs-engine/events/event.h"
#include "fs-utils/log/log.h"
#include "fs-engine/gfx/logomanager.h"

#include "menus/gamemenuid.h"
#include "core/gamecontroller.h"
#include "core/gamesession.h"

using fs_eng::FontManager;

DebriefMenu::DebriefMenu(fs_eng::MenuManager *m) : fs_eng::Menu(m, fs_game_menus::kMenuIdDebrief, fs_game_menus::kMenuIdMain, true) {
    cursorOnShow_ = kMenuCursor;
    int y = 35;
    int right_x = 310;
    int left_x = 20;
    std::string str;

    addStatic(85, y, 545, "#DEBRIEF_TITLE", FontManager::SIZE_4, false);
    y = 100;
    getMessage("DEBRIEF_SUBTITLE", str);
    addStatic(left_x, y, str.c_str(), FontManager::SIZE_2, true);
    separatorSize_ = getMenuFont(FontManager::SIZE_2)->textWidth(str);

    y = 118;
    addStatic(left_x, y, "#DEBRIEF_STATUS", FontManager::SIZE_2, true);
    txtStatusId_ = addStatic(right_x, y, "", FontManager::SIZE_2, true);
    y += 14;
    addStatic(left_x, y, "#DEBRIEF_AGENTS", FontManager::SIZE_2, true);
    txtUsedId_ = addStatic(right_x, y, "", FontManager::SIZE_2, true);
    y += 14;
    addStatic(left_x, y, "#DEBRIEF_NEW_AGENT", FontManager::SIZE_2, true);
    txtAgentCapturedId_ = addStatic(right_x, y, "", FontManager::SIZE_2, true);
    y += 14;
    addStatic(left_x, y, "#DEBRIEF_TIME", FontManager::SIZE_2, true);
    txtTimeId_ = addStatic(right_x, y, "", FontManager::SIZE_2, true);
    y += 14;
    addStatic(left_x, y, "#DEBRIEF_AGT_KILLED", FontManager::SIZE_2, true);
    txtAgentKilledId_ = addStatic(right_x, y, "", FontManager::SIZE_2, true);
    y += 14;
    addStatic(left_x, y, "#DEBRIEF_CRIM_KILLED", FontManager::SIZE_2, true);
    txtCrimKilledId_ = addStatic(right_x, y, "", FontManager::SIZE_2, true);
    y += 14;
    addStatic(left_x, y, "#DEBRIEF_CIV_KILLED", FontManager::SIZE_2, true);
    txtCivilKilledId_ = addStatic(right_x, y, "", FontManager::SIZE_2, true);
    y += 14;
    addStatic(left_x, y, "#DEBRIEF_POL_KILLED", FontManager::SIZE_2, true);
    txtPoliceKilledId_ = addStatic(right_x, y, "", FontManager::SIZE_2, true);
    y += 14;
    addStatic(left_x, y, "#DEBRIEF_GUARD_KILLED", FontManager::SIZE_2, true);
    txtGardKilledId_ = addStatic(right_x, y, "", FontManager::SIZE_2, true);
    y += 14;
    addStatic(left_x, y, "#DEBRIEF_PERSUADED", FontManager::SIZE_2, true);
    txtConvincedId_ = addStatic(right_x, y, "", FontManager::SIZE_2, true);
    y += 14;
    addStatic(left_x, y, "#DEBRIEF_HIT", FontManager::SIZE_2, true);
    txtPrecisionId_ = addStatic(right_x, y, "", FontManager::SIZE_2, true);

    y += 20;
    txtSearchId_ = addStatic(left_x, y, "", FontManager::SIZE_2, true);
    y += 20;
    txtNewWeap1Id_ = addStatic(left_x, y, "", FontManager::SIZE_2, true);
    y += 14;
    txtNewWeap2Id_ = addStatic(left_x, y, "", FontManager::SIZE_2, true);

    addOption(16, 346, 129, 25, "#MENU_ACC_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdMap);
    addOption(500, 347,  128, 25, "#MENU_MAIN_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdMain);
}

/*!
 * This method sets the menu cursor and shows it and
 * updates infos based on mission statistics.
 */
bool DebriefMenu::handleBeforeShow() {
    fs_knl::Mission *pMission = g_missionCtrl.mission();

    // update game state and listen for any change
    ListenerHandle handleResearchEnd = EventManager::listen<fs_knl::ResearchEndEvent>(this, &DebriefMenu::onResearchEndEvent);
    g_gameCtrl.handle_mission_end(pMission);
    EventManager::remove_listener(handleResearchEnd);

    updateStatsFields(pMission);

    checkNewWeaponFound();

    return true;
}

void DebriefMenu::updateStatsFields(fs_knl::Mission *pMission) {
    fs_knl::MissionStats *pStats = pMission->stats();

    if (pMission->getStatus() == fs_knl::Mission::kMissionStatusFailed) {
        getStatic(txtStatusId_)->setText("#DEBRIEF_MIS_FAILED");
    } else if (pMission->getStatus() == fs_knl::Mission::kMissionStatusCompleted) {
        getStatic(txtStatusId_)->setText("#DEBRIEF_MIS_COMP");
    } else if (pMission->getStatus() == fs_knl::Mission::kMissionStatusAborted) {
        getStatic(txtStatusId_)->setText("#DEBRIEF_MIS_ABORT");
    }

    getStatic(txtUsedId_)->setTextFormated("%i", pStats->agents());
    getStatic(txtAgentCapturedId_)->setTextFormated("%i", pStats->agentCaptured());

    int days = 0;
    int hours = 0;
    g_Session.getDayHourFromPeriod(pStats->missionDuration(), days, hours);
    getStatic(txtTimeId_)->setTextFormated("#DEBRIEF_TIME_FORMAT", hours, days);
    getStatic(txtAgentKilledId_)->setTextFormated("%i", pStats->enemyKilled());
    getStatic(txtCrimKilledId_)->setTextFormated("%i", pStats->criminalKilled());
    getStatic(txtCivilKilledId_)->setTextFormated("%i", pStats->civilKilled());
    getStatic(txtPoliceKilledId_)->setTextFormated("%i", pStats->policeKilled());
    getStatic(txtGardKilledId_)->setTextFormated("%i", pStats->guardKilled());
    getStatic(txtConvincedId_)->setTextFormated("%i", pStats->convinced());

    if (pStats->nbOfShots() == 0) {
        getStatic(txtPrecisionId_)->setText("#DEBRIEF_NO_BULLET");
    } else {
        // NOTE: we count persudatron direct uses too, although don't count
        // hits for objects other then original target
        getStatic(txtPrecisionId_)->setTextFormated("%i%%", pStats->precision());
    }
}

void DebriefMenu::checkNewWeaponFound() {
    fs_knl::Squad *pSquad = g_missionCtrl.mission()->getSquad();
    for (auto &pAgent : *pSquad) {
        for (size_t wi=0; wi < pAgent->numWeapons(); wi++) {
            fs_knl::Weapon *pWeapon = pAgent->weapon(wi)->getClass();

            if (!g_gameCtrl.weaponManager().isAvailable(pWeapon)) {
                if (g_Session.researchManager().handleWeaponDiscovered(pWeapon)) {
                    getStatic(txtNewWeap1Id_)->setText("#DEBRIEF_WEAP_FOUND1");
                    getStatic(txtNewWeap2Id_)->setText("#DEBRIEF_WEAP_FOUND2");
                }
            }
        }
    }
}

void DebriefMenu::handleRender() {
    // Display team logo
    g_LogoMgr.draw({18, 14}, g_Session.getLogo(), g_Session.getLogoColour(), true);

    // Draws separator line between title and statistics
    g_System.drawHLine({20, 117}, separatorSize_, getMenuManager()->kMenuColorLightGreen);
    g_System.drawHLine({20, 116}, separatorSize_, getMenuManager()->kMenuColorLightGreen);
}

void DebriefMenu::handleLeave() {
    // We're leaving to the map menu
    // So at this point the current mission is no more needed
    // as a new one will be selected in the map menu
    g_missionCtrl.destroyMission();

    getStatic(txtNewWeap1Id_)->setText("");
    getStatic(txtNewWeap2Id_)->setText("");
    getStatic(txtSearchId_)->setText("");
}

void DebriefMenu::onResearchEndEvent(fs_knl::ResearchEndEvent *pEvt) {
    LOG(Log::k_FLG_GAME, "DebriefMenu", "onResearchEndEvent", ("Received ResearchEndEvent"))
    // A research has ended, so check which type
    fs_knl::Research *pRes = pEvt->pResearch;
    // Is it equipment or mods research?
    if (pRes->getType() == fs_knl::Research::EQUIPS) {
        // Get researched weapon type
        fs_knl::Weapon::WeaponType wt= pRes->getSearchWeapon();
        assert(wt);

        // Get weapon
        fs_knl::Weapon *pWeap = g_gameCtrl.weaponManager().getWeapon(wt);
        assert(pWeap);

        // Draw name of it
        getStatic(txtSearchId_)->setTextFormated("#DEBRIEF_SEARCH", pWeap->getName());
    } else {
        // Must be mods research so draw it
        getStatic(txtSearchId_)->setTextFormated("#DEBRIEF_SEARCH", pRes->getName().c_str());
    }
}

/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2015, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "searchmissionmenu.h"

#include "fs-engine/menus/menu.h"
#include "fs-engine/menus/menumanager.h"
#include "fs-engine/system/system.h"
#include "fs-kernel/mgr/missionmanager.h"
#include "fs-kernel/model/mission.h"
#include "fs-kernel/model/vehicle.h"

#include "editorapp.h"
#include "editormenuid.h"

using fs_eng::MenuManager;
using fs_eng::Menu;
using fs_eng::FontManager;

std::string PedTypeAdapter::getName() {
    switch (type_) {
    case fs_knl::PedInstance::kPedTypeAgent:
        return "AGENT";
    case fs_knl::PedInstance::kPedTypeCivilian:
        return "CIVILIAN";
    case fs_knl::PedInstance::kPedTypeCriminal:
        return "CRIMINAL";
    case fs_knl::PedInstance::kPedTypeGuard:
        return "GUARD";
    case fs_knl::PedInstance::kPedTypePolice:
        return "POLICE";
    default:
        return "UNKOWN";
    }
}

std::string VehicleTypeAdapter::getName() {
    if (type_ == fs_knl::Vehicle::kVehicleTypeTrainHead) {
        return "TRAIN";
    }

    return "UNKOWN";
}

SearchMissionMenu::SearchMissionMenu(MenuManager * m):
    Menu(m, fs_edit_menus::kMenuIdSrchMis, fs_edit_menus::kMenuIdMain)
{
    isCachable_ = false;
    cursorOnShow_ = kMenuCursor;
    addStatic(0, 40, fs_eng::kScreenWidth, "SEARCH MISSION", FontManager::SIZE_4, false);

    initPedTypeListAndWidget();

    initVehicleTypeListAndWidget();

    // Accept button
    addOption(17, 347, 128, 25, "BACK", FontManager::SIZE_2, fs_edit_menus::kMenuIdMain);
    // Main menu button
    searchButId_ = addOption(500, 347,  128, 25, "SEARCH", FontManager::SIZE_2);
}

SearchMissionMenu::~SearchMissionMenu() {
    for (unsigned int i=0; i<pedTypeList_.size(); i++) {
        PedTypeAdapter *pType = pedTypeList_.get(i);
        delete pType;
    }
}

void SearchMissionMenu::initPedTypeListAndWidget() {
    pedTypeList_.add(new PedTypeAdapter(fs_knl::PedInstance::kPedTypeAgent));
    pedTypeList_.add(new PedTypeAdapter(fs_knl::PedInstance::kPedTypeCivilian));
    pedTypeList_.add(new PedTypeAdapter(fs_knl::PedInstance::kPedTypeCriminal));
    pedTypeList_.add(new PedTypeAdapter(fs_knl::PedInstance::kPedTypeGuard));
    pedTypeList_.add(new PedTypeAdapter(fs_knl::PedInstance::kPedTypePolice));

    pPedTypeListBox_ = addListBox(20, 84,  70, 120, true);
    pPedTypeListBox_->setModel(&pedTypeList_);
}

void SearchMissionMenu::initVehicleTypeListAndWidget() {
    vehicleTypeList_.add(new VehicleTypeAdapter(fs_knl::Vehicle::kVehicleTypeTrainHead));

    pVehicleTypeListBox_ = addListBox(110, 84, 70, 120, true);
    pVehicleTypeListBox_->setModel(&vehicleTypeList_);
}

void SearchMissionMenu::initSearchCriterias() {
    searchOnPedType_ = false;
    pedTypeCriteria_ = fs_knl::PedInstance::kPedTypeAgent;
    searchOnVehicleType_ = false;
    vehicleTypeCriteria_ = 0;
}

bool SearchMissionMenu::handleBeforeShow()
{
    initSearchCriterias();

    return true;
}

bool SearchMissionMenu::matchMissionWithPedType(fs_knl::Mission *pMission) {
    if (searchOnPedType_) {
        for (size_t pedId = 0; pedId < pMission->numPeds(); pedId++) {
            fs_knl::PedInstance *pPed = pMission->ped(pedId);

            if (pPed->type() == pedTypeCriteria_) {
                return true;
            }
        }
        return false;
    }

    return true;
}

bool SearchMissionMenu::matchMissionWithVehicleType(fs_knl::Mission *pMission) {
    if (searchOnVehicleType_) {
        for (size_t vId = 0; vId < pMission->numVehicles(); vId++) {
            fs_knl::Vehicle *pVehicle = pMission->vehicle(vId);

            if (pVehicle->getType() == vehicleTypeCriteria_) {
                return true;
            }
        }
        return false;
    }

    return true;
}

void SearchMissionMenu::handleAction(const ActionDesc &action) {
    if (action.id == searchButId_) {
        // first clear result list
        g_editorCtrl.getMissionResultList().clear();

        for (int misId = 1; misId <= 50; misId++) {
            fs_knl::Mission *pMission = g_missionCtrl.loadMission(misId, 1);

            if (pMission) {
                bool keepMission = matchMissionWithPedType(pMission);

                if (keepMission) {
                    keepMission = matchMissionWithVehicleType(pMission);
                }

                if (keepMission) {
                    g_editorCtrl.getMissionResultList().push_back(misId);
                }

                g_missionCtrl.destroyMission();
            }
        }

        menu_manager_->gotoMenu(fs_edit_menus::kMenuIdListMis);
    } else if (action.id == pPedTypeListBox_->getId()) {
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (action.ctx);
        PedTypeAdapter *pType = static_cast<PedTypeAdapter *> (pPair->second);

        searchOnPedType_ = true;
        pedTypeCriteria_ = pType->getType();
    } else if (action.id == pVehicleTypeListBox_->getId()) {
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (action.ctx);
        VehicleTypeAdapter *pType = static_cast<VehicleTypeAdapter *> (pPair->second);

        searchOnVehicleType_ = true;
        vehicleTypeCriteria_ = pType->getType();
    }
}

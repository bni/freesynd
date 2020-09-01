/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2015  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "menus/menu.h"
#include "menus/menumanager.h"
#include "editor/editorapp.h"
#include "editor/searchmissionmenu.h"
#include "editor/editormenuid.h"
#include "gfx/screen.h"
#include "system.h"
#include "missionmanager.h"
#include "mission.h"
#include "model/vehicle.h"

std::string PedTypeAdapter::getName() {
    switch (type_) {
    case PedInstance::kPedTypeAgent:
        return "AGENT";
    case PedInstance::kPedTypeCivilian:
        return "CIVILIAN";
    case PedInstance::kPedTypeCriminal:
        return "CRIMINAL";
    case PedInstance::kPedTypeGuard:
        return "GUARD";
    case PedInstance::kPedTypePolice:
        return "POLICE";
    default:
        return "UNKOWN";
    }
}

std::string VehicleTypeAdapter::getName() {
    if (type_ == Vehicle::kVehicleTypeTrainHead) {
        return "TRAIN";
    }

    return "UNKOWN";
}

SearchMissionMenu::SearchMissionMenu(MenuManager * m):
    Menu(m, fs_edit_menus::kMenuIdSrchMis, fs_edit_menus::kMenuIdMain, "mscrenup.dat", "")
{
    isCachable_ = false;
    addStatic(0, 40, g_Screen.gameScreenWidth(), "SEARCH MISSION", FontManager::SIZE_4, false);

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
    pedTypeList_.add(new PedTypeAdapter(PedInstance::kPedTypeAgent));
    pedTypeList_.add(new PedTypeAdapter(PedInstance::kPedTypeCivilian));
    pedTypeList_.add(new PedTypeAdapter(PedInstance::kPedTypeCriminal));
    pedTypeList_.add(new PedTypeAdapter(PedInstance::kPedTypeGuard));
    pedTypeList_.add(new PedTypeAdapter(PedInstance::kPedTypePolice));

    pPedTypeListBox_ = addListBox(20, 84,  70, 120, true);
    pPedTypeListBox_->setModel(&pedTypeList_);
}

void SearchMissionMenu::initVehicleTypeListAndWidget() {
    vehicleTypeList_.add(new VehicleTypeAdapter(Vehicle::kVehicleTypeTrainHead));

    pVehicleTypeListBox_ = addListBox(110, 84, 70, 120, true);
    pVehicleTypeListBox_->setModel(&vehicleTypeList_);
}

void SearchMissionMenu::initSearchCriterias() {
    searchOnPedType_ = false;
    pedTypeCriteria_ = PedInstance::kPedTypeAgent;
    searchOnVehicleType_ = false;
    vehicleTypeCriteria_ = 0;
}

void SearchMissionMenu::handleShow()
{
    // If we came from the intro, the cursor is invisible
    // otherwise, it does no harm
    g_System.useMenuCursor();
    g_System.showCursor();

    initSearchCriterias();
}

void SearchMissionMenu::handleLeave() {
    g_System.hideCursor();
}

bool SearchMissionMenu::matchMissionWithPedType(Mission *pMission) {
    if (searchOnPedType_) {
        for (size_t pedId = 0; pedId < pMission->numPeds(); pedId++) {
            PedInstance *pPed = pMission->ped(pedId);

            if (pPed->type() == pedTypeCriteria_) {
                return true;
            }
        }
        return false;
    }

    return true;
}

bool SearchMissionMenu::matchMissionWithVehicleType(Mission *pMission) {
    if (searchOnVehicleType_) {
        for (size_t vId = 0; vId < pMission->numVehicles(); vId++) {
            Vehicle *pVehicle = pMission->vehicle(vId);

            if (pVehicle->getType() == vehicleTypeCriteria_) {
                return true;
            }
        }
        return false;
    }

    return true;
}

void SearchMissionMenu::handleAction(const int actionId, void *ctx, const int modKeys) {
    if (actionId == searchButId_) {
        MissionManager missionMgr;

        // first clear result list
        g_App.getMissionResultList().clear();

        for (int misId = 1; misId <= 50; misId++) {
            Mission *pMission = missionMgr.loadMission(misId);

            if (pMission) {
                bool keepMission = matchMissionWithPedType(pMission);

                if (keepMission) {
                    keepMission = matchMissionWithVehicleType(pMission);
                }

                if (keepMission) {
                    g_App.getMissionResultList().push_back(misId);
                }

                delete pMission;
            }
        }

        menu_manager_->gotoMenu(fs_edit_menus::kMenuIdListMis);
    } else if (actionId == pPedTypeListBox_->getId()) {
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (ctx);
        PedTypeAdapter *pType = static_cast<PedTypeAdapter *> (pPair->second);

        searchOnPedType_ = true;
        pedTypeCriteria_ = pType->getType();
    } else if (actionId == pVehicleTypeListBox_->getId()) {
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (ctx);
        VehicleTypeAdapter *pType = static_cast<VehicleTypeAdapter *> (pPair->second);

        searchOnVehicleType_ = true;
        vehicleTypeCriteria_ = pType->getType();
    }
}

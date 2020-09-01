#ifndef SEARCHMISSIONMENU_H_
#define SEARCHMISSIONMENU_H_

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

#include "utils/seqmodel.h"
#include "ped.h"

class Mission;

class PedTypeAdapter {
public:
    PedTypeAdapter(PedInstance::PedType type) {
        type_ = type;
    }

    PedInstance::PedType getType() { return type_; }
    std::string getName();

private:
    PedInstance::PedType type_;
};

class VehicleTypeAdapter {
public:
    VehicleTypeAdapter(uint8 type) {
        type_ = type;
    }

    uint8 getType() { return type_; }
    std::string getName();

private:
    uint8 type_;
};

/*!
 * Search mission menu.
 */
class SearchMissionMenu : public Menu {
public:
    SearchMissionMenu(MenuManager *m);
    ~SearchMissionMenu();

    void handleShow();
    void handleLeave();
    void handleAction(const int actionId, void *ctx, const int modKeys);

protected:
    void initPedTypeListAndWidget();
    void initSearchCriterias();
    void initVehicleTypeListAndWidget();

    bool matchMissionWithPedType(Mission *pMission);
    bool matchMissionWithVehicleType(Mission *pMission);

protected:
    int searchButId_;
    ListBox *pPedTypeListBox_;
    ListBox *pVehicleTypeListBox_;

    VectorModel<PedTypeAdapter *> pedTypeList_;
    VectorModel<VehicleTypeAdapter *> vehicleTypeList_;

    bool searchOnPedType_;
    PedInstance::PedType pedTypeCriteria_;

    bool searchOnVehicleType_;
    uint8 vehicleTypeCriteria_;
};

#endif // SEARCHMISSIONMENU_H_

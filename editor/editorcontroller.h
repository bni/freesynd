/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2023-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef EDITOR_EDITORCONTROLLER_H_
#define EDITOR_EDITORCONTROLLER_H_

#include <cassert>
#include <list>
#include <memory>

#include "fs-utils/misc/singleton.h"
#include "fs-kernel/mgr/agentmanager.h"
#include "fs-kernel/mgr/weaponmanager.h"
#include "fs-kernel/mgr/modmanager.h"
#include "fs-kernel/mgr/missionmanager.h"

/*!
 * The game controller holds the game logic.
 */
class EditorController : public Singleton < EditorController > {
  public:
    EditorController();
    virtual ~EditorController();

    /*!
     * Resets controller.
     * \returns True if reset has succeeded
     */
    bool initialize();

    //! Delete all ressources. Called by App:destroy()
    void destroy();

    //*************************************
    // Managers
    //*************************************
    AgentManager &agents() {
        return agents_;
    }

    WeaponManager &weaponManager() {
        return weaponMgr_;
    }

    ModManager &mods() {
        return mods_;
    }

    //*************************************
    // Editor services
    //*************************************
    //! Return the list of missions found in the search menu
    std::list<int> & getMissionResultList() { return searchResLst_;}

private:
    /*!
     * Manager of agent.
     */
    AgentManager agents_;
    /*! Manager of weapons.*/
    WeaponManager weaponMgr_;
    /*! Manager of mods.*/
    ModManager mods_;
    //! For loading tiles
    fs_eng::TileManager tileMgr_;
    /*! Manager of missions.*/
    MissionManager missions_;

    /*!
     * Use to store id of missions that are found in the search menu.
     */
    std::list<int> searchResLst_;
};

#define g_editorCtrl    EditorController::singleton()

#endif  // EDITOR_EDITORCONTROLLER_H_

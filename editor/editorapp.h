/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net>
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

#ifndef EDITORAPP_H
#define EDITORAPP_H

#include "editorcontroller.h"

#include <memory>

#include "fs-utils/common.h"
#include "fs-engine/base_app.h"

class EditorCliParam : public fs_eng::CliParam {

public:
    EditorCliParam() {}

protected:
    void addOptions(CLI::App &app) override {};


};
/*!
 * Editor Application class.
 */
class EditorApp : public fs_eng::BaseApp {
  public:
    EditorApp();
    virtual ~EditorApp();

#ifdef _DEBUG
public:
    uint8 debug_breakpoint_trigger_;
#endif

protected:
    //! Initialize application
    bool doInitialize() override;
    //! Define the menuid that will be displayed at the application's start
    int getStartMenuId() override;

    bool isLoadIntroResources() override;

    fs_eng::CliParam & getCliParam() override {
        return cliParam_;
    };

private:
    /*! Controls the editor logic. */
    std::unique_ptr<EditorController> editorCtlr_;
    EditorCliParam cliParam_;
};

#endif // EDITORAPP_H

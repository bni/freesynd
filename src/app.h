/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#ifndef APP_H
#define APP_H

#include <memory>

#include "fs-utils/common.h"
#include "fs-engine/base_app.h"
#include "fs-engine/appcontext.h"
#include "fs-kernel/mgr/mapmanager.h"
#include "core/gamecontroller.h"

/*!
 * Application class.
 * Used for managing game settings and workflows.
 */
class App : public BaseApp {
  public:
    App();
    virtual ~App();

    void setCheatCode(const char *name);

    MapManager &maps() {
        return maps_;
    }

#ifdef _DEBUG
public:
    uint8 debug_breakpoint_trigger_;
#endif

protected:
    //! Initialize application
    bool doInitialize(const CliParam& param);
    //! Destroy all components
    void doDestroy();
    //! Define the menuid that will be displayed at the application's start
    int getStartMenuId(const CliParam& param);

private:
    /*! Controls the game logic. */
    std::unique_ptr<GameController> game_ctlr_;

    MapManager maps_;
};

#endif  // APP_H


/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
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

#include "editorapp.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <iostream>
#include <fstream>
#include <set>

#ifdef __APPLE__
// Carbon includes an AIFF header which conflicts with fliplayer.h
// So we will redefine ChunkHeader temporarily to work around that.
#define ChunkHeader CarbonChunkHeader
#include <Carbon/Carbon.h>
#undef ChunkHeader
#endif

#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "fs-utils/io/configfile.h"
#include "fs-utils/io/portablefile.h"

#include "editormenufactory.h"
#include "editormenuid.h"

EditorApp::EditorApp()
    : BaseApp(new EditorMenuFactory()),
      editorCtlr_(std::make_unique<EditorController>(&maps_))
{
#ifdef _DEBUG
    debug_breakpoint_trigger_ = 0;
#endif
}

EditorApp::~EditorApp() {
}


/*!
 * Initialize application.
 * \param iniPath The path to the config file.
 * \return True if initialization is ok.
 */
bool EditorApp::doInitialize(const CliParam& param) {

    LOG(Log::k_FLG_INFO, "EditorApp", "initialize", ("loading game tileset..."))
    if (!maps().initialize()) {
        return false;
    }

    editorCtlr_->reset();

    return true;
}

/*!
 * This method returns the menu Id used to start the app.
 */
int EditorApp::getStartMenuId(const CliParam& param) {
    // Go directly to the main menu
    return fs_edit_menus::kMenuIdMain;
}


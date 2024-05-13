/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>
#include <stdio.h>
#include <filesystem>

#include "fs-utils/common.h"
#include "fs-utils/io/configfile.h"

namespace fs = std::filesystem;

/*!
 * File class gives a list of convenient methods to retrieve
 * files and directories important for the application.
 */
class File {
public:
    static bool getFreesyndConf(const std::string& iniFolder, ConfigFile &freesyndIni);
    //! Return the path for the user config file
    static bool getUserConfFullPath(fs::path& confFullPath);
    //! Return the path for the user folder
    //static bool getUserConfFolderPath(const std::string& confFolder, std::string& confFolderPath);
    static bool getOrCreateUserConfFolder(const std::string& userConfFolder);
    //! Sets the path to the original data files.*/
    static void setOriginalDataFolder(const std::string& path);
    //! Sets the path to our data files.*/
    static void setFreesyndDataFolder(const std::string& path);
    //! Sets the path where to save all files.*/
    static void upsertSaveDataFolder(const std::string& path);
    static void getDefaultSaveFolder(std::string& confFolderPath);

    //*************************************
    // Original files apis
    //*************************************
    static uint8 *loadOriginalFile(const std::string& filename, size_t &filesize);
    static FILE *openOriginalFile(const std::string& filename);

    //! Tests Syndicate original data for existence and correctness
    static bool testOriginalData();

    //*************************************
    // Freesynd files apis
    //*************************************
    //! Returns the full path of the given resource using the current root path.
    static std::string getFreesyndDataFullPath(const std::string& filename);

    //! Sets the filename fullpath for the given slot (from 0 to 9)
    static void getFullPathForSaveSlot(int slot, std::string &path);
    //! Returns the list of game saved names
    static void getGameSavedNames(std::vector<std::string> &files);
    static uint8 *loadOriginalFileToMem(const std::string& filename, size_t &filesize);

private:
    //! Return the path to ini file base on iniFolder
    static void getIniFullPath(const std::string& iniFolder, fs::path&  iniFullPath);

    static void addSaveFilenameAtIndex(const fs::path& filename, std::vector<std::string> &files);
    //! Adds a trailing slash to the string
    static void addMissingSlash(std::string& str);
    //! Returns the full path of the given original game resource using the current root path.
    static std::string getOriginalDataFullPath(const std::string& filename, bool uppercase);

private:
    /*! The path to the original game data.*/
    static fs::path dataPath_;
    /*! The path to our data files.*/
    static fs::path ourDataPath_;
    //! The path to the folder containing the user conf file
    static fs::path userConfFolderPath_;
    /*! The path to the freesynd.ini file and save directory.*/
    static fs::path savePath_;
};

#endif

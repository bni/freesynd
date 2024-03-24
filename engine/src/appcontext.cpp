/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2013  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "fs-engine/appcontext.h"

#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"

AppContext::AppContext() {
    time_for_click_ = 80;
    fullscreen_ = false;
    playIntro_ = true;
    language_ = NULL;
}

AppContext::~AppContext() {
    if (language_) {
        delete language_;
        language_ = NULL;
    }
}

bool AppContext::readConfiguration(const std::string& iniFolder, const std::string& userConfFolder) {

    if (!readFreesyndIni(iniFolder)) {
        return false;
    }

    if (!readOrCreateUserConf(userConfFolder)) {
        return false;
    }

    return true;
}

bool AppContext::readFreesyndIni(const std::string& iniFolder) {
    std::string iniPath;
    ConfigFile freesyndIni;
    bool iniExist = File::getIniFullPath(iniFolder, iniPath);

    if (iniPath.size() == 0) {
        // There was a problem when finding ini path
        FSERR(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Error reading configuration file %s\n", iniFolder.c_str()));
        return false;
    }

    if (!iniExist) {
        FSERR(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Cannot find configuration file %s\n", iniPath.c_str()));
        return false;
    } else {
        FSINFO(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Reading configuration from file %s.\n", iniPath.c_str()));
        std::ifstream in( iniPath.c_str() );

        if( !in ) {
            return false;
        }

        in >> freesyndIni;
    }

    time_for_click_ = freesyndIni.read("time_for_click", 80);

    std::string freesynDataDir;
    if (freesyndIni.readInto(freesynDataDir, "freesynd_data_dir")) {
        File::setFreesyndDataFolder(freesynDataDir);
    } else {
        FSERR(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Cannot find key freesynd_data_dir\n"));
        return false;
    }


    std::string originalDataDir;
    if (freesyndIni.readInto(originalDataDir, "data_dir")) {
        File::setOriginalDataFolder(originalDataDir);
    } else {
        FSERR(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Cannot find key data_dir\n"));
        return false;
    }

    return true;
}

/** \brief
 *
 * \param userConfFolder const std::string&
 * \return bool
 *
 */
bool AppContext::readOrCreateUserConf(const std::string& userConfFolder) {
    ConfigFile userConf;
    bool confExist = File::getUserConfFullPath(userConfFolder, userConfPath_);

    if (confExist) {
        // Load file
        FSINFO(Log::k_FLG_IO, "AppContext", "readOrCreateUserConf", ("Reading user configuration from existing file %s.\n", userConfPath_.c_str()));
        std::ifstream in( userConfPath_.c_str() );

        if( !in ) {
            return false;
        }

        in >> userConf;
    }

    fullscreen_ = userConf.read("fullscreen", false);
    playIntro_ = userConf.read("play_intro", true);
    test_files_ = userConf.read("test_data", true);
    const int languageID = userConf.read("language", 0);

    std::string saveDataDir = userConf.read("save_data_dir", File::getDefaultUserFolder());
    File::setSaveDataFolder(saveDataDir);

    if (!confExist) {
        // Save first ini file with default parameters
        try {
            FSINFO(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Initializing user configuration file in %s\n", userConfPath_.c_str()));
            userConf.add("fullscreen", fullscreen_);
            userConf.add("play_intro", playIntro_);
            userConf.add("test_data", test_files_);
            userConf.add("language", languageID);
            userConf.add("save_data_dir", saveDataDir);

            std::ofstream file(userConfPath_.c_str(), std::ios::out | std::ios::trunc);
            if (file) {
                file << userConf;
                file.close();
            } else {
                FSERR(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Cannot write new configuration to file"))
                return false;
            }
        } catch (...) {
            FSERR(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Error while writing configuration to file"))
            return false;
        }
    }

    return readLanguage(languageID);
}

bool AppContext::readLanguage(const int languageId) {
    std::string filename;

    switch (languageId) {
        case 1:
            curr_language_ = AppContext::FRENCH;
            filename = "lang/french.lng";
            break;
        case 2:
            curr_language_ = AppContext::ITALIAN;
            filename = "lang/italian.lng";
            break;
        case 3:
            curr_language_ = AppContext::GERMAN;
            filename = "lang/german.lng";
            break;
        case 0: // 0 is english
        default:
            curr_language_ = AppContext::ENGLISH;
            filename = "lang/english.lng";
            break;
    }

    try {
        language_ = new ConfigFile(File::getFreesyndDataFullPath(filename));
        return true;
    } catch (...) {
        FSERR(Log::k_FLG_IO, "AppContext", "setLanguage", ("Unable to load language file %s!\n", filename.c_str()));
        language_ = NULL;
        return false;
    }
}

std::string AppContext::getMessage(const std::string & id) {
    std::string msg;
    getMessage(id, msg);
    return msg;
}

void AppContext::getMessage(const std::string & id, std::string & msg) {
    if (!language_ || !language_->readInto(msg, id)) {
        msg = "?";
    }
}

/*!
 * Set the play intro parameter in the config file.
 * \param files
 */
void AppContext::updateIntroFlag() {
    try {
        LOG(Log::k_FLG_IO, "App", "updateIntroFlag", ("Setting play_intro to false in %s", userConfPath_.c_str()))
        ConfigFile conf(userConfPath_);
        conf.add("play_intro", false);

        std::ofstream file(userConfPath_.c_str(), std::ios::out | std::ios::trunc);
        if (file) {
            file << conf;
            file.close();
        } else {
            LOG(Log::k_FLG_IO, "App", "updateIntroFlag", ("Could not update configuration file!"))
        }
    } catch (...) {
        LOG(Log::k_FLG_IO, "App", "updateIntroFlag", ("Could not update configuration file!"))
    }
}

void AppContext::deactivateTestFlag() {
    try {
        ConfigFile conf(userConfPath_);
        conf.add("test_data", false);

        std::ofstream file(userConfPath_.c_str(), std::ios::out | std::ios::trunc);
        if (file) {
            file << conf;
            file.close();
        } else {
            FSERR(Log::k_FLG_IO, "AppContext", "deactivateTestFlag", ("Could not update configuration file for test_data parameter!"))
        }
    } catch (...) {
        FSERR(Log::k_FLG_IO, "AppContext", "deactivateTestFlag", ("Could not update configuration file for test_data parameter!"))
    }
}

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

bool AppContext::readConfiguration(const std::string& iniFolder) {
    ConfigFile conf;
    bool iniExist = File::getIniFullPath(iniFolder, iniPath_);

    if (iniPath_.size() == 0) {
        // There was a problem when finding ini path
        return false;
    }

    if (iniExist) {
        // Load file
        FSINFO(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Reading configuration from existing file %s.\n", iniPath_.c_str()));
        std::ifstream in( iniPath_.c_str() );

        if( !in ) {
            return false;
        }

        in >> conf;
    }

    fullscreen_ = conf.read("fullscreen", false);
    playIntro_ = conf.read("play_intro", true);
    time_for_click_ = conf.read("time_for_click", 80);
    test_files_ = conf.read("test_data", true);

    std::string freesynDataDir = conf.read("freesynd_data_dir", File::getDefaultFreesyndDataFolder());
    File::setFreesyndDataFolder(freesynDataDir);

    std::string originalDataDir = conf.read("data_dir", freesynDataDir);
    File::setOriginalDataFolder(originalDataDir);

    std::string saveDataDir = conf.read("save_data_dir", File::getDefaultIniFolder());
    File::setSaveDataFolder(saveDataDir);

    if (!iniExist) {
        // Save first ini file with default parameters
        try {
            FSINFO(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Configuration file did not exist, create one in %s\n", iniPath_.c_str()));
            conf.add("fullscreen", fullscreen_);
            conf.add("play_intro", playIntro_);
            conf.add("time_for_click", time_for_click_);
            conf.add("test_data", test_files_);
            conf.add("freesynd_data_dir", freesynDataDir);
            conf.add("data_dir", originalDataDir);
            conf.add("save_data_dir", saveDataDir);

            std::ofstream file(iniPath_.c_str(), std::ios::out | std::ios::trunc);
            if (file) {
                file << conf;
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

    return readLanguage(conf);
}

bool AppContext::readLanguage(const ConfigFile& conf) {
    std::string filename;

    switch (conf.read("language", 0)) {
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
        ConfigFile conf(iniPath_);
        conf.add("play_intro", false);

        std::ofstream file(iniPath_.c_str(), std::ios::out | std::ios::trunc);
        if (file) {
            file << conf;
            file.close();
        } else {
            LOG(Log::k_FLG_GFX, "App", "updateIntroFlag", ("Could not update configuration file!"))
        }
    } catch (...) {
        LOG(Log::k_FLG_GFX, "App", "updateIntroFlag", ("Could not update configuration file!"))
    }
}

void AppContext::deactivateTestFlag() {
    try {
        ConfigFile conf(iniPath_);
        conf.add("test_data", false);

        std::ofstream file(iniPath_.c_str(), std::ios::out | std::ios::trunc);
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

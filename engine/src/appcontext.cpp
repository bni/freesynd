/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2013, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-engine/appcontext.h"

#include <locale>
#include <format>

#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "fs-engine/system/system.h"

namespace fs_eng {

AppContext::AppContext() {
    time_for_click_ = 80;
    fullscreen_ = false;
    playIntro_ = true;
    language_ = NULL;
    soundVolume_ = 0;
    scaleFactor_ = 0;
}

AppContext::~AppContext() {
    if (language_) {
        delete language_;
        language_ = NULL;
    }
}

/*!
 * Read configuration in freesynd.ini and user.conf
 * @param iniFolder 
 * @param userConfFolder
 * @throw InitializationFailedException if any problem during reading config files
 */
void AppContext::readConfiguration(const std::string& iniFolder, const std::string& userConfFolder) {

    readFreesyndIni(iniFolder);

    readOrCreateUserConf(userConfFolder);
}

/*!
 * @brief Read configuration in freesynd.ini
 * @param iniFolder Folder where freesynd.ini file should be
 * @throw InitializationFailedException if any problem during reading config file
 */
void AppContext::readFreesyndIni(const std::string& iniFolder) {
    ConfigFile freesyndIni;

    if (!fs_utl::File::getFreesyndConf(iniFolder, freesyndIni)) {
        throw InitializationFailedException(std::format("Failed to read configuration in folder: {}", iniFolder));
    }

    time_for_click_ = freesyndIni.read("time_for_click", 80u);

    std::string freesynDataDir;
    if (freesyndIni.readInto(freesynDataDir, "freesynd_data_dir")) {
        fs_utl::File::setFreesyndDataFolder(freesynDataDir);
    } else {
        throw InitializationFailedException("Cannot find key freesynd_data_dir in config file freesynd.ini");
    }


    std::string originalDataDir;
    if (freesyndIni.readInto(originalDataDir, "data_dir")) {
        fs_utl::File::setOriginalDataFolder(originalDataDir);
    } else {
        throw InitializationFailedException("Cannot find key data_dir in config file freesynd.ini");
    }
}

/** \brief
 *
 * \param userConfFolder const std::string& Only set if there was a Cli param
 * @throw InitializationFailedException if any problem during reading config file
 */
void AppContext::readOrCreateUserConf(const std::string& userConfFolder) {
    ConfigFile userConf;
    fs::path userConfFullpath;

    if(!fs_utl::File::getOrCreateUserConfFolder(userConfFolder)) {
        throw InitializationFailedException(
                std::format("Failed to read user configuration in folder: {}", userConfFolder));
    }

    bool confExist = fs_utl::File::getUserConfFullPath(userConfFullpath);

    if (confExist) {
        // Load file
        FSINFO(Log::k_FLG_IO, "AppContext", "readOrCreateUserConf", ("Reading user configuration from existing file: %s", userConfFullpath.c_str()));
        std::ifstream in( userConfFullpath );

        if( !in ) {
            throw InitializationFailedException("Failed to read user configuration");
        }

        in >> userConf;
    }

    // Fullscreen is default in this fork
    fullscreen_ = userConf.read("fullscreen", true);
    playIntro_ = userConf.read("play_intro", true);
    test_files_ = userConf.read("test_data", true);
    soundVolume_ = userConf.read("sound_volume", 100);
    scaleFactor_ = userConf.read("scale_factor", 0);
    const int languageID = userConf.read("language", 0);
    std::string defaultDir;
    fs_utl::File::getDefaultSaveFolder(defaultDir);
    std::string saveDataDir(userConf.read("save_data_dir", defaultDir));
    fs_utl::File::upsertSaveDataFolder(saveDataDir);

    if (!confExist) {
        // Save first ini file with default parameters
        FSINFO(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Initializing user configuration file in %s\n", userConfFullpath.string().c_str()));
        userConf.add("fullscreen", fullscreen_);
        userConf.add("play_intro", playIntro_);
        userConf.add("test_data", test_files_);
        userConf.add("language", languageID);
        userConf.add("save_data_dir", saveDataDir);
        userConf.add("sound_volume", soundVolume_);

        if (!updateUserConf(userConf, userConfFullpath)) {
            throw InitializationFailedException(std::format("Failed to update user configuration in folder: {}", userConfFullpath.string()));
        }
    }

    readLanguage(languageID);
}

bool AppContext::updateUserConf(const ConfigFile& userConf, const std::filesystem::path userConfPath) {
    try {
        std::ofstream file(userConfPath.c_str(), std::ios::out | std::ios::trunc);
        if (file) {
            file << userConf;
            file.close();
        } else {
            FSERR(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Cannot write new configuration to file %s", userConfPath.string().c_str()))
            return false;
        }
        return true;
    } catch (...) {
        FSERR(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Error while writing configuration to file"))
        return false;
    }
}

/*!
 * Loads the right language file based on the given language id.
 * If id is 0, then look in the CTYPE category of the system Locale,
 * to find the language. By default, the language is English.
 * @param languageId const int
 * @throw InitializationFailedException if any problem during reading language file
 *
 */
void AppContext::readLanguage(const int languageId) {
    std::string filename;
        
    switch (languageId) {
    case 1:
        curr_language_ = fs_eng::FRENCH;
        break;
    case 2:
        curr_language_ = fs_eng::ITALIAN;
        break;
    case 3:
        curr_language_ = fs_eng::GERMAN;
        break;
    case 4:
        curr_language_ = fs_eng::ENGLISH;
        break;
    default: // In this case, we use the OS locale to define the language
        curr_language_ = g_System.getLanguageFromSystem();
        break;
    }

    switch (curr_language_) {
        case fs_eng::FRENCH:
            filename = "lang/french.lng";
            break;
        case fs_eng::ITALIAN:
            filename = "lang/italian.lng";
            break;
        case fs_eng::GERMAN:
            filename = "lang/german.lng";
            break;
        default:
            filename = "lang/english.lng";
            break;
    }

    try {
        language_ = new ConfigFile(fs_utl::File::getFreesyndDataFullPath(filename));
    } catch (...) {
        language_ = NULL;
        throw InitializationFailedException(std::format("Unable to load language file : {}", filename));
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
    std::filesystem::path userConfPath;
    fs_utl::File::getUserConfFullPath(userConfPath);
    LOG(Log::k_FLG_IO, "AppContext", "updateIntroFlag", ("Setting play_intro to false in %s", userConfPath.c_str()))
    ConfigFile conf(userConfPath.string());
    conf.add("play_intro", false);

    updateUserConf(conf, userConfPath);
}

void AppContext::deactivateTestFlag() {
    std::filesystem::path userConfPath;
    fs_utl::File::getUserConfFullPath(userConfPath);
    LOG(Log::k_FLG_IO, "AppContext", "deactivateTestFlag", ("Setting test_data to false in %s", userConfPath.c_str()))
    ConfigFile conf(userConfPath.string());
    conf.add("test_data", false);

    updateUserConf(conf, userConfPath);
}

/*!
 * Update the sound_volume parameter in the config file 
 * @param volume new volume. Must greater or equal zero
 */
void AppContext::updateSoundVolume(int volume) {
    if (volume < 0) {
        return;
    }
    std::filesystem::path userConfPath;
    fs_utl::File::getUserConfFullPath(userConfPath);
    LOG(Log::k_FLG_IO, "AppContext", "updateSoundVolume", ("Updating sound volume to %d in %s", volume, userConfPath.c_str()))
    ConfigFile conf(userConfPath.string());
    conf.add("sound_volume", volume);

    updateUserConf(conf, userConfPath);
}

}

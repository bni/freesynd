/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2013-2024                                            *
 *                   Benoit Blancard <benblan@users.sourceforge.net>    *
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

#include <locale>

#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "fs-engine/system/system.h"

namespace fs_eng {

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
    ConfigFile freesyndIni;

    if (!fs_utl::File::getFreesyndConf(iniFolder, freesyndIni)) {
        return false;
    }

    time_for_click_ = freesyndIni.read("time_for_click", 80u);

    std::string freesynDataDir;
    if (freesyndIni.readInto(freesynDataDir, "freesynd_data_dir")) {
        fs_utl::File::setFreesyndDataFolder(freesynDataDir);
    } else {
        FSERR(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Cannot find key freesynd_data_dir\n"));
        return false;
    }


    std::string originalDataDir;
    if (freesyndIni.readInto(originalDataDir, "data_dir")) {
        fs_utl::File::setOriginalDataFolder(originalDataDir);
    } else {
        FSERR(Log::k_FLG_IO, "AppContext", "readConfiguration", ("Cannot find key data_dir\n"));
        return false;
    }

    return true;
}

/** \brief
 *
 * \param userConfFolder const std::string& Only set if there was a Cli param
 * \return bool
 *
 */
bool AppContext::readOrCreateUserConf(const std::string& userConfFolder) {
    ConfigFile userConf;
    fs::path userConfFullpath;

    if(!fs_utl::File::getOrCreateUserConfFolder(userConfFolder)) {
        return false;
    }

    bool confExist = fs_utl::File::getUserConfFullPath(userConfFullpath);

    if (confExist) {
        // Load file
        FSINFO(Log::k_FLG_IO, "AppContext", "readOrCreateUserConf", ("Reading user configuration from existing file.\n"));
        std::ifstream in( userConfFullpath );

        if( !in ) {
            return false;
        }

        in >> userConf;
    }

    fullscreen_ = userConf.read("fullscreen", false);
    playIntro_ = userConf.read("play_intro", true);
    test_files_ = userConf.read("test_data", true);
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

        if (!updateUserConf(userConf, userConfFullpath)) {
            return false;
        }
    }

    return readLanguage(languageID);
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
 * \param languageId const int
 * \return bool True if the language file has been correctly loaded.
 *
 */
bool AppContext::readLanguage(const int languageId) {
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
    default: // In this case, we use the OS locale to define the language
        {
#ifdef __APPLE__
        curr_language_ = g_System.getLanguageFromSystem();
#else
        setlocale(LC_ALL, "");
        std::string lang(setlocale(LC_CTYPE, NULL));

        if (lang.starts_with("French") || lang.starts_with("fr")) {
            curr_language_ = fs_eng::FRENCH;
        } else if (lang.starts_with("Italian") || lang.starts_with("it")) {
            curr_language_ = fs_eng::ITALIAN;
        } else if (lang.starts_with("German") || lang.starts_with("de")) {
            curr_language_ = fs_eng::GERMAN;
        } else {
            curr_language_ = fs_eng::ENGLISH;
        }
#endif
        }
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

}

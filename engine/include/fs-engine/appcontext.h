/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2013  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef CORE_APPCONTEXT_H_
#define CORE_APPCONTEXT_H_

#include <filesystem>

#include "fs-utils/common.h"
#include "fs-utils/misc/singleton.h"
#include "fs-utils/io/configfile.h"
#include "fs-engine/enginecommon.h"

namespace fs_eng {
/*!
 * This class stores application level parameters.
 */
class AppContext : public Singleton < AppContext > {
public:
    AppContext();
    ~AppContext();

    //! Reads the game config file from the given path
    bool readConfiguration(const std::string& iniPath, const std::string& userConfFolder);

    bool isFullScreen() { return fullscreen_; }

    bool isPlayIntro() { return playIntro_; }

    bool isTestFiles() { return test_files_; }
    //! Return the sound volume parameter
    int getSoundVolume() { return soundVolume_; }

    //! Sets the intro flag to false in the config file
    void updateIntroFlag();
    //! Sets the test files flag to false in the config file
    void deactivateTestFlag();
    //! Sets the sound volume parameter to the given value in the config file
    void updateSoundVolume(int volume);

    uint32_t getTimeForClick() { return time_for_click_; }

    fs_eng::FS_Lang currLanguage(void) {return curr_language_; }
    std::string getMessage(const std::string & id);
    void getMessage(const std::string & id, std::string & msg);

private:
    bool readLanguage(const int languageId);
    bool readFreesyndIni(const std::string& iniFolder);
    bool readOrCreateUserConf(const std::string& userConfFolder);
    bool updateUserConf(const ConfigFile& userConf, const std::filesystem::path userConfPath);

private:
    /*! True means the game will run in fullscreen. */
    bool fullscreen_;
    /*! True means the intro will be played.*/
    bool playIntro_;
    /*! Time range between mouse up and down that is treated as click,
     * if it will be longer it will be treated as dragging
    */
    uint32_t time_for_click_;
    /*! True means data files will be verified.*/
    bool test_files_;
    /*! Language file. */
    ConfigFile  *language_;
    fs_eng::FS_Lang curr_language_;
    //! Volume of sound : from 0 to 128
    int soundVolume_;
};

}

#define g_Ctx   fs_eng::AppContext::singleton()

#endif // CORE_APPCONTEXT_H_

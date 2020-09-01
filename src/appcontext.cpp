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

#include "appcontext.h"
#include "utils/file.h"
#include "utils/log.h"

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

void AppContext::setLanguage(FS_Lang lang) {
    std::string filename(File::dataFullPath("lang/"));
    switch (lang) {
        case ENGLISH:
            filename.append("english.lng");
            break;
        case FRENCH:
            filename.append("french.lng");
            break;
        case ITALIAN:
            filename.append("italian.lng");
            break;
        case GERMAN:
            filename.append("german.lng");
            break;
        default:
            filename.append("english.lng");
            lang = ENGLISH;
            break;
    }

    try {
        language_ = new ConfigFile(filename);
        curr_language_ = lang;
    } catch (...) {
        printf("ERROR : Unable to load language file %s.\n", filename.c_str());
        language_ = NULL;
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

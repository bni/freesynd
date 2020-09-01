/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
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

#include "log.h"

#include <stdarg.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

const int Log::k_FLG_ALL  = 0xffffffff;
const int Log::k_FLG_NONE = 0x00000000;
const int Log::k_FLG_INFO = 0x00000001;
const int Log::k_FLG_UI   = 0x00000002;
const int Log::k_FLG_GFX  = 0x00000004;
const int Log::k_FLG_MEM  = 0x00000008;
const int Log::k_FLG_IO   = 0x00000010;
const int Log::k_FLG_GAME = 0x00000020;
const int Log::k_FLG_SND  = 0x00000040;

// the log file
FILE *Log::logfile_ = NULL;
// Current mask
int Log::logMask_ = Log::k_FLG_ALL;

/*!
 * Returns a string representing the given type of category.
 * If the flag is not part of the regular types, an UNKNW string is 
 * returned.
 * \param type One if the k_FLG_XXX except k_FLG_NONE or k_FLG_ALL.
 * \return A human readable representation of the type.
 */
const char * Log::typeToStr(int type) {
    switch (type) {
        case k_FLG_INFO :
            return "INFOS";
        case k_FLG_UI :
            return "UI   ";
        case k_FLG_GFX :
            return "GFX  ";
        case k_FLG_MEM :
            return "MEMRY";
        case k_FLG_IO :
            return "IO   ";
        case k_FLG_GAME:
            return "GAME ";
        case k_FLG_SND:
            return "SOUND";
        default :
            return "UNKWN";
    }
}

/*!
 * This method sets the logging mask to specify which type of log is enable
 * and then tries to open the logging file.
 * \param mask Either k_FLG_NONE to disable the logger or k_FLG_ALL to 
 * enable all types or a combination k_FLG_XXX with the '|' operator.
 * \param filename The name of the log file.
 * \return true if the logging system has correctly been initialized.
 */
bool Log::initialize(int mask, const char *filename) {
    // sets the current mask
    logMask_ = mask;

    // try to open the log file only if logging has been enabled
    if (logMask_ != k_FLG_NONE) {
        logfile_ = fopen(filename, "w");
        if (logfile_ == NULL) {
            std::cerr << "Failed to open log file" << std::endl;
            return false;
        }

        // Writes a starting message
        // TODO(benblan): adds the date
        fprintf(logfile_, "---- Starts logging ----\n");
        fflush(logfile_);
    }

    return true;
};

/*!
 * Checks if logging is enabled for type. The type is &ed against
 * the current log mask.
 * \param type One if the k_FLG_XXX except k_FLG_NONE or k_FLG_ALL.
 * \return true if logging can be done.
 */
int Log::canLog(int type) {
    // Test if the logfile is opened first
    return logfile_ && (type & logMask_);
};

/*!
 * Writes the formated message in the logfile.
 * The message can be any formated string.
 * \param format A formated string
 */
void Log::logMessage(const char * format, ...) {
    if (logfile_) {
        va_list list;

        va_start(list, format);
        vfprintf(logfile_, format, list);
        va_end(list);

        fprintf(logfile_, "\n");

        fflush(logfile_);
    }
};

/*!
 * Each line in the log file starts with a header that specifies
 * the logging category, the component and method that initiated
 * the log.
 * \param type One if the k_FLG_XXX except k_FLG_NONE or k_FLG_ALL.
 * \param comp The component that issued the logging order.
 * \param method The method that issued the logging order.
 */
void Log::logHeader(int type, const char * comp, const char * method) {
    if (logfile_) {
        fprintf(logfile_, "[%s] [%s] [%s] : ", typeToStr(type), comp, method);
        fflush(logfile_);
    }
};

/*!
 * Closes the logger.
 */
void Log::close() {
    if (logfile_) {
        fprintf(logfile_, "---- End of logging. ----\n");
        fflush(logfile_);
        fclose(logfile_);
        logfile_ = NULL;
    }
};

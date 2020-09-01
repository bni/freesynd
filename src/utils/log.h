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

#ifndef FREESYND_UTILS_LOG_H_
#define FREESYND_UTILS_LOG_H_

#include <stdio.h>

// Logging is enabled only in debug mode
#ifdef _DEBUG

#define LOG(t, c, m, str) { if (Log::canLog(t)) {Log::logHeader(t, c, m); Log::logMessage str;} }  // NOLINT
#define FSERR(t, c, m, str) { if (Log::canLog(t)) {Log::logHeader(t, c, m); Log::logMessage str;} printf str; }  // NOLINT

#else

#define LOG(type, comp, meth, str)
#define FSERR(t, c, m, str) { printf str; }  // NOLINT

#endif

//! A logger for displaying debug informations.
/*! 
 * The logging system allows the application to write debug
 * informations to a file. There are different categories (called type) of 
 * information which can be filtered through the mask value.<BR>
 * The logger must initialized by calling the initialize() method and 
 * closed using the close() method.<BR>
 * The logger can then be used with the LOG macro which is enabled only in debug mode.
 * Here is an example of a call to the logger :<BR>
 * <code>
 * LOG(Log::k_FLG_GFX, "run", "run", ("Loading %d sprites from mfnt-0.dat", tabSize / 6))
 * </code>
 */
class Log {
 public:
    /*! This flag enables all types of logging.*/
    static const int k_FLG_ALL;
    /*! This flag disables logging.*/
    static const int k_FLG_NONE;
    /*! This flag enables logging of general info.*/
    static const int k_FLG_INFO;
    /*! This flag enables logging relatives to the UI.*/
    static const int k_FLG_UI;
    /*! This flag enables logging relatives to the graphic component.*/
    static const int k_FLG_GFX;
    /*! This flag enables logging relatives to memory.*/
    static const int k_FLG_MEM;
    /*! This flag enables logging relatives input-output events.*/
    static const int k_FLG_IO;
    /*! This flag enables logging relatives to the game IA.*/
    static const int k_FLG_GAME;
    /*! This flag enables logging relatives to the sound system.*/
    static const int k_FLG_SND;

    //! Log initialization.
    static bool initialize(int mask, const char *filename);

    //! Returns true if logging is enabled for the given type.
    static int canLog(int type);

    //! Prints the message header
    static void logHeader(int type, const char * comp, const char * method);

    //! Prints the log message
    static void logMessage(const char * format, ...);

    //! Closes the logger
    static void close();

 private:

    //! Returns a readable representation of the given type.
    static const char * typeToStr(int type);

    /*! The current logging mask. By default , ALL is set.*/
    static int logMask_;

    /*! A pointer to a log file.*/
    static FILE *logfile_;
};

#endif  // FREESYND_UTILS_LOG_H_

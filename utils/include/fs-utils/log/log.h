/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2010, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef FREESYND_UTILS_LOG_H_
#define FREESYND_UTILS_LOG_H_

#include <cstdint>
#include <stdio.h>
#include <string>

// Logging is enabled only in debug mode
#ifdef _DEBUG

#define LOG(t, c, m, str) { if (Log::canLog(t)) {Log::logHeader(t, c, m, "DEBUG"); Log::logMessage str;} }  // NOLINT
#define FSERR(t, c, m, str) { if (Log::canLog(t)) {Log::logHeader(t, c, m, "ERROR"); Log::logMessage str;} printf ("ERROR: "); printf str; }  // NOLINT
#define FSINFO(t, c, m, str) { if (Log::canLog(t)) {Log::logHeader(t, c, m, "INFO "); Log::logMessage str;} printf ("INFO : "); printf str; }  // NOLINT

#else

#define LOG(type, comp, meth, str)
#define FSERR(t, c, m, str)  { printf ("ERROR: "); printf str; }  // NOLINT
#define FSINFO(t, c, m, str) { printf ("INFO : "); printf str; }  // NOLINT

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
    static const uint64_t k_FLG_ALL;
    /*! This flag disables logging.*/
    static const uint64_t k_FLG_NONE;
    /*! This flag enables logging of general info.*/
    static const uint64_t k_FLG_INFO;
    /*! This flag enables logging relatives to the UI.*/
    static const uint64_t k_FLG_UI;
    /*! This flag enables logging relatives to the graphic component.*/
    static const uint64_t k_FLG_GFX;
    /*! This flag enables logging relatives to memory.*/
    static const uint64_t k_FLG_MEM;
    /*! This flag enables logging relatives input-output events.*/
    static const uint64_t k_FLG_IO;
    /*! This flag enables logging relatives to the game IA.*/
    static const uint64_t k_FLG_GAME;
    /*! This flag enables logging relatives to the sound system.*/
    static const uint64_t k_FLG_SND;

    //! Log initialization.
    static bool initialize(std::string mask, const char *filename);

    //! Returns true if logging is enabled for the given type.
    static int canLog(uint64_t type);

    //! Prints the message header
    static void logHeader(uint64_t type, const char * comp, const char * method, const char * level);

    //! Prints the log message
    static void logMessage(const char * format, ...);

    //! Closes the logger
    static void close();

 private:

    //! Returns a readable representation of the given type.
    static const char * typeToStr(uint64_t type);
    //! Returns a log mask from parsing the input
    static uint64_t maskFromString(std::string mask);

    /*! The current logging mask. By default , ALL is set.*/
    static uint64_t logMask_;

    /*! A pointer to a log file.*/
    static FILE *logfile_;
};

#endif  // FREESYND_UTILS_LOG_H_

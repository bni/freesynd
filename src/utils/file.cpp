/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "file.h"
#include "dernc.h"
#include "log.h"
#include "portablefile.h"

std::string File::dataPath_ = "./data/";
std::string File::ourDataPath_ = "./data/";
std::string File::homePath_ = "./";

/*!
 * The methods returns a string composed of the root path and given file name.
 * No control is made on the result format or file existence.
 * \param filename The relative path to one of the original data files.
 * \param uppercase If true, the resulting string will uppercased.
 */
std::string File::originalDataFullPath(const std::string& filename, bool uppercase) {
    std::string second_part = filename;

    std::string::iterator it;
    for (it = second_part.begin(); it != second_part.end(); it++) {
        (*it) = uppercase ? toupper(*it) : tolower(*it);
    }

    return dataPath_ + second_part;
}

/*!
 * The methods returns a string composed of the root path and given file name.
 * No control is made on the result format or file existence.
 * \param filename The relative path to one of our data files.
 */
std::string File::dataFullPath(const std::string& filename) {
    return ourDataPath_ + filename;
}

void File::getFullPathForSaveSlot(int slot, std::string &path) {
    path.erase();

    path.append(homePath_);
    char c = path[path.size() - 1];
    if (c != '\\' && c != '/')
        path.append("/");
    path.append("save/");

    std::ostringstream out;
    if (slot < 10) {
        out << "0";
    }
    out << slot << ".fsg";

    path.append(out.str());
}

/*!
 * \return NULL if file cannot be read.
 */
uint8 *File::loadOriginalFileToMem(const std::string& filename, int &filesize) {
    // try lowercase, then uppercase.
    FILE *fp = fopen(originalDataFullPath(filename, false).c_str(), "rb");
    if (!fp) fp = fopen(originalDataFullPath(filename, true).c_str(), "rb");

    if (fp) {
        fseek(fp, 0, SEEK_END);
        filesize = ftell(fp);
        uint8 *mem = new uint8[filesize + 1];
        mem[filesize] = '\0';
        fseek(fp, 0, SEEK_SET);
        size_t  n = fread(mem, 1, filesize, fp);
        if (n == 0) {
            FSERR(Log::k_FLG_IO, "File", "loadFileToMem", ("WARN: File '%s' (using path: '%s') is empty\n",
               filename.c_str(), dataPath_.c_str()));
         }
        fclose(fp);
        return mem;
    }

    // If we're here, there's a problem
    FSERR(Log::k_FLG_IO, "File", "loadFileToMem", ("ERROR: Couldn't open file '%s' (using path: '%s')\n",
       filename.c_str(), dataPath_.c_str()));

    filesize = 0;
    return NULL;
}

FILE *File::openOriginalFile(const std::string& filename) {
    // try lowercase, then uppercase.
    FILE *fp = fopen(originalDataFullPath(filename, false).c_str(), "r");
    if (!fp) fp = fopen(originalDataFullPath(filename, true).c_str(), "r");
    return fp;
}

void File::setDataPath(const std::string& path) {
    dataPath_ = path;
    LOG(Log::k_FLG_IO, "File", "setDataPath", ("set data path to %s", path.c_str()));
}

void File::setOurDataPath(const std::string& path) {
    ourDataPath_ = path;
    LOG(Log::k_FLG_IO, "File", "setOurDataPath", ("set our data path to %s", path.c_str()));
}

void File::setHomePath(const std::string& path) {
    homePath_ = path;
    LOG(Log::k_FLG_IO, "File", "setHomePath", ("set home path to %s", path.c_str()));
}

uint8 *File::loadOriginalFile(const std::string& filename, int &filesize) {
    uint8 *data = loadOriginalFileToMem(filename, filesize);
    if (data) {
        if (READ_BE_UINT32(data) == RNC_SIGNATURE) {    //File is RNC compressed
            filesize = rnc::unpackedLength(data);
            assert(filesize > 0);
            uint8 *buffer = new uint8[filesize + 1];
            buffer[filesize] = '\0';
            int result = rnc::unpack(data, buffer);
            delete[] data;

            if (result < 0) {
                FSERR(Log::k_FLG_IO, "File", "loadFile", ("Error loading file: %s!", rnc::errorString(result)));
                filesize = 0;
                delete[] buffer;
            }

            if (result != filesize) {
                FSERR(Log::k_FLG_IO, "File", "loadFile", ("Uncompressed size mismatch for file %s!\n", filename.c_str()));
                filesize = 0;
                delete[] buffer;
            }

            return buffer;
        }
    }
    return data;
}

void File::processSaveFile(const std::string& filename, std::vector<std::string> &files) {
    size_t extPos = filename.find_last_of('.');
    if (extPos == std::string::npos) return;

    std::string name = filename.substr(0, extPos);
    std::string ext = filename.substr(extPos);
    if (ext.compare(".fsg") != 0) return;

    std::istringstream iss( name );
    int index;
    iss >> index;
    if (index < 10) {
        PortableFile infile;
        std::string full_filename;
        getFullPathForSaveSlot(index, full_filename);
        infile.open_to_read(full_filename.c_str());

        if (infile) {
            // FIXME: detect original game saves
            // Read version first
            unsigned char vMaj = infile.read8();
            unsigned char vMin = infile.read8();
            FormatVersion v(vMaj, vMin);
            // Read slot name
            if (v == 0x0100) {
                files[index] = infile.read_string(25, true);
            } else {
                files[index] = infile.read_string(31, true);
            }
        }
    }
}

/*!
 * Returns the list of names to display in load/save menu.
 * \param files
 */
void File::getGameSavedNames(std::vector<std::string> &files) {
    std::string savePath(homePath_);
    char c = savePath[savePath.size() - 1];
    if (c != '\\' && c != '/')
        savePath.append("/");
    savePath.append("save");

#ifdef _WIN32
    SECURITY_ATTRIBUTES sa;
    WIN32_FIND_DATA File;
    HANDLE hSearch;

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    if (CreateDirectory(savePath.c_str(), &sa) == ERROR_PATH_NOT_FOUND) {
        FSERR(Log::k_FLG_IO, "File", "getGameSavedNames", ("Cannot create save directory in %s", homePath_.c_str()))
        return;
    }

    savePath.append("/*.fsg");
    hSearch = FindFirstFile(savePath.c_str(), &File);
    if (hSearch != INVALID_HANDLE_VALUE) {
        do {
            processSaveFile(File.cFileName, files);
        } while (FindNextFile(hSearch, &File));
    }
#else
    DIR * rep = opendir(savePath.c_str());
    struct dirent * ent;

    if (rep == NULL) {
        if (mkdir(savePath.c_str(), 0777) == -1) {  // Create the directory
            FSERR(Log::k_FLG_IO, "File", "getGameSavedNames", ("Cannot create save directory in %s", homePath_.c_str()))
            return;
         }
        rep = opendir(savePath.c_str());
    }

    while ((ent = readdir(rep)) != NULL) {
        processSaveFile(ent->d_name, files);
    }

    closedir(rep);
#endif
}

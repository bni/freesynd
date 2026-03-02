/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>
 *   Copyright (C) 2010, 2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "fs-utils/io/file.h"

#include <stdio.h>
#include <stdlib.h>
#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "CRC.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifdef __APPLE__
// Carbon includes an AIFF header which conflicts with fliplayer.h
// So we will redefine ChunkHeader temporarily to work around that.
#define ChunkHeader CarbonChunkHeader
#include <Carbon/Carbon.h>
#undef ChunkHeader
#endif

#include "fs-utils/io/dernc.h"
#include "fs-utils/log/log.h"
#include "fs-utils/io/portablefile.h"
#include "fs-utils/io/formatversion.h"

namespace fs_utl {

    fs::path File::dataPath_ = "./data/";
    fs::path File::ourDataPath_ = "./data/";
    fs::path File::savePath_ = ".";
    fs::path File::userConfFolderPath_ = "";

#ifdef _WIN32
static std::string exeFolder() {
    char buf[1024];
    GetModuleFileName(NULL, buf, 1024);
    std::string tmp(buf);
    size_t pos = tmp.find_last_of('\\');
    if (pos != std::string::npos) tmp.erase(pos + 1);
    else tmp = ".";
    return tmp;
}
#endif

#ifdef __APPLE__
/*!
 * Return the path to the resources storesd in the bundle.
 * @param resourcePath The path to set
 * @return True if everything is ok.
 */
static bool getResourcePath(fs::path& resourcePath) {
    // let's check to see if we're inside an application bundle first.
    CFBundleRef main = CFBundleGetMainBundle();
    CFStringRef appid = NULL;
    if (main) appid = CFBundleGetIdentifier(main);
    if (!appid) return false;

    // we're in an app bundle.
    CFURLRef url = CFBundleCopyResourcesDirectoryURL(main);
    if (!url) {
        // this shouldn't happen.
        FSERR(Log::k_FLG_GFX, "File", "getResourcePath", ("Unable to locate resources.\n"))
        return false;
    }
    FSRef fs;
    if (!CFURLGetFSRef(url, &fs)) {
        // this shouldn't happen.
        FSERR(Log::k_FLG_GFX, "File", "getResourcePath", ("Unable to translate URL.\n"))
        return false;
    }

    char *buf = (char *)malloc(1024);
    FSRefMakePath(&fs, (UInt8 *)buf, 1024);
    CFRelease(url);

    resourcePath.assign(buf);
    free(buf);
    return true;
}
#endif


    /*! @brief
    * @param iniFolder
    * @param freesyndIni
    * @return
    */
    bool File::getFreesyndConf(const std::string& iniFolder, ConfigFile &freesyndIni) {
    #if defined(__APPLE__)
        // On MacOS there is no freesynd.ini, we use Preferences utilities
        CFStringRef key = CFSTR("data_dir");
        CFStringRef value;
        // For converting CFStringRef to std:string
        const CFIndex kCStringSize = 128;
        char temporaryCString[kCStringSize];

        // Read the "data_dir" preference.
        value = (CFStringRef)CFPreferencesCopyAppValue(key,
                                    kCFPreferencesCurrentApplication);
        if (value) {
            // All this code only to convert a CFStringRef to a std::string!
            bzero(temporaryCString,kCStringSize);
            CFStringGetCString(value, temporaryCString, kCStringSize, kCFStringEncodingUTF8);
            std::string *oldDataDirAsStr = new std::string(temporaryCString);
            freesyndIni.add("data_dir", *oldDataDirAsStr);
            CFRelease(value);
            CFRelease(key);
            delete oldDataDirAsStr;
        } else {
            // Empty string signals setOriginalDataFolder() to use the bundle Resources/data path
            freesyndIni.add("data_dir", "");
        }

        // Read the freesynd_data_dir preference
        key = CFSTR("freesynd_data_dir");
        value = (CFStringRef)CFPreferencesCopyAppValue(key,
                                    kCFPreferencesCurrentApplication);
        if (value) {
            // All this code only to convert a CFStringRef to a std::string!
            bzero(temporaryCString,kCStringSize);
            CFStringGetCString(value, temporaryCString, kCStringSize, kCFStringEncodingUTF8);
            std::string *freesyndDataDirAsStr = new std::string(temporaryCString);
            freesyndIni.add("freesynd_data_dir", *freesyndDataDirAsStr);
            CFRelease(value);
            CFRelease(key);
            delete freesyndDataDirAsStr;
        } else {
            // Sets a default dir that will be seen as to be set
            freesyndIni.add("freesynd_data_dir", "");
        }

    #else
        // On Windows or Linux, we use a freesynd.ini file
        fs::path iniPath;

        File::getIniFullPath(iniFolder, iniPath);

        if (!fs::exists(iniPath)) {
            FSERR(Log::k_FLG_IO, "File", "getFreesyndConf", ("Cannot find configuration file %s.", iniPath.c_str()));
            return false;
        } else {
            FSINFO(Log::k_FLG_IO, "File", "getFreesyndConf", ("Reading configuration from file %s.", iniPath.c_str()));
            std::ifstream in( iniPath.c_str() );

            if( !in ) {
                return false;
            }

            in >> freesyndIni;
        }
    #endif
        return true;
    }

    /*!
    * Set iniFullPath with the absolute path to the freesynd.ini file.
    * \param iniFolder If empty that means we use a default folder. Else use the value.
    * \param iniFullPath Result of the full path
    */
    void File::getIniFullPath(const std::string& iniFolder, fs::path& iniFullPath) {
        if (iniFolder.size() == 0) {
    #ifdef _WIN32
        // Under windows config file is in the same directory as freesynd.exe
        iniFullPath.assign(exeFolder());
        // Since we're defaulting to the exe's folder, no need to try to create a directory.
    #else
        // should only be on Linux
        iniFullPath.assign(FS_ETC_DIR);
    #endif

        } else {
            iniFullPath.assign(iniFolder);
        }

        iniFullPath /= "freesynd.ini";
    }

    bool File::getOrCreateUserConfFolder(const std::string& userConfFolder) {
        if (userConfFolder.size() != 0) {
            // The user has given a path using the cli so use it
            userConfFolderPath_ = userConfFolder;
            if (!fs::exists(userConfFolderPath_)) {
                FSERR(Log::k_FLG_GFX, "File", "getOrCreateUserConfFolder", ("Directory does not exist: %s.", userConfFolder.c_str()));
                return false;
            }

            return true;
        }

        // We need to use default path and create folder if it does not exist
        // On all platforms, we use the user's home folder.
    #ifdef _WIN32
        // on Windows, home is defined using USERPROFILE env variable
        userConfFolderPath_ = getenv("USERPROFILE");
    #else
        // Under unix (and Mac) it's in the user home directory
        userConfFolderPath_ = getenv("HOME");
    #endif

        userConfFolderPath_ /= ".freesynd";

        if (!fs::exists(userConfFolderPath_)) {
            LOG(Log::k_FLG_IO, "File", "getOrCreateUserConfFolder", ("Creating user config folder %s", userConfFolderPath_.string().c_str()));
            if (!fs::create_directories(userConfFolderPath_)) {
                FSERR(Log::k_FLG_GFX, "File", "getOrCreateUserConfFolder", ("Could not create user conf folder."));
                return false;
            }
        }

        return true;
    }

    void File::getDefaultSaveFolder(std::string& confFolderPath) {
        fs::path savepath = userConfFolderPath_ / "save";
        confFolderPath.append(savepath.string());
    }

    bool File::getUserConfFullPath(fs::path& confFullPath) {
        confFullPath = userConfFolderPath_ / "user.conf";

        // Test if file exists
        return fs::exists(confFullPath);
    }

    /*!
    * The methods returns a string composed of the root path and given file name.
    * No control is made on the result format or file existence.
    * \param filename The relative path to one of the original data files.
    * \param uppercase If true, the resulting string will uppercased.
    */
    std::string File::getOriginalDataFullPath(const std::string& filename, bool uppercase) {
        std::string second_part = filename;

        if (uppercase) {
            std::transform(second_part.begin(), second_part.end(), second_part.begin(),
                        [](unsigned char c){
                            return (std::toupper(c)); }
                    );
        } else {
            std::transform(second_part.begin(), second_part.end(), second_part.begin(),
                        [](unsigned char c){
                            return (std::tolower(c)); }
                    );
        }


        return (dataPath_ / second_part).string();
    }

    /*!
    * The methods returns a string composed of the root path and given file name.
    * No control is made on the result format or file existence.
    * \param filename The relative path to one of our data files.
    */
    std::string File::getFreesyndDataFullPath(const std::string& filename) {
        return (ourDataPath_ / filename).string();
    }

    void File::getFullPathForSaveSlot(int slot, std::string &path) {
        std::ostringstream filename;
        if (slot < 10) {
            filename << "0";
        }
        filename << slot << ".fsg";

        path.assign((savePath_ / filename.str()).string());
    }

    /*!
    * \return NULL if file cannot be read.
    */
    uint8 *File::loadOriginalFileToMem(const std::string& filename, size_t &filesize) {
        // try lowercase, then uppercase.
        FILE *fp = fopen(getOriginalDataFullPath(filename, false).c_str(), "rb");
        if (!fp) fp = fopen(getOriginalDataFullPath(filename, true).c_str(), "rb");

        if (fp) {
            fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            if (size >= 0) {
                filesize = static_cast<size_t>(size);
                uint8 *mem = new uint8[filesize + 1];
                mem[filesize] = '\0';
                fseek(fp, 0, SEEK_SET);
                size_t  n = fread(mem, 1, filesize, fp);
                if (n == 0) {
                    Error::setError(Log::k_FLG_IO, "File", "loadFileToMem", "File '{}' in path {} is empty", filename, dataPath_.string());
                    delete[] mem;
                    mem = NULL;
                }
                fclose(fp);
                return mem;
            }
        }

        // If we're here, there's a problem
        Error::setError(Log::k_FLG_IO, "File", "loadFileToMem", "Couldn't open file '{}' in path {}", filename, dataPath_.string());

        if (fp) {
            fclose(fp);
        }

        filesize = 0;
        return NULL;
    }

    FILE *File::openOriginalFile(const std::string& filename) {
        // try lowercase, then uppercase.
        FILE *fp = fopen(getOriginalDataFullPath(filename, false).c_str(), "r");
        if (!fp) fp = fopen(getOriginalDataFullPath(filename, true).c_str(), "r");
        return fp;
    }

    /*!
    * Adds a trailing slash if needed to the given string
    */
    void File::addMissingSlash(std::string& str) {
        if (str[str.length() - 1] != '/') str.push_back('/');
    }

    void File::setOriginalDataFolder(const std::string& path) {
        if (path.size() != 0) {
            dataPath_ = path;
        } else {

    #if defined(__APPLE__)
        // Default: original data lives inside the app bundle at Resources/data
        if (!getResourcePath(dataPath_)) {
            FSERR(Log::k_FLG_GFX, "File", "setOriginalDataFolder", ("Unable to locate app bundle resources.\n"));
        }
        dataPath_ /= "data";
    #else
        // Under Windows/unix it's in the same directory as our data
        dataPath_ = ourDataPath_;
    #endif
        }
        LOG(Log::k_FLG_IO, "File", "setOriginalDataPath", ("set data path to %s", dataPath_.string().c_str()));
    }


    /*!
    * @brief
    * @param path
    */
    void File::setFreesyndDataFolder(const std::string& path) {
        if (path.size() != 0) {
            ourDataPath_ = path;
        } else {
            // User did not set a path so use default one
    #ifdef _WIN32
        // On windows default installation of data is in the folder where the exe is.
        ourDataPath_ = exeFolder();
        ourDataPath_ /= "data";
    #elif defined(__APPLE__)
        if (!getResourcePath(ourDataPath_)) {
            FSERR(Log::k_FLG_GFX, "File", "getDefaultFreesyndDataFolder", ("Unable to locate app bundle resources.\n"));
        }
    #else
        // Under unix it's in the data directory
        ourDataPath_ = FS_DATA_DIR;
    #endif
        }
        LOG(Log::k_FLG_IO, "File", "setOurDataPath", ("set our data path to %s", ourDataPath_.string().c_str()));
    }

    void File::upsertSaveDataFolder(const std::string& path) {
        savePath_ = path;

        if (!fs::exists(savePath_)) {
            if (!fs::create_directories(savePath_)) {
                FSERR(Log::k_FLG_GFX, "File", "upsertSaveDataFolder", ("Could not create save folder.\n"));
            }
        }

        LOG(Log::k_FLG_IO, "File", "setSaveDataFolder", ("set save path to %s", path.c_str()));
    }

    uint8 *File::loadOriginalFile(const std::string& filename, size_t &filesize) {
        uint8 *data = loadOriginalFileToMem(filename, filesize);
        if (data) {
            if (rnc::isRncCompressed(data)) {    //File is RNC compressed
                rnc::RncRetCode result = rnc::unpackedLength(data, filesize );

                if (result != 0) {
                    Error::setError(Log::k_FLG_IO, "File", "loadFile", "Error reading length for file {} : {}", filename, rnc::errorString(result));
                    filesize = 0;
                } else if (filesize == 0) {
                    Error::setError(Log::k_FLG_IO, "File", "loadFile", "Read length is zero for file '{}'", filename);
                }

                uint8_t *buffer = new uint8[filesize + 1];
                buffer[filesize] = '\0';
                size_t realSize = 0;

                result = rnc::unpack(data, buffer, realSize);
                delete[] data;

                if (result < 0) {
                    Error::setError(Log::k_FLG_IO, "File", "loadFile", "Error loading file '{}': {}", filename, rnc::errorString(result));
                    filesize = 0;
                    delete[] buffer;
                }

                if (realSize != filesize) {
                    Error::setError(Log::k_FLG_IO, "File", "loadFile", "Uncompressed size mismatch for file '{}'", filename);
                    filesize = 0;
                    delete[] buffer;
                }

                return buffer;
            }
        }
        return data;
    }

    /** \brief
     *
     * \param filename const std::string&
     * \param files std::vector<std::string>&
     * \return void
     *
     */
    void File::addSaveFilenameAtIndex(const fs::path& filename, std::vector<std::string> &files) {

        if (filename.extension().compare(".fsg") == 0) {
            std::istringstream iss( filename.stem().string() );
            size_t index;
            iss >> index;
            if (index < 10) {
                fs_utl::PortableFile infile;
                infile.open_to_read(filename.string().c_str());

                if (infile) {
                    // FIXME: detect original game saves
                    // Read version first
                    unsigned char vMaj = infile.read8();
                    unsigned char vMin = infile.read8();
                    fs_utl::FormatVersion v(vMaj, vMin);
                    // Read slot name
                    if (v == 0x0100) {
                        files[index] = infile.read_string(25, true);
                    } else {
                        files[index] = infile.read_string(31, true);
                    }
                }
            }
        }
    }

    /*!
    * Returns the list of names to display in load/save menu.
    * \param files
    */
    void File::getGameSavedNames(std::vector<std::string> &files) {
        for (const auto & entry : fs::directory_iterator(savePath_)) {
            addSaveFilenameAtIndex(entry.path(), files);
        }
    }

    bool File::testOriginalData() {

        FSINFO(Log::k_FLG_IO, "File", "testOriginalData", ("Testing original Syndicate data..."));

        std::string crcflname = File::getFreesyndDataFullPath("ref/original_data.crc");
        std::ifstream od(crcflname.c_str());
        if (od.fail()) {
            FSERR(Log::k_FLG_IO, "File", "testOriginalData",
                ("Checksums file for original data is not found. Look at INSTALL/README file for possible solutions."));
            return false;
        }

        bool rsp = true;
        while (od) {
            std::string line;
            std::getline(od, line);
            if (line.size() > 0) {
                std::string::size_type pos = line.find(' ');
                if (pos != std::string::npos) {
                    // skipping commented
                    if (line[0] == '#' || line[0] == ';')
                        continue;
                    std::string flname = line.substr(0, pos);
                    std::string str_crc32 = line.substr(pos+1);
                    uint64_t ui_crc32 = std::stoul(str_crc32, nullptr, 16);
                    size_t sz;
                    uint8 *data = File::loadOriginalFileToMem(flname, sz);
                    if (!data) {
                        FSERR(Log::k_FLG_IO, "App", "testOriginalData", ("file not found \"%s\" \"%s\"\nLook at INSTALL/README file for possible solutions.", getOriginalDataFullPath(flname, false).c_str(), flname.c_str()));

                        rsp = false;
                        continue;
                    }
                    if (ui_crc32 != CRC::Calculate(data, sz, CRC::CRC_32())) {
                        rsp = false;
                        FSERR(Log::k_FLG_IO, "App", "testOriginalData", ("file test failed \"%s\"", flname.c_str()));
                    }
                    delete[] data;
                }
            }
        }
        if (rsp) {
            FSINFO(Log::k_FLG_IO, "App", "testOriginalData", ("Test passed. CRC32 for data is correct."));
        } else {
            FSERR(Log::k_FLG_IO, "File", "testOriginalData", ("failed to test original Syndicate data..."))
        }
        od.close();

        return rsp;
    }

};
/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
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

#ifndef PORTABLEFILE_H
#define PORTABLEFILE_H

#include <string>
#include <vector>
#include <fstream>
#include "fs-utils/common.h"

namespace fs_utl {

/*!
 * Portable file class.  Simplifies implementation of portable file formats.
 *
 * NOTE: does not inherit from std::fstream to avoid any usage which might
 * circumvent endian-aware functionality.
 */
class PortableFile {
public:
    PortableFile();
    void open_to_read(const char *path);
    void open_to_write(const char *path);
    void open_to_overwrite(const char *path);

    operator bool() const;
    bool operator !() const;
    bool big_endian() const; // defaults to true
    void set_big_endian(bool value); // false sets little-endian
    void set_system_endian(); // use the native endian-ness of the system

    void skip(int64_t bytes_forward);
    void seek(int64_t byte_position);
    void rewind(int64_t bytes_backward);
    int64_t offset();

    void write64(uint64_t value);
    void write32(uint32_t value);
    void write16(uint16_t value);
    void write8(uint8_t value);
    void write8b(bool value);

    void write_float(float value);
    void write_double(double value);

    void write_string(const std::string& value, size_t length); // nul-padded if length > value.length
    void write_variable_string(const std::string& value, bool nul_terminate);

    void write_zeros(size_t length);

    uint64_t read64();
    uint32_t read32();
    int32_t reads32();
    uint16_t read16();
    uint8_t read8();
    bool read8b();

    float read_float();
    double read_double();

    std::string read_string(); // stops on and consumes a nul
    std::string read_string(int length, bool strip_nul); // reads length bytes exactly

private:
    std::fstream f_;
    bool big_endian_;
};

};

#endif

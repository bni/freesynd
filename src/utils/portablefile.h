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
#include "common.h"

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

    void skip(int64 bytes_forward);
    void seek(int64 byte_position);
    void rewind(int64 bytes_backward);
    int64 offset();

    void write64(uint64 value);
    void write32(uint32 value);
    void write16(uint16 value);
    void write8(uint8 value);
    void write8b(bool value);

    void write_float(float value);
    void write_double(double value);

    void write_string(const std::string& value, size_t length); // nul-padded if length > value.length
    void write_variable_string(const std::string& value, bool nul_terminate);

    void write_zeros(size_t length);

    uint64 read64();
    uint32 read32();
    uint16 read16();
    uint8 read8();
    bool read8b();

    float read_float();
    double read_double();

    std::string read_string(); // stops on and consumes a nul
    std::string read_string(size_t length, bool strip_nul); // reads length bytes exactly

private:
    std::fstream f_;
    bool big_endian_;
};

#endif

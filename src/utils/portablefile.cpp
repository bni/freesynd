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

#include "portablefile.h"

// runtime endianness test
static const uint32 endianness_one = 1;
static const unsigned char * const endianness_test_ptr = (unsigned char *)&endianness_one;
#define system_big_endian (endianness_test_ptr[0] == 0)

// We could use htonl and friends, at least for 16- and 32-bit swaps on Intel,
// but on Windows those are not macros or inline functions and require linking
// against the winsock libraries.  To avoid the extra dependency, I'm using
// the generic macros below on all platforms.
#ifdef swap64
#undef swap64
#endif
#define swap64(x) ( \
                     ((uint64)(x) << 56) | \
                    (((uint64)(x) << 40) & 0x00ff000000000000ULL) | \
                    (((uint64)(x) << 24) & 0x0000ff0000000000ULL) | \
                    (((uint64)(x) <<  8) & 0x000000ff00000000ULL) | \
                    (((uint64)(x) >>  8) & 0x00000000ff000000ULL) | \
                    (((uint64)(x) >> 24) & 0x0000000000ff0000ULL) | \
                    (((uint64)(x) >> 40) & 0x000000000000ff00ULL) | \
                     ((uint64)(x) >> 56) \
                  )

#ifdef swap32
#undef swap32
#endif
#define swap32(x) ( \
                     ((uint32)(x) << 24) | \
                    (((uint32)(x) <<  8) & 0x00ff0000) | \
                    (((uint32)(x) >>  8) & 0x0000ff00) | \
                     ((uint32)(x) >> 24) \
                  )

#ifdef swap16
#undef swap16
#endif
#define swap16(x) ( \
                     ((uint16)(x) <<  8) | \
                     ((uint16)(x) >>  8) \
                  )

PortableFile::PortableFile()
    : big_endian_(true)
{
}

void PortableFile::open_to_read(const char *path)
{
    f_.open(path, std::ios::in | std::ios::binary);
}

void PortableFile::open_to_write(const char *path)
{
    f_.open(path, std::ios::out | std::ios::binary);
}

void PortableFile::open_to_overwrite(const char *path)
{
    f_.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
}

bool PortableFile::big_endian() const
{
    return big_endian_;
}

void PortableFile::set_big_endian(bool value)
{
    big_endian_ = value;
}

void PortableFile::set_system_endian()
{
    big_endian_ = system_big_endian;
}

bool PortableFile::operator !() const
{
    return !f_.good();
}

PortableFile::operator bool() const
{
    return f_.good();
}

void PortableFile::skip(int64 bytes_forward)
{
    f_.seekg(bytes_forward, std::ios::cur);
}

void PortableFile::seek(int64 byte_position)
{
    f_.seekg(byte_position, std::ios::beg);
}

void PortableFile::rewind(int64 bytes_backward)
{
    f_.seekg(-bytes_backward, std::ios::cur);
}

int64 PortableFile::offset()
{
    return f_.tellg();
}

void PortableFile::write64(uint64 value)
{
    if (system_big_endian != big_endian_) {
        value = swap64(value);
    }
    f_.write((const char *)&value, 8);
}

void PortableFile::write32(uint32 value)
{
    if (system_big_endian != big_endian_) {
        value = swap32(value);
    }
    f_.write((const char *)&value, 4);
}

void PortableFile::write16(uint16 value)
{
    if (system_big_endian != big_endian_) {
        value = swap16(value);
    }
    f_.write((const char *)&value, 2);
}

void PortableFile::write8(uint8 value)
{
    f_.write((const char *)&value, 1);
}

void PortableFile::write8b(bool value)
{
    uint8 u8 = value ? 1 : 0;
    f_.write((const char *)&u8, 1);
}

void PortableFile::write_float(float value)
{
    uint32 *u32 = (uint32 *)&value;
    write32(*u32);
}

void PortableFile::write_double(double value)
{
    uint64 *u64 = (uint64 *)&value;
    write64(*u64);
}

// nul-padded if length > value.length
void PortableFile::write_string(const std::string& value, size_t length)
{
    if (length > value.size()) {
        f_.write(value.c_str(), value.size());
        write_zeros(length - value.size());
    } else {
        f_.write(value.c_str(), length);
    }
}

void PortableFile::write_variable_string(const std::string& value, bool nul_terminate)
{
    f_.write(value.c_str(), value.size());
    if (nul_terminate) f_.put(0);
}

void PortableFile::write_zeros(size_t length)
{
    size_t i;
    for (i = 0; i < length; i++) {
        f_.put(0);
    }
}

uint64 PortableFile::read64()
{
    uint64 value = 0;
    f_.read((char *)&value, 8);
    if (system_big_endian != big_endian_) {
        value = swap64(value);
    }
    return value;
}

uint32 PortableFile::read32()
{
    uint32 value = 0;
    f_.read((char *)&value, 4);
    if (system_big_endian != big_endian_) {
        value = swap32(value);
    }
    return value;
}

uint16 PortableFile::read16()
{
    uint16 value = 0;
    f_.read((char *)&value, 2);
    if (system_big_endian != big_endian_) {
        value = swap16(value);
    }
    return value;
}

uint8 PortableFile::read8()
{
    uint8 value = 0;
    f_.read((char *)&value, 1);
    return value;
}

bool PortableFile::read8b()
{
    uint8 u8 = 0;
    f_.read((char *)&u8, 1);
    return (u8 != 0);
}

float PortableFile::read_float()
{
    float value = 0.0;
    uint32 *u32 = (uint32 *)&value;
    *u32 = read32();
    return value;
}

double PortableFile::read_double()
{
    double value = 0.0;
    uint64 *u64 = (uint64 *)&value;
    *u64 = read64();
    return value;
}

// stops on and consumes a nul
std::string PortableFile::read_string()
{
    std::string value;
    char buf[256];
    while (f_.good()) {
        f_.get(buf, 256, '\0');
        int n = (int)f_.gcount();
        value.append(buf, n);
        if (f_.peek() == '\0') {
            break;
        }
    }
    return value;
}

// reads length bytes exactly
std::string PortableFile::read_string(size_t length, bool strip_nul)
{
    std::string value;
    char buf[256];
    while (f_.good() && length) {
        int n = (length < 256) ? length : 256;
        f_.read(buf, n);
        length -= n;
        value.append(buf, (uint32)f_.gcount());
        if (f_.fail()) break;
    }

    if (strip_nul) {
        size_t n = value.find_last_not_of('\0');
        if (n != std::string::npos) {
            n++;
            if (n < value.size()) {
                value.erase(n);
            }
        }
    }
    return value;
}


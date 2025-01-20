/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>
 *   Copyright (C) 2023-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef VERSION_H
#define VERSION_H

namespace fs_utl {
    class FormatVersion
    {
        public:
            FormatVersion(unsigned char vMaj, unsigned char vMin)
                : major_version_(vMaj), minor_version_(vMin)
            {
            }

            inline int majorVersion() const { return major_version_; }
            inline int minorVersion() const { return minor_version_; }

            // example: v1.1 has combined value of 0x0101
            inline int combined() const {
                return ((major_version_ << 8) | minor_version_);
            }

            /*!
            * Return true if this version is greater than the given
            * major.minor version.
            * \param vMaj Major version
            * \param vMin Minor version
            */
            inline bool gt(unsigned char vMaj, unsigned char vMin) {
                if (major_version_ > vMaj) {
                    return true;
                } else if (major_version_ == vMaj) {
                    return minor_version_ > vMin;
                } else {
                    return false;
                }
            }

            inline bool operator==(int value) const { return combined() == value; }
            inline bool operator!=(int value) const { return combined() != value; }

        private:
            int major_version_;
            int minor_version_;
    };

};

#endif

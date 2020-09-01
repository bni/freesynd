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

#ifndef VERSION_H
#define VERSION_H

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

#endif

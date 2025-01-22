#
#  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
#
#   Copyright (C) 2023-2025  Benoit Blancard <benblan@users.sourceforge.net>
#
#   This program is free software: you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as 
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of 
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#  along with this program. If not, see <https://www.gnu.org/licenses/>. 
# 

# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Install data directory
if (APPLE)
    # On Mac, the data folder is a part of the app bundle.
    # Defined here but not used cause I can't move the folders
    set(MACOS_ASSETS_DATA 
        "${CMAKE_CURRENT_SOURCE_DIR}/cursors/cursors.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/lang/english.lng"
        "${CMAKE_CURRENT_SOURCE_DIR}/lang/french.lng"
        "${CMAKE_CURRENT_SOURCE_DIR}/lang/german.lng"
        "${CMAKE_CURRENT_SOURCE_DIR}/lang/italian.lng"
        "${CMAKE_CURRENT_SOURCE_DIR}/ref/original_data.crc"
        "${CMAKE_CURRENT_SOURCE_DIR}/ref/research.dat"
        "${CMAKE_CURRENT_SOURCE_DIR}/ref/weapons.dat"
        PARENT_SCOPE)

    set_source_files_properties(${MACOS_ASSETS_DATA}
        PROPERTIES MACOSX_PACKAGE_LOCATION "data")
elseif(WIN32)
    install (DIRECTORY . DESTINATION "data"
    PATTERN "freesynd.ini" EXCLUDE
    PATTERN "CMakeLists.txt" EXCLUDE)

    # Put the freesynd file in the same directory as the freesynd.exe
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini DESTINATION ".")
else ()
    install (DIRECTORY . DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/freesynd"
            PATTERN "freesynd.ini" EXCLUDE
            PATTERN "CMakeLists.txt" EXCLUDE)

    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini DESTINATION "${FS_ETC_DIR}")
endif()

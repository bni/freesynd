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

set(HEADER_LIST
    "${FreeSynd_SOURCE_DIR}/utils/include/fs-utils/common.h"
    "${FreeSynd_SOURCE_DIR}/utils/include/fs-utils/log/log.h"
    "${FreeSynd_SOURCE_DIR}/utils/include/fs-utils/io/dernc.h"
    "${FreeSynd_SOURCE_DIR}/utils/include/fs-utils/io/file.h"
    "${FreeSynd_SOURCE_DIR}/utils/include/fs-utils/io/configfile.h"
    "${FreeSynd_SOURCE_DIR}/utils/include/fs-utils/io/portablefile.h"
    "${FreeSynd_SOURCE_DIR}/utils/include/fs-utils/io/formatversion.h"
    "${FreeSynd_SOURCE_DIR}/utils/include/fs-utils/misc/singleton.h"
    "${FreeSynd_SOURCE_DIR}/utils/include/fs-utils/misc/seqmodel.h"
    "${FreeSynd_SOURCE_DIR}/utils/include/fs-utils/misc/timer.h"
    )

set(SOURCE_LIST
    "${FreeSynd_SOURCE_DIR}/utils/src/log.cpp"
    "${FreeSynd_SOURCE_DIR}/utils/src/file.cpp"
    "${FreeSynd_SOURCE_DIR}/utils/src/configfile.cpp"
    "${FreeSynd_SOURCE_DIR}/utils/src/portablefile.cpp"
    "${FreeSynd_SOURCE_DIR}/utils/src/dernc.cpp"
    "${FreeSynd_SOURCE_DIR}/utils/src/seqmodel.cpp"
    )

# Definition of the fs_utils library - will be static or dynamic based on user setting
add_library(fs_utils ${SOURCE_LIST} ${HEADER_LIST})
add_library(Freesynd::Utils ALIAS fs_utils)

find_package(CRCpp REQUIRED)

target_link_libraries (fs_utils PRIVATE freesynd_warnings crcpp::crcpp)

# We need this directory, and users of our library will need it too (ie PUBLIC)
target_include_directories(fs_utils PUBLIC include)

# Set standard definitions for all platforms.
target_compile_definitions(fs_utils PRIVATE FS_ETC_DIR=\"${FS_ETC_DIR}\")
target_compile_definitions(fs_utils PRIVATE FS_DATA_DIR=\"${FS_DATA_DIR}\")

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/utils/include"
  FILES ${HEADER_LIST})



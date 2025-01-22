#
#  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
#
#   Copyright (C) 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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
set(CPACK_PACKAGE_DESCRIPTION "This package will install the Freesynd application.")

#set(CPACK_PACKAGE_ICON "/icon/sword.png")

set(CPACK_PACKAGE_CHECKSUM SHA256)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")

# Resulting package will be generated in this directory
set(CPACK_PACKAGE_DIRECTORY "PackageOutput")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})

set(CPACK_STRIP_FILES YES)

set(
    CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

#
# Debian specific variables
#
set(CPACK_PACKAGE_CONTACT "benblan@sourceforge.net")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Benoit Blancard <${CPACK_PACKAGE_CONTACT}>")

set(CPACK_DEBIAN_PACKAGE_SECTION "games")

set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

#
# Debian specific variables
#
set(CPACK_DMG_BACKGROUND_IMAGE
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/dmg/freesynd-DMGBackground.tiff")

set(CPACK_DMG_DS_STORE_SETUP_SCRIPT
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/dmg/AppDMGSetup.scpt")

set(CPACK_DMG_SLA_USE_RESOURCE_FILE_LICENSE ON)
set(CPACK_DMG_SLA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/packaging/dmg/")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/dmg/license.txt")
set(CPACK_DMG_SLA_LANGUAGES English)
set(CPACK_DMG_VOLUME_NAME "${CMAKE_PROJECT_NAME}")

# that is if you want every group to have its own package,
# although the same will happen if this is not set (so it defaults to ONE_PER_GROUP)
# and CPACK_DEB_COMPONENT_INSTALL is set to YES
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)#ONE_PER_GROUP)
# without this you won't be able to pack only specified component
set(CPACK_DEB_COMPONENT_INSTALL YES)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS YES)

#
# Source packaging specific variables
#

# We'll generate those two source file formats
set(CPACK_SOURCE_GENERATOR "ZIP")

set(CPACK_SOURCE_IGNORE_FILES
        /build.*
        /\\\\.git/
        /\\\\.gitignore
        game/version.h)

# Generator selection per platform
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(CPACK_GENERATOR ZIP)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(CPACK_GENERATOR DragNDrop)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(CPACK_GENERATOR DEB)
else ()
  set(CPACK_GENERATOR TGZ)
endif ()

# Sets CPack
include(CPack)

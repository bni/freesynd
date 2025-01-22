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

if (APPLE)
    set(MACOS_ASSETS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon/sword.icns" PARENT_SCOPE)

    # MACOSX_PACKAGE_LOCATION doesn't seem to work -> assets are copied directly under Resources dir
    #set_source_files_properties(${MACOS_ASSETS_ICON}
    #    PROPERTIES MACOSX_PACKAGE_LOCATION "icon")
    
elseif (WIN32)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../README" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../AUTHORS" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../COPYING" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../INSTALL" DESTINATION .)

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../extern/png/lib/" DESTINATION .
            FILES_MATCHING PATTERN "*.dll")

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../extern/sdl2/lib/${FREESYND_ARCH}/" DESTINATION .
            FILES_MATCHING PATTERN "*.dll")

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../extern/SDL2_image/lib/${FREESYND_ARCH}/" DESTINATION .
            FILES_MATCHING PATTERN "*.dll")

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../extern/SDL2_mixer/lib/${FREESYND_ARCH}/" DESTINATION .
            FILES_MATCHING PATTERN "*.dll")
else ()
    # Linux
    # Generate a changelog file
    add_custom_command(
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog"
        COMMAND gzip -cn9 "${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog" > "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
    )

    add_custom_target(debian ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
    )

    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/debian/copyright"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
    )

endif ()


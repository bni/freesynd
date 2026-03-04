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

# Add this folder to the include directories.
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

set (SOURCES
	app.cpp
	core/gamesession.cpp
	core/gamecontroller.cpp
	freesynd.cpp
	menus/agentselectorrenderer.cpp
	menus/maprenderer.cpp
	menus/minimaprenderer.cpp
	menus/briefmenu.cpp
	menus/confmenu.cpp
	menus/debriefmenu.cpp
	menus/gameplaymenu.cpp
	menus/loadingmenu.cpp
	menus/loadsavemenu.cpp
	menus/mainmenu.cpp
	menus/mapmenu.cpp
	menus/gamemenufactory.cpp
	menus/researchmenu.cpp
	menus/selectmenu.cpp
	menus/squadselection.cpp
)

if (APPLE AND "${SDL_INCLUDE_DIR}" MATCHES ".framework")
	# When using the official SDL 1.2 framework for Mac,
	# you must link in an implementation of SDLmain.
	set (SOURCES ${SOURCES} SDLMain.m)
endif()

set(HEADERS
    version.h
	app.h
	core/gamesession.h
	core/gamecontroller.h
	menus/agentselectorrenderer.h
	menus/maprenderer.h
	menus/minimaprenderer.h
	menus/briefmenu.h
	menus/confmenu.h
	menus/debriefmenu.h
	menus/gameplaymenu.h
	menus/loadingmenu.h
	menus/loadsavemenu.h
	menus/mainmenu.h
	menus/mapmenu.h
	menus/gamemenufactory.h
	menus/gamemenuid.h
	menus/researchmenu.h
	menus/selectmenu.h
	menus/squadselection.h
)

source_group(Menus REGULAR_EXPRESSION menus/*)
source_group(Core REGULAR_EXPRESSION core/*)

# When building for Mac, it will build as a Bundle
add_executable (freesynd MACOSX_BUNDLE ${SOURCES} ${HEADERS})

target_link_libraries (freesynd PRIVATE freesynd_warnings Freesynd::Utils Freesynd::Engine Freesynd::Kernel)

# This option enables the calculation of the frame per second counter and
# display
option(FS_TRACK_FPS "Track FPS" OFF)

if (FS_TRACK_FPS)
    target_compile_definitions(freesynd PRIVATE "FS_TRACK_FPS")
endif()

# We only define an install target if we're doing a release build.
if (APPLE)
    install (TARGETS freesynd BUNDLE DESTINATION .)

	set(MACOS_ASSETS "${MACOS_ASSETS_ICON}")
	target_sources(freesynd PUBLIC ${MACOS_ASSETS})

	# I use this custom command to copy the data folder under the Resources directy
	# I can't make it work using MACOSX_PACKAGE_LOCATION source property
	add_custom_command(TARGET freesynd POST_BUILD
  						COMMAND ${CMAKE_COMMAND} -E copy_directory
						"${CMAKE_SOURCE_DIR}/data"
						"$<TARGET_FILE_DIR:freesynd>/../Resources") 

	# Remove files from what has been copied
	add_custom_command(TARGET freesynd POST_BUILD
						COMMAND ${CMAKE_COMMAND} -E rm
					  "$<TARGET_FILE_DIR:freesynd>/../Resources/CMakeLists.txt"
					  "$<TARGET_FILE_DIR:freesynd>/../Resources/Freesynd.ini")

	find_program(DYLIBBUNDLER_EXE dylibbundler REQUIRED)

	add_custom_command(TARGET freesynd POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory
				"$<TARGET_BUNDLE_DIR:freesynd>/Contents/Frameworks"
		COMMAND ${DYLIBBUNDLER_EXE}
				-od -b -of
				-x "$<TARGET_BUNDLE_DIR:freesynd>/Contents/MacOS/FreeSynd"
				-d "$<TARGET_BUNDLE_DIR:freesynd>/Contents/Frameworks/"
				-p "@loader_path/../Frameworks/"
		COMMENT "Bundling dylibs into FreeSynd.app/Contents/Frameworks"
		VERBATIM)
		
	# Override certain properties to make the freesynd
	# executable into an application bundle for OS X.
	set_target_properties (freesynd PROPERTIES
		OUTPUT_NAME "FreeSynd"
	#	MACOSX_BUNDLE true
		MACOSX_BUNDLE_BUNDLE_NAME "FreeSynd"
		MACOSX_BUNDLE_ICON_FILE "sword.icns"
		MACOSX_BUNDLE_GUI_IDENTIFIER "com.freesynd.FreeSynd"
		MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
		MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
		MACOSX_BUNDLE_COPYRIGHT "© 2024 FreeSynd Team"
		RESOURCE "${MACOS_ASSETS}"
	)
	
elseif (WIN32)
    install (TARGETS freesynd DESTINATION .)

    # Runtime DLLs live in the target sysroot bin, not the host compiler bin.
    # MINGW_PREFIX and MINGW_TRIPLE are defined in cmake/toolchain-mingw64.cmake.
    set(MINGW_RUNTIME_DIR "${MINGW_PREFIX}/${MINGW_TRIPLE}/bin")

    add_custom_command(TARGET freesynd POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CMAKE_SOURCE_DIR}/extern/sdl2/lib/${FREESYND_ARCH}/SDL2.dll"
                "${CMAKE_SOURCE_DIR}/extern/SDL2_image/lib/${FREESYND_ARCH}/SDL2_image.dll"
                "${CMAKE_SOURCE_DIR}/extern/SDL2_mixer/lib/${FREESYND_ARCH}/SDL2_mixer.dll"
                "${MINGW_RUNTIME_DIR}/libgcc_s_seh-1.dll"
                "${MINGW_RUNTIME_DIR}/libstdc++-6.dll"
                "${MINGW_RUNTIME_DIR}/libwinpthread-1.dll"
                "$<TARGET_FILE_DIR:freesynd>"
        COMMENT "Copying SDL2 and MinGW runtime DLLs next to FreeSynd.exe"
        VERBATIM)

    add_custom_command(TARGET freesynd POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/data"
                "$<TARGET_FILE_DIR:freesynd>/data"
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CMAKE_SOURCE_DIR}/data/freesynd.ini"
                "$<TARGET_FILE_DIR:freesynd>/freesynd.ini"
        COMMAND ${CMAKE_COMMAND} -E rm
                "$<TARGET_FILE_DIR:freesynd>/data/CMakeLists.txt"
                "$<TARGET_FILE_DIR:freesynd>/data/freesynd.ini"
        COMMENT "Copying data folder and freesynd.ini next to FreeSynd.exe"
        VERBATIM)
else ()
    # We use the default installation directory
    install (TARGETS freesynd DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
endif ()



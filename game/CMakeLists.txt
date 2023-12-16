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
	menus/flimenu.cpp
	menus/gameplaymenu.cpp
	menus/loadingmenu.cpp
	menus/loadsavemenu.cpp
	menus/logoutmenu.cpp
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
	menus/flimenu.h
	menus/gameplaymenu.h
	menus/loadingmenu.h
	menus/loadsavemenu.h
	menus/logoutmenu.h
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
source_group(Model REGULAR_EXPRESSION model/*)
source_group(IA REGULAR_EXPRESSION ia/*)

add_executable (freesynd ${SOURCES} ${HEADERS})

target_link_libraries (freesynd PRIVATE Freesynd::Utils Freesynd::Engine Freesynd::Kernel)

# We only define an install target if we're doing a release build.
if (APPLE)
    install (TARGETS freesynd DESTINATION /Applications)
else ()
    install (TARGETS freesynd DESTINATION ${BIN_DIR})
endif ()

if (APPLE)
	# Override certain properties to make the freesynd
	# executable into an application bundle for OS X.
	set_target_properties (freesynd PROPERTIES
		OUTPUT_NAME "FreeSynd"
		MACOSX_BUNDLE true
		MACOSX_BUNDLE_BUNDLE_NAME "FreeSynd"
		MACOSX_BUNDLE_ICON_FILE "sword.icns"
		MACOSX_BUNDLE_GUI_IDENTIFIER "com.freesynd.FreeSynd"
		MACOSX_BUNDLE_SHORT_VERSION_STRING "${FREESYND_VERSION}"
		MACOSX_BUNDLE_COPYRIGHT "© 2011 FreeSynd Team"
	)
	include (tools/osx.cmake)
	copy_resource_file (freesynd "Copying icon" ${CMAKE_SOURCE_DIR}/icon/sword.icns)
	copy_resource_folder (freesynd "Copying data" ${CMAKE_SOURCE_DIR}/data)
	include_libs (freesynd "Including non-standard OS X libraries")
endif()


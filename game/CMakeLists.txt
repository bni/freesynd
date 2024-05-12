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

# When building for Mac, it will build as a Bundle
add_executable (freesynd MACOSX_BUNDLE ${SOURCES} ${HEADERS})

target_link_libraries (freesynd PRIVATE freesynd_warnings Freesynd::Utils Freesynd::Engine Freesynd::Kernel)

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
else ()
    # We use the default installation directory
    install (TARGETS freesynd DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
endif ()



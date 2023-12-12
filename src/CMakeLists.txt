# Add this folder to the include directories.
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

# Generate the ini header, add it's location to the include directories.
#include (tools/embed.cmake)
#embed_data (../freesynd.ini embedded_default_ini default_ini.h)
#include_directories (${CMAKE_CURRENT_BINARY_DIR})

set (SOURCES
#	agent.cpp
#	agentmanager.cpp
	app.cpp
	model/objectivedesc.cpp
#	model/weaponholder.cpp
	model/shot.cpp
	model/vehicle.cpp
	model/train.cpp
	model/weapon.cpp
#	model/research.cpp
	model/squad.cpp
	core/gamesession.cpp
	core/gamecontroller.cpp
	core/missionbriefing.cpp
	core/researchmanager.cpp
	ia/actions.cpp
	ia/behaviour.cpp
	freesynd.cpp
    ipastim.cpp
	static.cpp
#	mapmanager.cpp
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
	mission.cpp
	missionmanager.cpp
#	modmanager.cpp
	ped.cpp
	pedactions.cpp
	pedmanager.cpp
	pedpathfinding.cpp
#	weaponmanager.cpp
)

if (APPLE AND "${SDL_INCLUDE_DIR}" MATCHES ".framework")
	# When using the official SDL 1.2 framework for Mac,
	# you must link in an implementation of SDLmain.
	set (SOURCES ${SOURCES} SDLMain.m)
endif()

set(HEADERS
    version.h
#	agent.h
#	agentmanager.h
	app.h
    ipastim.h
#	mapmanager.h
#	static.h
	mission.h
	missionmanager.h
#	modmanager.h
#	modowner.h
#	path.h
#	pathsurfaces.h
	ped.h
	pedmanager.h
	resources.h
#	weaponmanager.h
	core/gameevent.h
	core/gamesession.h
	core/gamecontroller.h
	core/missionbriefing.h
	core/researchmanager.h
	ia/actions.h
	ia/behaviour.h
	model/vehicle.h
#	model/weapon.h
	model/objectivedesc.h
#	model/shot.h
#	model/weaponholder.h
	model/train.h
#	model/research.h
	model/squad.h
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
source_group(Editor REGULAR_EXPRESSION editor/*)
source_group(IA REGULAR_EXPRESSION ia/*)

add_executable (freesynd ${SOURCES} ${HEADERS})

target_link_libraries (freesynd PRIVATE Freesynd::Utils Freesynd::Engine Freesynd::Kernel)

# Use -DBIN_DIR to override binary installation location
if(NOT BIN_DIR)
    if(UNIX)
        SET(BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin")
    endif(UNIX)
endif(NOT BIN_DIR)

if (BUILD_DEV_TOOLS)
	set(DEV_TOOLS_HEADERS
		editor/editorapp.h
		editor/editormenufactory.h
		editor/editormenuid.h
		editor/logoutmenu.h
		editor/mainmenu.h
		editor/fontmenu.h
		editor/animmenu.h
		editor/searchmissionmenu.h
		editor/listmissionmenu.h)

	add_executable (dump
		dump.cpp
#		mapmanager.cpp
		static.cpp
#		agent.cpp
#		agentmanager.cpp
		ipastim.cpp
		pedmanager.cpp
		ped.cpp
		pedactions.cpp
		pedpathfinding.cpp
#		modmanager.cpp
		missionmanager.cpp
		model/vehicle.cpp
		model/train.cpp
#		weaponmanager.cpp
		core/gamecontroller.cpp
		core/gamesession.cpp
		core/missionbriefing.cpp
		core/researchmanager.cpp
#		model/research.cpp
		model/squad.cpp
		model/objectivedesc.cpp
		model/shot.cpp
#		model/weaponholder.cpp
		model/weapon.cpp
		ia/actions.cpp
		ia/behaviour.cpp
		mission.cpp
		editor/editorapp.cpp
		editor/editormenufactory.cpp
		editor/logoutmenu.cpp
		editor/mainmenu.cpp
		editor/fontmenu.cpp
		editor/animmenu.cpp
		editor/searchmissionmenu.cpp
		editor/listmissionmenu.cpp
		${DEV_TOOLS_HEADERS}
	)

	target_link_libraries (dump PRIVATE Freesynd::Utils Freesynd::Engine Freesynd::Kernel)

else ()
	# We only define an install target if we're doing a release build.
	if (APPLE)
		install (TARGETS freesynd DESTINATION /Applications)
	elseif (UNIX)
		install (TARGETS freesynd DESTINATION ${BIN_DIR})
	endif ()
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


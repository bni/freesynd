cmake_minimum_required (VERSION 2.6)
project (FreeSynd)
set (FREESYND_VERSION "0.9")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Use -DDATA_DIR to override data location
if(NOT DATA_DIR)
    if(UNIX)
        SET(DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/freesynd/")
    endif(UNIX)
endif(NOT DATA_DIR)

# Set standard definitions for all platforms.
add_definitions (-DSYSTEM_SDL)
add_definitions (-DPREFIX=\"${DATA_DIR}\")

# Locate libs we depend on directly.
find_package (SDL2 REQUIRED)

find_package (SDL2_image REQUIRED)

find_package (SDL2_mixer REQUIRED)

find_package (PNG)
if (NOT PNG_FOUND)
	message (FATAL_ERROR "Unable to locate PNG")
endif ()

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "debug")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (BUILD_DEV_TOOLS TRUE)
else ()
	set (BUILD_DEV_TOOLS FALSE)
	# We only define an install target if we're doing a release build.
	# Furthermore, on Mac, the data folder is a part of the app bundle.
	if (UNIX)
		install (DIRECTORY data DESTINATION ${DATA_DIR} PATTERN .svn EXCLUDE)
	endif ()
endif ()

# Make sure we can include from the libs we found.
include_directories (${PNG_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})

# Build the sources in this subdirectory.
subdirs (src)


cmake_minimum_required (VERSION 3.22)

project(
  Freesynd
  VERSION 0.8
  DESCRIPTION "FreeSynd is a cross-platform, GPLed reimplementation of the classic Bullfrog game, Syndicate."
  LANGUAGES CXX)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h)

if (WIN32)
	# Search the bundled libs directory when using MSVC.
	set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
		${CMAKE_SOURCE_DIR}/extern/MSVC/lib
	)
	set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}
		${CMAKE_SOURCE_DIR}/extern/MSVC/include
	)
endif ()

if (MSVC)
	# Add extra definitions just for MSVC.
	add_definitions (-D_CONSOLE -D_CRT_SECURE_NO_WARNINGS)
	# Override default flags for release builds.
	set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /Oi /Ot /GL")
	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Oi /Ot /GL")
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
else ()
	# Set extra warnings.
    if (APPLE)
        set (EXTRA_C_FLAGS "-Wall -Wshadow -Wimplicit -Wno-multichar")
    else ()
        set (EXTRA_C_FLAGS "-Wall -Wshadow -Wno-multichar")
    endif ()

	set (EXTRA_CXX_FLAGS "${EXTRA_C_FLAGS} -Wreorder -Wnon-virtual-dtor")

	# Set debug flag.
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
endif ()

# Set extra compiler flags, possibly different per-platform.
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_FLAGS}")

# Use -DDATA_DIR to override data location
if(NOT DATA_DIR)
    if(UNIX)
        SET(DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/freesynd/")
    endif(UNIX)
endif(NOT DATA_DIR)

# Set standard definitions for all platforms.
add_definitions (-DPREFIX=\"${DATA_DIR}\")

set(CMAKE_CXX_STANDARD 23 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

# Build the sources in this subdirectory.
add_subdirectory(utils)
add_subdirectory(engine)
add_subdirectory(kernel)
subdirs (src)


cmake_minimum_required (VERSION 3.22)

project(
  Freesynd
  VERSION 0.8
  DESCRIPTION "FreeSynd is a cross-platform, GPLed reimplementation of the classic Bullfrog game, Syndicate."
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/game/version.h)

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

# Use -DBIN_DIR to override binary installation location
if(NOT BIN_DIR)
    if(UNIX)
        SET(BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin")
    endif(UNIX)
endif(NOT BIN_DIR)

# Use -DDATA_DIR to override data location
if(NOT DATA_DIR)
    if (WIN32)
        SET(DATA_DIR "${CMAKE_INSTALL_PREFIX}")
    elseif (UNIX)
        SET(DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/freesynd")
    endif()
endif(NOT DATA_DIR)

# Set standard definitions for all platforms.
add_definitions (-DPREFIX=\"${DATA_DIR}\")
if (APPLE)
    # On Mac, the data folder is a part of the app bundle.
else ()
    install (DIRECTORY data DESTINATION ${DATA_DIR})
endif()

# Generate a version of freesynd.ini with data dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini.in ${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini)
if (WIN32)
    # On windows it is install next to the freesynd exe file
    install(FILES freesynd.ini DESTINATION ${BIN_DIR})
elseif (UNIX)
    if($ENV{SUDO_USER})
        # If installed with sudo, use the home directory of the invoking user
        execute_process(
            COMMAND sh -c "eval echo ~$ENV{SUDO_USER}"
            OUTPUT_VARIABLE USER
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else()
        # If not installed with sudo, use the current user's home directory
        execute_process(
            COMMAND sh -c "echo $HOME"
            OUTPUT_VARIABLE USER_HOME
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND sh -c "echo $USER"
            OUTPUT_VARIABLE USER
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()

    # On linux, it's in the user directory
    install(FILES freesynd.ini DESTINATION "${USER_HOME}/.freesynd")

    # Change the owner of the file in case the install has been made with sudo
    install(CODE "execute_process(COMMAND sh -c \"chown ${USER} ${USER_HOME}/.freesynd\" )")
    install(CODE "execute_process(COMMAND sh -c \"chown ${USER} ${USER_HOME}/.freesynd/freesynd.ini\" )")

endif()


if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "debug")
endif ()

# Docs only available if this is the main app
find_package(Doxygen)
if(Doxygen_FOUND)
    add_subdirectory(docs)
else()
    message(STATUS "Doxygen not found, not building docs")
endif()

# Build the sources in this subdirectory.
add_subdirectory (utils)
add_subdirectory (engine)
add_subdirectory (kernel)
add_subdirectory (game)
add_subdirectory (editor)


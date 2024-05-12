cmake_minimum_required (VERSION 3.22)

project(
  Freesynd
  VERSION 0.8
  DESCRIPTION "Cross-platform, GPLed reimplementation of the classic Bullfrog game, Syndicate."
  HOMEPAGE_URL "https://freesynd.sourceforge.io/"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui, ccmake
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif ()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/game/version.h)

if (WIN32)
	# Search the bundled libs directory when using MSVC.
	set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
		${CMAKE_SOURCE_DIR}/extern/sdl2/lib/${FREESYND_ARCH}
        ${CMAKE_SOURCE_DIR}/extern/SDL2_mixer/lib/${FREESYND_ARCH}
        ${CMAKE_SOURCE_DIR}/extern/SDL2_image/lib/${FREESYND_ARCH}
        ${CMAKE_SOURCE_DIR}/extern/png/lib
	)
	set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}
		${CMAKE_SOURCE_DIR}/extern/sdl2/include
        ${CMAKE_SOURCE_DIR}/extern/png/include
        ${CMAKE_SOURCE_DIR}/extern/SDL2_mixer/include
        ${CMAKE_SOURCE_DIR}/extern/SDL2_image/include
	)
endif ()

#################################
# Compilation flags management
#################################

if (MSVC)
	# Add extra definitions just for MSVC.
	add_definitions (-D_CONSOLE -D_CRT_SECURE_NO_WARNINGS)
	# Override default flags for release builds.
	set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /Oi /Ot /GL")
	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Oi /Ot /GL")
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
endif ()

include(cmake/CompilerWarnings.cmake)

add_library(freesynd_warnings INTERFACE)

set_project_warnings(freesynd_warnings)

#################################
# Installation management
#################################

# Using GNU Coding Standards
include(GNUInstallDirs)

# FS_ETC_DIR controls the location of the freesynd.ini directory
if (NOT FS_ETC_DIR)
    if (UNIX)
        SET(FS_ETC_DIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}/freesynd")
    elseif (WIN32)
        SET(FS_ETC_DIR "${CMAKE_INSTALL_PREFIX}")
    endif()
endif (NOT FS_ETC_DIR)

# FS_DATA_DIR controls the location of the freesynd data
if (NOT FS_DATA_DIR)
    if (UNIX)
        SET(FS_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/freesynd")
    elseif (WIN32)
        SET(FS_DATA_DIR "${CMAKE_INSTALL_PREFIX}")
    endif()
endif (NOT FS_DATA_DIR)

#################################
# Specify the project's components
#################################

# Docs only available if this is the main app
find_package(Doxygen)
if(Doxygen_FOUND)
    add_subdirectory(docs)
else()
    message(STATUS "Doxygen not found, not building docs")
endif()

# Build the sources in these subdirectory.
add_subdirectory (packaging)
add_subdirectory (data)
add_subdirectory (utils)
add_subdirectory (engine)
add_subdirectory (kernel)
add_subdirectory (game)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # The editor is build only in Development mode
    add_subdirectory (editor)
endif()

#################################
# For all packaging instructions
# Only in Release mode
#################################
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(Packing)
endif()


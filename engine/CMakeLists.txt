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

set(HEADER_LIST
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/enginecommon.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/base_app.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/appcontext.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/config.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/system/system.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/cp437.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/dirtylist.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/font.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/fontmanager.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/logomanager.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/sprite.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/spritemanager.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/animationloader.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/animationmanager.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/tile.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/tilemanager.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/fstexture.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/gfx/animationplayer.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/sound/audio.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/sound/musicmanager.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/sound/sound.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/sound/soundmanager.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/io/keys.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/io/resources.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/menus/menu.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/menus/menumanager.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/menus/widget.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/menus/fliplayer.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/menus/flimenu.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/menus/logoutmenu.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/events/event.h"
    "${FreeSynd_SOURCE_DIR}/engine/include/fs-engine/events/default_events.h"
    )

set(SOURCE_LIST
    "${FreeSynd_SOURCE_DIR}/engine/src/base_app.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/appcontext.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/system/system.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/gfx/dirtylist.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/gfx/font.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/gfx/fontmanager.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/gfx/sprite.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/gfx/spritemanager.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/gfx/animationloader.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/gfx/animationmanager.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/gfx/animationplayer.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/gfx/logomanager.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/gfx/tile.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/gfx/tilemanager.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/sound/audio.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/sound/musicmanager.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/sound/soundmanager.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/menus/menu.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/menus/menumanager.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/menus/widget.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/menus/fliplayer.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/menus/flimenu.cpp"
    "${FreeSynd_SOURCE_DIR}/engine/src/menus/logoutmenu.cpp"
    )


find_package (PNG REQUIRED)
find_package (CLI11 REQUIRED)
find_package (utf8cpp REQUIRED)

# This is the main library for this directory
add_library(fs_engine ${SOURCE_LIST} ${HEADER_LIST})
add_library(Freesynd::Engine ALIAS fs_engine)

# We need this directory, and users of our library will need it too
target_include_directories(fs_engine PUBLIC include)

target_link_libraries(fs_engine PRIVATE freesynd_warnings Freesynd::Utils PNG::PNG CLI11::CLI11 utf8cpp)
target_link_libraries(fs_engine INTERFACE CLI11::CLI11 utf8cpp)

# The USE_SYSTEM_SDL option enables the creation of the fs_engine_sdl
# library. If not on, there will be an error as there is not other implementation
option(USE_SYSTEM_SDL "Use SDL Library" ON)

if (USE_SYSTEM_SDL)
    target_compile_definitions(fs_engine PRIVATE "SYSTEM_SDL")

    # Find the SDL2 library
    find_package (SDL2 REQUIRED)
    if (NOT SDL2_FOUND)
        message (FATAL_ERROR "Unable to locate SDL2 Library")
    endif ()

    find_package (SDL2_image)
    if (NOT SDL2_IMAGE_FOUND)
        message (FATAL_ERROR "Unable to locate SDL2_image")
    endif ()

    add_library(fs_engine_sdl STATIC
                  "${FreeSynd_SOURCE_DIR}/engine/src/system/sdl/system_sdl.cpp"
                  "${FreeSynd_SOURCE_DIR}/engine/src/system/sdl/fstexture_sdl.cpp"

                  "${FreeSynd_SOURCE_DIR}/engine/src/system/sdl/system_sdl.h"
                  "${FreeSynd_SOURCE_DIR}/engine/src/system/sdl/fstexture_sdl.h"
                )
    add_library(Freesynd::Engine::Sdl ALIAS fs_engine_sdl)

    target_include_directories(fs_engine_sdl PUBLIC src/system)
    target_include_directories(fs_engine_sdl PRIVATE include)

    target_link_libraries(fs_engine_sdl PUBLIC SDL2::Main SDL2::Image Freesynd::Utils)
    target_link_libraries(fs_engine_sdl PRIVATE freesynd_warnings utf8cpp)

    target_link_libraries(fs_engine PRIVATE Freesynd::Engine::Sdl)
endif()

# The USE_SDL_MIXER option enables the creation of the fs_engine_sdlmixer
# library. If not on, there will be the default Audio implementation
option(USE_SDL_MIXER "Use SDL Mixer library" ON)

if (USE_SDL_MIXER)

    find_package (SDL2_mixer REQUIRED)

    add_subdirectory(extern)

    add_library(fs_engine_sdlmixer STATIC
                  "${FreeSynd_SOURCE_DIR}/engine/src/sound/mixer/sdlmixeraudio.cpp"
                  "${FreeSynd_SOURCE_DIR}/engine/src/sound/mixer/sdlmixersound.cpp"

                  "${FreeSynd_SOURCE_DIR}/engine/src/sound/mixer/sdlmixeraudio.h"
                  "${FreeSynd_SOURCE_DIR}/engine/src/sound/mixer/sdlmixersound.h"
                )
    add_library(Freesynd::Engine::SdlMixer ALIAS fs_engine_sdlmixer)

    target_include_directories(fs_engine_sdlmixer PUBLIC src/sound)
    target_include_directories(fs_engine_sdlmixer PRIVATE include)
    target_link_libraries(fs_engine_sdlmixer PRIVATE freesynd_warnings Freesynd::Utils SDL2::Main SDL2::Mixer ADLMIDI)

    target_compile_definitions(fs_engine_sdl PRIVATE "HAVE_SDL_MIXER")

    target_link_libraries(fs_engine_sdl PUBLIC Freesynd::Engine::SdlMixer)
endif()


# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/engine/include"
  FILES ${HEADER_LIST})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()


set(HEADER_LIST
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/base_app.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/appcontext.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/config.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/system/system.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/gfx/cp437.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/gfx/dirtylist.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/gfx/font.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/gfx/fontmanager.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/gfx/screen.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/gfx/sprite.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/gfx/spritemanager.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/gfx/tile.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/gfx/tilemanager.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/audio.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/music.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/musicmanager.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/sound.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/soundmanager.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/xmidi.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/io/keys.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/io/resources.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/menus/menu.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/menus/menumanager.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/menus/widget.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/menus/fliplayer.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/events/event.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/events/default_events.h"
    )

set(SOURCE_LIST
    "${Freesynd_SOURCE_DIR}/engine/src/base_app.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/appcontext.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/system/system.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/dirtylist.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/font.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/fontmanager.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/screen.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/sprite.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/spritemanager.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/tile.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/tilemanager.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/sound/audio.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/sound/musicmanager.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/sound/soundmanager.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/sound/xmidi.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/menus/menu.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/menus/menumanager.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/menus/widget.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/menus/fliplayer.cpp"
    )


find_package (PNG REQUIRED)

# This is the main library for this directory
add_library(fs_engine ${SOURCE_LIST} ${HEADER_LIST})
add_library(Freesynd::Engine ALIAS fs_engine)

# We need this directory, and users of our library will need it too
target_include_directories(fs_engine PUBLIC include "${Freesynd_SOURCE_DIR}/extern/utfcpp")

target_link_libraries(fs_engine PRIVATE freesynd_warnings Freesynd::Utils PNG::PNG )

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
                  "${Freesynd_SOURCE_DIR}/engine/src/system/sdl/system_sdl.cpp"

                  "${Freesynd_SOURCE_DIR}/engine/src/system/sdl/system_sdl.h"
                )
    add_library(Freesynd::Engine::Sdl ALIAS fs_engine_sdl)

    target_include_directories(fs_engine_sdl PUBLIC src/system "${Freesynd_SOURCE_DIR}/extern/utfcpp")
    target_include_directories(fs_engine_sdl PRIVATE include)

    target_link_libraries(fs_engine_sdl PUBLIC SDL2::Main SDL2::Image Freesynd::Utils)
    target_link_libraries(fs_engine_sdl PRIVATE freesynd_warnings )

    target_link_libraries(fs_engine PRIVATE Freesynd::Engine::Sdl)
endif()

# The USE_SDL_MIXER option enables the creation of the fs_engine_sdlmixer
# library. If not on, there will be the default Audio implementation
option(USE_SDL_MIXER "Use SDL Mixer library" ON)

if (USE_SDL_MIXER)

    find_package (SDL2_mixer REQUIRED)

    add_library(fs_engine_sdlmixer STATIC
                  "${Freesynd_SOURCE_DIR}/engine/src/sound/mixer/sdlmixeraudio.cpp"
                  "${Freesynd_SOURCE_DIR}/engine/src/sound/mixer/sdlmixersound.cpp"
                  "${Freesynd_SOURCE_DIR}/engine/src/sound/mixer/sdlmixermusic.cpp"

                  "${Freesynd_SOURCE_DIR}/engine/src/sound/mixer/sdlmixeraudio.h"
                  "${Freesynd_SOURCE_DIR}/engine/src/sound/mixer/sdlmixersound.h"
                  "${Freesynd_SOURCE_DIR}/engine/src/sound/mixer/sdlmixermusic.h"
                )
    add_library(Freesynd::Engine::SdlMixer ALIAS fs_engine_sdlmixer)

    target_include_directories(fs_engine_sdlmixer PUBLIC src/sound)
    target_include_directories(fs_engine_sdlmixer PRIVATE include)
    target_link_libraries(fs_engine_sdlmixer PRIVATE freesynd_warnings Freesynd::Utils SDL2::Main SDL2::Mixer)

    target_compile_definitions(fs_engine_sdl PRIVATE "HAVE_SDL_MIXER")

    target_link_libraries(fs_engine_sdl PUBLIC Freesynd::Engine::SdlMixer)
endif()


# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/engine/include"
  FILES ${HEADER_LIST})

option(USE_INTRO_OGG "Use Ogg file for intro music instead of midi file" ON)
if (USE_INTRO_OGG)
    target_compile_definitions(fs_engine PRIVATE "USE_INTRO_OGG")
endif()

option(USE_ASSASSINATE_OGG "Use Ogg file for assassinate music instead of midi file" ON)
if (USE_ASSASSINATE_OGG)
    target_compile_definitions(fs_engine PRIVATE "USE_ASSASSINATE_OGG")
endif()

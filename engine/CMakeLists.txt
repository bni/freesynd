
set(HEADER_LIST
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/base_app.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/system.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/system_sdl.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/config.h"
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
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/sdlmixersound.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/music.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/musicmanager.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/sdlmixermusic.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/sound.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/soundmanager.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/sound/xmidi.h"
    "${Freesynd_SOURCE_DIR}/engine/include/fs-engine/io/keys.h"
    )

set(SOURCE_LIST
    "${Freesynd_SOURCE_DIR}/engine/src/base_app.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/system_sdl.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/dirtylist.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/font.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/fontmanager.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/screen.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/sprite.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/spritemanager.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/tile.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/gfx/tilemanager.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/sound/audio.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/sound/sdlmixersound.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/sound/sdlmixermusic.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/sound/musicmanager.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/sound/soundmanager.cpp"
    "${Freesynd_SOURCE_DIR}/engine/src/sound/xmidi.cpp"
    )

add_definitions (-DSYSTEM_SDL)

# Locate libs we depend on directly.
find_package (SDL)
if (NOT SDL_FOUND)
	message (FATAL_ERROR "Unable to locate SDL")
endif ()

find_package (SDL_image)
if (NOT SDLIMAGE_FOUND)
	message (FATAL_ERROR "Unable to locate SDL_image")
endif ()

find_package (PNG REQUIRED)

find_package (SDL_mixer REQUIRED)

# Make an automatic library - will be static or dynamic based on user setting
add_library(fs_engine ${SOURCE_LIST} ${HEADER_LIST})
add_library(Freesynd::Engine ALIAS fs_engine)

# We need this directory, and users of our library will need it too
target_include_directories(fs_engine PUBLIC include)

# This depends on (header only) boost
target_link_libraries(fs_engine PUBLIC SDL::SDL ${SDL_MIXER_LIBRARIES})

target_link_libraries(fs_engine PRIVATE Freesynd::Utils PNG::PNG ${SDLIMAGE_LIBRARY})

# All users of this library will need at least C++11
#target_compile_features(modern_library PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/engine/include"
  FILES ${HEADER_LIST})



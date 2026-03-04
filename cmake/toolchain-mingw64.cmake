# CMake toolchain for cross-compiling to Windows x86-64 using MinGW-w64

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

set(MINGW_PREFIX /opt/local)
set(MINGW_TRIPLE x86_64-w64-mingw32)

set(CMAKE_C_COMPILER   ${MINGW_PREFIX}/bin/${MINGW_TRIPLE}-gcc)
set(CMAKE_CXX_COMPILER ${MINGW_PREFIX}/bin/${MINGW_TRIPLE}-g++)
set(CMAKE_RC_COMPILER  ${MINGW_PREFIX}/bin/${MINGW_TRIPLE}-windres)

# Where to look for target headers/libraries
set(CMAKE_FIND_ROOT_PATH ${MINGW_PREFIX}/${MINGW_TRIPLE})

# Never search for programs in the target sysroot (they won't run on the host)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Only look in the target sysroot for libraries and headers
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# For package config files (find_package config mode), only search in the
# target sysroot and in paths explicitly given via *_DIR / CMAKE_PREFIX_PATH.
# Without this, CMake also searches macOS system paths and picks up host
# (macOS) packages (e.g. MacPorts SDL2_mixer) instead of the Windows ones.
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# pkg-config should not be used for the target platform
set(PKG_CONFIG_EXECUTABLE "")

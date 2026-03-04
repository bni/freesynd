#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$SCRIPT_DIR/.."
EXTERN="$ROOT/extern"

echo "==> Conan install"
conan install "$ROOT" \
  --profile:build=default \
  --profile:host="$SCRIPT_DIR/conan-profile-win64-mingw.ini" \
  --output-folder="$ROOT/build-win64/conan" \
  --build=missing

echo "==> CMake configure"
cmake -B "$ROOT/build-win64" -S "$ROOT" \
  -DCMAKE_TOOLCHAIN_FILE="$SCRIPT_DIR/toolchain-mingw64.cmake" \
  -DFREESYND_ARCH=x64 \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=TRUE \
  -DCRCpp_DIR="$ROOT/build-win64/conan/build/Release/generators" \
  -DCatch2_DIR="$ROOT/build-win64/conan/build/Release/generators" \
  -DPNG_DIR="$ROOT/build-win64/conan/build/Release/generators" \
  -DZLIB_DIR="$ROOT/build-win64/conan/build/Release/generators" \
  -Dutf8cpp_DIR="$ROOT/build-win64/conan/build/Release/generators" \
  -DCLI11_DIR="$ROOT/build-win64/conan/build/Release/generators" \
  -DSDL2_INCLUDE_DIR="$EXTERN/sdl2/include" \
  -DSDL2_LIBRARY="$EXTERN/sdl2/lib/x64/libSDL2.dll.a" \
  -DSDL2MAIN_LIBRARY="$EXTERN/sdl2/lib/x64/libSDL2main.a" \
  -DSDL2_IMAGE_INCLUDE_DIR="$EXTERN/SDL2_image/include" \
  -DSDL2_IMAGE_LIBRARY="$EXTERN/SDL2_image/lib/x64/libSDL2_image.dll.a" \
  -DSDL2_MIXER_INCLUDE_DIR="$EXTERN/SDL2_mixer/include" \
  -DSDL2_MIXER_LIBRARY="$EXTERN/SDL2_mixer/lib/x64/libSDL2_mixer.dll.a"

echo "==> Build"
cmake --build "$ROOT/build-win64" --parallel

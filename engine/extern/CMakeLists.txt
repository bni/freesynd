#
#  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
#
#   Copyright (C) 2025  Benoit Blancard <benblan@users.sourceforge.net>
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

include(FetchContent)

FetchContent_Declare(
  libAdl
  GIT_REPOSITORY https://github.com/Wohlstand/libADLMIDI.git
  # We use the version in the main branch because release 1.5.1
  # does not build on MacOS
  GIT_TAG        2b350f9ef5fa7bafd90b8ce3beb2a77c1e87af25 # release-1.6.1
)

set(USE_JAVA_EMULATOR OFF CACHE INTERNAL "Turn off JAva Emulator")
set(WITH_MUS_SUPPORT OFF CACHE INTERNAL "Turn off Mus Support")

FetchContent_MakeAvailable(libAdl)
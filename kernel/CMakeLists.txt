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
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/mod.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/modowner.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/position.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/path.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/pathsurfaces.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/leveldata.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/agent.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/ipastim.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/mapobject.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/missionbriefing.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/research.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/sfxobject.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/shot.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/static.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/damage.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/map.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/mission.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/objectivedesc.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/ped.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/train.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/vehicle.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/squad.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/time.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/weapon.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/model/weaponholder.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/ia/actions.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/ia/behaviour.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/agentmanager.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/pedmanager.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/mapmanager.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/modmanager.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/researchmanager.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/weaponmanager.h"
    "${FreeSynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/missionmanager.h"
    )

set(SOURCE_LIST
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/agent.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/ipastim.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/mod.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/map.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/mapobject.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/missionbriefing.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/mission.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/objectivedesc.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/ped.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/pedactions.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/pedpathfinding.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/research.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/static.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/sfxobject.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/shot.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/squad.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/time.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/train.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/vehicle.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/weapon.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/model/weaponholder.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/ia/actions.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/ia/behaviour.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/mgr/agentmanager.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/mgr/pedmanager.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/mgr/mapmanager.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/mgr/modmanager.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/mgr/researchmanager.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/mgr/weaponmanager.cpp"
    "${FreeSynd_SOURCE_DIR}/kernel/src/mgr/missionmanager.cpp"
    )

# Make an automatic library - will be static or dynamic based on user setting
add_library(fs_kernel ${SOURCE_LIST} ${HEADER_LIST})
add_library(Freesynd::Kernel ALIAS fs_kernel)

# We need this directory, and users of our library will need it too
target_include_directories(fs_kernel PUBLIC include)

target_link_libraries(fs_kernel PRIVATE freesynd_warnings Freesynd::Utils Freesynd::Engine )

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/kernel/include"
  FILES ${HEADER_LIST})

if(BUILD_TESTING)
    add_subdirectory(test)
endif()


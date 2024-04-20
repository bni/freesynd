
set(HEADER_LIST
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/mod.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/modowner.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/position.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/path.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/pathsurfaces.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/leveldata.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/agent.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/ipastim.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/mapobject.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/missionbriefing.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/research.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/sfxobject.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/shot.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/static.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/damage.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/map.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/mission.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/objectivedesc.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/ped.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/train.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/vehicle.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/squad.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/weapon.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/weaponholder.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/ia/actions.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/ia/behaviour.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/agentmanager.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/pedmanager.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/mapmanager.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/modmanager.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/researchmanager.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/weaponmanager.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/mgr/missionmanager.h"
    )

set(SOURCE_LIST
    "${Freesynd_SOURCE_DIR}/kernel/src/model/agent.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/ipastim.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/mod.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/map.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/mapobject.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/missionbriefing.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/mission.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/objectivedesc.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/ped.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/pedactions.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/pedpathfinding.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/research.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/static.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/sfxobject.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/shot.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/squad.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/train.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/vehicle.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/weapon.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/model/weaponholder.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/ia/actions.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/ia/behaviour.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/mgr/agentmanager.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/mgr/pedmanager.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/mgr/mapmanager.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/mgr/modmanager.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/mgr/researchmanager.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/mgr/weaponmanager.cpp"
    "${Freesynd_SOURCE_DIR}/kernel/src/mgr/missionmanager.cpp"
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


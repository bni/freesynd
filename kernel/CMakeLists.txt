
set(HEADER_LIST
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/mod.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/position.h"
    "${Freesynd_SOURCE_DIR}/kernel/include/fs-kernel/model/leveldata.h"
    )

set(SOURCE_LIST
    "${Freesynd_SOURCE_DIR}/kernel/src/model/mod.cpp"
    )

# Make an automatic library - will be static or dynamic based on user setting
add_library(fs_kernel ${SOURCE_LIST} ${HEADER_LIST})
add_library(Freesynd::Kernel ALIAS fs_kernel)

# We need this directory, and users of our library will need it too
target_include_directories(fs_kernel PUBLIC include)

target_link_libraries(fs_engine PRIVATE Freesynd::Utils )

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/kernel/include"
  FILES ${HEADER_LIST})


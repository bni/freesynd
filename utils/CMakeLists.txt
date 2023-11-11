# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
set(HEADER_LIST
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/common.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/log/log.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/crc/ccrc32.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/crc/dernc.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/file.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/configfile.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/portablefile.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/formatversion.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/utf8.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/utf8/checked.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/utf8/core.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/utf8/unchecked.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/misc/singleton.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/misc/seqmodel.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/misc/timer.h"
    )

set(SOURCE_LIST
    "${Freesynd_SOURCE_DIR}/utils/src/log.cpp"
    "${Freesynd_SOURCE_DIR}/utils/src/file.cpp"
    "${Freesynd_SOURCE_DIR}/utils/src/configfile.cpp"
    "${Freesynd_SOURCE_DIR}/utils/src/portablefile.cpp"
    "${Freesynd_SOURCE_DIR}/utils/src/ccrc32.cpp"
    "${Freesynd_SOURCE_DIR}/utils/src/dernc.cpp"
    "${Freesynd_SOURCE_DIR}/utils/src/seqmodel.cpp"
    )

# Make an automatic library - will be static or dynamic based on user setting
add_library(fs_utils ${SOURCE_LIST} ${HEADER_LIST})
add_library(Freesynd::Utils ALIAS fs_utils)

# We need this directory, and users of our library will need it too
target_include_directories(fs_utils PUBLIC include)

# All users of this library will need at least C++11
#target_compile_features(modern_library PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/utils/include"
  FILES ${HEADER_LIST})



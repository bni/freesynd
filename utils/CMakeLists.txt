# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

set(HEADER_LIST
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/common.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/log/log.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/crc/ccrc32.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/crc/dernc.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/file.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/configfile.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/portablefile.h"
    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/formatversion.h"
#    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/utf8.h"
#    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/utf8/checked.h"
#    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/utf8/core.h"
#    "${Freesynd_SOURCE_DIR}/utils/include/fs-utils/io/utf8/unchecked.h"
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

# Definition of the fs_utils library - will be static or dynamic based on user setting
add_library(fs_utils ${SOURCE_LIST} ${HEADER_LIST})
add_library(Freesynd::Utils ALIAS fs_utils)

target_link_libraries (fs_utils PRIVATE freesynd_warnings)

# We need this directory, and users of our library will need it too (ie PUBLIC)
target_include_directories(fs_utils PUBLIC include)

# Set standard definitions for all platforms.
target_compile_definitions(fs_utils PRIVATE FS_ETC_DIR=\"${FS_ETC_DIR}\")
target_compile_definitions(fs_utils PRIVATE FS_DATA_DIR=\"${FS_DATA_DIR}\")

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/utils/include"
  FILES ${HEADER_LIST})



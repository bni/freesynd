
if (APPLE)
    # On Mac, the data folder is a part of the app bundle.
else ()
    install (DIRECTORY . DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/freesynd"
            PATTERN "freesynd.ini" EXCLUDE
            PATTERN "freesynd.ini.in" EXCLUDE
            PATTERN "CMakeLists.txt" EXCLUDE)
endif()

# Generate a version of freesynd.ini with data dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini.in ${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini DESTINATION "${FS_ETC_DIR}")



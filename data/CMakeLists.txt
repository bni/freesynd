
if (APPLE)
    # On Mac, the data folder is a part of the app bundle.
else ()
    install (DIRECTORY . DESTINATION ${DATA_DIR}
            PATTERN "freesynd.ini.in" EXCLUDE
            PATTERN "CMakeLists.txt" EXCLUDE)
endif()

# Generate a version of freesynd.ini with data dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini.in ${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini.ref)



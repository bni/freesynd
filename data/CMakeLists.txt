# Generate a version of freesynd.ini with data dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini.in ${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini)

# Then install data directory
if (APPLE)
    # On Mac, the data folder is a part of the app bundle.
elseif(WIN32)
    install (DIRECTORY . DESTINATION "data"
    PATTERN "freesynd.ini" EXCLUDE
    PATTERN "freesynd.ini.in" EXCLUDE
    PATTERN "CMakeLists.txt" EXCLUDE)

    # Put the freesynd file in the same directory as the freesynd.exe
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini DESTINATION ".")
else ()
    install (DIRECTORY . DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/freesynd"
            PATTERN "freesynd.ini" EXCLUDE
            PATTERN "freesynd.ini.in" EXCLUDE
            PATTERN "CMakeLists.txt" EXCLUDE)
    
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/freesynd.ini DESTINATION "${FS_ETC_DIR}")
endif()

if (APPLE)
    # TBC
elseif (WIN32)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../README" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../AUTHORS" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../COPYING" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../INSTALL" DESTINATION .)

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../extern/MSVC/lib/" DESTINATION .
            FILES_MATCHING PATTERN "*.dll")
else ()

# Generate a changelog file
add_custom_command(
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog"
    COMMAND gzip -cn9 "${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog" > "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
)

add_custom_target(debian ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}"
)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/debian/copyright"
    DESTINATION "${CMAKE_INSTALL_DOCDIR}"
)

endif ()


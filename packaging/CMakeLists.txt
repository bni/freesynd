# Generate a changelog file
add_custom_command(
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog"
    COMMAND gzip -cn9 "${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog" > "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
)

add_custom_target(changelog ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/freesynd"
)

add_custom_target(copyright ALL DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/debian/copyright")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/debian/copyright"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/freesynd"
)


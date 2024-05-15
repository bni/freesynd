if (APPLE)
    set(MACOS_ASSETS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon/sword.icns" PARENT_SCOPE)

    # MACOSX_PACKAGE_LOCATION doesn't seem to work -> assets are copied directly under Resources dir
    #set_source_files_properties(${MACOS_ASSETS_ICON}
    #    PROPERTIES MACOSX_PACKAGE_LOCATION "icon")
    
elseif (WIN32)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../README" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../AUTHORS" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../COPYING" DESTINATION .)
    install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/../INSTALL" DESTINATION .)

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../extern/png/lib/" DESTINATION .
            FILES_MATCHING PATTERN "*.dll")

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../extern/sdl2/lib/${FREESYND_ARCH}/" DESTINATION .
            FILES_MATCHING PATTERN "*.dll")

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../extern/SDL2_image/lib/${FREESYND_ARCH}/" DESTINATION .
            FILES_MATCHING PATTERN "*.dll")

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../extern/SDL2_mixer/lib/${FREESYND_ARCH}/" DESTINATION .
            FILES_MATCHING PATTERN "*.dll")
else ()
    # Linux
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


set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_EXTRACT_PRIVATE YES)
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)

doxygen_add_docs(docs
                 "${PROJECT_SOURCE_DIR}/utils" "${PROJECT_SOURCE_DIR}/engine" "${PROJECT_SOURCE_DIR}/kernel" "${PROJECT_SOURCE_DIR}/game" "${PROJECT_SOURCE_DIR}/editor"
                 )
if (UNIX)
	install (FILES freesynd.6 DESTINATION ${CMAKE_INSTALL_MANDIR}/man6)
endif()

set(DEV_TOOLS_HEADERS
	editorapp.h
	editorcontroller.h
	editormenufactory.h
	editormenuid.h
	logoutmenu.h
	mainmenu.h
	fontmenu.h
	animmenu.h
	searchmissionmenu.h
	listmissionmenu.h)

add_executable (fs-editor
	editor.cpp
	editorapp.cpp
	editorcontroller.cpp
	editormenufactory.cpp
	logoutmenu.cpp
	mainmenu.cpp
	fontmenu.cpp
	animmenu.cpp
	searchmissionmenu.cpp
	listmissionmenu.cpp
	${DEV_TOOLS_HEADERS}
)

target_link_libraries (fs-editor PRIVATE freesynd_warnings Freesynd::Utils Freesynd::Engine Freesynd::Kernel)

# We only define an install target if we're doing a release build.
if (APPLE)
    install (TARGETS fs-editor DESTINATION /Applications)
elseif (WIN32)
    install (TARGETS fs-editor DESTINATION .)
else ()
    install (TARGETS fs-editor DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
endif ()



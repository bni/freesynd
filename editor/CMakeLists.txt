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

add_executable (editor
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

target_link_libraries (editor PRIVATE Freesynd::Utils Freesynd::Engine Freesynd::Kernel)




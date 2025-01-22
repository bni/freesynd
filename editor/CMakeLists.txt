#
#  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
#
#   Copyright (C) 2023-2025  Benoit Blancard <benblan@users.sourceforge.net>
#
#   This program is free software: you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as 
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of 
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#  along with this program. If not, see <https://www.gnu.org/licenses/>. 
# 

# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

set(DEV_TOOLS_HEADERS
	editorapp.h
	editorcontroller.h
	editormenufactory.h
	editormenuid.h
	mainmenu.h
	fontmenu.h
	audiomenu.h
	animmenu.h
	searchmissionmenu.h
	listmissionmenu.h)

add_executable (fs-editor
	editor.cpp
	editorapp.cpp
	editorcontroller.cpp
	editormenufactory.cpp
	mainmenu.cpp
	fontmenu.cpp
	audiomenu.cpp
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



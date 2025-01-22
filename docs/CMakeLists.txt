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

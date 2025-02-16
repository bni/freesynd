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

add_executable(test-kernel
    testcase.cpp
    mapobjecttest.cpp
    weaponholdertest.cpp
    squadtest.cpp
    objectivedesctest.cpp)

target_link_libraries(test-kernel PRIVATE Catch2::Catch2WithMain Freesynd::Kernel Freesynd::Utils Freesynd::Engine)

# catch_discover_tests(test-kernel)
add_test(test-kernel test-kernel)

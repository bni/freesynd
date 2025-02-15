/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2013, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
 *
 *   This program is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as 
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>. 
 * 
 */
#include <catch2/catch_test_macros.hpp>

#include "testcase.h"

void initWeaponConfigFile( ConfigFile &config ) {
    config.add("weapon.1.name", "WEAPON_PISTOL");
    config.add("weapon.1.icon.small", 15);
    config.add("weapon.1.icon.big", 65);
    config.add("weapon.1.cost", 0);
    config.add("weapon.1.ammo.nb", 13);
    config.add("weapon.1.ammo.price", 0);
    config.add("weapon.1.range", 1280);
    config.add("weapon.1.rank", 1);
    config.add("weapon.1.anim", 368);
    config.add("weapon.1.ammopershot", 1);
    config.add("weapon.1.timeforshot", 200);
    config.add("weapon.1.timereload", 600);
    config.add("weapon.1.damagerange", 0);
    config.add("weapon.1.shotangle", 5.0);
    config.add("weapon.1.shotaccuracy", 0.9);
    config.add("weapon.1.shotspeed", 0);
    config.add("weapon.1.dmg_per_shot", 2);
    config.add("weapon.1.ammo.impactNb", 1);
    config.add("weapon.1.weight", 1);

    config.add("weapon.12.name", "scanner");
    config.add("weapon.12.icon.small", 26);
    config.add("weapon.12.icon.big", 76);
    config.add("weapon.12.cost", 1000);
    config.add("weapon.12.range", 256);
    config.add("weapon.12.anim", 379);
    config.add("weapon.12.timeforshot", 1);
    config.add("weapon.12.timereload", 1);
    config.add("weapon.12.weight", 1);

    config.add("weapon.9.name", "WEAPON_SCANNER");
    config.add("weapon.9.icon.small", 23);
    config.add("weapon.9.icon.big", 73);
    config.add("weapon.9.cost", 500);
    config.add("weapon.9.range", 4096);
    config.add("weapon.9.anim", 376);
    config.add("weapon.9.weight", 1);

    config.add("weapon.13.name", "WEAPON_ENERGY_SHIELD");
    config.add("weapon.13.icon.small", 28);
    config.add("weapon.13.icon.big", 78);
    config.add("weapon.13.cost", 8000);
    config.add("weapon.13.ammo.nb", 200);
    config.add("weapon.13.ammo.price", 15);
    config.add("weapon.13.range", 768);
    config.add("weapon.13.anim", 381);
    config.add("weapon.13.ammopershot", 1);
    config.add("weapon.13.timeforshot", 75);
    config.add("weapon.13.ammo.impactNb", 1);
    config.add("weapon.13.weight", 8);
}

TEST_CASE( "1: All test cases reside in other .cpp files (empty)", "[multi-file:1]" ) {
}
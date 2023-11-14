/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#include "app.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <iostream>
#include <fstream>
#include <set>

#ifdef __APPLE__
// Carbon includes an AIFF header which conflicts with fliplayer.h
// So we will redefine ChunkHeader temporarily to work around that.
#define ChunkHeader CarbonChunkHeader
#include <Carbon/Carbon.h>
#undef ChunkHeader
#endif

#include "fs-engine/gfx/spritemanager.h"
#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "fs-utils/io/configfile.h"
#include "fs-utils/io/portablefile.h"
#include "agent.h"
#include "menus/gamemenufactory.h"
#include "menus/gamemenuid.h"

App::App()
    : BaseApp(),
      session_(new GameSession()), game_ctlr_(new GameController),
#ifdef SYSTEM_SDL
      system_(new SystemSDL())
#else
#error A suitable System object has not been defined!
#endif
    , intro_sounds_(), game_sounds_(), music_(),
    menus_(new GameMenuFactory(), &game_sounds_)
{
    running_ = true;
#ifdef _DEBUG
    debug_breakpoint_trigger_ = 0;
#endif
}

App::~App() {
}

/*!
 * Initialize application.
 * \param iniPath The path to the config file.
 * \return True if initialization is ok.
 */
bool App::doInitialize(const std::string& iniPath, bool disable_sound) {

    LOG(Log::k_FLG_INFO, "App", "initialize", ("initializing system..."))
    if (!system_->initialize(context_->isFullScreen())) {
        return false;
    }

    LOG(Log::k_FLG_INFO, "App", "initialize", ("initializing menus..."))
    if (!menus_.initialize(context_->isPlayIntro())) {
        return false;
    }

    LOG(Log::k_FLG_INFO, "App", "initialize", ("loading game sprites..."))
    if (!gameSprites().loaded())
        gameSprites().load();

    LOG(Log::k_FLG_INFO, "App", "initialize", ("loading game tileset..."))
    if (!maps().initialize()) {
        return false;
    }

    if (context_->isPlayIntro()) {
        LOG(Log::k_FLG_INFO, "App", "initialize", ("Loading intro sounds..."))
        intro_sounds_.initialize(disable_sound, system_->getAudio(), SoundManager::SAMPLES_INTRO);
    }

    LOG(Log::k_FLG_INFO, "App", "initialize", ("Loading game sounds..."))
    game_sounds_.initialize(disable_sound, system_->getAudio(), SoundManager::SAMPLES_GAME);

    LOG(Log::k_FLG_INFO, "App", "initialize", ("Loading music..."))
    music_.initialize(disable_sound, system_->getAudio());

    LOG(Log::k_FLG_INFO, "App", "initialize", ("Loading game data..."))
    g_gameCtrl.agents().loadAgents();
    return reset();
}

/*!
 * Activate cheat mode in which all completed missions can be replayed.
 */
void App::cheatRepeatOrCompleteMission() {
    g_Session.cheatReplayMission();
}

void App::cheatWeaponsAndMods() {
    g_gameCtrl.weaponManager().cheatEnableAllWeapons();
    g_gameCtrl.mods().cheatEnableAllMods();
}

void App::cheatEquipAllMods() {
    for (int agent = 0; agent < AgentManager::MAX_AGENT; agent++) {
        Agent *pAgent = g_gameCtrl.agents().agent(agent);
        if (pAgent) {
            pAgent->clearSlots();

            pAgent->addMod(g_gameCtrl.mods().getMod(Mod::MOD_LEGS, Mod::MOD_V3));
            pAgent->addMod(g_gameCtrl.mods().getMod(Mod::MOD_ARMS, Mod::MOD_V3));
            pAgent->addMod(g_gameCtrl.mods().getMod(Mod::MOD_EYES, Mod::MOD_V3));
            pAgent->addMod(g_gameCtrl.mods().getMod(Mod::MOD_BRAIN, Mod::MOD_V3));
            pAgent->addMod(g_gameCtrl.mods().getMod(Mod::MOD_CHEST, Mod::MOD_V3));
            pAgent->addMod(g_gameCtrl.mods().getMod(Mod::MOD_HEART, Mod::MOD_V3));
        }
    }
}

/*!
 * Activate cheat mode in which all missions are playable.
 */
void App::cheatAnyMission() {
    g_Session.cheatEnableAllMission();
}

void App::cheatResurrectAgents() {
    // TODO: Implement cheatResurrectAgents()
}

void App::cheatOwnAllCountries() {
    // TODO: Implement cheatOwnAllCountries()
}

void App::cheatAccelerateTime() {
    g_Session.cheatAccelerateTime();
}

void App::cheatFemaleRecruits() {
    g_gameCtrl.agents().reset(true);

    for (size_t i = 0; i < AgentManager::kMaxSlot; i++)
        g_gameCtrl.agents().setSquadMember(i, g_gameCtrl.agents().agent(i));
}

void App::cheatEquipFancyWeapons() {
    for (int i = 0; i < AgentManager::MAX_AGENT; i++) {
        if (g_gameCtrl.agents().agent(i)) {
        g_gameCtrl.agents().agent(i)->destroyAllWeapons();
#ifdef _DEBUG
        g_gameCtrl.agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::Minigun)));
        g_gameCtrl.agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::TimeBomb)));
        g_gameCtrl.agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::GaussGun)));
        g_gameCtrl.agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::Flamer)));
        g_gameCtrl.agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::Uzi)));
        g_gameCtrl.agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::Persuadatron)));
        g_gameCtrl.agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::EnergyShield)));
        g_gameCtrl.agents().agent(i)->addWeapon(
            WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::Shotgun)));
#else
        g_gameCtrl.agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::Minigun)));
        g_gameCtrl.agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::Minigun)));
        g_gameCtrl.agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::Persuadatron)));
        g_gameCtrl.agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::TimeBomb)));
        g_gameCtrl.agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::EnergyShield)));
        g_gameCtrl.agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::EnergyShield)));
        g_gameCtrl.agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::Laser)));
        g_gameCtrl.agents().agent(i)->addWeapon(
                WeaponInstance::createInstance(g_gameCtrl.weaponManager().getWeapon(Weapon::Laser)));
#endif
        }
    }
}

/*!
 * Activate a cheat code with the given name. Possible
 * cheat codes are :
 * - DO IT AGAIN : Possibility to replay a completed mission
 * - NUK THEM : Enable all missions and resurrect dead agents
 * - OWN THEM : All countries belong to the user
 * - ROB A BANK : Puts $100 000 000 in funds
 * - TO THE TOP : Puts $100 000 000 in funds and activates all missions
 * - COOPER TEAM : $100 000 000 in funds, select any mission, all weapons
 *   and mods
 * - WATCH THE CLOCK : Accelerates time
 *
 * \param name The name of a cheat code.
 */
void App::setCheatCode(const char *name) {

    // Repeat mission with previously obtained items, press 'C' or 'Ctrl-C'
    // to instantly complete a mission
    if (!strcmp(name, "DO IT AGAIN"))
        cheatRepeatOrCompleteMission();
    else if (!strcmp(name, "NUK THEM")) {
        // Select any mission, resurrect dead agents
        cheatAnyMission();
        cheatResurrectAgents();
    }
    else if (!strcmp(name, "OWN THEM")) {
        // Own all countries
        cheatOwnAllCountries();
    }
    else if (!strcmp(name, "ROB A BANK")) {
        // $100 000 000 in funds
        cheatFunds();
    }
    else if (!strcmp(name, "TO THE TOP")) {
        // $100 000 000 in funds, select any mission
        cheatFunds();
        cheatAnyMission();
    }
    else if (!strcmp(name, "COOPER TEAM")) {
        // $100 000 000 in funds, select any mission, all weapons and mods
        cheatFemaleRecruits();
        cheatFunds();
        cheatAnyMission();
        cheatWeaponsAndMods();
        cheatEquipAllMods();
        cheatEquipFancyWeapons();
    }
    else if (!strcmp(name, "WATCH THE CLOCK")) {
        // Accelerate time for faster research completion
        cheatAccelerateTime();
    }
}

/*!
 * Reset application data.
 * \returns True if reset is ok.
 */
bool App::reset() {
    g_gameCtrl.reset();

    // Reset user session
    if (!g_Session.reset()) {
        return false;
    }

    return true;
}

/*!
 * Destroy the application.
 */
void App::doDestroy() {
    game_ctlr_->clearAllListeners();
    menus_.destroy();

    game_ctlr_->destroy();
}

void App::waitForKeyPress() {

    while (running_) {
        // small pause while waiting for key, also mouse event
        system_->delay(20);
        menus_.handleEvents();
    }
}

/*!
 * This method defines the application loop.
 * \param start_mission Mission id used to start the application in debug mode
 * In standard mode start_mission is always -1.
 */
void App::run(int start_mission) {

    // load palette
    menus().setDefaultPalette();

#if 0
    system_->updateScreen();
    int nx = 0, ny = 0, my = 0;
    for (int i = 0; i < tabSize / 6; i++) {
        Sprite *s = menu_sprites_.sprite(i);
        if (nx + s->width() >= Screen::kScreenWidth) {
            nx = 0;
            ny += my;
            my = 0;
        }
        if (ny + s->height() > Screen::kScreenHeight)
            break;
        s->draw(nx, ny, 0);
        system_->updateScreen();
        nx += s->width();
        if (s->height() > my)
            my = s->height();
    }

    waitForKeyPress();
    exit(1);
#endif

    if (start_mission == -1) {
        if (context_->isPlayIntro()) {
            menus_.gotoMenu(fs_game_menus::kMenuIdFliIntro);
            // Update intro flag so intro won't be played next time
            context_->updateIntroFlag();
        } else {
            // play title before going to main menu
            menus_.gotoMenu(fs_game_menus::kMenuIdFliTitle);
        }
    }
    else {
        // Debug scenario : start directly with the brief menu
        // in the given mission
        // First, we find the block associated with the given
        // mission number
        for (int i = 0; i < 50; i++) {
            if (g_Session.getBlock(i).mis_id == start_mission) {
                g_Session.setSelectedBlockId(i);
                break;
            }
        }
        // Then we go to the brief menu
        menus_.gotoMenu(fs_game_menus::kMenuIdBrief);
    }

    int lasttick = system_->getTicks();
    while (running_) {
        int curtick = system_->getTicks();
        int diff_ticks = curtick - lasttick;
        menus_.updtSinceMouseDown(diff_ticks);
        menus_.handleEvents();
        if (diff_ticks < 30) {
            system_->delay(30 - diff_ticks);
            continue;
        }
        menus_.handleTick(diff_ticks);
        menus_.renderMenu();
        lasttick = curtick;
        system_->updateScreen();
    }

#ifdef GP2X
#ifndef WIN32
    // return to the menu
    chdir("/usr/gp2x");
    execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif
#endif
}

bool App::saveGameToFile(int fileSlot, std::string name) {
    LOG(Log::k_FLG_IO, "App", "saveGameToFile", ("Saving %s in slot %d", name.c_str(), fileSlot))

    PortableFile outfile;
    std::string path;

    File::getFullPathForSaveSlot(fileSlot, path);
    LOG(Log::k_FLG_IO, "App", "saveGameToFile", ("Saving to file %s", path.c_str()))

    outfile.open_to_overwrite(path.c_str());

    if (outfile) {
        // write file format version
        outfile.write8(1); // major
        outfile.write8(2); // minor

        // Slot name is 31 characters long, nul-padded
        outfile.write_string(name, 31);

        // Session
        g_Session.saveToFile(outfile);

        // Weapons
        g_gameCtrl.weaponManager().saveToFile(outfile);

        // Mods
        g_gameCtrl.mods().saveToFile(outfile);

        // Agents
        // TODO move in sesion saveToFile
        g_gameCtrl.agents().saveToFile(outfile);

        // save researches
        // TODO move in sesion saveToFile
        g_Session.researchManager().saveToFile(outfile);

        return true;
    }

    return false;
}

bool App::loadGameFromFile(int fileSlot) {

    std::string path;
    PortableFile infile;

    File::getFullPathForSaveSlot(fileSlot, path);

    infile.open_to_read(path.c_str());

    if (infile) {
        // FIXME: detect original game saves

        // Read version
        unsigned char vMaj = infile.read8();
        unsigned char vMin = infile.read8();
        FormatVersion v(vMaj, vMin);

        // validate that this is a supported version.
        if (v.gt(1,2)) {
            FSERR(Log::k_FLG_IO, "App", "loadGameFromFile", ("Cannot load file, unsupported version %d.%d", vMaj, vMin))
            return false;
        }

        if (v == 0x0100) {
            // the 1.0 format is in native byte order instead of big-endian.
            infile.set_system_endian();
        }

        reset();

        // Read slot name
        std::string slotName;
        // Original game: 20 chars on screen, 20 written, 19 read.
        // v1.0: 25 characters.
        // v1.1: 31 characters.
        slotName = infile.read_string((v == 0x0100) ? 25 : 31, true);

        g_Session.loadFromFile(infile, v);

        // Weapons
        g_gameCtrl.weaponManager().loadFromFile(infile, v);

        // Mods
        g_gameCtrl.mods().loadFromFile(infile, v);

        // Agents
        g_gameCtrl.agents().loadFromFile(infile, v);

        // Research
        g_Session.researchManager().loadFromFile(infile, v);

        return true;
    }

    return false;
}

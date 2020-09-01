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

#include "gfx/spritemanager.h"
#include "gfx/screen.h"
#include "sound/audio.h"
#include "utils/ccrc32.h"
#include "utils/file.h"
#include "utils/log.h"
#include "utils/configfile.h"
#include "utils/portablefile.h"
#include "agent.h"
#include "menus/gamemenufactory.h"
#include "menus/gamemenuid.h"

App::App(bool disable_sound):
context_(new AppContext),
session_(new GameSession()), game_ctlr_(new GameController),
    screen_(new Screen(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT))
#ifdef SYSTEM_SDL
    , system_(new SystemSDL())
#else
#error A suitable System object has not been defined!
#endif
    , intro_sounds_(disable_sound), game_sounds_(disable_sound), music_(disable_sound),
    menus_(new GameMenuFactory(), &game_sounds_)
{
    running_ = true;
#ifdef _DEBUG
    debug_breakpoint_trigger_ = 0;
#endif
}

App::~App() {
}

static void addMissingSlash(string& str) {
    if (str[str.length() - 1] != '/') str.push_back('/');
}

#ifdef __APPLE__
static bool getResourcePath(string& resourcePath) {
    // let's check to see if we're inside an application bundle first.
    CFBundleRef main = CFBundleGetMainBundle();
    CFStringRef appid = NULL;
    if (main) appid = CFBundleGetIdentifier(main);
    if (!appid) return false;

    // we're in an app bundle.
    printf("OS X application bundle detected.\n");
    CFURLRef url = CFBundleCopyResourcesDirectoryURL(main);
    if (!url) {
        // this shouldn't happen.
        printf("Unable to locate resources.\n");
        exit(1);
    }
    FSRef fs;
    if (!CFURLGetFSRef(url, &fs)) {
        // this shouldn't happen.
        printf("Unable to translate URL.\n");
        exit(1);
    }

    char *buf = (char *)malloc(1024);
    FSRefMakePath(&fs, (UInt8 *)buf, 1024);
    CFRelease(url);

    resourcePath.assign(buf);
    resourcePath.push_back('/');
    free(buf);
    return true;
}
#endif

#ifdef _WIN32
static string exeFolder() {
    char buf[1024];
    GetModuleFileName(NULL, buf, 1024);
    string tmp(buf);
    size_t pos = tmp.find_last_of('\\');
    if (pos != std::string::npos) tmp.erase(pos + 1);
    else tmp = ".";
    return tmp;
}
#endif

string App::defaultIniFolder()
{
    string folder;
#ifdef _WIN32
    // Under windows config file is in the same directory as freesynd.exe
    folder.assign(exeFolder());
    // Since we're defaulting to the exe's folder, no need to try to create a directory.
#elif defined(__APPLE__)
    // Make a symlink for convenience for *nix people who own Macs.
    folder.assign(getenv("HOME"));
    folder.append("/.freesynd");
    symlink("Library/Application Support/FreeSynd", folder.c_str());
    // On OS X, applications tend to store config files in this sort of path.
    folder.assign(getenv("HOME"));
    folder.append("/Library/Application Support/FreeSynd");
    mkdir(folder.c_str(), 0755);
#else
    // Under unix it's in the user home directory
    folder.assign(getenv("HOME"));
    folder.append("/.freesynd");
    mkdir(folder.c_str(), 0755);
#endif
    // note that we don't care if the mkdir() calls above succeed or not.
    // if they fail because they already exist, then it's no problem.
    // if they fail for any other reason, then we won't be able to open
    // or create freesynd.ini, and we'll surely detect that below.

    return folder;
}

bool App::readConfiguration() {
    File::setHomePath(defaultIniFolder());

    try {
        ConfigFile conf(iniPath_);
        string origDataDir;
        string ourDataDir;
        context_->setFullScreen(conf.read("fullscreen", false));
        context_->setPlayIntro(conf.read("play_intro", true));
        context_->setTimeForClick(conf.read("time_for_click", 80));
        bool origDataDirFound = conf.readInto(origDataDir, "data_dir");
        bool ourDataDirFound = conf.readInto(ourDataDir, "freesynd_data_dir");

        if (ourDataDirFound == false) {
#ifdef _WIN32
            ourDataDir = exeFolder() + "\\data";
#elif defined(__APPLE__)
            if (getResourcePath(ourDataDir)) {
                // this is an app bundle, so let's default the data dir
                // to the one included in the app bundle's resources.
                ourDataDir += "data/";
            } else {
                LOG(Log::k_FLG_GFX, "App", "readConfiguration", ("Unable to locate app bundle resources.\n"));
                return false;
            }
#else
            ourDataDir = PREFIX"/data";
#endif
        }
        addMissingSlash(ourDataDir);
        File::setOurDataPath(ourDataDir);

        if (origDataDirFound == false) {
            origDataDir = ourDataDir;
        }
        addMissingSlash(origDataDir);
        File::setDataPath(origDataDir);

        switch (conf.read("language", 0)) {
            case 0:
                context_->setLanguage(AppContext::ENGLISH);
                break;
            case 1:
                context_->setLanguage(AppContext::FRENCH);
                break;
            case 2:
                context_->setLanguage(AppContext::ITALIAN);
                break;
            case 3:
                context_->setLanguage(AppContext::GERMAN);
                break;
            default:
                context_->setLanguage(AppContext::ENGLISH);
                break;
        }

        return true;
    } catch (...) {
        LOG(Log::k_FLG_GFX, "App", "readConfiguration", ("failed to load config file \"%s\"\n", iniPath_.c_str()));
        return false;
    }
}

bool App::testOriginalData() {
    bool test_files = true;
    ConfigFile conf(iniPath_);
    conf.readInto(test_files, "test_data", true);
    if (test_files == false)
        return true;

    std::string crcflname = File::dataFullPath("ref/original_data.crc");
    std::ifstream od(crcflname.c_str());
    if (od.fail()) {
        LOG(Log::k_FLG_GFX, "App", "testOriginalData",
            ("Checksums file for original data is not found. Look at INSTALL/README file for possible solutions."));
        return false;
    }
    printf("Testing original Syndicate data...\n");
    LOG(Log::k_FLG_GFX, "App", "testOriginalData", ("Testing original Syndicate data..."));
    CCRC32 crc32_test;
    crc32_test.Initialize();
    bool rsp = true;
    while (od) {
        std::string line;
        std::getline(od, line);
        if (line.size() > 0) {
            std::string::size_type pos = line.find(' ');
            if (pos != std::string::npos) {
                // skipping commented
                if (line[0] == '#' || line[0] == ';')
                    continue;
                std::string flname = line.substr(0, pos);
                std::string str_crc32 = line.substr(pos+1);
                uint32 ui_crc32 = 0;
                uint32 multiply = 1 << (4 * 7);
                // String hex to uint32
                for (char i = 0; i < 8; i++) {
                    char c = str_crc32[i];
                    if ( c >= '0' && c <= '9')
                        c -= '0';
                    if ( c >= 'a' && c <= 'z')
                        c -= 'a' - 10;
                    ui_crc32 += c * multiply;
                    multiply >>= 4;
                }
                int sz;
                uint8 *data = File::loadOriginalFileToMem(flname, sz);
                if (!data) {
                    LOG(Log::k_FLG_GFX, "App", "testOriginalData", ("file not found \"%s\"\n", flname.c_str()));
                    printf("file not found \"%s\". Look at INSTALL/README file for possible solutions.\n", flname.c_str());
                    rsp = false;
                    continue;
                }
                if (ui_crc32 != crc32_test.FullCRC(data, sz)) {
                    rsp = false;
                    LOG(Log::k_FLG_GFX, "App", "testOriginalData", ("file test failed \"%s\"\n", flname.c_str()));
#ifdef _DEBUG
                    printf("file test failed \"%s\"\n", flname.c_str());
#endif
                }
                delete[] data;
            }
        }
    }
    if (rsp == false) {
        printf("Test failed.\n");
    } else {
        try {
            conf.add("test_data", false);

            std::ofstream file(iniPath_.c_str(), std::ios::out | std::ios::trunc);
            if (file) {
                file << conf;
                file.close();
            } else {
                LOG(Log::k_FLG_GFX, "App", "testOriginalData", ("Could not update configuration file!"))
            }
        } catch (...) {
            LOG(Log::k_FLG_GFX, "App", "testOriginalData", ("Could not update configuration file!"))
        }
        printf("Test passed. crc32 for data is correct.\n");
        LOG(Log::k_FLG_GFX, "App", "testOriginalData", ("Test passed. CRC32 for data is correct."));
    }
    od.close();
    return rsp;
}

void App::updateIntroFlag() {
    try {
        ConfigFile conf(iniPath_);
        conf.add("play_intro", false);

        std::ofstream file(iniPath_.c_str(), std::ios::out | std::ios::trunc);
        if (file) {
            file << conf;
            file.close();
        } else {
            LOG(Log::k_FLG_GFX, "App", "updateIntroFlag", ("Could not update configuration file!"))
        }
    } catch (...) {
        LOG(Log::k_FLG_GFX, "App", "updateIntroFlag", ("Could not update configuration file!"))
    }
}

/*!
 * Initialize application.
 * \param iniPath The path to the config file.
 * \return True if initialization is ok.
 */
bool App::initialize(const std::string& iniPath) {
    iniPath_ = iniPath;

    LOG(Log::k_FLG_INFO, "App", "initialize", ("reading configuration..."))
    if (!readConfiguration()) {
        LOG(Log::k_FLG_GFX, "App", "initialize", ("failed to read configuration..."))
        return false;
    }

    LOG(Log::k_FLG_INFO, "App", "initialize", ("testing original Syndicate data..."))
    if (!testOriginalData()) {
        LOG(Log::k_FLG_GFX, "App", "initialize", ("failed to test original Syndicate data..."))
        return false;
    }

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
        if (!intro_sounds_.loadSounds(SoundManager::SAMPLES_INTRO)) {
            return false;
        }
    }

    LOG(Log::k_FLG_INFO, "App", "initialize", ("Loading game sounds..."))
    if (!game_sounds_.loadSounds(SoundManager::SAMPLES_GAME)) {
        return false;
    }

    LOG(Log::k_FLG_INFO, "App", "initialize", ("Loading music..."))
    music_.loadMusic();

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
void App::destroy() {
    game_ctlr_->clearAllListeners();
    menus_.destroy();

    game_ctlr_->destroy();
}

void App::waitForKeyPress() {

    while (running_) {
        // small pause while waiting for key, also mouse event
        SDL_Delay(20);
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
        if (nx + s->width() >= GAME_SCREEN_WIDTH) {
            nx = 0;
            ny += my;
            my = 0;
        }
        if (ny + s->height() > GAME_SCREEN_HEIGHT)
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
            updateIntroFlag();
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

    int lasttick = SDL_GetTicks();
    while (running_) {
        int curtick = SDL_GetTicks();
        int diff_ticks = curtick - lasttick;
        menus_.updtSinceMouseDown(diff_ticks);
        menus_.handleEvents();
        if (diff_ticks < 30) {
            SDL_Delay(30 - diff_ticks);
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

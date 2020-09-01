/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2011  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "researchmanager.h"
#include "utils/log.h"
#include "utils/file.h"
#include "utils/configfile.h"
#include "core/gamecontroller.h"
#include "appcontext.h"

ResearchManager::ResearchManager() {
    pCurrResearch_ = NULL;
}

ResearchManager::~ResearchManager() {
    LOG(Log::k_FLG_MEM, "ResearchManager", "~ResearchManager", ("Destruction..."))
    destroy();
}

void ResearchManager::destroy() {
    // Destroy researches on mods
    for (unsigned int i=0; i<availableModsSearch_.size(); i++) {
        Research *pRes = availableModsSearch_.get(i);
        delete pRes;
    }

    // Destroy researches on weapons
    for (unsigned int i=0; i<availableWeaponsSearch_.size(); i++) {
        Research *pRes = availableWeaponsSearch_.get(i);
        delete pRes;
    }

    availableModsSearch_.clear();
    availableWeaponsSearch_.clear();
    pCurrResearch_ = NULL;
}

/*!
 * Loads a research for the given weapon type from the configuration file.
 * \param wt Weapon type to research
 * \return NULL if a problem has happened
 */
Research *ResearchManager::loadResearch(Weapon::WeaponType wt) {
    std::string filename(File::dataFullPath("ref/research.dat"));
    try {
        // Loads configuration file
        ConfigFile conf(filename);

        try {
            std::string name;
            int fund;
            int next;
            Weapon::WeaponType nextWeap = Weapon::Unknown;
            char tmp[25];
            const char *pattern = "res.weap.%d.%s";

            sprintf(tmp, pattern, wt, "name");
            conf.readInto(name, tmp);
            g_Ctx.getMessage(name, name);
            sprintf(tmp, pattern, wt, "minFund");
            conf.readInto(fund, tmp);
            sprintf(tmp, pattern, wt, "next");
            conf.readInto(next, tmp);

            // Convert file constants to game constants
            if (next == 2) {
                nextWeap = Weapon::GaussGun;
            } else if (next == 4) {
                nextWeap = Weapon::Uzi;
            } else if (next == 5) {
                nextWeap = Weapon::Minigun;
            } else if (next == 6) {
                nextWeap = Weapon::Laser;
            } else if (next == 7) {
                nextWeap = Weapon::Flamer;
            } else if (next == 8) {
                nextWeap = Weapon::LongRange;
            } else if (next == 11) {
                nextWeap = Weapon::TimeBomb;
            } else if (next == 12) {
                nextWeap = Weapon::AccessCard;
            } else if (next == 13) {
                nextWeap = Weapon::EnergyShield;
            }

            // Create new research
            Research *pRes = new Research(wt, name, fund, nextWeap);

            // Check if searched weapon has already been discovered
            Weapon *pW = g_gameCtrl.weaponManager().getWeapon(wt);
            if (pW->wasSubmittedToSearch()) {
                pRes->improve(pW);
            }

            return pRes;
        } catch (...) {
            FSERR(Log::k_FLG_GAME, "ResearchManager", "loadResearch", ("Cannot load weapon research %d", wt))
        }
    } catch (...) {
        FSERR(Log::k_FLG_IO, "ResearchManager", "loadResearch", ("File %s cannot be read", filename.c_str()))
    }
    return NULL;
}

Research *ResearchManager::loadResearch(Mod::EModType mt, Mod::EModVersion version) {
    std::string filename(File::dataFullPath("ref/research.dat"));
    try {
        // Loads configuration file
        ConfigFile conf(filename);

        std::string name;
        int min;

        char tmp[25];
        const char *pattern = "res.mod.%d.%d.%s";

        sprintf(tmp, pattern, mt, version, "name");
        conf.readInto(name, tmp);
        g_Ctx.getMessage(name, name);
        sprintf(tmp, pattern, mt, version, "minFund");
        conf.readInto(min, tmp, 0);

        if (min == 0 || name.size() == 0) {
            FSERR(Log::k_FLG_GAME, "ResearchManager", "loadResearch", ("Cannot load mod research %d, version %d", mt, version))
            return NULL;
        }
        return new Research(mt, version, name, min);

    } catch (...) {
        FSERR(Log::k_FLG_IO, "ResearchManager", "loadResearch", ("File %s cannot be read", filename.c_str()))
        return NULL;
    }
}

bool ResearchManager::reset() {
    destroy();

    Weapon::WeaponType defWeapons[] = {Weapon::Uzi, Weapon::Laser, Weapon::Flamer, Weapon::AccessCard};

    for (int i=0; i<4; i++) {
        Research *pRes = loadResearch(defWeapons[i]);
        if (pRes == NULL) {
            return false;
        }
        availableWeaponsSearch_.add(pRes);
    }

    Mod::EModType defMods[] = {Mod::MOD_LEGS, Mod::MOD_ARMS, Mod::MOD_CHEST,Mod::MOD_HEART, Mod::MOD_EYES, Mod::MOD_BRAIN};
    for (int i=0; i<6; i++) {
        Research *pRes = loadResearch(defMods[i], Mod::MOD_V2);
        if (pRes == NULL) {
            return false;
        }
        availableModsSearch_.add(pRes);
    }

    return true;
}

void ResearchManager::fireGameEvent(Research *pResearch) {
    GameEvent evt;
    evt.stream = GameEvent::kGame;
    evt.type = GameEvent::kResearch;
    evt.pCtxt = pResearch;
    g_gameCtrl.fireGameEvent(evt);
}

void ResearchManager::start(Research *pResearch) {
    // If there is already a running search => suspends it
    if (pCurrResearch_) {
        pCurrResearch_->suspend();
    }

    // Starts research
    pCurrResearch_ = pResearch;
    pCurrResearch_->start();
}

/*!
 * Terminate the given search and loads a new one if
 * the search has defined a next search.
 * Enables Weapon/Mod associated with that search.
 * \param pResearch The completed search.
 */
void ResearchManager::complete(Research *pResearch) {
    Research *pNextRes = NULL;
    // Enable new weapon or mods
    if (pResearch->getType() == Research::EQUIPS) {
        g_gameCtrl.weaponManager().enableWeapon(pResearch->getSearchWeapon());
        // Loads next research
        if (pResearch->getNextWeaponRes() != Weapon::Unknown) {
            pNextRes = loadResearch(pResearch->getNextWeaponRes());
        }

    } else {
        g_gameCtrl.mods().enableMod(pResearch->getSearchModType(), pResearch->getSearchModVersion());
        // Loads next research
        if (pResearch->getSearchModVersion() == Mod::MOD_V2) {
            pNextRes = loadResearch(pResearch->getSearchModType(), Mod::MOD_V3);
        }
    }

    // There is a new research of the same field
    if (pNextRes) {
        // Replace with new search
        replaceSearch(pResearch, pNextRes);
    } else {
        // There's no more research for this category
        removeSearch(pResearch);
    }

    // alerts of change
    fireGameEvent(pResearch);

    if (pResearch == pCurrResearch_) {
        pCurrResearch_ = NULL;
    }

    delete pResearch;
}

/*!
 * Process all started research and returns the amount of money
 * corresponding those researches
 */
int ResearchManager::process(int hourElapsed, int moneyLeft) {
    int amount = 0;

    if (pCurrResearch_ && pCurrResearch_->getStatus() == Research::STARTED) {
            amount = pCurrResearch_->updateProgression(hourElapsed, moneyLeft);

            if (pCurrResearch_->getStatus() == Research::FINISHED) {
                complete(pCurrResearch_);
            }
        }

    return amount;
}

void ResearchManager::replaceSearch(Research *pOldSearch, Research *pNewSearch) {
    VectorModel < Research * > *pList =
        pOldSearch->getType() == Research::EQUIPS ? &availableWeaponsSearch_ : &availableModsSearch_;

    for (unsigned int i=0; i<pList->size(); i++) {
        if (pOldSearch->getId() == pList->get(i)->getId()) {
            pList->setAt(i, pNewSearch);
            return;
        }
    }
}

// There's no more research for this category
void ResearchManager::removeSearch(Research *pOldSearch) {
    VectorModel < Research * > *pList =
        pOldSearch->getType() == Research::EQUIPS ? &availableWeaponsSearch_ : &availableModsSearch_;

    for (unsigned int i=0; i<pList->size(); i++) {
        if (pOldSearch->getId() == pList->get(i)->getId()) {
            pList->remove(i);
            return;
        }
    }
}

/*!
 * Tells the ResearchManager that a unknown weapon has been collected during a mission.
 * The new weapon will accelerate given research.
 * \param pWeapon The newly discovered weapon
 * \return True if it's the first time that the weapon has been submitted
 */
bool ResearchManager::handleWeaponDiscovered(Weapon *pWeapon) {
    if (pWeapon && !pWeapon->wasSubmittedToSearch()) {
        // It's the first time the weapon is submitted
        pWeapon->submitToSearch();
        // Find if there is a search already available for this weapon
        for (unsigned int i=0; i<availableWeaponsSearch_.size(); i++) {
            if (pWeapon->getType() == availableWeaponsSearch_.get(i)->getSearchWeapon()) {
                // There is currently a research on that weapon
                Research *pRes = availableWeaponsSearch_.get(i);
                pRes->improve(pWeapon);

                if (pRes->getStatus() == Research::FINISHED) {
                    // Research is completed after bonus
                    complete(pRes);
                } else {
                    // Replace with the same element to refresh listeners
                    availableWeaponsSearch_.setAt(i, pRes);
                }
                break;
            }
        }

        return true;
    }

    return false;
}

bool ResearchManager::saveToFile(PortableFile &file) {
    // Search on mods
    file.write32(availableModsSearch_.size());

    for (unsigned int iMod=0; iMod < availableModsSearch_.size(); iMod++) {
        Research *pRes = availableModsSearch_.get(iMod);
        pRes->saveToFile(file);
    }

    // Search on weapons
    file.write32(availableWeaponsSearch_.size());

    for (unsigned int iWeap=0; iWeap < availableWeaponsSearch_.size(); iWeap++) {
        Research *pRes = availableWeaponsSearch_.get(iWeap);
        pRes->saveToFile(file);
    }

    // Write the current search
    file.write32(pCurrResearch_ ? pCurrResearch_->getId() : 0);

    return true;
}

bool ResearchManager::loadFromFile(PortableFile &infile, const FormatVersion& v) {
    // Destroy existing researchs
    destroy();

    // Research on mods
    unsigned int nbRes = infile.read32();
    for (unsigned int iRes=0; iRes<nbRes; iRes++) {
        Mod::EModType mt = Mod::Unknown;
        Mod::EModVersion mv = Mod::MOD_V1;
        int type = infile.read32();
        switch (type) {
            case 0: mt = Mod::MOD_LEGS;break;
            case 1: mt = Mod::MOD_ARMS;break;
            case 2: mt = Mod::MOD_CHEST;break;
            case 3: mt = Mod::MOD_HEART;break;
            case 4: mt = Mod::MOD_EYES;break;
            case 5: mt = Mod::MOD_BRAIN;break;
            default: mt = Mod::Unknown;
        }

        int ver = infile.read32();
        switch (ver) {
            case 0: mv = Mod::MOD_V1;break;
            case 1: mv = Mod::MOD_V2;break;
            case 2: mv = Mod::MOD_V3;break;
            default: mv = Mod::MOD_V1;
        }

        Research *pRes = loadResearch(mt, mv);
        pRes->loadFromFile(infile, Research::MODS, v);
        if (pRes == NULL) {
            return false;
        }
        availableModsSearch_.add(pRes);
    }

    // Research on weapons
    nbRes = infile.read32();
    for (unsigned int iRes=0; iRes<nbRes; iRes++) {
        int type = infile.read32();
        Weapon::WeaponType wt = Weapon::Unknown;
        switch (type) {
            case Weapon::Persuadatron:
                wt = Weapon::Persuadatron;
                break;
            case Weapon::Pistol:
                wt = Weapon::Pistol;
                break;
            case Weapon::GaussGun:
                wt = Weapon::GaussGun;
                break;
            case Weapon::Shotgun:
                wt = Weapon::Shotgun;
                break;
            case Weapon::Uzi:
                wt = Weapon::Uzi;
                break;
            case Weapon::Minigun:
                wt = Weapon::Minigun;
                break;
            case Weapon::Laser:
                wt = Weapon::Laser;
                break;
            case Weapon::Flamer:
                wt = Weapon::Flamer;
                break;
            case Weapon::LongRange:
                wt = Weapon::LongRange;
                break;
            case Weapon::Scanner:
                wt = Weapon::Scanner;
                break;
            case Weapon::MediKit:
                wt = Weapon::MediKit;
                break;
            case Weapon::TimeBomb:
                wt = Weapon::TimeBomb;
                break;
            case Weapon::AccessCard:
                wt = Weapon::AccessCard;
                break;
            case Weapon::EnergyShield:
                wt = Weapon::EnergyShield;
                break;
            default:
                wt = Weapon::Unknown;
        }

        Research *pRes = loadResearch(wt);
        pRes->loadFromFile(infile, Research::EQUIPS, v);
        if (pRes == NULL) {
            return false;
        }
        availableWeaponsSearch_.add(pRes);
    }

    int id = infile.read32();
    if (id != 0) {
        for (unsigned int i=0; i<availableModsSearch_.size(); i++) {
            Research *pRes = availableModsSearch_.get(i);
            if (pRes && pRes->getId() == id) {
                pCurrResearch_ = pRes;
                return true;
            }
        }

        for (unsigned int i=0; i<availableWeaponsSearch_.size(); i++) {
            Research *pRes = availableWeaponsSearch_.get(i);
            if (pRes && pRes->getId() == id) {
                pCurrResearch_ = pRes;
                return true;
            }
        }
    }

    return true;
}

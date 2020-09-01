/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#ifndef MODMANAGER_H
#define MODMANAGER_H

#include <fstream>

#include "common.h"
#include "model/mod.h"
#include "utils/seqmodel.h"
#include "utils/portablefile.h"

/*!
 * Modifications manager class.
 */
class ModManager {
public:
    //! Constructor
    ModManager();
    //! Destructor
    ~ModManager();

    //! Resources destruction
    void destroy();
    //! Reset Data
    void reset();
    //! Cheating mode to enable all mods
    void cheatEnableAllMods();

    Mod *getMod(Mod::EModType mt, Mod::EModVersion ver);

    //! Enable mod of given type and version
    void enableMod(Mod::EModType mt, Mod::EModVersion ver);
    //! Returns the list of currently available weapons
    SequenceModel * getAvalaibleMods() { return &mods_; }

    //! Save instance to file
    bool saveToFile(PortableFile &file);
    //! Load instance from file
    bool loadFromFile(PortableFile &infile, const FormatVersion& v);
    Mod *getHighestVersion(Mod::EModType mt);

protected:
    //! Loads the mod from file
    Mod *loadMod(Mod::EModType mt, Mod::EModVersion ver);

protected:
    /*! This vector is used to store necessary but unavailable mods until there
     * are made available.*/
    std::vector<Mod *> preFetch_;
    //! The list of currently available mods
    VectorModel<Mod *> mods_;
};

#endif

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2011  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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
#ifndef AGENT_H
#define AGENT_H

#include <string>
#include <vector>
#include <fstream>

#include "common.h"
#include "model/weaponholder.h"
#include "modowner.h"
#include "utils/portablefile.h"

/*!
 * Stores information about agent health, weapons inventory and modifications.
 */
class Agent : public WeaponHolder, public ModOwner {
public:
    Agent(const char *agent_name, bool male);
    ~Agent() {}

    int getId() { return id_;}
    const char *getName() { return name_.c_str(); }
    bool isMale() { return male_; }
    bool isActive() { return active_; }
    void setActive(bool a) { active_ = a; }
    bool isAlive() { return is_alive_; }
    void set_dead() { is_alive_ = false; }

    //! Save instance to file
    bool saveToFile(PortableFile &file);
    //! Load instance from file
    bool loadFromFile(PortableFile &infile, const FormatVersion& v);

protected:
    /*! A counter to have unique IDs.*/
    static int agentCnt;
    /*! A unique id for the instance of Agent.*/
    int id_;
    std::string name_;
    bool male_;
    bool active_;
    bool is_alive_;
};

#endif

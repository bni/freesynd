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
#ifndef RESEARCH_H
#define RESEARCH_H

#include <string>
#include <list>
#include <fstream>

#include "model/weapon.h"
#include "model/mod.h"
#include "utils/portablefile.h"

/*!
 * Represents a research on a specific field.
 */
class Research {
public:

    enum EResType {
        EQUIPS = 0,
        MODS = 1
    };

    enum EResStatus {
        NOT_STARTED = 0,
        STARTED = 1,
        SUSPENDED = 2,
        FINISHED = 3
    };

    /*!
     * Represents a point on the progression graph.
     */
    struct ProgressPoint {
        float percentage;
        short hours;
        short coeffId;
    };

    Research(Weapon::WeaponType wType, std::string name, int min, Weapon::WeaponType nextWeapon);
    Research(Mod::EModType mType, Mod::EModVersion ver, std::string name, int min);
    ~Research() {}

    int getId() { return id_; }
    EResType getType() { return type_; }
    std::string getName() { return name_; }
    int getMinFunding() { return minFunding_; }
    int getMaxFunding() { return minFunding_ * 100; }
    int getCurrFunding();
    //! Returns the current status
    EResStatus getStatus() { return status_; }

    //! Weapon researched
    Weapon::WeaponType getSearchWeapon() { return weapon_; }
    //! Nex research on weapon
    Weapon::WeaponType getNextWeaponRes() { return nextWeapon_; }
    //! Mod research type
    Mod::EModType getSearchModType() { return modType_; }
    //! Mod research version
    Mod::EModVersion getSearchModVersion() { return modVersion_; }

    bool incrFunding();
    bool decrFunding();
    //!Starts the research
    void start() { status_ = STARTED; }
    //!Suspends the research
    void suspend() { status_ = SUSPENDED; }
    int getProjectedHour() { return projectedHour_; }
    std::list<ProgressPoint> &getProgressList() { return progressList_; }
    int updateProgression(short hour, int budget);
    //! Boost research with an existing weapon
    void improve(Weapon *pWeapon);

    //! Save instance to file
    bool saveToFile(PortableFile &file);
    //! Load instance from file
    bool loadFromFile(PortableFile &infile, EResType type, const FormatVersion& v);
protected:
    void init(std::string name, int min);
    void updateProjection();
protected:
    /*! A counter to have unique IDs.*/
    static int researchCnt;
    /*! Id of the research.*/
    int id_;
    /*! Type of research.*/
    EResType type_;
    /*! If research is on weapon, then this field is set.*/
    Weapon::WeaponType weapon_;
    /*! If research is on weapon, then this field is set.*/
    Weapon::WeaponType nextWeapon_;
    /*! If research is on mod, then this field is set.*/
    Mod::EModType modType_;
    /*! If research is on mod, then this field is set.*/
    Mod::EModVersion modVersion_;
    /*! Name of the research.*/
    std::string name_;
    /*! Minimum amount of funding.*/
    int minFunding_;
    /*! Current funding.*/
    int currFunding_;
    /*! True is search has started.*/
    EResStatus status_;
    /*! Index of the multiplicator coeff for funding and progression.*/
    short coeffInd_;
    /*! The projected duration in hour before the end of research.*/
    int projectedHour_;
    std::list<ProgressPoint> progressList_;
};

#endif //RESEARCH_H

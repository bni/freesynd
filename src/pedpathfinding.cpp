/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#include "common.h"
#include "mission.h"
#include "ped.h"
#include "pathsurfaces.h"
#include "gfx/tile.h"
#include "utils/log.h"

#if 0
#include "SDL.h"
#define EXECUTION_SPEED_TIME
#endif

const uint8 floodPointDesc::kBMaskDirNorth = 0x10;
const uint8 floodPointDesc::kBMaskDirNorthEast = 0x08;
const uint8 floodPointDesc::kBMaskDirEast = 0x04;
const uint8 floodPointDesc::kBMaskDirSouth = 0x01;
const uint8 floodPointDesc::kBMaskDirSouthEast = 0x02;
const uint8 floodPointDesc::kBMaskDirSouthWest = 0x80;
const uint8 floodPointDesc::kBMaskDirWest = 0x40;
const uint8 floodPointDesc::kBMaskDirNorthWest = 0x20;

/*!
 * Sets a destination point for the ped to reach at given speed.
 * \param m
 * \param node destination point
 * \param newSpeed Speed of movement
 * \return true if destination has been set correctly.
 */
bool PedInstance::initMovementToDestination(Mission *m, const TilePoint &destinationPt, int newSpeed) {

    dest_path_.clear();

    if (health_ <= 0) {
        return false;
    }

    TilePoint clippedDestPt(destinationPt);
    m->get_map()->clip(&clippedDestPt);

    // NOTE: this is a "flood" algorithm, it expands until it reaches other's
    // flood point, then it removes unrelated points
#ifdef EXECUTION_SPEED_TIME
    printf("---------------------------");
    printf("start time %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif


    floodPointDesc *targetd = &(m->mdpoints_[clippedDestPt.tx + clippedDestPt.ty * m->mmax_x_ + clippedDestPt.tz * m->mmax_m_xy]);

    floodPointDesc *based = &(m->mdpoints_[pos_.tx
        + pos_.ty * m->mmax_x_ + pos_.tz * m->mmax_m_xy]);

#if 0
#if _DEBUG
    printf("target t %x, dirm %x ; base t %x, dirm %x\n", targetd->bfNodeDesc,
        targetd->dirm, based->bfNodeDesc, based->dirm);
    printf("target dirh %x, dirl %x ; base dirh %x, dirl %x\n", targetd->dirh,
        targetd->dirl, based->dirh, based->dirl);
    printf("ttwd %X \n",m->mtsurfaces_[x + y * m->mmax_x_
        + z * m->mmax_m_xy].twd);
    printf("target pos: x %i; y %i; z %i, ox %i, oy %i\n",
        x, y, z, ox, oy);
    //printf("tileAt %x\n", g_App.maps().map(map())->tileAt(x,y,z));
    printf("base pos: x %i; y %i; z %i, ox %i, oy %i, oz %i\n",
        pos_.tx, pos_.ty, pos_.tz, pos_.ox, pos_.oy, pos_.oz);
    printf("zmax %x\n", m->mmax_z_);
    if ((z - 1)>= 0) {
        printf("lower twd %i\n", m->mtsurfaces_[x + y * m->mmax_x_
            + (z - 1) * m->mmax_m_xy].twd);
    }
#endif
#endif

    if(targetd->bfNodeDesc == m_fdNonWalkable) {
        std::string posAsStr;
        clippedDestPt.toString(&posAsStr);
        LOG(Log::k_FLG_GAME, "PedInstance", "initMovementToDestination", ("Ped %d : Movement to nonwalkable position %s", id_, posAsStr.c_str()));
        return false;
    }

    if(based->bfNodeDesc == m_fdNonWalkable) {
        std::string posAsStr;
        position().toString(&posAsStr);
        LOG(Log::k_FLG_GAME, "PedInstance", "initMovementToDestination", ("Ped %d : Movement from nonwalkable position %s", id_, posAsStr.c_str()));
        return false;
    }

    if (sameTile(clippedDestPt)) {
        // TODO : check if this case can be removed to follow the regular
        // path finding even if costly
        return false;
    }
#ifdef EXECUTION_SPEED_TIME
    printf("directions-map copy start %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif
    floodPointDesc *mdpmirror = m->mdpoints_cp_;
    memcpy((void *)mdpmirror, (void *)m->mdpoints_,
        m->mmax_x_ * m->mmax_y_ * m->mmax_z_ * sizeof(floodPointDesc));

#ifdef EXECUTION_SPEED_TIME
    printf("directions-map copy complete %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif

    if (!floodMap(m, clippedDestPt, mdpmirror)) {
        return false;
    }

#ifdef EXECUTION_SPEED_TIME
    printf("non-related removed time %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif

    // path is created here
    std::vector<TilePoint> cdestpath;
    cdestpath.reserve(256);

    createPath(m, mdpmirror, cdestpath);

#ifdef EXECUTION_SPEED_TIME
    printf("path creation time %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif

    // TODO: smoother path
    // stairs to surface, surface to stairs correction
    if (!cdestpath.empty()) {
        buildFinalDestinationPath(m, cdestpath, clippedDestPt);
    }

    if (dest_path_.empty()) {
        // destination was not set -> stop ped
        speed_ = 0;
        return false;
    } else {
        // if no speed was set, use ped's default speed
        speed_ = newSpeed != -1 ? newSpeed : getDefaultSpeed();
        return true;
    }

#if 0
    for (std::list <TilePoint>::iterator it = dest_path_.begin();
        it != dest_path_.end(); ++it) {
        printf("x %i, y %i, z %i\n", it->bfNodeDescileX(),it->tileY(),it->tileZ());
    }
#endif
#ifdef EXECUTION_SPEED_TIME
    dest_path_.clear();
    printf("+++++++++++++++++++++++++++");
    printf("end time %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif
}

bool PedInstance::floodMap(Mission *m, const TilePoint &clippedDestPt, floodPointDesc *mdpmirror) {
    unsigned char lt;
    unsigned short blvl = 0, tlvl = 0;
    // these are all tiles that belong to base and target
    std::vector <toSetDesc> bv;
    std::vector <toSetDesc> tv;
    bv.reserve(8192);
    tv.reserve(8192);
    // these are used for setting values through algorithm
    toSetDesc sadd;
    floodPointDesc *pfdp;
    // setup
    pfdp = &(mdpmirror[pos_.tx + pos_.ty * m->mmax_x_ + pos_.tz * m->mmax_m_xy]);
    pfdp->bfNodeDesc |= m_fdBasePoint;
    sadd.coords.x = pos_.tx;
    sadd.coords.y = pos_.ty;
    sadd.coords.z = pos_.tz;
    sadd.pNode = pfdp;
    bv.push_back(sadd);
    pfdp = &(mdpmirror[clippedDestPt.tx + clippedDestPt.ty * m->mmax_x_ + clippedDestPt.tz * m->mmax_m_xy]);
    pfdp->bfNodeDesc |= (m_fdTargetPoint | m_fdConstant);
    sadd.coords.x = clippedDestPt.tx;
    sadd.coords.y = clippedDestPt.ty;
    sadd.coords.z = clippedDestPt.tz;
    sadd.pNode = pfdp;
    tv.push_back(sadd);
    // for setting lvls data
    lvlNodesDesc ladd;
    ladd.indxs = 0;
    ladd.n = 1;
    // these are number of nodes per lvl and index start for "bv" and "tv"
    std::vector <lvlNodesDesc> bn;
    std::vector <lvlNodesDesc> tn;
    bn.reserve(512);
    tn.reserve(512);
    bn.push_back(ladd);
    tn.push_back(ladd);
    bool nodeset, lnknr = true;
#ifdef EXECUTION_SPEED_TIME
    printf("data allocation/setup complete %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif

#ifdef FIND_DEFINED_TILE
    bool assertion_bool = true;
    int x_check = 48, y_check = 23, z_check = 6;
#endif
    do {
        unsigned short mindx = bn[blvl].indxs + bn[blvl].n;
        unsigned short nlvl = blvl + 1;
        unsigned int cindx = 0;
        for (unsigned short i = bn[blvl].indxs; i < mindx; ++i) {
            toSetDesc bref = bv[i];
            cindx = bref.coords.x + bref.coords.y * m->mmax_x_
                + bref.coords.z * m->mmax_m_xy;
            if (bref.pNode->dirh != 0) {
                if ((bref.pNode->dirh & 0x01) == 0x01) {
                    sadd.pNode = &(mdpmirror[cindx + m->mmax_x_ + m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y + 1;
                        sadd.coords.z = bref.coords.z + 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirh & 0x04) == 0x04) {
                    sadd.pNode = &(mdpmirror[cindx + 1 + m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x + 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z + 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirh & 0x10) == 0x10) {
                    sadd.pNode = &(mdpmirror[cindx - m->mmax_x_ + m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y - 1;
                        sadd.coords.z = bref.coords.z + 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirh & 0x40) == 0x40) {
                    sadd.pNode = &(mdpmirror[cindx - 1 + m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x - 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z + 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
            }
            if (bref.pNode->dirl != 0) {
                if ((bref.pNode->dirl & 0x01) == 0x01) {
                    sadd.pNode = &(mdpmirror[cindx + m->mmax_x_ - m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y + 1;
                        sadd.coords.z = bref.coords.z - 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirl & 0x04) == 0x04) {
                    sadd.pNode = &(mdpmirror[cindx + 1 - m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x + 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z - 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirl & 0x10) == 0x10) {
                    sadd.pNode = &(mdpmirror[cindx - m->mmax_x_ - m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y - 1;
                        sadd.coords.z = bref.coords.z - 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirl & 0x40) == 0x40) {
                    sadd.pNode = &(mdpmirror[cindx - 1 - m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x - 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z - 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
            }
            if (bref.pNode->dirm != 0) {
                if ((bref.pNode->dirm & 0x01) == 0x01) {
                    sadd.pNode = &(mdpmirror[cindx + m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y + 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x02) == 0x02) {
                    sadd.pNode = &(mdpmirror[cindx + 1 + m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x + 1;
                        sadd.coords.y = bref.coords.y + 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x04) == 0x04) {
                    sadd.pNode = &(mdpmirror[cindx + 1]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x + 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x08) == 0x08) {
                    sadd.pNode = &(mdpmirror[cindx + 1 - m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x + 1;
                        sadd.coords.y = bref.coords.y - 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x10) == 0x10) {
                    sadd.pNode = &(mdpmirror[cindx - m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y - 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x20) == 0x20) {
                    sadd.pNode = &(mdpmirror[cindx - 1 - m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x - 1;
                        sadd.coords.y = bref.coords.y - 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x40) == 0x40) {
                    sadd.pNode = &(mdpmirror[cindx - 1]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x - 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x80) == 0x80) {
                    sadd.pNode = &(mdpmirror[cindx - 1 + m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x - 1;
                        sadd.coords.y = bref.coords.y + 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdBasePoint;
                        bv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdTargetPoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdBasePoint;
                        lnknr = false;
                    }
                }
            }
        }
        ladd.indxs = mindx;
        ladd.n = bv.size() - mindx;
        if (ladd.n > 0) {
            nodeset = true;
            bn.push_back(ladd);
            ++blvl;
        } else {
            nodeset = false;
            break;
        }
        if (!lnknr)
            break;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        mindx = tn[tlvl].indxs + tn[tlvl].n;
        nlvl = tlvl + 1;
        for (unsigned short i = tn[tlvl].indxs; i < mindx; ++i) {
            toSetDesc bref = tv[i];
            cindx = bref.coords.x + bref.coords.y * m->mmax_x_
                + bref.coords.z * m->mmax_m_xy;
            if (bref.pNode->dirh != 0) {
                if ((bref.pNode->dirh & 0x01) == 0x01) {
                    sadd.pNode = &(mdpmirror[cindx + m->mmax_x_ + m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y + 1;
                        sadd.coords.z = bref.coords.z + 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirh & 0x04) == 0x04) {
                    sadd.pNode = &(mdpmirror[cindx + 1 + m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x + 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z + 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirh & 0x10) == 0x10) {
                    sadd.pNode = &(mdpmirror[cindx - m->mmax_x_ + m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y - 1;
                        sadd.coords.z = bref.coords.z + 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirh & 0x40) == 0x40) {
                    sadd.pNode = &(mdpmirror[cindx - 1 + m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x - 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z + 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
            }
            if (bref.pNode->dirl != 0) {
                if ((bref.pNode->dirl & 0x01) == 0x01) {
                    sadd.pNode = &(mdpmirror[cindx + m->mmax_x_ - m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y + 1;
                        sadd.coords.z = bref.coords.z - 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirl & 0x04) == 0x04) {
                    sadd.pNode = &(mdpmirror[cindx + 1 - m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x + 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z - 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirl & 0x10) == 0x10) {
                    sadd.pNode = &(mdpmirror[cindx - m->mmax_x_ - m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y - 1;
                        sadd.coords.z = bref.coords.z - 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirl & 0x40) == 0x40) {
                    sadd.pNode = &(mdpmirror[cindx - 1 - m->mmax_m_xy]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x - 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z - 1;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
            }
            if (bref.pNode->dirm != 0) {
                if ((bref.pNode->dirm & 0x01) == 0x01) {
                    sadd.pNode = &(mdpmirror[cindx + m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y + 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x02) == 0x02) {
                    sadd.pNode = &(mdpmirror[cindx + 1 + m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x + 1;
                        sadd.coords.y = bref.coords.y + 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x04) == 0x04) {
                    sadd.pNode = &(mdpmirror[cindx + 1]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x + 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x08) == 0x08) {
                    sadd.pNode = &(mdpmirror[cindx + 1 - m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x + 1;
                        sadd.coords.y = bref.coords.y - 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x10) == 0x10) {
                    sadd.pNode = &(mdpmirror[cindx - m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x;
                        sadd.coords.y = bref.coords.y - 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0) {
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x20) == 0x20) {
                    sadd.pNode = &(mdpmirror[cindx - 1 - m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x - 1;
                        sadd.coords.y = bref.coords.y - 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x40) == 0x40) {
                    sadd.pNode = &(mdpmirror[cindx - 1]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x - 1;
                        sadd.coords.y = bref.coords.y;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
                if ((bref.pNode->dirm & 0x80) == 0x80) {
                    sadd.pNode = &(mdpmirror[cindx - 1 + m->mmax_x_]);
                    if ((sadd.pNode->bfNodeDesc & (m_fdWalkable | m_fdBasePoint | m_fdTargetPoint)) == m_fdWalkable) {
                        sadd.coords.x = bref.coords.x - 1;
                        sadd.coords.y = bref.coords.y + 1;
                        sadd.coords.z = bref.coords.z;
#ifdef FIND_DEFINED_TILE
                        if (sadd.coords.x == x_check && sadd.coords.y == y_check
                            && sadd.coords.z == z_check)
                            assert(assertion_bool);
#endif
                        sadd.pNode->lvl = nlvl;
                        sadd.pNode->bfNodeDesc |= m_fdTargetPoint;
                        tv.push_back(sadd);
                    } else if ((sadd.pNode->bfNodeDesc & m_fdBasePoint) != 0){
                        bref.pNode->bfNodeDesc |= m_fdLink;
                        sadd.pNode->bfNodeDesc |= m_fdLink;
                        lt = m_fdTargetPoint;
                        lnknr = false;
                    }
                }
            }
        }
        ladd.indxs = mindx;
        ladd.n = tv.size() - mindx;
        if (ladd.n > 0) {
            nodeset = true;
            tn.push_back(ladd);
            ++tlvl;
        } else {
            nodeset = false;
            break;
        }
    } while (lnknr);
    //printf("bv %i, tv %i\n", bv.size(), tv.size());
#ifdef EXECUTION_SPEED_TIME
    printf("blvl %i, tlvl %i\n",tlvl, blvl);
    printf("target reached in %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif
    if (!nodeset && lnknr) {
        return false;
    }
    if (blvl == bn.size())
        blvl--;
    if (tlvl == tn.size())
        tlvl--;
    // when link is set data of nlvl is useless, that is why it is removed
    if (nodeset) {
        if (lt == m_fdBasePoint) {
            unsigned short n = bn[blvl].n;
            std::vector <toSetDesc>::iterator it = bv.begin() + bn[blvl].indxs;
            for (unsigned short i = 0; i < n; ++i) {
                it->pNode->bfNodeDesc ^= m_fdBasePoint;
                it->pNode->lvl = 0;
                //bv.erase(it);
                ++it;
            }
            //bn.pop_back();
            --blvl;
        } else {
            unsigned short n = tn[tlvl].n;
            std::vector <toSetDesc>::iterator it = tv.begin() + tn[tlvl].indxs;
            for (unsigned short i = 0; i < n; ++i) {
                it->pNode->bfNodeDesc ^= m_fdTargetPoint;
                it->pNode->lvl = 0;
                //tv.erase(it);
                ++it;
            }
            //tn.pop_back();
            --tlvl;
        }
    }

    // level which created link have also non-link tiles they are useless
    if (blvl != 0) {
        unsigned short n = bn[blvl].n;
        unsigned short nr = 0;
        std::vector <toSetDesc>::iterator it = bv.begin() + bn[blvl].indxs;
        for (unsigned short i = 0; i < n; ++i) {
            if ((it->pNode->bfNodeDesc & m_fdLink) == 0) {
                it->pNode->bfNodeDesc ^= m_fdBasePoint;
                it->pNode->lvl = 0;
                //bv.erase(it);
                ++nr;
            }
            ++it;
        }
        bn[blvl].n -= nr;
    }

    if (tlvl != 0) {
        unsigned short n = tn[tlvl].n;
        unsigned short nr = 0;
        std::vector <toSetDesc>::iterator it = tv.begin() + tn[tlvl].indxs;
        for (unsigned short i = 0; i < n; ++i) {
            if ((it->pNode->bfNodeDesc & m_fdLink) == 0) {
                it->pNode->bfNodeDesc ^= m_fdTargetPoint;
                it->pNode->lvl = 0;
                //tv.erase(it);
                ++nr;
            }
            ++it;
        }
        tn[tlvl].n -= nr;
    }
#ifdef EXECUTION_SPEED_TIME
    printf("tops removed time %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif

    // tiles that have no childs are removed
    removeTilesWithNoChildsFromBase(m, blvl, bv, bn, mdpmirror);

    removeTilesWithNoChildsFromTarget(m, tlvl, tv, tn, mdpmirror);

    return true;
}

void PedInstance::removeTilesWithNoChildsFromBase(Mission *m, unsigned short blvl, std::vector <toSetDesc> &bv, std::vector <lvlNodesDesc> &bn, floodPointDesc *mdpmirror) {
    if (blvl > 1) {
        floodPointDesc *pfdp;
        --blvl;
        unsigned short indx = bn[blvl].indxs + bn[blvl].n;
        --indx;
        do {
            toSetDesc &bref = bv[indx];
            uint16 lvl_child = (bref.pNode->lvl + 1);
            bool remv = true;
            if (bref.pNode->dirh != 0) {
                if ((bref.pNode->dirh & 0x01) == 0x01) {
                    pfdp = &(mdpmirror[bref.coords.x
                        + (bref.coords.y + 1) * m->mmax_x_
                        + (bref.coords.z + 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirh & 0x04) == 0x04) {
                    pfdp = &(mdpmirror[(bref.coords.x + 1)
                        + bref.coords.y * m->mmax_x_
                        + (bref.coords.z + 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirh & 0x10) == 0x10) {
                    pfdp = &(mdpmirror[bref.coords.x
                        + (bref.coords.y - 1) * m->mmax_x_
                        + (bref.coords.z + 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirh & 0x40) == 0x40) {
                    pfdp = &(mdpmirror[(bref.coords.x - 1)
                        + bref.coords.y * m->mmax_x_
                        + (bref.coords.z + 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
            }
            if (bref.pNode->dirl != 0) {
                if ((bref.pNode->dirl & 0x01) == 0x01) {
                    pfdp = &(mdpmirror[bref.coords.x
                        + (bref.coords.y + 1) * m->mmax_x_
                        + (bref.coords.z - 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirl & 0x04) == 0x04) {
                    pfdp = &(mdpmirror[(bref.coords.x + 1)
                        + bref.coords.y * m->mmax_x_
                        + (bref.coords.z - 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirl & 0x10) == 0x10) {
                    pfdp = &(mdpmirror[(bref.coords.x)
                        + (bref.coords.y - 1) * m->mmax_x_
                        + (bref.coords.z - 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirl & 0x40) == 0x40) {
                    pfdp = &(mdpmirror[(bref.coords.x - 1)
                        + bref.coords.y * m->mmax_x_
                        + (bref.coords.z - 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
            }
            if (bref.pNode->dirm != 0) {
                if ((bref.pNode->dirm & 0x01) == 0x01) {
                    pfdp = &(mdpmirror[bref.coords.x
                        + (bref.coords.y + 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x02) == 0x02) {
                    pfdp = &(mdpmirror[(bref.coords.x + 1)
                        + (bref.coords.y + 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x04) == 0x04) {
                    pfdp = &(mdpmirror[(bref.coords.x + 1)
                        + bref.coords.y * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x08) == 0x08) {
                    pfdp = &(mdpmirror[(bref.coords.x + 1)
                        + (bref.coords.y - 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x10) == 0x10) {
                    pfdp = &(mdpmirror[bref.coords.x
                        + (bref.coords.y - 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x20) == 0x20) {
                    pfdp = &(mdpmirror[(bref.coords.x - 1)
                        + (bref.coords.y - 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x40) == 0x40) {
                    pfdp = &(mdpmirror[(bref.coords.x - 1)
                        + bref.coords.y * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x80) == 0x80) {
                    pfdp = &(mdpmirror[(bref.coords.x - 1)
                        + (bref.coords.y + 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
            }
            if (remv) {
                bref.pNode->bfNodeDesc ^= m_fdBasePoint;
                bref.pNode->lvl = 0;
                //bv.erase(it);
            }
            --indx;
        } while(indx != 0);
    }
}

void PedInstance::removeTilesWithNoChildsFromTarget(Mission *m, unsigned short tlvl, std::vector <toSetDesc> &tv, std::vector <lvlNodesDesc> &tn, floodPointDesc *mdpmirror) {
    if (tlvl > 1) {
        --tlvl;
        unsigned short indx = tn[tlvl].indxs + tn[tlvl].n;
        --indx;
        floodPointDesc *pfdp;
        do {
            toSetDesc &bref = tv[indx];
            uint16 lvl_child = (bref.pNode->lvl + 1);
            bool remv = true;
            if (bref.pNode->dirh != 0) {
                if ((bref.pNode->dirh & 0x01) == 0x01) {
                    pfdp = &(mdpmirror[bref.coords.x
                        + (bref.coords.y + 1) * m->mmax_x_
                        + (bref.coords.z + 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirh & 0x04) == 0x04) {
                    pfdp = &(mdpmirror[(bref.coords.x + 1)
                        + bref.coords.y * m->mmax_x_
                        + (bref.coords.z + 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirh & 0x10) == 0x10) {
                    pfdp = &(mdpmirror[bref.coords.x
                        + (bref.coords.y - 1) * m->mmax_x_
                        + (bref.coords.z + 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirh & 0x40) == 0x40) {
                    pfdp = &(mdpmirror[(bref.coords.x - 1)
                        + bref.coords.y * m->mmax_x_
                        + (bref.coords.z + 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
            }
            if (bref.pNode->dirl != 0) {
                if ((bref.pNode->dirl & 0x01) == 0x01) {
                    pfdp = &(mdpmirror[bref.coords.x
                        + (bref.coords.y + 1) * m->mmax_x_
                        + (bref.coords.z - 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirl & 0x04) == 0x04) {
                    pfdp = &(mdpmirror[(bref.coords.x + 1)
                        + bref.coords.y * m->mmax_x_
                        + (bref.coords.z - 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirl & 0x10) == 0x10) {
                    pfdp = &(mdpmirror[(bref.coords.x)
                        + (bref.coords.y - 1) * m->mmax_x_
                        + (bref.coords.z - 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirl & 0x40) == 0x40) {
                    pfdp = &(mdpmirror[(bref.coords.x - 1)
                        + bref.coords.y * m->mmax_x_
                        + (bref.coords.z - 1) * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
            }
            if (bref.pNode->dirm != 0) {
                if ((bref.pNode->dirm & 0x01) == 0x01) {
                    pfdp = &(mdpmirror[bref.coords.x
                        + (bref.coords.y + 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x02) == 0x02) {
                    pfdp = &(mdpmirror[(bref.coords.x + 1)
                        + (bref.coords.y + 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x04) == 0x04) {
                    pfdp = &(mdpmirror[(bref.coords.x + 1)
                        + bref.coords.y * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x08) == 0x08) {
                    pfdp = &(mdpmirror[(bref.coords.x + 1)
                        + (bref.coords.y - 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x10) == 0x10) {
                    pfdp = &(mdpmirror[bref.coords.x
                        + (bref.coords.y - 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x20) == 0x20) {
                    pfdp = &(mdpmirror[(bref.coords.x - 1)
                        + (bref.coords.y - 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x40) == 0x40) {
                    pfdp = &(mdpmirror[(bref.coords.x - 1)
                        + bref.coords.y * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
                if ((bref.pNode->dirm & 0x80) == 0x80) {
                    pfdp = &(mdpmirror[(bref.coords.x - 1)
                        + (bref.coords.y + 1) * m->mmax_x_
                        + bref.coords.z * m->mmax_m_xy]);
                    if (lvl_child == pfdp->lvl)
                        remv = false;
                }
            }
            if (remv) {
                bref.pNode->bfNodeDesc ^= m_fdTargetPoint;
                bref.pNode->lvl = 0;
                //tv.erase(it);
            }
            --indx;
        } while(indx != 0);
    }
}

void PedInstance::createPath(Mission *m, floodPointDesc *mdpmirror, std::vector<TilePoint> &pathToDestination) {
    TilePoint currentTile(pos_.tx, pos_.ty, pos_.tz);
    unsigned char ct = m_fdBasePoint;
    bool tnr = true, np = true;
    floodPointDesc *pfdp;
    toSetDesc sadd;

    do {
        unsigned char nt = ct;
        char dist = 5;
        WorldPoint toadd;
        pfdp = &(mdpmirror[currentTile.tx + currentTile.ty * m->mmax_x_
                    + currentTile.tz * m->mmax_m_xy]);
        uint16 lvl_child = ct == m_fdBasePoint ? pfdp->lvl + 1
            : pfdp->lvl - 1;
        if (pfdp->dirh != 0) {
            if (pfdp->isDirectionUpContains(floodPointDesc::kBMaskDirSouth)) {
                sadd.coords.x = currentTile.tx;
                sadd.coords.y = currentTile.ty + 1;
                sadd.coords.z = currentTile.tz + 1;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (3 < dist) {
                            toadd = sadd.coords;
                            dist = 3;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (0 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = 0;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionUpContains(floodPointDesc::kBMaskDirEast)) {
                sadd.coords.x = currentTile.tx + 1;
                sadd.coords.y = currentTile.ty;
                sadd.coords.z = currentTile.tz + 1;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (3 < dist) {
                            toadd = sadd.coords;
                            dist = 3;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (0 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = 0;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionUpContains(floodPointDesc::kBMaskDirNorth)) {
                sadd.coords.x = currentTile.tx;
                sadd.coords.y = currentTile.ty - 1;
                sadd.coords.z = currentTile.tz + 1;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (3 < dist) {
                            toadd = sadd.coords;
                            dist = 3;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (0 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = 0;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionUpContains(floodPointDesc::kBMaskDirWest)) {
                sadd.coords.x = currentTile.tx - 1;
                sadd.coords.y = currentTile.ty;
                sadd.coords.z = currentTile.tz + 1;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (3 < dist) {
                            toadd = sadd.coords;
                            dist = 3;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (0 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = 0;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
        }
        if (pfdp->dirl != 0) {
            if (pfdp->isDirectionDownContains(floodPointDesc::kBMaskDirSouth)) {
                sadd.coords.x = currentTile.tx;
                sadd.coords.y = currentTile.ty + 1;
                sadd.coords.z = currentTile.tz - 1;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (3 < dist) {
                            toadd = sadd.coords;
                            dist = 3;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (0 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = 0;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionDownContains(floodPointDesc::kBMaskDirEast)) {
                sadd.coords.x = currentTile.tx + 1;
                sadd.coords.y = currentTile.ty;
                sadd.coords.z = currentTile.tz - 1;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (3 < dist) {
                            toadd = sadd.coords;
                            dist = 3;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (0 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = 0;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionDownContains(floodPointDesc::kBMaskDirNorth)) {
                sadd.coords.x = currentTile.tx;
                sadd.coords.y = currentTile.ty - 1;
                sadd.coords.z = currentTile.tz - 1;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (3 < dist) {
                            toadd = sadd.coords;
                            dist = 3;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (0 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = 0;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionDownContains(floodPointDesc::kBMaskDirWest)) {
                sadd.coords.x = currentTile.tx - 1;
                sadd.coords.y = currentTile.ty;
                sadd.coords.z = currentTile.tz - 1;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (3 < dist) {
                            toadd = sadd.coords;
                            dist = 3;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (0 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = 0;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
        }
        if (pfdp->dirm != 0) {
            if (pfdp->isDirectionGroundContains(floodPointDesc::kBMaskDirSouth)) {
                sadd.coords.x = currentTile.tx;
                sadd.coords.y = currentTile.ty + 1;
                sadd.coords.z = currentTile.tz;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        unsigned char twd = m->mtsurfaces_[sadd.coords.x
                            + sadd.coords.y * m->mmax_x_
                            + sadd.coords.z * m->mmax_m_xy];
                        if (twd > 0x00 && twd < 0x05) {
                            if (3 < dist) {
                                toadd = sadd.coords;
                                dist = 3;
                            }
                        } else {
                            if (1 < dist) {
                                toadd = sadd.coords;
                                dist = 1;
                            }
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    unsigned char twd = m->mtsurfaces_[sadd.coords.x
                        + sadd.coords.y * m->mmax_x_
                        + sadd.coords.z * m->mmax_m_xy];
                    if (twd > 0x00 && twd < 0x05) {
                        if (-1 < dist) {
                            nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                            dist = -1;
                            toadd = sadd.coords;
                        }
                    } else {
                        if (-2 < dist) {
                            nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                            dist = -2;
                            toadd = sadd.coords;
                        }
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionGroundContains(floodPointDesc::kBMaskDirSouthEast)) {
                sadd.coords.x = currentTile.tx + 1;
                sadd.coords.y = currentTile.ty + 1;
                sadd.coords.z = currentTile.tz;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (2 < dist) {
                            toadd = sadd.coords;
                            dist = 2;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (-1 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = -1;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionGroundContains(floodPointDesc::kBMaskDirEast)) {
                sadd.coords.x = currentTile.tx + 1;
                sadd.coords.y = currentTile.ty;
                sadd.coords.z = currentTile.tz;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child== sadd.pNode->lvl) {
                        unsigned char twd = m->mtsurfaces_[sadd.coords.x
                            + sadd.coords.y * m->mmax_x_
                            + sadd.coords.z * m->mmax_m_xy];
                        if (twd > 0x00 && twd < 0x05) {
                            if (3 < dist) {
                                toadd = sadd.coords;
                                dist = 3;
                            }
                        } else {
                            if (1 < dist) {
                                toadd = sadd.coords;
                                dist = 1;
                            }
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    unsigned char twd = m->mtsurfaces_[sadd.coords.x
                        + sadd.coords.y * m->mmax_x_
                        + sadd.coords.z * m->mmax_m_xy];
                    if (twd > 0x00 && twd < 0x05) {
                        if (-1 < dist) {
                            nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                            dist = -1;
                            toadd = sadd.coords;
                        }
                    } else {
                        if (-2 < dist) {
                            nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                            dist = -2;
                            toadd = sadd.coords;
                        }
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionGroundContains(floodPointDesc::kBMaskDirNorthEast)) {
                sadd.coords.x = currentTile.tx + 1;
                sadd.coords.y = currentTile.ty - 1;
                sadd.coords.z = currentTile.tz;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (2 < dist) {
                            toadd = sadd.coords;
                            dist = 2;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (-1 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = -1;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionGroundContains(floodPointDesc::kBMaskDirNorth)) {
                sadd.coords.x = currentTile.tx;
                sadd.coords.y = currentTile.ty - 1;
                sadd.coords.z = currentTile.tz;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        unsigned char twd = m->mtsurfaces_[sadd.coords.x
                            + sadd.coords.y * m->mmax_x_
                            + sadd.coords.z * m->mmax_m_xy];
                        if (twd > 0x00 && twd < 0x05) {
                            if (3 < dist) {
                                toadd = sadd.coords;
                                dist = 3;
                            }
                        } else {
                            if (1 < dist) {
                                toadd = sadd.coords;
                                dist = 1;
                            }
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    unsigned char twd = m->mtsurfaces_[sadd.coords.x
                        + sadd.coords.y * m->mmax_x_
                        + sadd.coords.z * m->mmax_m_xy];
                    if (twd > 0x00 && twd < 0x05) {
                        if (-1 < dist) {
                            nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                            dist = -1;
                            toadd = sadd.coords;
                        }
                    } else {
                        if (-2 < dist) {
                            nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                            dist = -2;
                            toadd = sadd.coords;
                        }
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionGroundContains(floodPointDesc::kBMaskDirNorthWest)) {
                sadd.coords.x = currentTile.tx - 1;
                sadd.coords.y = currentTile.ty - 1;
                sadd.coords.z = currentTile.tz;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (2 < dist) {
                            toadd = sadd.coords;
                            dist = 2;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (-1 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = -1;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionGroundContains(floodPointDesc::kBMaskDirWest)) {
                sadd.coords.x = currentTile.tx - 1;
                sadd.coords.y = currentTile.ty;
                sadd.coords.z = currentTile.tz;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        unsigned char twd = m->mtsurfaces_[sadd.coords.x
                            + sadd.coords.y * m->mmax_x_
                            + sadd.coords.z * m->mmax_m_xy];
                        if (twd > 0x00 && twd < 0x05) {
                            if (3 < dist) {
                                toadd = sadd.coords;
                                dist = 3;
                            }
                        } else {
                            if (1 < dist) {
                                toadd = sadd.coords;
                                dist = 1;
                            }
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    unsigned char twd = m->mtsurfaces_[sadd.coords.x
                        + sadd.coords.y * m->mmax_x_
                        + sadd.coords.z * m->mmax_m_xy];
                    if (twd > 0x00 && twd < 0x05) {
                        if (-1 < dist) {
                            nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                            dist = -1;
                            toadd = sadd.coords;
                        }
                    } else {
                        if (-2 < dist) {
                            nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                            dist = -2;
                            toadd = sadd.coords;
                        }
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
            if (pfdp->isDirectionGroundContains(floodPointDesc::kBMaskDirSouthWest)) {
                sadd.coords.x = currentTile.tx - 1;
                sadd.coords.y = currentTile.ty + 1;
                sadd.coords.z = currentTile.tz;
                sadd.pNode = &(mdpmirror[sadd.coords.x
                    + sadd.coords.y * m->mmax_x_
                    + sadd.coords.z * m->mmax_m_xy]);
                if ((sadd.pNode->bfNodeDesc & ct) != 0) {
                    if (lvl_child == sadd.pNode->lvl) {
                        if (2 < dist) {
                            toadd = sadd.coords;
                            dist = 2;
                        }
                    }
                } else if(np && (sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint)) != 0) {
                    if (-1 < dist) {
                        nt = sadd.pNode->bfNodeDesc & (m_fdBasePoint | m_fdTargetPoint);
                        dist = -1;
                        toadd = sadd.coords;
                    }
                }
                if ((sadd.pNode->bfNodeDesc & m_fdConstant) != 0)
                    tnr = false;
            }
        }
        if (dist < 1) {
            np = false;
            ct = nt;
        }
        pathToDestination.push_back(TilePoint(toadd.x, toadd.y, toadd.z));
        // this assert might save from memory fill up,
        assert(currentTile.tx != toadd.x || currentTile.ty != toadd.y || currentTile.tz != toadd.z);

        currentTile.tx = toadd.x;
        currentTile.ty = toadd.y;
        currentTile.tz = toadd.z;
    } while (tnr);
}

void PedInstance::buildFinalDestinationPath(Mission *m, std::vector<TilePoint> &cdestpath, const TilePoint &destinationPt) {
    TilePoint prvpn = TilePoint(pos_.tx, pos_.ty, pos_.tz, pos_.ox, pos_.oy);
    for (std::vector <TilePoint>::iterator it = cdestpath.begin();
            it != cdestpath.end(); ++it) {
        std::vector <TilePoint>::iterator fit = it + 1;
        bool modified = false;
        unsigned char twd = m->mtsurfaces_[prvpn.tx
            + prvpn.ty * m->mmax_x_
            + prvpn.tz * m->mmax_m_xy];
        unsigned char twdn = m->mtsurfaces_[it->tx
            + it->ty * m->mmax_x_
            + it->tz * m->mmax_m_xy];
        char xf = prvpn.tx - it->tx;
        char yf = prvpn.ty - it->ty;
        char zf = prvpn.tz - it->tz;
            if (twd > 0x0 && twd < 0x05) {
                if (twdn > 0x0 && twdn < 0x05) {
                    dest_path_.push_back(*it);
                } else {
                    switch (twd) {
                        case Tile::kSlopeSN:
                            if (zf == -1) {
                                if (xf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (xf == 1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (xf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 0) {
                                if (xf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (xf == 1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (xf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 1)
                                dest_path_.push_back(*it);
                            break;
                        case 0x02:
                            if (zf == -1) {
                                if (xf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (xf == 1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (xf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 0) {
                                if (xf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (xf == 1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (xf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 1)
                                dest_path_.push_back(*it);
                            break;
                        case 0x03:
                            if (zf == -1) {
                                if (yf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (yf == 1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (yf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 0) {
                                if (yf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (yf == 1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (yf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 1)
                                dest_path_.push_back(*it);
                            break;
                        case 0x04:
                            if (zf == -1) {
                                if (yf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (yf == 1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (yf == -1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 0) {
                                if (yf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (yf == 1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (yf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 1)
                                dest_path_.push_back(*it);
                            break;
                    }
                }
            } else {
                if (twdn > 0x0 && twdn < 0x05) {
                    switch (twdn) {
                        case 0x01:
                            if (zf == 1) {
                                if (xf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (xf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (xf == 1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 0) {
                                if (xf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (xf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (xf == 1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == -1)
                                dest_path_.push_back(*it);
                            break;
                        case 0x02:
                            if (zf == 1) {
                                if (xf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (xf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (xf == 1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 0) {
                                if (xf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (xf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (xf == 1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == -1)
                                dest_path_.push_back(*it);
                            break;
                        case 0x03:
                            if (zf == 1) {
                                if (yf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (yf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (yf == 1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 0) {
                                if (yf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (yf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (yf == 1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == -1)
                                dest_path_.push_back(*it);
                            break;
                        case 0x04:
                            if (zf == 1) {
                                if (yf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (yf == -1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (yf == 1) {
                                    prvpn.ox = 0;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 0;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == 0) {
                                if (yf == 0) {
                                    dest_path_.push_back(*it);
                                    break;
                                }
                                if (yf == -1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 255;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 0;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                                if (yf == 1) {
                                    prvpn.ox = 255;
                                    prvpn.oy = 0;
                                    dest_path_.push_back(prvpn);
                                    it->ox = 255;
                                    it->oy = 255;
                                    dest_path_.push_back(*it);
                                    modified = true;
                                    break;
                                }
                            }
                            if (zf == -1)
                                dest_path_.push_back(*it);
                            break;
                    }
                } else {
                    dest_path_.push_back(*it);
                }
            }
        prvpn = *it;
        if (fit == cdestpath.end()) {
            if (modified) {
                dest_path_.push_back(TilePoint(destinationPt));
            } else {
                // untill correct smoothing implemented this
                // will prevent walking on non-walkable tile
                if (xf == -1 && yf == -1) {
                    dest_path_.back().ox = 0;
                    dest_path_.back().oy = 0;
                    dest_path_.push_back(prvpn);
                }
                if (xf == 1 && yf == -1) {
                    dest_path_.back().ox = 255;
                    dest_path_.back().oy = 0;
                    dest_path_.push_back(prvpn);
                }
                if (xf == 1 && yf == 1) {
                    dest_path_.back().ox = 255;
                    dest_path_.back().oy = 255;
                    dest_path_.push_back(prvpn);
                }
                if (xf == -1 && yf == 1) {
                    dest_path_.back().ox = 0;
                    dest_path_.back().oy = 255;
                    dest_path_.push_back(prvpn);
                }
                dest_path_.back().ox = destinationPt.ox;
                dest_path_.back().oy = destinationPt.oy;
            }
        }
    }

#ifdef EXECUTION_SPEED_TIME
    printf("smoothing time %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif
}

bool PedInstance::doMove(int elapsed, Mission *pMission)
{
    bool updated = false;
    int used_time = elapsed;

    while ((!dest_path_.empty()) && used_time != 0) {
        int nxtTileX = dest_path_.front().tx;
        int nxtTileY = dest_path_.front().ty;
        int nxtTileZ = dest_path_.front().tz;
        if (hold_on_.wayFree != 0 && hold_on_.pathBlocker->isPathBlocker()) {
            if (hold_on_.xadj || hold_on_.yadj) {
                if(abs(hold_on_.tilex - nxtTileX) <= hold_on_.xadj
                    && abs(hold_on_.tiley - nxtTileY) <= hold_on_.yadj
                    && hold_on_.tilez == nxtTileZ)
                {
                    if (hold_on_.wayFree == 1)
                        return updated;
                    // hold_on_.wayFree == 2
                    dest_path_.clear();
                    speed_ = 0;
                    return updated;
                }
            } else {
                if (hold_on_.tilex == nxtTileX && hold_on_.tiley == nxtTileY
                    && hold_on_.tilez == nxtTileZ)
                {
                    if (hold_on_.wayFree == 1)
                        return updated;
                    // hold_on_.wayFree == 2
                    dest_path_.clear();
                    speed_ = 0;
                    return updated;
                }
            }
        } else
            hold_on_.wayFree = 0;
        // TODO: not ignore Z, if tile is stairs diffz is wrong
        int adx =
             nxtTileX * 256 + dest_path_.front().ox;
        int ady =
             nxtTileY * 256 + dest_path_.front().oy;
        int atx = pos_.tx * 256 + pos_.ox;
        int aty = pos_.ty * 256 + pos_.oy;
        int diffx = adx - atx, diffy = ady - aty;

        if (abs(diffx) < 16 && abs(diffy) < 16) {
            // TODO: maybe something better? then using diffx/diffy?
            // for this check
            pos_.oy = dest_path_.front().oy;
            pos_.ox = dest_path_.front().ox;
            pos_.tz = nxtTileZ;
            pos_.ty = nxtTileY;
            pos_.tx = nxtTileX;
            dest_path_.pop_front();
            if (dest_path_.empty())
                speed_ = 0;
            updated = true;
        } else {
            setDirection(diffx, diffy, &dir_);

            int dx = 0, dy = 0;
            double d = sqrt((double)(diffx * diffx + diffy * diffy));
            // object will not move over a distance he can actually move
            double avail_time_use = (d / (double)speed_) * 1000.0;
            // correcting time availiable for this distance to time
            // we can use
            if (avail_time_use > used_time)
                avail_time_use = used_time;

            if (abs(diffx) > 0)
                // dx = diffx * (speed_ * used_time / 1000) / d;
                dx = (int)((diffx * (speed_ * avail_time_use) / d) / 1000);
            if (abs(diffy) > 0)
                // dy = diffy * (speed_ * used_time / 1000) / d;
                dy = (int)((diffy * (speed_ * avail_time_use) / d) / 1000);

            if (dx || dy) {
                int prv_time = used_time;
                if (dx) {
                    used_time -= (int)(((double) dx * 1000.0 * d)
                        / (double)(diffx * speed_));
                } else if (dy) {
                    used_time -= (int)(((double) dy * 1000.0 * d)
                        / (double)(diffy * speed_));
                } else
                    used_time = 0;
                if (used_time < 0 || prv_time == used_time)
                    used_time = 0;
            } else
                used_time = 0;

            addOffsetToPosition(dx, dy);
            // TODO : what obstacles? cars? doors are already
            // setting stop signal, reuse it?
#if 0
            if (addOffsetToPosition(dx, dy)) {
                ;
            } else {
                // TODO: avoid obstacles.
                speed_ = 0;
            }
#endif
            if(nxtTileX == pos_.tx && nxtTileY == pos_.ty)
                pos_.tz = nxtTileZ;

            if(nxtTileX == pos_.tx && nxtTileY == pos_.ty
                && nxtTileZ == pos_.tz
                && dest_path_.front().ox == pos_.ox
                && dest_path_.front().oy == pos_.oy)
            {
                dest_path_.pop_front();
            }
            if (dest_path_.size() == 0)
                speed_ = 0;

            updated = true;
        }

        if ((state_ & pa_smFollowing) != 0) {
            // TODO: too big elapsed makes ped move to close to target
            // possible solution will be to use movedir like movement
            // and calculate distance at every step, but it is
            // a high cpu consuming
            if (!dest_path_.empty()) {
                WorldPoint wpt(dest_path_.back());
                double dist_cur = distanceToPosition(wpt);
                if (dist_cur < (double)dist_to_pos_) {
                    dest_path_.clear();
                    speed_ = 0;
                }
            }
        }

        offzOnStairs(pMission->mtsurfaces_[pos_.tx + pos_.ty * pMission->mmax_x_
            + pos_.tz * pMission->mmax_m_xy]);
    }
#ifdef _DEBUG
    if (dest_path_.empty() && speed_) {
        printf("Was running at speed %i, destination unknown\n", speed_);
        speed_ = 0;
    }
#endif

    return updated;
}

/*! \returns bitmask :
 * 0b(1) - success, 1b(2) - bounced, 2b(4) - need bounce (for bounce forbidden),
 * 3b(8) - non-walkable tile as base, 4b(16) - wrong direction,
 * 5b(32) - dist passed set, 6b(64) -  bouncing restored original dir
 * (loop is possible)
 */
uint8 PedInstance::moveToDir(Mission* m, int elapsed, DirMoveType &dir_move,
    int dir, int t_posx, int t_posy, int* dist, bool set_dist)
{
    // TODO: better non-posiotional random walking
    floodPointDesc *based = &(m->mdpoints_[pos_.tx
        + pos_.ty * m->mmax_x_ + pos_.tz * m->mmax_m_xy]);
    if (based->bfNodeDesc == m_fdNonWalkable) {
        printf("==== unwalk pos: x %i; y %i; z %i, ox %i, oy %i, oz %i\n",
            pos_.tx, pos_.ty, pos_.tz, pos_.ox, pos_.oy, pos_.oz);
        printf("moveToDir, Movement from nonwalkable postion\n");
        return 8;
    }

    // TODO: set safewalk need, somewhere
    bool check_safe_walk = dir_move.safe_walk;

    // TODO: find safewalk tile and use normal pathfinding
    // to get there
    if ((based->bfNodeDesc & m_fdSafeWalk) == 0)
        check_safe_walk = false;
    bool move_to_pos = false;
    if (dir == -1) {
        if (t_posx != -1 && t_posy != -1) {
            setDirection(t_posx - pos_.tx * 256 - pos_.ox,
                t_posy - pos_.ty * 256 - pos_.oy, &dir);
            if (dir == -1)
                return 16;
            move_to_pos = true;
            if (dir_move.dir_modifier == 0)
                dir_move.dir_orig = dir;
        } else {
            dir = dir_;
            dir_move.dir_orig = dir;
        }
    }

    double dist_curr = (elapsed * speed_) / 1000.0;
    if (dist == NULL || (dist && *dist == 0)) {
         if (dist_to_pos_ > 0 && (int)dist_curr > dist_to_pos_)
             dist_curr = (double) dist_to_pos_;
    } else if ((int) dist_curr > (*dist))
        dist_curr = (double)(*dist);
    bool should_bounce = dir_move.bounce;

    if (dir_move.dir_modifier != 0) {
        dir = dir_move.dir_last;
    }
    double dist_total = 0;
    uint8 move_mask = 1;

    while ((int)dist_curr > 0) {
        bool need_bounce = false;
        double diffx = 0.0, diffy = 0.0;
        if (dir == 0) {
            diffy = 1.0;
        } else if (dir == 64) {
            diffx = 1.0;
        } else if (dir == 128) {
            diffy = -1.0;
        } else if (dir == 192) {
            diffx = -1.0;
        } else if (dir < 64) {
            diffx = sin((dir / 128.0) * PI);
            diffy = cos((dir / 128.0) * PI);
        } else if (dir < 128) {
            int dirn = dir % 64;
            diffy = -sin((dirn / 128.0) * PI);
            diffx = cos((dirn / 128.0) * PI);
        } else if (dir < 192) {
            int dirn = dir % 64;
            diffx = -sin((dirn / 128.0) * PI);
            diffy = -cos((dirn / 128.0) * PI);
        } else if (dir < 256) {
            int dirn = dir % 64;
            diffy = sin((dirn / 128.0) * PI);
            diffx = -cos((dirn / 128.0) * PI);
        }

        double posx = (double)(pos_.tx * 256 + pos_.ox);
        double posy = (double)(pos_.ty * 256 + pos_.oy);
        floodPointDesc *fpd = &(m->mdpoints_[pos_.tx + pos_.ty * m->mmax_x_ +
            pos_.tz * m->mmax_m_xy]);
        double dist_passsed = 0;
        double dist_inc = sqrt(diffx * diffx + diffy * diffy);

        do {
            double px = posx + diffx;
            double py = posy + diffy;
            int tilenx = diffx >= 0.0 ? ((int)ceil(px)) / 256
                : ((int)floor(px)) / 256;
            int tileny = diffy >= 0.0 ? ((int)ceil(py)) / 256
                : ((int)floor(py)) / 256;
            if (tilenx < 0 ||  tileny < 0 || tilenx >= m->mmax_x_
                || tileny >= m->mmax_y_)
            {
                need_bounce = true;
                break;
            }
            if (pos_.tx != tilenx || pos_.ty != tileny) {
                // TODO: check for stairs and offset should be correct,
                // to avoid jumping on top of stairs
                int32 dec_z = 0;
                if (tilenx - pos_.tx == 0) {
                    if (tileny - pos_.ty > 0) {
                        if ((fpd->dirh & 0x01) == 0x01) {
                            ++pos_.tz;
                            --dec_z;
                        } else if ((fpd->dirl & 0x01) == 0x01) {
                            --pos_.tz;
                            ++dec_z;
                        } else if ((fpd->dirm & 0x01) != 0x01) {
                            need_bounce = true;
                            break;
                        }
                    } else {
                        if ((fpd->dirh & 0x10) == 0x10) {
                            ++pos_.tz;
                            --dec_z;
                        } else if ((fpd->dirl & 0x10) == 0x10) {
                            --pos_.tz;
                            ++dec_z;
                        } else if ((fpd->dirm & 0x10) != 0x10) {
                            need_bounce = true;
                            break;
                        }
                    }
                } else if (tileny - pos_.ty == 0) {
                    if (tilenx - pos_.tx > 0) {
                        if ((fpd->dirh & 0x04) == 0x04) {
                            ++pos_.tz;
                            --dec_z;
                        } else if ((fpd->dirl & 0x04) == 0x04) {
                            --pos_.tz;
                            ++dec_z;
                        } else if ((fpd->dirm & 0x04) != 0x04) {
                            need_bounce = true;
                            break;
                        }
                    } else {
                        if ((fpd->dirh & 0x40) == 0x40) {
                            ++pos_.tz;
                            --dec_z;
                        } else if ((fpd->dirl & 0x40) == 0x40) {
                            --pos_.tz;
                            ++dec_z;
                        } else if ((fpd->dirm & 0x40) != 0x40) {
                            need_bounce = true;
                            break;
                        }
                    }
                } else if (tileny - pos_.ty > 0) {
                    if (tilenx - pos_.tx > 0) {
                        if ((fpd->dirm & 0x02) != 0x02) {
                            need_bounce = true;
                            break;
                        }
                    } else {if ((fpd->dirm & 0x80) != 0x80) {
                            need_bounce = true;
                            break;
                        }
                    }
                } else {
                    // (tileny - pos_.ty < 0)
                    if (tilenx - pos_.tx > 0) {
                        if ((fpd->dirm & 0x08) != 0x08) {
                            need_bounce = true;
                            break;
                        }
                    } else {if ((fpd->dirm & 0x20) != 0x20) {
                            need_bounce = true;
                            break;
                        }
                    }
                }

#if 0
#ifdef _DEBUG
                if (getDebugID() == 8) {
                    printf("x %i, y %i, z %i\n", pos_.tx, pos_.ty, pos_.tz + dec_z);
                    printf("nx %i, ny %i, nz %i\n", tilenx, tileny, pos_.tz);
                }
#endif
#endif

                floodPointDesc *fpd_prv = fpd;
                fpd = &(m->mdpoints_[tilenx + tileny * m->mmax_x_
                    + pos_.tz * m->mmax_m_xy]);
                if (check_safe_walk && (fpd->bfNodeDesc & m_fdSafeWalk) == 0) {
                    pos_.tz += dec_z;
                    need_bounce = true;
                    break;
                }

                pos_.tx = tilenx;
                pos_.ty = tileny;
                if (dir_move.dir_modifier != 0) {
                    dist_passsed += dist_inc;
                    posx = px;
                    posy = py;
                    if (move_to_pos) {
                        dir_move.on_new_tile = true;
                        // avoiding direction recovery where tile is same as
                        // the one that forced us to bounce
                        if (fpd_prv->dirh != fpd->dirh
                            || fpd_prv->dirm != fpd->dirm
                            || fpd_prv->dirl != fpd->dirl)
                        {
                            if (dir_move.dir_modifier < 3) {
                                dir_move.dir_modifier = 0;
                                dir_move.dir_last = -1;
                                dir = dir_move.dir_orig;
                            } else {
                                dir_move.dir_modifier -= 2;
                                // & 0x00C0 = ((% 256) / 64) * 64
                                dir = (256 + dir - dir_move.modifier_value) & 0x00C0;
                                dir_move.dir_last = dir;
                            }
                        }
                    } else {
                        int rand_inc = rand();
                        if ((rand_inc & 0x00FF) < 32) {
                            dir_move.dir_last = -1;
                            dir_move.dir_modifier = 0;
                            // & 0x003F = % 64, & 0x00FF = % 256
                            dir = (256 + dir + (rand_inc & 0x003F) - 32) & 0x00FF;
                        }
                    }
                    break;
                }
            }
            dist_passsed += dist_inc;
            posx = px;
            posy = py;
        } while (dist_passsed < dist_curr);

        if (diffx >= 0.0)
            // & 0x00FF = % 256
            pos_.ox = ((int)ceil(posx)) & 0x00FF;
        else
            pos_.ox = ((int)floor(posx)) & 0x00FF;
        if (diffy >= 0.0)
            pos_.oy = ((int)ceil(posy)) & 0x00FF;
        else
            pos_.oy = ((int)floor(posy)) & 0x00FF;

        dist_curr -= dist_passsed;
        if (set_dist)
            dist_total += dist_passsed;

        if (need_bounce && should_bounce) {
            move_mask |= 2;
            if (move_to_pos) {
                if (dir_move.dir_modifier == 0) {
                    dir_move.modifier_value = 64;
                    dir_move.modifier_value *= getClosestDirs(dir_move.dir_orig,
                        dir_move.dir_closest, dir_move.dir_closer);;
                    dir_move.dir_modifier = 1;
                    dir = dir_move.dir_closest;
                    dir_move.dir_last = dir_move.dir_closest;
                    dir_move.on_new_tile = false;
                } else if (dir_move.dir_modifier == 1) {
                    if (dir_move.on_new_tile) {
                        dir_move.dir_modifier += 2;
                        // & 0x00C0 = ((% 256) / 64) * 64
                        // dir based on closest
                        dir = (256 + dir + dir_move.modifier_value) & 0x00C0;
                    } else {
                        dir_move.modifier_value *= -1;
                        dir_move.dir_modifier = 4;
                        dir = dir_move.dir_closer;
                        dir_move.dir_last = dir_move.dir_closer;
                    }
                    dir_move.dir_last = dir;

                // dir based on closest
                } else if (dir_move.dir_modifier == 3) {
                    dir = (256 + dir + dir_move.modifier_value) & 0x00C0;
                    dir_move.dir_last = dir;
                    dir_move.dir_modifier += 2;
                } else if (dir_move.dir_modifier == 5) {
                    dir = (256 + dir + dir_move.modifier_value) & 0x00C0;
                    dir_move.dir_last = dir;
                    dir_move.dir_modifier += 2;

                // dir based on closer
                } else if (dir_move.dir_modifier == 2) {
                    dir = dir_move.dir_closer;
                    dir_move.dir_last = dir_move.dir_closer;
                    dir_move.dir_modifier += 2;
                } else if (dir_move.dir_modifier == 4) {
                    dir = (256 + dir + dir_move.modifier_value) & 0x00C0;
                    dir_move.dir_last = dir;
                    dir_move.dir_modifier += 2;
                } else if (dir_move.dir_modifier == 6) {
                    dir = (256 + dir + dir_move.modifier_value) & 0x00C0;
                    dir_move.dir_last = dir;
                    dir_move.dir_modifier += 2;
                } else if (dir_move.dir_modifier == 8) {
                    dir = (256 + dir + dir_move.modifier_value) & 0x00C0;
                    dir_move.dir_last = dir;
                    dir_move.dir_modifier += 2;
                } else {
                    dir_move.dir_modifier = 0;
                    dir_move.dir_last = -1;
                    dir = dir_move.dir_orig;
                    move_mask |= 64;
                    break;
                }
            } else if (dir_move.dir_modifier) {
                if (dir_move.dir_modifier == 2)
                    dir_move.dir_last += 64;
                if (dir_move.dir_modifier == 3) {
                    dir_move.dir_last += (256 - 64);
                }
                dir_move.dir_last &= 0x00FF;
                dir = dir_move.dir_last;
                --dir_move.dir_modifier;
            } else {
                dir_move.dir_last = (dir_move.dir_last + 64) & 0x00FF;
                dir = dir_move.dir_last;
                dir_move.dir_modifier = 1;
            }
            setDirection(dir);
        } else if (!move_to_pos && dir_move.dir_modifier != 0) {
            setDirection(dir);
            if (dir_move.dir_modifier == 1) {
                ++dir_move.dir_modifier;
                dir_move.dir_last += (256 - 64);
            }
            if (dir_move.dir_modifier == 2) {
                ++dir_move.dir_modifier;
                dir_move.dir_last += 128;
            }
            if (dir_move.dir_modifier == 3) {
                ++dir_move.dir_modifier;
                dir_move.dir_last += (256 - 64);
            }
            if (dir_move.dir_modifier == 3) {
                dir_move.dir_modifier = 0;
                dir_move.dir_last += (256 - 64);
            }
            // &0x00FF = % 256
            dir_move.dir_last &= 0x00FF;
            dir = dir_move.dir_last;
        } else {
            setDirection(dir);
            if (need_bounce) {
                move_mask ^= 1;
                move_mask |= 4;
                break;
            }
        }
    }
    offzOnStairs(m->mtsurfaces_[pos_.tx + pos_.ty * m->mmax_x_
        + pos_.tz * m->mmax_m_xy]);
    if (set_dist && dist != NULL)
        *dist = (int)dist_total;

    return move_mask;
}

inline int PedInstance::getClosestDirs(int dir, int& closest, int& closer) {
    // & 0x003F = % 64
    int mod = dir & 0x003F;
    if (mod == 0) {
        // & 0x00C0 = ((% 256) / 64) * 64
        closest = (dir + 64) & 0x00C0;
        closer = (256 + dir - 64) & 0x00C0;
        return 1;
    } else {
        if (mod < 32) {
            closest = dir & 0x00C0;
            closer = (dir + 64) & 0x00C0;
            return -1;
        } else {
            closest = (dir + 64) & 0x00C0;
            closer = dir & 0x00C0;
        }
    }
    return 1;
}

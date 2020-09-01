/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2013  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "app.h"
#include "mission.h"
#include "core/gamesession.h"
#include "gfx/screen.h"
#include "vehicle.h"
#include "model/shot.h"

const uint8 Vehicle::kVehicleTypeLargeArmored = 0x01;
const uint8 Vehicle::kVehicleTypeLargeArmoredDamaged = 0x04;
const uint8 Vehicle::kVehicleTypeTrainHead = 0x05;
const uint8 Vehicle::kVehicleTypeTrainBody = 0x09;
const uint8 Vehicle::kVehicleTypeRegularCar = 0x0D;
const uint8 Vehicle::kVehicleTypeFireFighter = 0x11;
const uint8 Vehicle::kVehicleTypeSmallArmored = 0x1C;
const uint8 Vehicle::kVehicleTypePolice = 0x24;
const uint8 Vehicle::kVehicleTypeMedics = 0x28;

VehicleAnimation::VehicleAnimation() {
    vehicle_anim_ = kNormalAnim;
}

void VehicleAnimation::draw(int x, int y, int dir, int frame)
{
    switch (vehicle_anim_) {
        case kNormalAnim:
            g_App.gameSprites().drawFrame(anims_ + dir * 2, frame, x, y);
            break;
        case kOnFireAnim:
            g_App.gameSprites().drawFrame(anims_burning_ + dir, frame, x, y);
            break;
        case kBurntAnim:
            g_App.gameSprites().drawFrame(anims_burnt_ + dir, frame, x, y);
            break;
    }
}

void VehicleAnimation::set_base_anims(int anims) {
    anims_ = anims;
    anims_burning_ = anims + 8;
    anims_burnt_ = anims + 12;
}

void Vehicle::draw(int x, int y)
{
    y += TILE_HEIGHT / 3;
    addOffs(x, y);

    // ensure on map
    if (x < 90 || y < -20)
        return;

    animation_->draw(x, y, getDirection(4), frame_);
}

bool Vehicle::animate(int elapsed)
{
    bool updated = false;

    if (health_ > 0) {
        updated = doMove(elapsed, NULL);
    }

    if (animation_->animation_type() == VehicleAnimation::kOnFireAnim) {
        if (leftTimeShowAnim(elapsed))
            updated |= MapObject::animate(elapsed);
        else {
            animation_->set_animation_type(VehicleAnimation::kBurntAnim);
            frame_ = 0;
            updated = true;
        }
    }

    return updated;
}

/**
 * Adds given ped to the list of passengers.
 * \param pPed PedInstance*
 * \return void
 *
 */
void Vehicle::addPassenger(PedInstance *pPed) {
    if(!containsPed(pPed)) {
        passengers_.push_back(pPed);
        pPed->putInVehicle(this);
    }
}

/*!
 * Removes given passenger from vehicle.
 * \param pPed Ped to remove
 */
void Vehicle::dropPassenger(PedInstance *pPed) {
    for (std::list<PedInstance *>::iterator it = passengers_.begin();
        it != passengers_.end(); it++)
        {
            if ((*it)->id() == pPed->id()) {
                pPed->leaveVehicle();
                passengers_.erase(it);
                return;
            }
        }
}

/*!
 * Returns true if at least one of our agent is inside the vehicle.
 */
bool Vehicle::containsOurAgents() {
    for (std::list<PedInstance *>::iterator it = passengers_.begin();
        it != passengers_.end(); it++)
    {
        if ((*it)->isOurAgent()) {
            return true;
        }
    }
    return false;
}

/*!
 * Returns true if the vehicle contains peds considered hostile by the given ped.
 * \param pPed The ped evaluating the hostility of the vehicle
 * \param hostile_desc_alt Parameter for evaluating the hostility
 * \return True if at least one hostile ped is found.
 */
bool Vehicle::containsHostilesForPed(PedInstance* p,
                                          unsigned int hostile_desc_alt)
{
    for (std::list<PedInstance *>::iterator it = passengers_.begin();
        it != passengers_.end(); it++)
    {
        if (p->isHostileTo((ShootableMapObject *)(*it), hostile_desc_alt))
            return true;
    }
    return false;
}

GenericCar::GenericCar(VehicleAnimation * pAnimation, uint16 anId, uint8 aType, int m):
    Vehicle(anId, aType, m, pAnimation)
{
    pDriver_ = NULL;
    hold_on_.wayFree = 0;
}

uint16 GenericCar::tileDir(int x, int y, int z) {
    uint16 dir = 0x0;
    int near_tile;
    Map *pMap = g_App.maps().map(map());

    switch(pMap->tileAt(x, y, z)){
        case 80:
            if(g_App.maps().map(map())->tileAt(x + 1, y, z) == 80)
                dir = (0)|(0xFFF0);
            if(pMap->tileAt(x - 1, y, z) == 80)
                dir = (4<<8)|(0xF0FF);
            break;
        case 81:
            if(pMap->tileAt(x, y - 1, z) == 81)
                dir = (2<<4)|(0xFF0F);
            if(pMap->tileAt(x, y + 1, z) == 81)
                dir = (6<<12)|(0x0FFF);
            break;
        case 106:
            dir = (0)|(2<<4)|(6<<12)|(0x0F00);

            if(pMap->tileAt(x + 1, y - 1, z) != 118)
                dir |= 0x0FF0;
            if(pMap->tileAt(x + 1, y + 1, z) != 118)
                dir |= 0xFF00;
            near_tile = pMap->tileAt(x + 1, y, z);
            if (near_tile == 108 || near_tile == 109)
                dir = (dir & 0x0FFF) | 0x6000;

            break;
        case 107:
            dir = (2<<4)|(4<<8)|(6<<12)|(0x000F);

            if(pMap->tileAt(x - 1, y - 1, z) != 118)
                dir |= 0x00FF;
            if(pMap->tileAt(x - 1, y + 1, z) != 118)
                dir |= 0xF00F;
            near_tile = pMap->tileAt(x - 1, y, z);
            if (near_tile == 108 || near_tile == 109)
                dir = (dir & 0xFF0F) | 0x0020;

            break;
        case 108:
            dir = (0)|(2<<4)|(4<<8)|(0xF000);

            if(pMap->tileAt(x + 1, y - 1, z) != 118)
                dir |= 0xF00F;
            if(pMap->tileAt(x - 1, y - 1, z) != 118)
                dir |= 0xFF00;
            near_tile = pMap->tileAt(x, y - 1, z);
            if (near_tile == 106 || near_tile == 107)
                dir = dir & 0xFFF0;

            break;
        case 109:
            dir = (0)|(4<<8)|(6<<12)|(0x00F0);

            if(pMap->tileAt(x + 1, y + 1, z) != 118)
                dir |= 0x00FF;
            if(pMap->tileAt(x - 1, y + 1, z) != 118)
                dir |= 0x0FF0;
            near_tile = pMap->tileAt(x, y + 1, z);
            if (near_tile == 106 || near_tile == 107)
                dir = (dir & 0xF0FF) | 0x0400;

            break;
        case 110:
            dir = (0) | (2<<4)|(0xFF00);
            break;
        case 111:
            dir = (0) | (6<<12)|(0x0FF0);
            break;
        case 112:
            dir = (2<<4)|(4<<8)|(0xF00F);
            break;
        case 113:
            dir = (4<<8)|(6<<12)|(0x00FF);
            break;
        /*case 119:
            // TODO: Greenland map needs fixing
            dir = 0xFFFF;
            near_tile = pMap->tileAt(x, y + 1, z);
            if (near_tile == 107 || near_tile == 225 || near_tile == 226)
                dir = (dir & 0xF0FF) | 0x0400;
            near_tile = pMap->tileAt(x, y + 1, z);
            if (near_tile == 106 || near_tile == 225 || near_tile == 226)
               dir &= 0xFFF0;
            near_tile = pMap->tileAt(x + 1, y, z);
            if (near_tile == 109 || near_tile == 225 || near_tile == 226)
                dir = (dir & 0xFF0F) | 0x0020;
            near_tile = pMap->tileAt(x - 1, y, z);
            if (near_tile == 108 || near_tile == 225 || near_tile == 226)
                dir = (dir & 0x0FFF) | 0x6000;
            if (dir ==0xFFFF)
                dir = 0x0;
            break;*/
        case 120:
            dir = (0)|(2<<4)|(0xFF00);
            break;
        case 121:
            dir = (0)|(6<<12)|(0x0FF0);
            break;
        case 122:
            dir = (4<<8)|(6<<12)|(0x00FF);
            break;
        case 123:
            dir = (2<<4)|(4<<8)|(0xF00F);
            break;
        case 225:/*
            if(g_App.maps().map(map())->getTileAt(x + 1, y, z)->type() == Tile::kRoadPedCross)
                dir = (0)|(0xFFF0);
            else if(g_App.maps().map(map())->getTileAt(x - 1, y, z)->type() == Tile::kRoadPedCross)
                dir = (4<<8)|(0xF0FF);
            else {*/
                dir = 0xFFFF;
                near_tile = pMap->tileAt(x, y + 1, z);
                if (/*near_tile == 119 || */near_tile == 106
                    || near_tile == 107 || near_tile == 80 || near_tile == 225)
                    dir = (dir & 0xF0FF) | 0x0400;
                near_tile = pMap->tileAt(x, y - 1, z);
                if (/*near_tile == 119 || */near_tile == 106
                    || near_tile == 107 || near_tile == 80 || near_tile == 225)
                    dir &= 0xFFF0;
                near_tile = pMap->tileAt(x + 1, y, z);
                if (/*near_tile == 119 || */near_tile == 108 || near_tile == 81)
                    dir = (dir & 0xFF0F) | 0x0020;
                near_tile = pMap->tileAt(x - 1, y, z);
                if (/*near_tile == 119 || */near_tile == 109 || near_tile == 81)
                    dir = (dir & 0x0FFF) | 0x6000;
                if (dir == 0xFFFF)
                    dir = 0x0;
            //}
            break;
        case 226:/*
            if(g_App.maps().map(map())->getTileAt(x, y - 1, z)->type() == Tile::kRoadPedCross)
                dir = (2<<4)|(0xFF0F);
            else if(g_App.maps().map(map())->getTileAt(x, y + 1, z)->type() == Tile::kRoadPedCross)
                dir = (6<<12)|(0x0FFF);
            else {*/
                dir = 0xFFFF;
                near_tile = pMap->tileAt(x, y + 1, z);
                if (/*near_tile == 119 || */near_tile == 106 || near_tile == 80)
                    dir = (dir & 0xF0FF) | 0x0400;
                near_tile = pMap->tileAt(x, y - 1, z);
                if (/*near_tile == 119 || */near_tile == 107 || near_tile == 80)
                    dir &= 0xFFF0;
                near_tile = pMap->tileAt(x + 1, y, z);
                if (/*near_tile == 119 || */near_tile == 108 || near_tile == 109
                    || near_tile == 81 || near_tile == 226)
                    dir = (dir & 0xFF0F) | 0x0020;
                near_tile = pMap->tileAt(x - 1, y, z);
                if (/*near_tile == 119 || */near_tile == 108 || near_tile == 109
                    || near_tile == 81 || near_tile == 226)
                    dir = (dir & 0x0FFF) | 0x6000;
                if (dir == 0xFFFF)
                    dir = 0;
            //}
            break;
        default:
            dir = 0xFFFF;
    }

    return dir;
}

bool GenericCar::dirWalkable(TilePoint *p, int x, int y, int z) {
    Map *pMap = g_App.maps().map(map());
    if(!(pMap->isTileWalkableByCar(x,y,z)))
        return false;

    uint16 dirStart = tileDir(p->tx,p->ty,p->tz);
    uint16 dirEnd = tileDir(x,y,z);
    if (dirStart == 0x0 || dirEnd == 0x0)
        return false;
    if (dirStart == 0xFFFF || dirEnd == 0xFFFF)
        return true;

    if (((dirStart & 0xF000) != 0xF000)
        || ((dirEnd & 0xF000) != 0xF000))
        if ((dirStart & 0xF000) == (dirEnd & 0xF000))
                return true;
    if (((dirStart & 0x0F00) != 0x0F00)
        || ((dirEnd & 0x0F00) != 0x0F00))
        if ((dirStart & 0x0F00) == (dirEnd & 0x0F00))
                return true;
    if (((dirStart & 0x00F0) != 0x00F0)
        || ((dirEnd & 0x00F0) != 0x00F0))
        if ((dirStart & 0x00F0) == (dirEnd & 0x00F0))
                return true;
    if (((dirStart & 0x000F) != 0x000F)
        || ((dirEnd & 0x000F) != 0x000F))
        if ((dirStart & 0x000F) == (dirEnd & 0x000F))
                return true;

    return false;
}

/*!
 * Sets a destination point for the vehicle to reach at given speed.
 * \param m
 * \param locT destination point
 * \param newSpeed Speed of movement
 * \return true if destination has been set correctly.
 */
bool GenericCar::initMovementToDestination(Mission *pMission, const TilePoint &destinationPt, int newSpeed) {
    std::map < TilePoint, uint16 > open;
    std::set < TilePoint > closed;
    std::map < TilePoint, TilePoint > parent;
    int basex = pos_.tx, basey = pos_.ty;
    std::vector < TilePoint > path2add;
    path2add.reserve(16);
    Map *pMap = pMission->get_map();
    int x = destinationPt.tx;
    int y = destinationPt.ty;
    int z = destinationPt.tz;
    int ox = destinationPt.ox;
    int oy = destinationPt.oy;

    pMap->adjXYZ(x, y, z);
    // NOTE: we will be using lower tiles, later will restore Z coord
    z = pos_.tz - 1;

    clearDestination();

    if (!isDrawable() || isDead() || !(pMap->isTileWalkableByCar(x, y, z))) {
#if 0
#if _DEBUG
        if (!(map_ == -1 || health_ <= 0)) {
            printf("non-walking tile is target to drive\n");
            printf("tileAt %i\n",
                (unsigned int)g_App.maps().map(map())->tileAt(x, y, z));
            printf("tile x = %i, y = %i, z = %i\n", x, y, z);
        }
#endif
#endif
        return false;
    }

    if (!pMap->isTileWalkableByCar(pos_.tx, pos_.ty, z)) {
        TilePoint currentPos(pos_.tx , pos_.ty, z, pos_.ox, pos_.oy);

        if(!findPathToNearestWalkableTile(pMap, currentPos, &basex, &basey, &path2add)) {
            return false;
        }
    }

    TilePoint closest;
    float closest_dist = 100000;

    uint16 wrong_dir = (uint16)getDirection(4);
    if (wrong_dir == 0x0)
        wrong_dir = 0x0400;
    else if(wrong_dir == 0x1)
        wrong_dir = 0x6000;
    else if(wrong_dir == 0x2)
        wrong_dir = 0x0;
    else if(wrong_dir == 0x3)
        wrong_dir = 0x0020;
    open.insert(std::pair< TilePoint, uint16 >(TilePoint(basex, basey, z, pos_.ox, pos_.oy),
        wrong_dir));
    int watchDog = 1000;

    while (!open.empty()) {
        watchDog--;
        float dist = 100000;
        TilePoint p;
        std::map < TilePoint, uint16 >::iterator pit;
        for (std::map < TilePoint, uint16 >::iterator it = open.begin();
             it != open.end(); it++)
        {
            float d =
                sqrt((float) (x - it->first.tx) * (x - it->first.tx) +
                     (y - it->first.ty) * (y - it->first.ty));
            if (d < dist) {
                dist = d;
                p = it->first;
                pit = it;       // it cannot be const_iterator because of this assign
                wrong_dir = it->second;
            }
        }
        if (dist < closest_dist) {
            closest = p;
            closest_dist = dist;
        }
        //printf("found best dist %f in %i nodes\n", dist, open.size());
        open.erase(pit);
        closed.insert(p);

        if ((p.tx == x && p.ty == y && p.tz == z)
            || watchDog < 0)
        {
            if (watchDog < 0) {
                p = closest;
                dest_path_.
                    push_front(TilePoint
                               (p.tx, p.ty, p.tz, ox, oy));
            } else
                dest_path_.push_front(TilePoint(x, y, z, ox, oy));
            while (parent.find(p) != parent.end()) {
                p = parent[p];
                if (p.tx == pos_.tx && p.ty == pos_.ty
                    && p.tz == z)
                    break;
                dest_path_.push_front(p);
            }
            break;
        }

        std::map <TilePoint, uint16> neighbours;
        uint16 goodDir = tileDir(p.tx, p.ty, p.tz);

        if (wrong_dir != 0x6000 && p.tx > 0) {
            if (dirWalkable(&p, p.tx - 1, p.ty, p.tz)
                && ((goodDir & 0xF000) == 0x6000 || goodDir == 0xFFFF))
                neighbours[TilePoint(p.tx - 1, p.ty, p.tz)] = 0x0020;
        }

        if (wrong_dir != 0x0020 && p.tx < g_App.maps().map(map())->maxX()) {
            if (dirWalkable(&p, p.tx + 1, p.ty, p.tz)
                && ((goodDir & 0x00F0) == 0x0020 || goodDir == 0xFFFF))
                neighbours[TilePoint(p.tx + 1, p.ty, p.tz)] = 0x6000;
        }

        if (wrong_dir != 0x0400 && p.ty > 0)
            if (dirWalkable(&p, p.tx, p.ty - 1, p.tz)
                && ((goodDir & 0x0F00) == 0x0400 || goodDir == 0xFFFF))
                neighbours[TilePoint(p.tx, p.ty - 1, p.tz)] = 0x0;

        if (wrong_dir != 0x0000 && p.ty < g_App.maps().map(map())->maxY())
            if (dirWalkable(&p, p.tx, p.ty + 1, p.tz)
                && ((goodDir & 0x000F) == 0x0 || goodDir == 0xFFFF))
                neighbours[TilePoint(p.tx, p.ty + 1, p.tz)] = 0x0400;

        for (std::map <TilePoint, uint16>::iterator it = neighbours.begin();
            it != neighbours.end(); it++)
            if (dirWalkable(&p, it->first.tx, it->first.ty,
                it->first.tz)
                && open.find(it->first) == open.end()
                && closed.find(it->first) == closed.end())
            {
                parent[it->first] = p;
                open.insert(*it);
            }
    }

    if(!dest_path_.empty()) {
        // Adjusting offsets for correct positioning
        speed_ = newSpeed;
        int curox = pos_.ox;
        int curoy = pos_.oy;
        for(std::list < TilePoint >::iterator it = dest_path_.begin();
            it != dest_path_.end(); it++)
        {
            // TODO : adjust offsets respecting direction relative to
            // close next tiles
            switch(tileDir(it->tx, it->ty, it->tz)) {
                case 0xFFF0:
                case 0xFF20:
                    it->ox = 200;
                    it->oy = 32;
                    curox = 200;
                    curoy = 32;
                    break;
                case 0xF4FF:
                    it->ox = 32;
                    it->oy = 200;
                    curox = 32;
                    curoy = 200;
                    break;
                case 0xFF2F:
                case 0xF42F:
                    it->ox = 32;
                    it->oy = 32;
                    curox = 32;
                    curoy = 32;
                    break;
                case 0x6FFF:
                case 0x64FF:
                    it->ox = 32;
                    it->oy = 200;
                    curox = 32;
                    curoy = 200;
                    break;
                case 0x6FF0:
                    it->ox = 200;
                    it->oy = 200;
                    curox = 200;
                    curoy = 200;
                    break;
                default:
#if 0
#if _DEBUG
                    printf("hmm tileDir %X at %i, %i, %i\n",
                        (unsigned int)tileDir(it->tileX(), it->tileY(),
                        it->tileZ()), it->tileX(), it->tileY(), it->tileZ());
                    printf("tileAt %i\n",
                        (unsigned int)g_App.maps().map(map())->tileAt(
                        it->tileX(), it->tileY(), it->tileZ()));
#endif
#endif
                    it->ox = curox;
                    it->oy = curoy;
                    break;
            }
            it->tz = pos_.tz;
        }
    }
    if((!path2add.empty()) && (!dest_path_.empty())) {
        for (std::vector < TilePoint >::reverse_iterator it = path2add.rbegin();
            it != path2add.rend(); it++)
        {
            it->tz = pos_.tz;
            dest_path_.push_front(*it);
        }
    }

    return !dest_path_.empty();
}

bool GenericCar::findPathToNearestWalkableTile(Map *pMap, const TilePoint &startPt, int *basex, int *basey, std::vector < TilePoint > *path2add) {
    int dBest = 100000, dCur;
    std::vector < TilePoint > path2wtile;
    path2wtile.reserve(16);
    // we got somewhere we shouldn't, we need to find somewhere that is walkable
    TilePoint pntile = startPt;
    for (int i = 1; i < 16; i++) {
        if (pos_.tx + i >= pMap->maxX())
            break;
        pntile.tx = pos_.tx + i;
        path2wtile.push_back(pntile);
        if (pMap->isTileWalkableByCar(pos_.tx + i, pos_.ty, startPt.tz)) {
            dCur = i * i;
            if(dCur < dBest) {
                dBest = dCur;
                //path2add = path2wtile;
                path2add->assign(path2wtile.begin(), path2wtile.end());
                *basex = pos_.tx + i;
                *basey = pos_.ty;
                break;
            }
        }
    }

    path2wtile.clear();
    pntile = startPt;
    for (int i = -1; i > -16; --i) {
        if (pos_.tx + i < 0)
            break;
        pntile.tx = (pos_.tx + i);
        path2wtile.push_back(pntile);
        if (pMap->isTileWalkableByCar(pos_.tx + i, pos_.ty, startPt.tz)) {
            dCur = i * i;
            if(dCur < dBest) {
                dBest = dCur;
                //path2add = path2wtile;
                path2add->assign(path2wtile.begin(), path2wtile.end());
                *basex = pos_.tx + i;
                *basey = pos_.ty;
                break;
            }
        }
    }

    path2wtile.clear();
    pntile = startPt;
    for (int i = -1; i > -16; --i) {
        if (pos_.ty + i < 0)
            break;
        pntile.ty = (pos_.ty + i);
        path2wtile.push_back(pntile);
        if (pMap->isTileWalkableByCar(pos_.tx, pos_.ty + i, startPt.tz)) {
            dCur = i * i;
            if(dCur < dBest) {
                dBest = dCur;
                //path2add = path2wtile;
                path2add->assign(path2wtile.begin(), path2wtile.end());
                *basex = pos_.tx;
                *basey = pos_.ty + i;
                break;
            }
        }
    }

    path2wtile.clear();
    pntile = startPt;
    for (int i = 1; i < 16; i++) {
        if (pos_.ty + i >= pMap->maxY())
            break;
        pntile.ty = pos_.ty + i;
        path2wtile.push_back(pntile);
        if (pMap->isTileWalkableByCar(pos_.tx, pos_.ty + i, startPt.tz)) {
            dCur = i * i;
            if(dCur < dBest) {
                dBest = dCur;
                //path2add = path2wtile;
                path2add->assign(path2wtile.begin(), path2wtile.end());
                *basex = pos_.tx;
                *basey = pos_.ty + i;
                break;
            }
        }
    }
    return (dBest != 100000);
}

/*!
 * Moves a vehicle on the map.
 * \param elapsed Elapsed time sine last frame.
 */
bool GenericCar::doMove(int elapsed, Mission *m)
{
    bool updated = false;
    int used_time = elapsed;

    while ((!dest_path_.empty()) && used_time != 0) {
        if (hold_on_.wayFree == 1) { // Must wait
            return updated;
        } else if (hold_on_.wayFree == 2){
            // Must stop : clear destination and stop
            clearDestination();
            return updated;
        }

        // Get distance between car and next NodePath
        int adx =
            dest_path_.front().tx * 256 + dest_path_.front().ox;
        int ady =
            dest_path_.front().ty * 256 + dest_path_.front().oy;
        int atx = pos_.tx * 256 + pos_.ox;
        int aty = pos_.ty * 256 + pos_.oy;
        int diffx = adx - atx, diffy = ady - aty;

        if (abs(diffx) < 16 && abs(diffy) < 16) {
            // We reached the next point : remove it from path
            pos_.oy = dest_path_.front().oy;
            pos_.ox = dest_path_.front().ox;
            pos_.ty = dest_path_.front().ty;
            pos_.tx = dest_path_.front().tx;
            dest_path_.pop_front();
            // There's no following point so stop moving
            if (dest_path_.size() == 0)
                speed_ = 0;
            updated = true;
        } else {
            setDirection(diffx, diffy, &dir_);
            int dx = 0, dy = 0;
            double d = sqrt((double)(diffx * diffx + diffy * diffy));
            // This is the time for all the remaining distance to the node
            double avail_time_use = (d / (double)speed_) * 1000.0;
            // correcting time available regarding the time we have
            if (avail_time_use > used_time)
                avail_time_use = used_time;

            // computes distance travelled by vehicle in the available time
            if (abs(diffx) > 0)
                // dx = diffx * (speed_ * used_time / 1000) / d;
                dx = (int)((diffx * (speed_ * avail_time_use) / d) / 1000);
            if (abs(diffy) > 0)
                // dy = diffy * (speed_ * used_time / 1000) / d;
                dy = (int)((diffy * (speed_ * avail_time_use) / d) / 1000);

            // Updates the available time
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

            // Moves vehicle
            addOffsetToPosition(dx, dy);
#if 0
            if (addOffsetToPosition(dx, dy)) {
                ;
            } else {
                // TODO: avoid obstacles.
                speed_ = 0;
            }
#endif
            if(dest_path_.front().tx == pos_.tx
                && dest_path_.front().ty == pos_.ty
                && dest_path_.front().ox == pos_.ox
                && dest_path_.front().oy == pos_.oy)
                dest_path_.pop_front();
            if (dest_path_.size() == 0)
                speed_ = 0;

            updated = true;
        }
    }

    if (dest_path_.empty() && speed_) {
        printf("Destination Unknown, full speed driving = %i ... doing full stop\n",
               speed_);
        speed_ = 0;
    }
    if (!passengers_.empty()) {
        for (std::list<PedInstance *>::iterator it = passengers_.begin();
            it != passengers_.end(); it++
        ) {
            (*it)->setPosition(pos_);
        }
    }

    return updated;
}


/*!
 * Method called when object is hit by a weapon shot.
 * \param d Damage description
 */
void GenericCar::handleHit(fs_dmg::DamageToInflict &d) {
    if (health_ <= 0)
        return;

    decreaseHealth(d.dvalue);
    if (health_ == 0) {
        clearDestination();
        switch ((unsigned int)d.dtype) {
            case fs_dmg::kDmgTypeBullet:
            case fs_dmg::kDmgTypeLaser:
            case fs_dmg::kDmgTypeBurn:
            case fs_dmg::kDmgTypeExplosion:
                animation_->set_animation_type(VehicleAnimation::kOnFireAnim);
                setTimeShowAnim(10000);
                break;
        }
        pDriver_ = NULL;
        while (passengers_.size() != 0)
        {
            PedInstance *p = *(passengers_.begin());
            dropPassenger(p);
        }

        Explosion::createExplosion(g_Session.getMission(), this, 512.0);
    } else if (pDriver_ != NULL && !pDriver_->isOurAgent()) {
        // in case the car is drived by someone else than our agents
        // and one of our agent shot the car then
        // the driver is ejected from the car
        // Usually he is alone in the car so don't bother with any passengers
        PedInstance *pShooter = dynamic_cast<PedInstance *>(d.d_owner);
        if (pShooter && pShooter->isOurAgent()) {
            PedInstance *pPed = pDriver_;
            dropPassenger(pPed);
            pPed->behaviour().handleBehaviourEvent(Behaviour::kBehvEvtEjectedFromVehicle, pShooter);
        }
    }
}

/*!
 * Adds the given ped to the passenger but if the vehicle
 * has no driver, ped becomes the driver.
 * \param p The ped
 */
void GenericCar::addPassenger(PedInstance *p) {
    Vehicle::addPassenger(p);
    if (pDriver_ == NULL && !p->isPersuaded()) {
        // Ped becomes the driver
        pDriver_ = p;
    }
}

/*!
 * Overload initial method to manage driver.
 * \param pPed The ped to remove.
 */
void GenericCar::dropPassenger(PedInstance *pPed) {
    Vehicle::dropPassenger(pPed);
    if (pDriver_ == pPed) {
        pDriver_ = NULL;
        clearDestination();

        // find another driver in the remaining passengers
        for (std::list<PedInstance *>::iterator it = passengers_.begin();
            it != passengers_.end(); it++) {
            // take the first non persuaded
            if (!(*it)->isPersuaded()) {
                pDriver_ = *it;
                break;
            }
        }
    }
}

/**
 * Set this ped as the driver of the vehicle and add him as a passenger
 * if he's not already in the vehicle.
 * \param pPed PedInstance*
 * \param forceDriver bool if true, set the driver even if there is already
 * another driver
 * \return void
 *
 */
void GenericCar::setDriver(PedInstance *pPed, bool forceDriver) {
    if (pPed != NULL) {
        if (pDriver_ == NULL || forceDriver) {
            pDriver_ = pPed;
        }

        if (!containsPed(pPed)) {
            Vehicle::addPassenger(pPed);
        }
    }
}

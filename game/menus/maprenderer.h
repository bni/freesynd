/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net>
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

#ifndef MENUS_MAPRENDERER_H_
#define MENUS_MAPRENDERER_H_

#include <list>
#include <vector>
#include <set>
#include <map>

#include "fs-utils/common.h"
#include "fs-engine/enginecommon.h"
#include "fs-utils/log/log.h"
#include "fs-kernel/model/position.h"
#include "fs-kernel/model/mission.h"
#include "squadselection.h"

class PoolableResource {
public:
    PoolableResource() { free_ = true; }
    virtual ~PoolableResource() {}

    bool isFree() { return free_; }
    void setFree(bool free) { free_ = free; }

    virtual void handleRelease() {}

private:
    bool free_;
};

class ObjectToDraw : public PoolableResource {
public:
    ObjectToDraw() {
        handleRelease();
    }

    void handleRelease() {
        pObject_ = NULL;
        pNext_ = NULL;
    }

    fs_knl::MapObject * getObject() { return pObject_; }
    ObjectToDraw * getNext() { return pNext_; }

    void setObject(fs_knl::MapObject *pObj) { pObject_ = pObj; }
    void setNext(ObjectToDraw *pNext) {
        pNext_ = pNext;
    }
    void insertNext(ObjectToDraw *pNext) {
        pNext->setNext(pNext_);
        pNext_ = pNext;
    }


private:
    fs_knl::MapObject *pObject_;
    ObjectToDraw *pNext_;
};

template<class T>
class Pool {
public:
    Pool(int nbInitialRes) {
        totalResourceCreated_ = 0;
        for (int nbRes = 0; nbRes < nbInitialRes; nbRes++) {
            pool_.push_back(new T());
            totalResourceCreated_++;
        }
    }
    ~Pool() {
        for (typename std::list < T * >::iterator it = pool_.begin();
         it != pool_.end(); it++) {
             T * pRes = (*it);
            delete pRes;
            totalResourceCreated_--;
        }

        pool_.clear();

        if (totalResourceCreated_ > 0) {
            FSERR(Log::k_FLG_MEM, "Pool", "~Pool", ("There %d unfreed resource(s)", totalResourceCreated_));
        }
    };

    T * getResource() {
        T *pResource = NULL;

        if (pool_.empty()) {
            pResource = new T();
            totalResourceCreated_++;
        } else {
            pResource = pool_.front();
            pool_.pop_front();
        }

        pResource->setFree(false);
        return pResource;
    }

    void releaseResource(T * pResource) {
        if (!pResource->isFree()) {
            pResource->handleRelease();
            pool_.push_back(pResource);
            pResource->setFree(true);
        }
    }

private:
    std::list<T *> pool_;
    int totalResourceCreated_;
};

class MapRenderer {
public:
    MapRenderer() : pool_(10) {}

    void init(fs_knl::Mission *pMission, SquadSelection *pSelection);

    void render(const Point2D &worldPos);

private:
    //! The width of the left control panel in Gameplaymenu
    static const int kGameplayPanelWidth;

    /**
     * Return a integer which is a hash for identifying easily
     * a tile instead of use the 3 usual coordinates.
     * \param tilePos
     * \return int
     */
    static int tileHashKey(const fs_knl::TilePoint & tilePos) {
        return tilePos.tx | (tilePos.ty << 8) | (tilePos.tz << 16);
    }

    //! Get the hashkey of the tile for the given object
    static int tileHashKey(fs_knl::MapObject * m);

    void listObjectsToDraw(const Point2D &viewport);
    bool isObjectInsideDrawingArea(fs_knl::MapObject *pObject, const Point2D &viewport);
    int drawObjectsOnTile(const fs_knl::TilePoint & tilePos, const Point2D &screenPos);
    void addObjectToDraw(fs_knl::MapObject *pObject);
    void freeUnreleasedResources();

private:
    fs_knl::Mission *pMission_;
    fs_knl::Map *pMap_;
    SquadSelection *pSelection_;

    Pool<ObjectToDraw> pool_;
    /*! This map contains for each tile the list of objects to draw.*/
    std::map<int, ObjectToDraw *> objectsByTile_;
};

#endif  // MENUS_MAPRENDERER_H_

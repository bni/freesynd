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

#include "menus/maprenderer.h"

#include "fs-engine/gfx/tilemanager.h"
#include "fs-engine/system/system.h"
#include "fs-kernel/model/mission.h"
#include "fs-kernel/model/vehicle.h"
#include "fs-kernel/model/squad.h"
#include "fs-kernel/mgr/agentmanager.h"

#include "menus/squadselection.h"
#include "fs-engine/config.h"

const int MapRenderer::kGameplayPanelWidth = 128;

void MapRenderer::init(fs_knl::Mission *pMission, SquadSelection *pSelection) {
    pMission_ = pMission;
    pMap_ = pMission->get_map();
    pSelection_ = pSelection;
}

/**
 * Draw tiles and map objects.
 */
void MapRenderer::render(const Point2D &viewport) {
    // TODO: list of bugs to fix in rendering
    //  - Some advert panels lack a corner
    fs_knl::TilePoint mtp = pMap_->screenToTilePoint(viewport.x, viewport.y);
    int sw = mtp.tx;
    // widthOffset accounts for the extended screen width in isometric tile space.
    // Moving from screen left to screen right decreases tile_y by gameWidth/kTileWidth,
    // so sh must start that many rows earlier to cover the top-right corner.
    // The same offset is added to chk so that shm (= sh + chk) is unchanged,
    // preserving full bottom-left coverage.
    int widthOffset = g_System.getGameWidth() / fs_eng::Tile::kTileWidth;
    int chk = g_System.getGameWidth() / (fs_eng::Tile::kTileWidth / 2) + 2
        + g_System.getGameHeight() / (fs_eng::Tile::kTileHeight / 3) + pMap_->maxZ() * 2
        + widthOffset;
    int sh = mtp.ty - 8 - widthOffset;

    int shm = sh + chk;

    DEBUG_SPEED_INIT

    listObjectsToDraw(viewport);

    int cmw = viewport.x + g_System.getGameWidth() -
                kGameplayPanelWidth + 128;
    int cmh = viewport.y + g_System.getGameHeight() + 128;
    int cmx = viewport.x - kGameplayPanelWidth;
     //  z = 0 - is minimap data and mapdata
    int chky = sh < 0 ? 0 : sh;
    int zr = shm + pMap_->maxZ() + 1;
    // Start 4 tiles earlier so tiles that render in the panel column (x=0..127)
    // are also drawn below the panel UI area.
    int sw_ext = sw - kGameplayPanelWidth / (fs_eng::Tile::kTileWidth / 2);
    for (int inc = 0; inc < zr; ++inc) {
        int ye = sh + inc;
        int ys = ye - pMap_->maxZ() - 2;
        int tile_z = pMap_->maxZ() + 1;  // the Z coord of the next tile to draw
        for (int yb = ys; yb < ye; ++yb) {
            if (yb < 0 || yb < sh || yb >= shm) {
                --tile_z;
                continue;
            }
            int tile_y = yb;  // The Y coord of the tile to draw
            for (int tile_x = sw_ext; tile_y >= chky && tile_x < pMap_->maxX(); ++tile_x) {
                if (tile_x < 0 || tile_y >= pMap_->maxY()) {
                    --tile_y;
                    continue;
                }
                int screen_w = (pMap_->maxX() + (tile_x - tile_y)) * (fs_eng::Tile::kTileWidth / 2);
                int coord_h = ((pMap_->maxZ() + tile_x + tile_y) - (tile_z - 1)) * (fs_eng::Tile::kTileHeight / 3);
                if (screen_w >= viewport.x - kGameplayPanelWidth - fs_eng::Tile::kTileWidth * 2
                    && screen_w + fs_eng::Tile::kTileWidth * 2 < cmw
                    && coord_h >= viewport.y - fs_eng::Tile::kTileHeight * 2
                    && coord_h + fs_eng::Tile::kTileHeight * 2 < cmh) {
#if 0
                    if (z > 2)
                        continue;
#endif
                    // draw a tile
                    if (tile_z < pMap_->maxZ()) {
                        fs_eng::Tile *pTile = pMap_->getTileAt(tile_x, tile_y, tile_z);
                        if (pTile->notTransparent()) {
                            int dx = 0, dy = 0;
                            if (screen_w - viewport.x < 0)
                                dx = -(screen_w - viewport.x);
                            if (coord_h - viewport.y < 0)
                                dy = -(coord_h - viewport.y);
                            if (dx < fs_eng::Tile::kTileWidth + kGameplayPanelWidth && dy < fs_eng::Tile::kTileHeight) {
                                pMap_->getTileManager()->drawTile(pTile, screen_w - cmx, coord_h - viewport.y);
                            }
                        }
                    }

                    // draw everything that's on the tile
                    if (tile_z - 1 >= 0) {
                        fs_knl::TilePoint currentTile(tile_x, tile_y, tile_z - 1);
                        Point2D screenPos = {screen_w - cmx + fs_eng::Tile::kTileWidth / 2,
                            coord_h - viewport.y + fs_eng::Tile::kTileHeight / 3 * 2};

                        drawObjectsOnTile(currentTile, screenPos);
                    }
                }
                --tile_y;
            }
            --tile_z;
        }
    }

    freeUnreleasedResources();

#ifdef _DEBUG
    if (g_System.getKeyModState() & fs_eng::KMD_LALT) {
        fs_eng::FSColor yellow {227, 219, 40, 0xFF};
        for (SquadSelection::Iterator it = pSelection_->begin();
            it != pSelection_->end(); ++it) {
            (*it)->showPath(viewport.x, viewport.y, yellow);
        }
    }
#endif

    DEBUG_SPEED_LOG("MapRenderer::render")
}

int MapRenderer::tileHashKey(fs_knl::MapObject * m) {
    return tileHashKey(m->position());
}

void MapRenderer::listObjectsToDraw(const Point2D &viewport) {
    /*if (tilex < 0)
        tilex = 0;
    if (tiley < 0)
        tiley = 0;
    if (maxtilex >= pMap_->maxX())
        maxtilex = pMap_->maxX();
    if (maxtiley >= pMap_->maxY())
        maxtiley = pMap_->maxY();*/


    // Include peds
    for (size_t i = 0; i < pMission_->numPeds(); i++) {
        fs_knl::PedInstance *pPed = pMission_->ped(i);
        if (pPed->isDrawable() && isObjectInsideDrawingArea(pPed, viewport)) {
            addObjectToDraw(pPed);
        }
    }

    // vehicles
    for (size_t i = 0; i < pMission_->numVehicles(); i++) {
        fs_knl::Vehicle *pVehicle = pMission_->vehicle(i);
        if (isObjectInsideDrawingArea(pVehicle, viewport)) {
            addObjectToDraw(pVehicle);
        }
    }

    // weapons
    for (size_t i = 0; i < pMission_->numWeaponsOnGround(); i++) {
        fs_knl::WeaponInstance *pWeapon = pMission_->weaponOnGround(i);
        if (pWeapon->isDrawable() && isObjectInsideDrawingArea(pWeapon, viewport)) {
            addObjectToDraw(pWeapon);
        }
    }

    // statics
    for (size_t i = 0; i < pMission_->numStatics(); i++) {
        fs_knl::Static *pStatic = pMission_->statics(i);
        if (isObjectInsideDrawingArea(pStatic, viewport)) {
            addObjectToDraw(pStatic);
        }
    }

    // sfx objects
    for (size_t i = 0; i < pMission_->numSfxObjects(); i++) {
        fs_knl::SFXObject *pSfx = pMission_->sfxObjects(i);
        if (pSfx->isDrawable() && isObjectInsideDrawingArea(pSfx, viewport)) {
            addObjectToDraw(pSfx);
        }
    }
}

/**
 * Return true if the object appears on the screen and so should be drawn.
 * \param pObject MapObject*
 * \param viewport const Point2D&
 * \return bool
 *
 */
bool MapRenderer::isObjectInsideDrawingArea(fs_knl::MapObject *pObject, const Point2D &viewport) {
    Point2D objectViewport;
    pMission_->get_map()->tileToScreenPoint(pObject->position(), &objectViewport);

    // Limits are larger than screen size in order to have a smooth display
    // of appearance/disappearance of objects on screen. Otherwise they popup when
    // entering the display screen.
    return  objectViewport.x > (viewport.x - kGameplayPanelWidth - fs_eng::Tile::kTileWidth / 2) && objectViewport.y > viewport.y &&
            objectViewport.x <= (viewport.x + g_System.getGameWidth() - kGameplayPanelWidth + 10) &&
            objectViewport.y <= (viewport.y + g_System.getGameHeight() + pObject->position().tz * 48);
}

/**
 * Draw all objects on the given tile.
 * \param tilePos const TilePoint& tile coordinates
 * \param screenPos const Point2D& position of tile on the screen
 * \return int number of objects for debug
 *
 */
int MapRenderer::drawObjectsOnTile(const fs_knl::TilePoint & tilePos, const Point2D &screenPos) {
    int tileKey = tileHashKey(tilePos);
    int nbDrawnObjects = 0;

    std::map<int, ObjectToDraw *>::iterator it = objectsByTile_.find(tileKey);
    if(it != objectsByTile_.end()) {
        ObjectToDraw *pObj = it->second;
        objectsByTile_.erase(it);
        while(pObj != NULL) {
            pObj->getObject()->draw(screenPos);
            ObjectToDraw *pNext = pObj->getNext();
            pool_.releaseResource(pObj);
            pObj = pNext;
            nbDrawnObjects++;
        }
    }

    return nbDrawnObjects;
}


/**
 * Adds an object to the list of objects to draw for the tile it's on.
 * For a given tile object are sorted from back to front so that
 * objects in the back are drawn first.
 * \param pObjectToAdd MapObject* Object to add
 * \return void
 *
 */
void MapRenderer::addObjectToDraw(fs_knl::MapObject *pObjectToAdd) {
    int tileKey;
    ObjectToDraw *pNewEntry = pool_.getResource();
    pNewEntry->setObject(pObjectToAdd);

    if (pObjectToAdd->is(fs_knl::MapObject::kNatureVehicle)) {
        // vehicle are associated with the tile just above (z+1)
        // because it is bigger than a tile so all tiles below must be drawn first
        fs_knl::TilePoint vehiclePos( pObjectToAdd->position());
        vehiclePos.tz += 1;
        tileKey = tileHashKey(vehiclePos);
    } else {
        tileKey = tileHashKey(pObjectToAdd);
    }

    std::map<int, ObjectToDraw *>::iterator element = objectsByTile_.find(tileKey);
    if(element == objectsByTile_.end()) {
        // no element has been set with the tile so add the first element
        objectsByTile_[tileKey] = pNewEntry;
    } else {
        // there is at leastone element already set with the tile
        ObjectToDraw *pObjectInList = element->second;
        if (pObjectToAdd->isBehindObjectOnSameTile(pObjectInList->getObject())) {
            // first case is when the new object should be first in the list
            pNewEntry->setNext(pObjectInList);
            objectsByTile_[tileKey] = pNewEntry;
        } else {
            // second case is when new object is somewhere in the list
            while (pObjectInList != NULL) {
                if (pObjectInList->getNext() != NULL) {
                    if (pObjectToAdd->isBehindObjectOnSameTile(pObjectInList->getNext()->getObject())) {
                        pObjectInList->insertNext(pNewEntry);
                        break;
                    } else {
                        pObjectInList = pObjectInList->getNext();
                    }
                } else {
                    pObjectInList->setNext(pNewEntry);
                    break;
                }
            }
        }
    }
}

/**
 * Objects that were listed for drawing may be bigger than objects really drawn.
 * So remove those objects from the list.
 * \return void
 *
 */
void MapRenderer::freeUnreleasedResources() {
    int nbFreed = 0;
    std::map<int, ObjectToDraw *>::iterator itr = objectsByTile_.begin();
    while (itr != objectsByTile_.end()) {
        std::map<int, ObjectToDraw *>::iterator toErase = itr;
        ++itr;
        ObjectToDraw *pObj = toErase->second;
        while(pObj != NULL) {
            ObjectToDraw *pNext = pObj->getNext();
            pool_.releaseResource(pObj);
            nbFreed++;
            pObj = pNext;
        }
        objectsByTile_.erase(toErase);
    }
}

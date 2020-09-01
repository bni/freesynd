/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2012  Benoit Blancard <benblan@users.sourceforge.net>*
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;without even  the impliedwarranty of      *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project'sweb  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#include "app.h"
#include "menus/minimaprenderer.h"
#include "core/missionbriefing.h"
#include "mission.h"
#include "gfx/screen.h"
#include "model/vehicle.h"
#include "ped.h"

const int MinimapRenderer::kMiniMapSizePx = 128;
const int GamePlayMinimapRenderer::kEvacuationRadius = 15;

void MinimapRenderer::setZoom(EZoom zoom) {
    zoom_ = zoom;
    pixpertile_ = 10 - zoom_;
    updateRenderingInfos();
}

BriefMinimapRenderer::BriefMinimapRenderer() : mm_timer(500) {
    scroll_step_ = 0;
}

/*!
 * Init the renderer with a new mission, zoom level and draw_enemies params.
 */
void BriefMinimapRenderer::init(MissionBriefing *p_briefing, EZoom zoom, bool draw_enemies) {
    p_mb_ = p_briefing;
    setZoom(zoom);
    b_draw_enemies_ = draw_enemies;
    mm_timer.reset();
    minimap_blink_ = 0;

    // Initialize minimap origin by looking for the position
    // of the first found agent on the map
    initMinimapLocation();
}

/*!
 * Centers the minimap on the starting position of agents
 */
void BriefMinimapRenderer::initMinimapLocation() {
    bool found = false;
    int maxx = p_mb_->minimap()->max_x();
    int maxy = p_mb_->minimap()->max_y();

    for (int x = 0; x < maxx && (!found); x++) {
        for (int y = 0; y < maxy && (!found); y++) {
            if (p_mb_->getMinimapOverlay(x, y) == MiniMap::kOverlayOurAgent) {
                // We found a tile with an agent on it
                // stop searching and memorize position
                world_tx_ = x;
                world_ty_ = y;
                found = true;
            }
        }
    }

    uint16 halftiles = mm_maxtile_ / 2;
    world_tx_ = (world_tx_ < halftiles) ? 0 : (world_tx_ - halftiles + 1);
    world_ty_ = (world_ty_ < halftiles) ? 0 : (world_ty_ - halftiles + 1);

    clipMinimapToRightAndDown();
}

/*!
 *
 */
void BriefMinimapRenderer::clipMinimapToRightAndDown() {
    if ((world_tx_ + mm_maxtile_) >= p_mb_->minimap()->max_x()) {
        // We assume that map size in tiles (p_mission_->mmax_x_)
        // is bigger than the minimap size (mm_maxtile_)
        world_tx_ = p_mb_->minimap()->max_x() - mm_maxtile_;
    }

    if ((world_ty_ + mm_maxtile_) >= p_mb_->minimap()->max_y()) {
        // We assume that map size in tiles (p_mission_->mmax_y_)
        // is bigger than the minimap size (mm_maxtile_)
        world_ty_ = p_mb_->minimap()->max_y() - mm_maxtile_;
    }
}

void BriefMinimapRenderer::updateRenderingInfos() {
    scroll_step_ = 30 / pixpertile_;
    mm_maxtile_ = 120 / pixpertile_;
}

void BriefMinimapRenderer::zoomOut() {
    switch (zoom_) {
    case ZOOM_X2:
        setZoom(ZOOM_X1);
        break;
    case ZOOM_X3:
        setZoom(ZOOM_X2);
        break;
    case ZOOM_X4:
        setZoom(ZOOM_X3);
        break;
    default:
        break;
    }

    // check if map should be aligned with right and bottom border
    // as when zooming out only displays more tiles but does not
    // move the minimap origin
    clipMinimapToRightAndDown();
}

bool BriefMinimapRenderer::handleTick(int elapsed) {
    if (mm_timer.update(elapsed)) {
        minimap_blink_ ^= 1;
        return true;
    }

    return false;
}

/*!
 * Scrolls right using current scroll step. If scroll is too far,
 * clips scrolling to the map's right border.
 */
void BriefMinimapRenderer::scrollRight() {
    world_tx_ += scroll_step_;
    clipMinimapToRightAndDown();
}

/*!
 * Scrolls left using current scroll step. If scroll is too far,
 * clips scrolling to the map's left border.
 */
void BriefMinimapRenderer::scrollLeft() {
    // if scroll_step is bigger than world_tx_
    // then world_tx_ -= scroll_step_ would be negative
    // but world_tx_ is usigned so it would be an error
    if (world_tx_ < scroll_step_) {
        world_tx_ = 0;
    } else {
        // we know that world_tx_ >= scroll_step_
        world_tx_ -= scroll_step_;
    }
}

/*!
 * Scrolls up using current scroll step. If scroll is too far,
 * clips scrolling to the map's top border.
 */
void BriefMinimapRenderer::scrollUp() {
    if (world_ty_ < scroll_step_) {
        world_ty_ = 0;
    } else {
        // we know that world_ty_ >= scroll_step_
        world_ty_ -= scroll_step_;
    }
}

/*!
 * Scrolls down using current scroll step. If scroll is too far,
 * clips scrolling to the map's bottom border.
 */
void BriefMinimapRenderer::scrollDown() {
    world_ty_ += scroll_step_;
    clipMinimapToRightAndDown();
}

/*!
 * Renders the minimap at the given position on the screen.
 * \param screen_x X coord in absolute pixels.
 * \param screen_y Y coord in absolute pixels.
 */
void BriefMinimapRenderer::render(uint16 screen_x, uint16 screen_y) {
    for (uint16 tx = world_tx_; tx < (world_tx_ + mm_maxtile_); tx++) {
        uint16 xc = screen_x + (tx - world_tx_) * pixpertile_;
        for (uint16 ty = world_ty_; ty < (world_ty_ + mm_maxtile_); ty++) {
            uint8 c = p_mb_->getMinimapOverlay(tx, ty);
            switch (c) {
                case 0:
                    c = p_mb_->minimap()->getColourAt(tx, ty);
                    break;
                case 1:
                    c = minimap_blink_ ? 14 : 12;
                    break;
                case 2:
                    if (b_draw_enemies_)
                        c = minimap_blink_ ? 14 : 5;
                    else
                        c = p_mb_->minimap()->getColourAt(tx, ty);
            }
            g_Screen.drawRect(xc, screen_y + (ty - world_ty_) * pixpertile_, pixpertile_,
                pixpertile_, c);
        }
    }
}

/*!
 * Default constructor.
 */
GamePlayMinimapRenderer::GamePlayMinimapRenderer() :
    mm_timer_weap(300, false), mm_timer_ped(260, false),
    mm_timer_signal(250) {
    p_mission_ = NULL;
    handleClearSignal();
    g_gameCtrl.addListener(this, GameEvent::kMission);
}

/*!
 * Sets a new mission for rendering the minimap.
 * \param pMission A mission.
 * \param b_scannerEnabled True if scanner is enabled -> changes zoom level.
 */
void GamePlayMinimapRenderer::init(Mission *pMission, bool b_scannerEnabled) {
    p_mission_ = pMission;
    p_minimap_ = pMission->getMiniMap();
    setScannerEnabled(b_scannerEnabled);
    world_tx_ = 0;
    world_ty_ = 0;
    offset_x_ = 0;
    offset_y_ = 0;
    cross_x_ = 64;
    cross_y_ = 64;
    mm_timer_weap.reset();
    mm_timer_signal.reset();
    handleClearSignal();
}

void GamePlayMinimapRenderer::updateRenderingInfos() {
    // mm_maxtile_ can be 17 or 33
    mm_maxtile_ = 128 / pixpertile_ + 1;
}

/*!
 * Setting the scanner on or off will play on the zooming level.
 * \param b_enabled True will set a zoom of X1, else X3.
 */
void GamePlayMinimapRenderer::setScannerEnabled(bool b_enabled) {
    setZoom(b_enabled ? ZOOM_X1 : ZOOM_X3);
}

/*!
 * Centers the minimap on the given tile. Usually, the minimap is centered
 * on the selected agent. If the agent is too close from the border, the minimap
 * does not move anymore.
 * \param tileX The X coord of the tile.
 * \param tileX The Y coord of the tile.
 * \param offX The offset of the agent on the tile.
 * \param offY The offset of the agent on the tile.
 */
void GamePlayMinimapRenderer::centerOn(uint16 tileX, uint16 tileY, int offX, int offY) {
    uint16 halfSize = mm_maxtile_ / 2;

    if (tileX < halfSize) {
        // we're too close of the top border -> stop moving along X axis
        world_tx_ = 0;
        offset_x_ = 0;
    } else if ((tileX + halfSize) >= p_mission_->mmax_x_) {
        // we're too close of the bottom border -> stop moving along X axis
        world_tx_ = p_mission_->mmax_x_ - mm_maxtile_;
        offset_x_ = 0;
    } else {
        world_tx_ = tileX - halfSize;
        offset_x_ = offX / (256 / pixpertile_);
    }

    if (tileY < halfSize) {
        world_ty_ = 0;
        offset_y_ = 0;
    } else if ((tileY + halfSize) >= p_mission_->mmax_y_) {
        world_ty_ = p_mission_->mmax_y_ - mm_maxtile_;
        offset_y_ = 0;
    } else {
        world_ty_ = tileY - halfSize;
        offset_y_ = offY / (256 / pixpertile_);
    }

    // get the cross coordinate
    //
    // TODO : see if we can remove + 1
    cross_x_ = mapToMiniMapX(tileX + 1, offX);
    cross_y_ = mapToMiniMapY(tileY + 1, offY);
}

/**
 * Method to intercept game events.
 * The catched events are for detecting signals setup.
 */
void GamePlayMinimapRenderer::handleGameEvent(GameEvent evt) {
    switch (evt.type) {
    case GameEvent::kObjEvacuate:
        handleEvacuationSet(evt);
        break;
    case GameEvent::kObjTargetSet:
        handleTargetSet(evt);
        break;
    case GameEvent::kObjCompleted:
    case GameEvent::kObjFailed:
        p_minimap_->clearTarget();
        handleClearSignal();
        break;
    default:

        break;
    }
}

void GamePlayMinimapRenderer::handleEvacuationSet(GameEvent &evt) {
    handleClearSignal();
    WorldPoint * pPoint = static_cast<WorldPoint *>(evt.pCtxt);
    signalSourceLocW_.x = pPoint->x;
    signalSourceLocW_.y = pPoint->y;
    signalSourceLocW_.z = pPoint->z;

    signalType_ = kEvacuation;

    // Check if the evacuation point is already visible
    // in this case, we draw the circle in red
    int signal_px = signalXYZToMiniMapX();
    int signal_py = signalXYZToMiniMapY();
    if (isEvacuationCircleOnMinimap(signal_px, signal_py)) {
        i_signalColor_ = fs_cmn::kColorDarkRed;
        i_signalRadius_ = kEvacuationRadius;
    }
}

void GamePlayMinimapRenderer::handleClearSignal() {
    i_signalRadius_ = 0;
    i_signalColor_ = fs_cmn::kColorWhite;
    signalType_ = kNone;
    signalSourceLocW_.x = 0;
    signalSourceLocW_.y = 0;
    signalSourceLocW_.z = 0;
}

/*!
 * Defines a signal position on the map.
 */
void GamePlayMinimapRenderer::handleTargetSet(GameEvent &evt) {
    handleClearSignal();
    // get the target current position
    MapObject *pTarget = static_cast<MapObject *>(evt.pCtxt);
    p_minimap_->setTarget(pTarget);
    signalSourceLocW_.convertFromTilePoint(pTarget->position());
    signalType_ = kTarget;
}

bool GamePlayMinimapRenderer::handleTick(int elapsed) {
    mm_timer_ped.update(elapsed);
    mm_timer_weap.update(elapsed);

    if (signalType_ != kNone &&mm_timer_signal.update(elapsed)) {
        // Time hit max -> update radar circle size
        i_signalRadius_ += 16;
        int signal_px = signalXYZToMiniMapX();
        int signal_py = signalXYZToMiniMapY();

        if (signalType_ == kEvacuation && isEvacuationCircleOnMinimap(signal_px, signal_py)) {
            // the evacuation circle is completely on the map, so it's a red circle with fixed size
            i_signalColor_ = fs_cmn::kColorDarkRed;
            i_signalRadius_ = kEvacuationRadius;
        } else {
            int maxPx = mm_maxtile_ * pixpertile_;
            int signalRadius2 = i_signalRadius_ * i_signalRadius_;
            // Distance to the top right corner
            int dist1 = (maxPx - signal_px )* (maxPx - signal_px ) + (signal_py )* (signal_py );
            // Distance to the top left corner
            int dist2 = (signal_px )* (signal_px ) + (signal_py )* (signal_py );
            // Distance to the bottom left corner
            int dist3 = (signal_px )* (signal_px ) + (maxPx - signal_py )* (maxPx - signal_py );
            // Distance to the bottom left corner
            int dist4 = (maxPx - signal_px )* (maxPx - signal_px ) + (maxPx - signal_py )* (maxPx - signal_py );
            // All four corners of the minimap must be inside the circle to stop growing
            if (signalRadius2 > dist1 && signalRadius2 > dist2 &&
                signalRadius2 > dist3 && signalRadius2 > dist4) {
                i_signalRadius_ = 0;
                // Update signal position in case of a moving target
                if (signalType_ == kTarget) {
                    signalSourceLocW_.convertFromTilePoint(p_minimap_->target()->position());
                }
                // TODO : uncomment when assassinate.ogg doesn't have the pong sound in it
                //g_App.gameSounds().play(snd::TRACKING_PONG);
            }
            // reset color to white in case the red circle was displayed
            i_signalColor_ = fs_cmn::kColorWhite;
        }
    }

    return true;
}

/*!
 * \param mm_x coord on the minimap in pixels
 * \param mm_y coord on the minimap in pixels
 */
TilePoint GamePlayMinimapRenderer::minimapToMapPoint(int mm_x, int mm_y) {
    TilePoint pt;
    // I'm not sure this code is correct
    int tx = (mm_x + offset_x_) / pixpertile_;
    int ty = (mm_y + offset_y_) / pixpertile_;
    int ox = (mm_x + offset_x_) % pixpertile_;
    int oy = (mm_y + offset_y_) % pixpertile_;

    pt.tx = tx + world_tx_;
    pt.ty = ty + world_ty_;
    pt.tz = 0;
    pt.ox = ox * (256 / pixpertile_);
    pt.oy = oy * (256 / pixpertile_);

    return pt;
}

/*!
 * Renders the minimap at the given position on the screen.
 * \param screen_x X coord in absolute pixels.
 * \param screen_y Y coord in absolute pixels.
 */
void GamePlayMinimapRenderer::render(uint16 screen_x, uint16 screen_y) {
    // A temporary buffer composed of mm_maxtile + 1 columns and rows.
    // we use a slightly larger rendering buffer not to have
    // to check borders. At the end we only display  the mm_maxtile x mm_maxtile tiles.
    // On top of this we use one size for both resolutions as 18*18*8*8 > 34*34*4*4

    // NOTE: additional data is added to avoid stack corruption (18 * 8) * 4
    uint8 minimap_layer[18*18*8*8 + (18 * 8) * 4];
    // because of reading data from buffer in peds draw, to avoid
    // conditional on uninitialized value
    memset(minimap_layer, 0, 18*18*8*8 + (18 * 8) * 4);

    uint8 mm_layer_size = mm_maxtile_ + 1;
    // The final minimap that will be displayed : the minimap is 128*128 pixels
    uint8 minimap_final_layer[kMiniMapSizePx*kMiniMapSizePx];

    // In this loop, we fill the buffer with floor colour. the first row and column
    // is not filled

    for (int j = 0; j < mm_maxtile_; ++j) {
        // pointer to first row
        uint8* frow = minimap_layer + (j + 1) * pixpertile_ * pixpertile_ * mm_layer_size
            + pixpertile_;
        for (int i = 0; i < mm_maxtile_; i++) {
            uint8 gcolour = p_mission_->getMiniMap()->getColourAt(world_tx_ + i, world_ty_ + j);
            // pointer to row we draw at
            uint8 *drow = frow + i * pixpertile_;
            for (uint8 inc = 0; inc < pixpertile_; ++inc)
                *drow++ = gcolour;
        }
        for (uint8 inc = 1; inc < pixpertile_; ++inc) {
            uint8 *drow = frow + inc * pixpertile_ * mm_layer_size;
            uint8 *local_frow = frow;
            const uint32 sz = pixpertile_ * mm_maxtile_;
            for (uint32 i = 0; i < sz; ++i)
                *drow++ = *local_frow++;
        }
    }

    // Draw the minimap cross
    drawFillRect(minimap_layer, cross_x_, 0, 1, (mm_maxtile_ + 1) * pixpertile_, fs_cmn::kColorBlack);
    drawFillRect(minimap_layer, 0, cross_y_, (mm_maxtile_ + 1) * pixpertile_, 1, fs_cmn::kColorBlack);

    // draw all visible elements on the minimap
    drawPedestrians(minimap_layer);
    drawWeapons(minimap_layer);
    drawVehicles(minimap_layer);

    if (signalType_ != kNone) {
        int signal_px = signalXYZToMiniMapX();
        int signal_py = signalXYZToMiniMapY();
        drawSignalCircle(minimap_layer, signal_px, signal_py, i_signalRadius_, i_signalColor_);
    }

    // Copy the temp buffer in the final minimap using the tile offset so the minimap movement
    // is smoother
    for (int j = 0; j < kMiniMapSizePx; j++) {
        memcpy(minimap_final_layer + (kMiniMapSizePx * j),
            minimap_layer + (pixpertile_ * pixpertile_ * mm_layer_size) +
            (j + offset_y_) * pixpertile_ * mm_layer_size + pixpertile_ + offset_x_, kMiniMapSizePx);
    }

    // Draw the minimap on the screen
    g_Screen.blit(screen_x, screen_y, kMiniMapSizePx, kMiniMapSizePx, minimap_final_layer);
}

void GamePlayMinimapRenderer::drawVehicles(uint8 *a_minimap) {
    for (size_t i = 0; i < p_mission_->numVehicles(); i++) {
        Vehicle *p_vehicle = p_mission_->vehicle(i);
        int tx = p_vehicle->tileX();
        int ty = p_vehicle->tileY();

        if (isVisible(tx, ty)) {
            // vehicle is on minimap and must be drawn.
            // if a vehicle contains at least one of our agent
            // we only draw the yellow circle representing the agent
            if (p_vehicle->containsOurAgents()) {
                int px = mapToMiniMapX(tx + 1, p_vehicle->offX());
                int py = mapToMiniMapY(ty + 1, p_vehicle->offY());
                uint8 borderColor = (mm_timer_ped.state()) ? fs_cmn::kColorBlack : fs_cmn::kColorLightGreen;
                drawPedCircle(a_minimap, px, py, fs_cmn::kColorYellow, borderColor);

            } else {
                size_t vehicle_size = (zoom_ == ZOOM_X1) ? 2 : 4;
                int px = mapToMiniMapX(tx + 1, p_vehicle->offX()) - vehicle_size / 2;
                int py = mapToMiniMapY(ty + 1, p_vehicle->offY()) - vehicle_size / 2;

                drawFillRect(a_minimap, px, py, vehicle_size, vehicle_size, fs_cmn::kColorWhite);
            }
        }
    }
}

void GamePlayMinimapRenderer::drawWeapons(uint8 * a_minimap) {
    const size_t weapon_size = 2;
    for (size_t i = 0; i < p_mission_->numWeaponsOnGround(); i++)
    {
        WeaponInstance * w = p_mission_->weaponOnGround(i);
        // we draw weapons that have no owner ie that are on the ground
        // and are not destroyed
        if (w->map() == -1)
            continue;

        int tx = w->tileX();
        int ty = w->tileY();
        int ox = w->offX();
        int oy = w->offY();

        if (isVisible(tx, ty)) {
            if (mm_timer_weap.state()) {
                int px = mapToMiniMapX(tx + 1, ox) - 1;
                int py = mapToMiniMapY(ty + 1, oy) - 1;

                drawFillRect(a_minimap, px, py, weapon_size, weapon_size, fs_cmn::kColorLightGrey);
            }
        }
    }
}

void GamePlayMinimapRenderer::drawPedestrians(uint8 * a_minimap) {
    for (size_t i = 0; i < p_mission_->numPeds(); i++)
    {
        PedInstance *p_ped = p_mission_->ped(i);
        // we are not showing dead or peds inside vehicle
        if (p_ped->isDead() || p_ped->inVehicle())
            continue;

        int tx = p_ped->tileX();
        int ty = p_ped->tileY();
        int ox = p_ped->offX();
        int oy = p_ped->offY();

        if (isVisible(tx, ty))
        {
            int px = mapToMiniMapX(tx + 1, ox);
            int py = mapToMiniMapY(ty + 1, oy);
            if (p_ped->isPersuaded()) {
                // col_Yellow circle with a black or lightgreen border (blinking)
                uint8 borderColor = (mm_timer_ped.state()) ? fs_cmn::kColorLightGreen : fs_cmn::kColorBlack;
                drawPedCircle(a_minimap, px, py, fs_cmn::kColorYellow, borderColor);
            } else {
                switch (p_ped->type())
                {
                case PedInstance::kPedTypeCivilian:
                case PedInstance::kPedTypeCriminal:
                    {
                        // white rect 2x2 (opaque and transparent blinking)
                        size_t ped_width = 2;
                        size_t ped_height = 2;
                        if (mm_timer_ped.state()) {
                            --px;
                            --py;

                            // draw the square
                            drawFillRect(a_minimap, px, py, ped_width, ped_height, fs_cmn::kColorWhite);
                        }
                    break;
                    }
                case PedInstance::kPedTypeAgent:
                {
                    if (p_ped->isOurAgent())
                    {
                        // TODO : do not draw agent if he is in a vehicle
                        // col_Yellow circle with a black or lightgreen border (blinking)
                        uint8 borderColor = (mm_timer_ped.state()) ? fs_cmn::kColorBlack : fs_cmn::kColorLightGreen;
                        drawPedCircle(a_minimap, px, py, fs_cmn::kColorYellow, borderColor);
                    } else {
                        // col_LightRed circle with a black or dark red border (blinking)
                        uint8 borderColor = (mm_timer_ped.state()) ? fs_cmn::kColorBlack : fs_cmn::kColorDarkRed;
                        drawPedCircle(a_minimap, px, py, fs_cmn::kColorLightRed, borderColor);
                    }
                }
                break;
                case PedInstance::kPedTypePolice:
                    {
                    // blue circle with a black or col_BlueGrey (blinking)
                    uint8 borderColor = (mm_timer_ped.state()) ? fs_cmn::kColorBlack : fs_cmn::kColorBlueGrey;
                    drawPedCircle(a_minimap, px, py, fs_cmn::kColorBlue, borderColor);
                    }
                    break;
                case PedInstance::kPedTypeGuard:
                    {
                    // col_LightGrey circle with a black or white border (blinking)
                    uint8 borderColor = (mm_timer_ped.state()) ? fs_cmn::kColorWhite : fs_cmn::kColorBlack;
                    drawPedCircle(a_minimap, px, py, fs_cmn::kColorLightGrey, borderColor);
                    }
                    break;
                }
            }
        }
    }
}

/*!
 * This array is used as a mask to draw circle for peds.
 * Values of the mask are :
 * - 0 : use the color of the buffer
 * - 1 : use border color if buffer color is not the fillColor
 * - 2 : use the fillColor
 */
uint8 g_ped_circle_mask_[] = {
    0,  0,  1,  1,  1,  0,  0,
    0,  1,  2,  2,  2,  1,  0,
    1,  2,  2,  2,  2,  2,  1,
    1,  2,  2,  2,  2,  2,  1,
    1,  2,  2,  2,  2,  2,  1,
    0,  1,  2,  2,  2,  1,  0,
    0,  0,  1,  1,  1,  0,  0
};

/*!
    * Draw a circle with the given colors for fill and border. This is used to represent agents, police and guards.
    * \param a_buffer destination buffer
    * \param mm_x X coord in the destination buffer
    * \param mm_y Y coord in the destination buffer
    * \param fillColor the color to fill the rect
    * \param borderColor the color to draw the circle border
    */
void GamePlayMinimapRenderer::drawPedCircle(uint8 * a_buffer, int mm_x, int mm_y, uint8 fillColor, uint8 borderColor) {
    // Size of the mask : it's a square of 4x4 pixels.
    const uint8 kCircleMaskSize = 7;
    // centers the circle on the ped position and add pixels to skip the first row and column
    mm_x -= 3;
    mm_y -= 3;

    for (uint8 j = 0; j < kCircleMaskSize; j++) {
        for (uint8 i = 0; i < kCircleMaskSize; i++) {
            // get the color at the current point
            int i_index = (mm_y + j) * pixpertile_ * (mm_maxtile_ + 1) + mm_x + i;
            uint8 bufferColor = a_buffer[i_index];
            switch(g_ped_circle_mask_[j*kCircleMaskSize + i]) {
            case 2:
                a_buffer[i_index] = fillColor;
                break;
            case 1:
                if (bufferColor != fillColor) {
                    a_buffer[i_index] = borderColor;
                }
                break;
            default:
                break;
            }
        }
    }
}

// NOTE: this function is derived from http://cimg.sourceforge.net/
// and is under http://www.cecill.info/licences/Licence_CeCILL_V2-en.txt
// or http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.txt
// licenses
void GamePlayMinimapRenderer::drawSignalCircle(uint8 * a_buffer, int signal_px,
    int signal_py, uint16 radius, uint8 color)
{
    if (!radius)
    {
       drawPixel(a_buffer, signal_px, signal_py, color);
        return;
    }
    drawPixel(a_buffer, signal_px-radius,signal_py, color);
    drawPixel(a_buffer, signal_px+radius,signal_py, color);
    drawPixel(a_buffer, signal_px,signal_py-radius, color);
    drawPixel(a_buffer, signal_px,signal_py+radius, color);
    if (radius==1)
        return;
    for (int f = 1-radius, ddFx = 0, ddFy = -(radius<<1), x = 0, y = radius; x<y; ) {
        if (f>=0) { f+=(ddFy+=2); --y; }
        ++x; ++(f+=(ddFx+=2));
        if (x!=y+1) {
            const int x1 = signal_px-y, x2 = signal_px+y, y1 = signal_py-x,
                y2 = signal_py+x, x3 = signal_px-x, x4 = signal_px+x,
                y3 = signal_py-y, y4 = signal_py+y;
            drawPixel(a_buffer, x1,y1, color);
            drawPixel(a_buffer, x1,y2, color);
            drawPixel(a_buffer, x2,y1, color);
            drawPixel(a_buffer, x2,y2, color);
            if (x!=y)
            {
                drawPixel(a_buffer, x3,y3, color);
                drawPixel(a_buffer, x4,y4, color);
                drawPixel(a_buffer, x4,y3, color);
                drawPixel(a_buffer, x3,y4, color);
            }
        }
    }
}


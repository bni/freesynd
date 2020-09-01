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
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#ifndef MENUS_MINIMAPRENDERER_H_
#define MENUS_MINIMAPRENDERER_H_

#include <map>

#include "common.h"
#include "map.h"
#include "pathsurfaces.h"
#include "utils/timer.h"
#include "core/gameevent.h"

class Mission;
class MissionBriefing;

/*!
 * Base class Renderer for minimap.
 * A minimap renderer can display a minimap at a given position on the screen.
 * The minimap is rendered with a resolution that depends on the zoom level.
 * The resolution is the number of pixel used to render one tile.
 * The renderer also stores the coords for the top left corner of the minimap
 * in the map coordinates. Moving the minimap depends on the subclass.
 */
class MinimapRenderer {
 public:
    /*!
     * Enumeration for the available zoom levels.
     * The higher is the zoom, the more pixels is used to
     * render a tile.
     */
    enum EZoom {
        ZOOM_X1 = 6,
        ZOOM_X2 = 4,
        ZOOM_X3 = 2,
        ZOOM_X4 = 0
    };

    virtual ~MinimapRenderer() {}

    //! update the class with elapsed time
    bool handleTick(int elapsed) { return false; }

    //! Render the minimap at the given point
    void render(uint16 screen_x, uint16 screen_y) {}
 protected:
    void setZoom(EZoom zoom);
     //! called when zoom changes
    virtual void updateRenderingInfos() = 0;
 protected:
    /*! The size in pixel of the minimap. Minimap is a square.*/
    static const int kMiniMapSizePx;

    /*! Number of pixel used to draw a map tile.*/
    uint8 pixpertile_;
    /*! Current zoom level.*/
    EZoom zoom_;
    /*! Tile X coord on the map for the top left corner of the minimap.*/
    uint16 world_tx_;
    /*! Tile Y coord on the map for the top left corner of the minimap.*/
    uint16 world_ty_;
};

/*!
 * Renderer for minimap.
 * This class is used to display a minimap in the briefing menu.
 */
class BriefMinimapRenderer : public MinimapRenderer {
 public:

    //! Class Constructor.
    BriefMinimapRenderer();
    //! Reset the class with a new mission
    void init(MissionBriefing *p_briefing, EZoom zoom, bool drawEnemies);

    //! update the class with elapsed time
    bool handleTick(int elapsed);

    //! Render the minimap at the given point
    void render(uint16 screen_x, uint16 screen_y);

    //! Sets all parameters that depend on zooming level
    void zoomOut();
    //! Sets the flag whether to draw enemies on the minimap or not
    void setDrawEnemies(bool draw) { b_draw_enemies_ = draw; }

    //! Scrolls the minimap to the right
    void scrollRight();
    //! Scrolls the minimap to the left
    void scrollLeft();
    //! Scrolls the minimap to the top
    void scrollUp();
    //! Scrolls the minimap to the bottom
    void scrollDown();

 protected:
    //! Finds the minimap location
    void initMinimapLocation();
    void updateRenderingInfos();
    //! If minimap is too far right or down, align with right or down border
    void clipMinimapToRightAndDown();

 protected:
    /*! A timer to control the blinking on the minimap.*/
    fs_utils::Timer mm_timer;
    /*! Helps controling blinking.*/
    int minimap_blink_;
    /*!
     * Total number of tiles displayed in the minimap.
     * same for width and height.
     */
    uint16 mm_maxtile_;
    /*! The scrolling step : depends on the zoom level.*/
    uint8 scroll_step_;
    /*! The MissionBriefing that contains the minimap.*/
    MissionBriefing *p_mb_;
    /*! IF true, enemies are drawn on the minimap.*/
    bool b_draw_enemies_;
};


/*!
 * Renderer for minimap.
 * This class is used to display a minimap in the gameplay menu.
 */
class GamePlayMinimapRenderer : public MinimapRenderer, GameEventListener {
 public:
    //! Constructor for the class
    GamePlayMinimapRenderer();

    //! Reset the class with a new mission
    void init(Mission *pMission, bool b_scannerEnabled);

    //! Sets the minimap center on the given tile
    void centerOn(uint16 tileX, uint16 tileY, int offX, int offY);

    //! Tells whether scanner is on or off.
    void setScannerEnabled(bool b_enabled);

    //! update the class with elapsed time
    bool handleTick(int elapsed);

    //! Render the minimap
    void render(uint16 screen_x, uint16 screen_y);

    //! Returns the map coordinates of the point on the minimap.
    TilePoint minimapToMapPoint(int mm_x, int mm_y);

    //! Handles game events
    void handleGameEvent(GameEvent evt);
 protected:

    /*!
     * This enumeration lists the type of signal on the minimap.
     */
    enum ESignalType {
        //! No signal is emitted
        kNone,
        //! A signal for a target is emitted
        kTarget,
        //! A signal for an evacuation is emitted
        kEvacuation
    };
    //! called when zoom changes
    void updateRenderingInfos();
    //! Draw all visible cars
    void drawVehicles(uint8 * a_minimap);
    //! Draw all visible dropped weapons
    void drawWeapons(uint8 * a_minimap);
    //! Draw visible peds
    void drawPedestrians(uint8 * a_minimap);
    /*!
     * Returns true if coords is visible on the map
     * \param tx tile coord in world coord.
     * \param ty tile coord in world coord.
     */
    bool isVisible(int tx, int ty) {
        return (tx >= world_tx_ &&
            tx < (world_tx_ + mm_maxtile_) &&
            ty >= world_ty_ &&
            ty < (world_ty_ + mm_maxtile_));
    }
    /*!
     * Returns X coord of the given tile relatively
     * to the top letf corner of the minimap in pixel.
     */
    int mapToMiniMapX(int tileX, int offX) {
        return ((tileX - world_tx_) * pixpertile_) + (offX / (256 / pixpertile_));
    }

    /*!
     * Returns Y coord of the given tile relatively
     * to the top letf corner of the minimap in pixel.
     */
    int mapToMiniMapY(int tileY, int offY) {
        return ((tileY - world_ty_) * pixpertile_) + (offY / (256 / pixpertile_));
    }

    /*!
     * Returns X coord of the current real world coordinate of the signal relatively
     * to the top letf corner of the minimap in pixel.
     */
    int signalXYZToMiniMapX() {
        return signalSourceLocW_.x * pixpertile_ / 256 - world_tx_ * pixpertile_ + pixpertile_;
    }

    /*!
     * Returns Y coord of the current real world coordinate of the signal relatively
     * to the top letf corner of the minimap in pixel.
     */
    int signalXYZToMiniMapY() {
        return signalSourceLocW_.y * pixpertile_ / 256 - world_ty_ * pixpertile_ + pixpertile_;
    }

    /*!
     * Draw a rect to the given buffer with the given size and color.
     * \param a_buffer destination buffer
     * \param mm_x X coord in the destination buffer
     * \param mm_y Y coord in the destination buffer
     * \param width width of the rect to draw
     * \param height height of the rect to draw
     * \param color the color to fill the rect
     */
    inline void drawFillRect(uint8 * a_buffer, int mm_x, int mm_y, size_t width,
                        size_t height, uint8 color)
    {
        uint8 *draw_base = a_buffer + mm_y * pixpertile_ * (mm_maxtile_ + 1) + mm_x;
        for (size_t inc = 0; inc < height; ++inc) {
            uint8 *draw_at = draw_base + inc * pixpertile_ * (mm_maxtile_ + 1);
            for (size_t i = 0; i < width; ++i)
                *draw_at++ = color;
        }
    }

    //! Draw a circle to represent agents, police and guards.
    void drawPedCircle(uint8 * a_buffer, int mm_x, int mm_y, uint8 fillColor,
                            uint8 borderColor);
    //! Draw a circle on the minimap for the signal
    void drawSignalCircle(uint8 * a_buffer, int signal_px, int signal_py, uint16 radius, uint8 color);
    //! Draw a pixel on the minimap
    void drawPixel (uint8 * a_buffer, int x, int y, uint8 color) {
        int mm_maxtile_plus = (mm_maxtile_ + 1);
        if (x > 0 && x < (mm_maxtile_plus * pixpertile_) && y > 0 && y < (mm_maxtile_plus * pixpertile_)) {
            int i_index = y * pixpertile_ * mm_maxtile_plus + x;
            a_buffer[i_index] = color;
        }
    }

    /*!
     * Return true if the evacuation circle is completly on the minimap.
     */
    bool isEvacuationCircleOnMinimap(int signal_px, int signal_py) {
        int maxPx = mm_maxtile_ * pixpertile_;
        return (signal_px + kEvacuationRadius < maxPx &&
                signal_px - kEvacuationRadius > 0 &&
                signal_py + kEvacuationRadius < maxPx &&
                signal_py - kEvacuationRadius > 0);
    }

    //! Clear all signals on map
    void handleClearSignal();
    void handleTargetSet(GameEvent &evt);
    void handleEvacuationSet(GameEvent &evt);
 private:
     /*! Radius of the red evacuation circle.*/
    static const int kEvacuationRadius;

    /*! The mission that contains the minimap.*/
    Mission *p_mission_;
    /*! The minimap to display.*/
    MiniMap *p_minimap_;
    /*!
     * Total number of tiles displayed in the minimap.
     * same for width and height.
     */
    uint16 mm_maxtile_;
    /*! Offset for the tile.*/
    int offset_x_;
    /*! Offset for the tile.*/
    int offset_y_;
    /*! Coords in pixels of the cross.*/
    int cross_x_;
    /*! Coords in pixels of the cross.*/
    int cross_y_;
    /*! Coords on the world map of the signal source.*/
    WorldPoint signalSourceLocW_;
    /*! Type of emitted signal. If NONE, no signal is emitted.*/
    ESignalType signalType_;
    /*! Radius for the signal circle.*/
    uint16 i_signalRadius_;
    /*! Current signal color.*/
    uint8 i_signalColor_;
    /*! A timer to control the blinking of weapons.*/
    fs_utils::BoolTimer mm_timer_weap;
    /*! A timer to control the blinking of pedestrians.*/
    fs_utils::BoolTimer mm_timer_ped;
    /*! Timer for the signal.*/
    fs_utils::Timer mm_timer_signal;
};

#endif  // MENUS_MINIMAPRENDERER_H_

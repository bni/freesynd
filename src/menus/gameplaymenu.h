/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef GAMEPLAYMENU_H
#define GAMEPLAYMENU_H

#include "agentselectorrenderer.h"
#include "maprenderer.h"
#include "minimaprenderer.h"
#include "squadselection.h"
#include "core/gameevent.h"

class Mission;
class IPAStim;

/*!
 * Gameplay Menu class.
 */
class GameplayMenu : public Menu, GameEventListener {
public:
    GameplayMenu(MenuManager *m);
    //! Update the menu state
    void handleTick(int elapsed);
    void handleShow();
    void handleRender(DirtyList &dirtyList);
    void handleLeave();

    //! Handles game events
    void handleGameEvent(GameEvent evt);

protected:
    bool handleUnknownKey(Key key, const int modKeys);

    void handleMouseMotion(int x, int y, int state, const int modKeys);
    bool handleMouseDown(int x, int y, int button, const int modKeys);
    void handleMouseUp(int x, int y, int button, const int modKeys);
    //! Handles the user's click on weapon selector
    void handleClickOnWeaponSelector(int x, int y, int button, const int modKeys);
    //! sets IPA level for defined agent, selector
    void setIPAForAgent(size_t slot, IPAStim::IPAType ipa_type, int percentage);
    //! Handles the user's click on the map
    void handleClickOnMap(int x, int y, int button, const int modKeys);
    //! Handles the user's click on the minimap
    void handleClickOnMinimap(int x, int y);

    void drawSelectAllButton();
    void drawMissionHint(int elapsed);
    void drawWeaponSelectors();
    //! Scroll the map horizontally.
    bool scrollOnX();
    //! Scroll the map vertically.
    bool scrollOnY();
    //void improveScroll(int &newScrollX, int &newScrollY);
    void initWorldCoords();
    //! Selects/deselects an agent
    void selectAgent(size_t agentNo, bool addToGroup);
    //! Selects/deselects all agents
    void selectAllAgents();
    //! Reacts to a weapon selection/deselection
    void handleWeaponSelection(uint8 weapon_idx, bool ctrl);

    //! Deselect agent if he died
    void updateSelectionForDeadAgent(PedInstance *p_ped);
    //! updates visual markers for our agents
    void highlightLeaderMarker();
    //! Set pLocWToSet param with point on the map where player clicked to shoot
    bool getAimedAt(int x, int y, WorldPoint *pLocWToSet);
    void stopShootingEvent();
    //! Centers the minimap on the selection leader
    void centerMinimapOnLeader();
    //! Animate the minimap
    void updateMinimap(int elapsed);
    //! Update the select all button state
    void updateSelectAll();
    //! Update the target value for adrenaline etc for an agent
    void updateIPALevelMeters(int elapsed);

    void updateMarkersPosition();

protected:
    /*! Origin of the minimap on the screen.*/
    static const int kMiniMapScreenX;
    /*! Origin of the minimap on the screen.*/
    static const int kMiniMapScreenY;

    int tick_count_, last_animate_tick_;
    int last_motion_tick_, last_motion_x_, last_motion_y_;
    int mission_hint_ticks_, mission_hint_;
    Mission *mission_;

    /*! This is a projection in 2D of a point on a the map. This point represents the top
     * left corner of the screen and it moves inside the map's borders.*/
    Point2D displayOriginPt_;
    /*! Holds the amount of scroll on the X axis.*/
    int scroll_x_;
    /*! Holds the amount of scroll on the Y axis.*/
    int scroll_y_;
    /*! Agent selection manager.*/
    SquadSelection selection_;
    /*! Object mouse cursor is above*/
    ShootableMapObject *target_;
    /*! This renderer is in charge of drawing the map.*/
    MapRenderer map_renderer_;
    /*! This renderer is in charge of drawing the minimap.*/
    GamePlayMinimapRenderer mm_renderer_;
    /*! This renderer is in charge of drawing the IPA meters.*/
    AgentSelectorRenderer agt_sel_renderer_;
    //! A flag to keep track of the state of the select all button
    bool isButtonSelectAllPressed_;
    /*! Flag to store the fact that player is currently shooting.*/
    bool isPlayerShooting_;
    /*! Flag to play the sound of police warning an agent.*/
    bool canPlayPoliceWarnSound_;
    /*! Delay between 2 police warnings.*/
    fs_utils::Timer warningTimer_;

    // when ipa is manipulated this represents
    struct IPA_manipulation {
        // ipa that is manipulated
        int8 ipa_chng;
        // agent to base IPA's update on
        uint8 agent_used;
    } ipa_chng_;
};

#endif

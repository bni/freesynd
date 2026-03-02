/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2010, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net>
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

#ifndef GAMEPLAYMENU_H
#define GAMEPLAYMENU_H

#include "fs-engine/menus/menumanager.h"
#include "fs-engine/events/event.h"
#include "agentselectorrenderer.h"
#include "maprenderer.h"
#include "minimaprenderer.h"
#include "squadselection.h"


/*!
 * Gameplay Menu class.
 */
class GameplayMenu : public fs_eng::Menu {
public:
    GameplayMenu(fs_eng::MenuManager *m);
    //! Update the menu state
    bool handleTick(uint32_t elapsed) override;
    bool handleBeforeShow() override;
    void handleRender() override;
    void handleLeave() override;

protected:
    /**
     * @name Mouse/Key events handling
     */
    ///@{
    bool handleUnMappedKey(const fs_eng::FS_Key key) override;

    void handleMouseMotion(Point2D point, uint32_t state) override;
    bool handleMouseDown(Point2D point, int button) override;
    void handleMouseUp(Point2D point, int button) override;
    //! Handles the user's click on weapon selector
    void handleClickOnWeaponSelector(Point2D point, int button);
    //! sets IPA level for defined agent, selector
    void setIPAForAgent(size_t slot, IPAStim::IPAType ipa_type, uint8_t percentage);
    //! Handles the user's click on the map
    void handleClickOnMap(Point2D point, int button);
    //! Handles the user's click on the minimap
    void handleClickOnMinimap(Point2D point);
    ///@}

    /**
     * @name In game events handling
     */
    ///@{
    //! Handle when an agent has died
    void onAgentDiedEvent(fs_knl::AgentDiedEvent *pEvt);
    //! Handle when a shooting weapon was selected or deselected
    void onShootingWeaponSelectedEvent(fs_knl::ShootingWeaponSelectedEvent *pEvt);
    //!
    void onPoliceWarningEmittedEvent(fs_knl::PoliceWarningEmittedEvent *pEvt);
    //! When mission ends
    void onMissionEndedEvent(fs_knl::MissionEndedEvent *pEvt);
    ///@}

    /**
     * @name State update
     */
    ///@{
    //! Update the target value for adrenaline etc for an agent
    void updateIPALevelMeters(uint32_t elapsed);

    void updateMarkersPosition();
    //! Get the current hint and manage animation
    void updateMissionHint(uint32_t elapsed);
    ///@}

    /**
     * @name Draw screen elements
     */
    ///@{
    //!
    void drawSelectAllButton();
    void drawMissionHint();
    void drawWeaponSelectors();
    void drawPausePanel();
    ///@}
    
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
    void handleWeaponSelection(uint8_t weapon_idx, bool ctrl);

    //! Deselect agent if he died
    void updateSelectionForDeadAgent(fs_knl::PedInstance *p_ped);
    //! updates visual markers for our agents
    void highlightLeaderMarker();
    //! Set pLocWToSet param with point on the map where player clicked to shoot
    bool getAimedAt(int x, int y, fs_knl::WorldPoint *pLocWToSet);
    void stopShootingEvent();
    //! Centers the minimap on the selection leader
    void centerMinimapOnLeader();
    //! Centers the map view on a specific squad agent slot
    void centerViewOnAgent(size_t agentNo);
    //! Update the select all button state
    void updateSelectAll();

protected:
    /*! Origin of the minimap on the screen.*/
    static const Point2D kMiniMapScreenPos;

    uint32_t tick_count_, last_animate_tick_;
    /*! Tracks the last agent key (1-4) press time for double-press detection.*/
    uint32_t lastAgentKeyTime_;
    /*! Tracks which agent slot (0-3) was last pressed, -1 for none.*/
    int lastAgentKeyIndex_;
    int last_motion_x_, last_motion_y_;
    //! Text of the hint to display
    std::string hint_;
    //! Color to draw the hint text
    uint8_t hintColor_;
    //! Yellow rect displayed when the objective name is displayed to accentuate text
    bool drawHintYellowRect_;
    /*!
     * Timer to alternate between agent action and mission objective
     * When state is true, we display ped status.
     * When state if false, we display mission's objective
     */
    fs_utl::BoolTimer hintTimer_;
    //! Timer to control blinking color of hint when displaying objectives.
    fs_utl::BoolTimer hintColorTimer_;
    //! Timer to control the display of Press space message when mission is over
    fs_utl::BoolTimer hintSpaceTimer_;

    fs_knl::Mission *mission_;

    /*! This is a projection in 2D of a point on a the map. This point represents the top
     * left corner of the screen and it moves inside the map's borders.*/
    Point2D displayOriginPt_;
    /*! Holds the amount of scroll on the X axis.*/
    int scroll_x_;
    /*! Holds the amount of scroll on the Y axis.*/
    int scroll_y_;
    /*! Sub-pixel accumulators for smooth WASD panning.*/
    float wasd_pan_accum_x_;
    float wasd_pan_accum_y_;
    /*! True while the map is being panned (ctrl+drag or middle-mouse drag).*/
    bool isPanning_;
    /*! Agent selection manager.*/
    SquadSelection selection_;
    /*! Object mouse cursor is above*/
    fs_knl::ShootableMapObject *target_;
    //! The palette of colors used for this mission
    fs_eng::Palette missionPalette_;
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
    fs_utl::Timer warningTimer_;

    //! Handles used for removing listener
    ListenerHandle handleAgentDied_;
    ListenerHandle handleWeaponSelected_;
    ListenerHandle handleAgentWarned_;
    ListenerHandle handleMissionEnded_;

    // when ipa is manipulated this represents
    struct IPA_manipulation {
        // ipa that is manipulated
        int8_t ipa_chng;
        // agent to base IPA's update on
        size_t agent_used;
    } ipa_chng_;
};

#endif

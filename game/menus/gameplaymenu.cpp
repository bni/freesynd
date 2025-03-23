/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2007  Davor Ocelic <docelic@mail.inet.hr>
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

#include "gameplaymenu.h"

#include <cassert>

#include "fs-engine/gfx/animationmanager.h"
#include "fs-engine/sound/musicmanager.h"
#include "fs-engine/sound/soundmanager.h"
#include "menus/gamemenuid.h"
#include "fs-engine/menus/fliplayer.h"
#include "fs-kernel/model/vehicle.h"
#include "fs-kernel/mgr/missionmanager.h"
#include "fs-kernel/model/shot.h"
#include "core/gamecontroller.h"
#include "core/gamesession.h"

#ifdef _WIN32
#include <windows.h>
#endif

// The number of pixel of a scroll
const int SCROLL_STEP = 16;

const Point2D GameplayMenu::kMiniMapScreenPos = {0, 46 + 44 + 10 + 46 + 44 + 15 + 2 * 32 + 2};

//#define ANIM_PLUS_FRAME_VIEW

GameplayMenu::GameplayMenu(fs_eng::MenuManager *m) :
Menu(m, fs_game_menus::kMenuIdGameplay, fs_game_menus::kMenuIdDebrief),
tick_count_(0), last_animate_tick_(0), last_motion_tick_(0),
last_motion_x_(320), last_motion_y_(240), mission_hint_ticks_(0),
mission_hint_(0), mission_(NULL), selection_(),
target_(NULL),
mm_renderer_(kMiniMapScreenPos), warningTimer_(20000)
{
    cursorOnShow_ = kGameplayCursor;
    displayOriginPt_.x = 0;
    displayOriginPt_.y = 0;
    scroll_x_ = 0;
    scroll_y_ = 0;
    ipa_chng_.ipa_chng = -1;
    canPlayPoliceWarnSound_ = true;
}

/*!
 * Scroll the map horizontally.
 * Each map has a min and max value for the world origin coords and this
 * method moves that point between those limits. If scrolling hits the
 * map border, the scrolling is made along that border.
 * \return True is a scroll is made
 */
bool GameplayMenu::scrollOnX() {
    bool change = false;

    int newOriginX = displayOriginPt_.x + scroll_x_;

    fs_knl::TilePoint mpt = mission_->get_map()->screenToTilePoint(newOriginX, displayOriginPt_.y);

    // Scroll to the right
    if (scroll_x_ > 0) {
        if (mpt.ty < mission_->minY()) {
            // we hit the upper right border of the map
            // so we scroll down until the far right corner
            int newWorldY = displayOriginPt_.y + SCROLL_STEP;
            newOriginX += SCROLL_STEP;
            mpt = mission_->get_map()->screenToTilePoint(newOriginX, newWorldY);

            if (mpt.ty < mission_->minY() || mpt.tx > mission_->maxX()) {
                // We hit the corner so don't scroll
                return false;
            } else {
                displayOriginPt_.x = newOriginX;
                displayOriginPt_.y = newWorldY;
                change = true;
            }
        } else if (mpt.tx > mission_->maxX()) {
            // we hit the lower right border of the map
            // so we scroll up until the far right corner
            int newWorldY = displayOriginPt_.y - SCROLL_STEP;
            newOriginX += SCROLL_STEP;
            mpt = mission_->get_map()->screenToTilePoint(newOriginX, newWorldY);

            if (mpt.ty < mission_->minY() || mpt.tx > mission_->maxX()) {
                return false;
            } else {
                displayOriginPt_.x = newOriginX;
                displayOriginPt_.y = newWorldY;
                change = true;
            }
        } else {
            // This is a regular right scroll
            displayOriginPt_.x = newOriginX;
            change = true;
        }

    } else { // Scroll to the left
        if (mpt.tx < mission_->minX()) {
            // we hit the upper left border of the map
            // so we scroll down until the far left corner
            int newWorldY = displayOriginPt_.y + SCROLL_STEP;
            newOriginX -= SCROLL_STEP;
            mpt = mission_->get_map()->screenToTilePoint(newOriginX, newWorldY);

            if (mpt.tx < mission_->minX() || mpt.ty > mission_->maxY()) {
                return false;
            } else {
                displayOriginPt_.x = newOriginX;
                displayOriginPt_.y = newWorldY;
                change = true;
            }
        } else if (mpt.ty > mission_->maxY()) {
            // we hit the lower left border of the map
            // so we scroll up until the far left corner
            int newWorldY = displayOriginPt_.y - SCROLL_STEP;
            newOriginX -= SCROLL_STEP;
            mpt = mission_->get_map()->screenToTilePoint(newOriginX, newWorldY);

            if (mpt.tx < mission_->minX() || mpt.ty > mission_->maxY()) {
                return false;
            } else {
                displayOriginPt_.x = newOriginX;
                displayOriginPt_.y = newWorldY;
                change = true;
            }
        } else {
            displayOriginPt_.x = newOriginX;
            change = true;
        }
    }

    return change;
}

/*!
 * Scroll the map vertically.
 * Each map has a min and max value for the world origin coords and this
 * method moves that point between those limits. If scrolling hits the
 * map border, the scrolling is made along that border.
 * \return True is a scroll is made
 */
bool GameplayMenu::scrollOnY() {
    bool change = false;

    int newWorldY = displayOriginPt_.y + scroll_y_;

    fs_knl::TilePoint mpt = mission_->get_map()->screenToTilePoint(displayOriginPt_.x, newWorldY);

    // Scroll down
    if (scroll_y_ > 0) {
        if (mpt.tx > mission_->maxX()) {
            // we hit the lower right border of the map
            // so we scroll down until the lower corner
            int newOriginX = displayOriginPt_.x - 2*SCROLL_STEP;
            mpt = mission_->get_map()->screenToTilePoint(newOriginX, newWorldY);

            if (mpt.ty > mission_->maxY() || mpt.tx > mission_->maxX()) {
                return false;
            } else {
                displayOriginPt_.x = newOriginX;
                displayOriginPt_.y = newWorldY;
                change = true;
            }
        } else if (mpt.ty > mission_->maxY()) {
            // we hit the lower left border of the map
            // so we scroll down until the lower corner
            int newOriginX = displayOriginPt_.x + 2*SCROLL_STEP;
            mpt = mission_->get_map()->screenToTilePoint(newOriginX, newWorldY);

            if (mpt.ty > mission_->maxY() || mpt.tx > mission_->maxX()) {
                return false;
            } else {
                displayOriginPt_.x = newOriginX;
                displayOriginPt_.y = newWorldY;
                change = true;
            }
        } else {
            displayOriginPt_.y = newWorldY;
            change = true;
        }

    } else { // Scroll up
        if (mpt.tx < mission_->minX()) {
            // we hit the upper right border of the map
            // so we scroll up until the upper corner
            int newOriginX = displayOriginPt_.x + 2*SCROLL_STEP;
            mpt = mission_->get_map()->screenToTilePoint(newOriginX, newWorldY);

            if (mpt.ty < mission_->minY() || mpt.tx < mission_->minX()) {
                return false;
            } else {
                displayOriginPt_.x = newOriginX;
                displayOriginPt_.y = newWorldY;
                change = true;
            }
        } else if (mpt.ty < mission_->minY()) {
            // we hit the upper left border of the map
            // so we scroll up until the upper corner
            int newOriginX = displayOriginPt_.x - 2*SCROLL_STEP;
            mpt = mission_->get_map()->screenToTilePoint(newOriginX, newWorldY);

            if (mpt.ty < mission_->minY() || mpt.tx < mission_->minX()) {
                return false;
            } else {
                displayOriginPt_.x = newOriginX;
                displayOriginPt_.y = newWorldY;
                change = true;
            }
        } else {
            displayOriginPt_.y = newWorldY;
            change = true;
        }
    }

    return change;
}

/*!
 * Initialize the screen position centered on the squad leader.
 */
void GameplayMenu::initWorldCoords()
{
    // get the leader position on the map
    fs_knl::PedInstance *p_leader = selection_.leader();
    fs_knl::TilePoint leaderPos(p_leader->tileX(),
                                p_leader->tileY(),
                                mission_->mmax_z_ + 1,
                                0, 0);
    Point2D start;
    mission_->get_map()->tileToScreenPoint(leaderPos, &start);
    start.x -= (fs_eng::kScreenWidth - 129) / 2;
    start.y -= fs_eng::kScreenHeight / 2;

    if (start.x < 0)
        start.x = 0;

    if (start.y < 0)
        start.y = 0;

    // Check if the position is within map borders
    fs_knl::TilePoint mpt = mission_->get_map()->screenToTilePoint(start.x, start.y);

    if (mpt.tx < mission_->minX())
        mpt.tx = mission_->minX();

    if (mpt.ty < mission_->minY())
        mpt.ty = mission_->minY();

    if (mpt.tx > mission_->maxX())
        mpt.tx = mission_->maxX();

    if (mpt.ty > mission_->maxY())
        mpt.ty = mission_->maxY();

    // recalculating new screen coords
    fs_knl::TilePoint newPoint(mpt.tx,
                                mpt.ty,
                                mission_->mmax_z_ + 1, 
                                0, 0);
    Point2D msp;
    mission_->get_map()->tileToScreenPoint(newPoint, &msp);
    displayOriginPt_.x = msp.x;
    displayOriginPt_.y = msp.y;
}

/*!
 * Called before the menu is first shown.
 * Place to do some initializing.
 */
bool GameplayMenu::handleBeforeShow() {
    mission_ = g_missionCtrl.mission();
    mission_->start(g_gameCtrl.weaponManager());
    // init selection to the first selectable agent
    selection_.setSquad(mission_->getSquad());

    // init menu internal state
    isButtonSelectAllPressed_ = false;
    initWorldCoords();

    // set graphic palette
    missionPalette_ = mission_->get_map()->getTileManager()->getPalette();
    g_SpriteMgr.setPalette(missionPalette_);

    highlightLeaderMarker();
    updateMarkersPosition();

    // Init renderers
    map_renderer_.init(mission_, &selection_);
    mm_renderer_.init(mission_, mission_->getSquad()->hasScanner(), missionPalette_);
    centerMinimapOnLeader();
    isPlayerShooting_ = false;

    // Register event handlers
    handleAgentDied_ = EventManager::listen<fs_knl::AgentDiedEvent>(this, &GameplayMenu::onAgentDiedEvent);
    handleWeaponSelected_ = EventManager::listen<fs_knl::ShootingWeaponSelectedEvent>(this, &GameplayMenu::onShootingWeaponSelectedEvent);
    handleAgentWarned_ = EventManager::listen<fs_knl::PoliceWarningEmittedEvent>(this, &GameplayMenu::onPoliceWarningEmittedEvent);
    handleMissionEnded_ = EventManager::listen<fs_knl::MissionEndedEvent>(this, &GameplayMenu::onMissionEndedEvent);

    // play game track
    g_MusicMgr.playSong(fs_eng::MusicManager::kMusicSongAssassinate, true);
    
    menu_manager_->resetSinceMouseDown();

    return true;
}

#ifdef ANIM_PLUS_FRAME_VIEW
int qanim = 1959, qframe = 0;
#endif

bool GameplayMenu::handleTick(uint32_t elapsed)
{
    if (paused_)
        return true;
    bool change = false;
    tick_count_ += elapsed;

    mission_->handleTick(elapsed);

    if (!canPlayPoliceWarnSound_ && warningTimer_.update(elapsed)) {
        // wait an amount of time before allowing another warning
        canPlayPoliceWarnSound_ = true;
    }

    // Scroll the map
    if (scroll_x_ != 0) {
        change = scrollOnX();
        scroll_x_ = 0;
    }

    if (scroll_y_ != 0) {
        change = scrollOnY();
        scroll_y_ = 0;
    }

    if (tick_count_ - last_animate_tick_ > 33) {
        uint32_t diff = tick_count_ - last_animate_tick_;
        last_animate_tick_ = tick_count_;

        for (size_t i = 0; i < mission_->numSfxObjects(); i++) {
            fs_knl::SFXObject *pSfx = mission_->sfxObjects(i);
            pSfx->animate2(diff);
            if (pSfx->sfxLifeOver()) {
                mission_->delSfxObject(i);
                i--;
            }
        }

        for (size_t i = 0; i < mission_->numPeds(); i++)
            change |= mission_->ped(i)->animate(diff);


        for (size_t i = 0; i < mission_->numVehicles(); i++)
            mission_->vehicle(i)->animate2(diff);

        for (size_t i = 0; i < mission_->numWeaponsOnGround(); i++)
            mission_->weaponOnGround(i)->animate2(diff);

        for (size_t i = 0; i < mission_->numStatics(); i++) {
            mission_->statics(i)->animate2(elapsed);
        }

        for (size_t i = 0; i < mission_->numPrjShots(); i++) {
            change |= mission_->prjShots(i)->animate(diff, mission_);
            if (mission_->prjShots(i)->isLifeOver()) {
                mission_->delPrjShot(i);
                i--;
            }
        }

        updateMarkersPosition();
    }

    mm_renderer_.handleTick(elapsed);

    updateIPALevelMeters(elapsed);

    if (change) {
        // force target to update
        handleMouseMotion({last_motion_x_, last_motion_y_}, 0);
    }

    drawMissionHint(elapsed);

    return true;
}

void GameplayMenu::handleRender() {
    map_renderer_.render(displayOriginPt_);
    g_System.drawRect({0,0}, 129, fs_eng::kScreenHeight, menu_manager_->kMenuColorBlack);
    agt_sel_renderer_.render(selection_, mission_->getSquad(), missionPalette_);
    drawSelectAllButton();
    drawMissionHint(0);
    drawWeaponSelectors();
    mm_renderer_.render(missionPalette_);

    if (paused_) {
        drawPausePanel();
    }

#ifdef _DEBUG
    // drawing of different sprites
//    g_SpriteMgr.sprite(9 * 40 + 1)->draw(0, 0, 0, false, true);
#if 0
    // 1601 == start of weapons icons for sidebar
    // 1621 == start of selected weapons icons for sidebar
    // 1748 == start of agent selectors for sidebar
    // 1772 == start of selected agent selectors for sidebar
    g_Screen.clear(1);
    int x = 0, y = 0, my = 0;
    for (int i = 1756; i < g_SpriteMgr.spriteCount(); i++) {
        Sprite *s = g_SpriteMgr.sprite(i);

        if (y + s->height() > GAME_SCREEN_HEIGHT) {
            printf("last sprite %i\n", i - 1);
            break;
        }

        if (x + s->width() > GAME_SCREEN_WIDTH) {
            x = 0;
            y += my;
            my = 0;
        }

        s->draw(x, y, 0);
        x += s->width();
        if (s->height() > my)
            my = s->height();
    }
#endif
    // this is used in combination with keys
#ifdef ANIM_PLUS_FRAME_VIEW
    g_SpriteMgr.drawFrame(qanim, qframe, 320, 200);
#endif
#endif

#ifdef FS_TRACK_FPS
    std::stringstream fpsText;
    static int current_time = 0;
    static int last_time = 0;
    static float fps = 0;
    static int frame = 0;

    current_time = g_System.getTicks();

    int elapsed = current_time - last_time;
    frame++;

    if (elapsed > 1000) {
        fps = (float) frame / ((float) elapsed / 1000.0f);
        frame = 0;
        last_time = current_time;
    }

    fpsText << "FPS: " << fps; 
    gameFont()->drawText(10, fs_eng::kScreenHeight - 15, fpsText.str().c_str(), menu_manager_->kMenuColorYellow);
#endif
}

void GameplayMenu::handleLeave()
{
    g_MusicMgr.stopCurrentSong();

    // Remove handlers to prevent events coming after the end of mission
    EventManager::remove_listener(handleAgentDied_);
    EventManager::remove_listener(handleWeaponSelected_);
    EventManager::remove_listener(handleAgentWarned_);
    EventManager::remove_listener(handleMissionEnded_);

    selection_.clear();

    tick_count_ = 0;
    last_animate_tick_ = 0;
    last_motion_tick_ = 0;
    last_motion_x_ = 320;
    last_motion_y_ = 240;
    mission_hint_ticks_ = 0;
    mission_hint_ = 0;
    displayOriginPt_.x = 0;
    displayOriginPt_.y = 0;
    target_ = NULL;
    mission_ = NULL;
    scroll_x_ = 0;
    scroll_y_ = 0;
    paused_ = false;
    ipa_chng_.ipa_chng = -1;
}

void GameplayMenu::handleMouseMotion(Point2D point, uint32_t state)
{
    last_motion_tick_ = tick_count_;
    last_motion_x_ = point.x;
    last_motion_y_ = point.y;
    // locking mouse motion on ipa change until mouseup is recieved
    if (ipa_chng_.ipa_chng != -1 && menu_manager_->isMouseDragged()) {
        fs_knl::PedInstance *p = mission_->ped(ipa_chng_.agent_used);
        if (p->isAlive()) {
            int percent = agt_sel_renderer_.getPercentageAnyX(
                ipa_chng_.agent_used, point.x);

            // if agent is in selected group we will update all groups IPA
            if (selection_.isAgentSelected(ipa_chng_.agent_used)) {
                for (uint8 i = 0; i < fs_knl::Squad::kMaxSlot; ++i) {
                    if (selection_.isAgentSelected(i)) {
                        setIPAForAgent(i, (IPAStim::IPAType)ipa_chng_.ipa_chng,
                            percent);
                    }
                }
            } else {
                setIPAForAgent(ipa_chng_.agent_used,
                    (IPAStim::IPAType)ipa_chng_.ipa_chng, percent);
            }
            return;
        } else
            ipa_chng_.ipa_chng = -1;
    }

    if (last_motion_x_ < 5) {
        scroll_x_ = - SCROLL_STEP;
    } else if (last_motion_x_ > fs_eng::kScreenWidth - 5) {
        scroll_x_ = SCROLL_STEP;
    }

    if (last_motion_y_ < 5) {
        scroll_y_ = - SCROLL_STEP;
    } else if (last_motion_y_ > fs_eng::kScreenHeight - 5) {
        scroll_y_ = SCROLL_STEP;
    }

    bool inrange = false;
    target_ = NULL;

    if (point.x > 128) {
        for (size_t i = mission_->getSquad()->size(); mission_ && i < mission_->numPeds(); ++i) {
            fs_knl::PedInstance *p = mission_->ped(i);
            if (p->isAlive() && p->isDrawable()) {
                Point2D scPt;
                mission_->get_map()->tileToScreenPoint(p->position(), &scPt);
                int px = scPt.x - 10;
                int py = scPt.y - (1 + p->tileZ()) * fs_eng::Tile::kTileHeight/3
                    - (p->offZ() * fs_eng::Tile::kTileHeight/3) / 128;

                if (point.x - 129 + displayOriginPt_.x >= px && point.y + displayOriginPt_.y >= py &&
                    point.x - 129 + displayOriginPt_.x < px + 21 && point.y + displayOriginPt_.y < py + 34)
                {
                    // mouse pointer is on the object, so it's the new target
                    target_ = p;
                    inrange = selection_.isTargetInRange(mission_, target_);
                    break;
                }
            }
        }

        for (size_t i = 0; mission_ && i < mission_->numVehicles(); ++i) {
            fs_knl::Vehicle *v = mission_->vehicle(i);
            // TrainHead cannot be selected to prevent player from putting agents in it
            if (v->isAlive() && v->getType() != fs_knl::Vehicle::kVehicleTypeTrainHead) {
                Point2D scPt;
                mission_->get_map()->tileToScreenPoint(v->position(), &scPt);
                int px = scPt.x - 20;
                int py = scPt.y - 10 - v->tileZ() * fs_eng::Tile::kTileHeight/3;

                if (point.x - 129 + displayOriginPt_.x >= px && point.y + displayOriginPt_.y >= py &&
                    point.x - 129 + displayOriginPt_.x < px + 40 && point.y + displayOriginPt_.y < py + 32)
                {
                    target_ = v;
                    inrange = selection_.isTargetInRange(mission_, target_);
                    break;
                }
            }
        }

        for (size_t i = 0; mission_ && i < mission_->numWeaponsOnGround(); ++i) {
            fs_knl::WeaponInstance *w = mission_->weaponOnGround(i);

            if (w->isDrawable()) {
                Point2D scPt;
                mission_->get_map()->tileToScreenPoint(w->position(), &scPt);
                int px = scPt.x - 10;
                int py = scPt.y + 4 - w->tileZ() * fs_eng::Tile::kTileHeight/3
                    - (w->offZ() * fs_eng::Tile::kTileHeight/3) / 128;

                if (point.x - 129 + displayOriginPt_.x >= px && point.y + displayOriginPt_.y >= py &&
                    point.x - 129 + displayOriginPt_.x < px + 20 && point.y + displayOriginPt_.y < py + 15)
                {
                    target_ = w;
                    break;
                }
            }
        }
#if 0
#ifdef _DEBUG
        for (int i = 0; mission_ && i < mission_->numStatics(); ++i) {
            Static *s = mission_->statics(i);

            if (s->map() != -1) {
                Point2D scPt;
                mission_->get_map()->tileToScreenPoint(w->position(), &scPt);
                int px = scPt.x - 10;
                int py = scPt.y + 4 - s->tileZ() * Tile::kTileHeight/3
                    - (s->offZ() * Tile::kTileHeight/3) / 128;

                if (x - 129 + displayOriginPt_.x >= px && y + displayOriginPt_.y >= py &&
                    x - 129 + displayOriginPt_.x < px + 20 && y + displayOriginPt_.y < py + 15)
                {
                    target_ = s;
                    break;
                }
            }
        }
#endif
#endif
    }

    if (target_) {
        if (target_->is(fs_knl::MapObject::kNaturePed) ||
            target_->is(fs_knl::MapObject::kNatureVehicle)) {
            if (inrange)
                g_System.useTargetRedCursor();
            else
                g_System.useTargetCursor();
        } else if (target_->is(fs_knl::MapObject::kNatureWeapon)) {
            g_System.usePickupCursor();
        }
    } else if (point.x > 128) {
            g_System.usePointerCursor();
    } else {
            g_System.usePointerYellowCursor();
    }

    if (point.x < 129 && isPlayerShooting_) {
        stopShootingEvent();
    }

    if (isPlayerShooting_) {
        // update direction for each shooting player
        fs_knl::WorldPoint aimedAtLocW;
        if (getAimedAt(point.x, point.y, &aimedAtLocW)) {
            for (SquadSelection::Iterator it = selection_.begin(); it != selection_.end(); ++it) {
                fs_knl::PedInstance *pAgent = *it;
                if (pAgent->isUsingWeapon()) {
                    // If ped is currently shooting
                    // then update the action with new shooting target
                    pAgent->updateShootingTarget(aimedAtLocW);
                }
            }
        }
    }
}

bool GameplayMenu::handleMouseDown(Point2D point, int button)
{
    if (paused_)
        return true;

    if (point.x < 129) {
        // Is control button pressed
        bool ctrl = g_System.isKeyModStatePressed(fs_eng::KMD_CTRL);

        // First check if player has clicked on agent selectors
        SelectorEvent selEvt;
        if (agt_sel_renderer_.hasClickedOnAgentSelector(point.x, point.y, selEvt)) {
            switch (selEvt.eventType) {
            case SelectorEvent::kSelectAgent:
                // Handle agent selection. Click on an agent changes selection
                // to it. If control key is pressed, add or removes agent from
                // current selection.
                selectAgent(selEvt.agentSlot, ctrl);
                break;
            case SelectorEvent::kSelectIpa:
                ipa_chng_.ipa_chng = selEvt.IpaType;
                ipa_chng_.agent_used = selEvt.agentSlot;
                setIPAForAgent(selEvt.agentSlot, selEvt.IpaType, selEvt.percentage);
                break;
            case SelectorEvent::kNone:
                break;
            }
        } else if (point.y >= 42 + 48 && point.y < 42 + 48 + 10) {
            // User clicked on the select all button
            selectAllAgents();
        }
        else if (point.y >= 2 + 46 + 44 + 10 + 46 + 44 + 15
                 && point.y < 2 + 46 + 44 + 10 + 46 + 44 + 15 + 64)
        {
            // user clicked on the weapon selector
            handleClickOnWeaponSelector(point, button);
        } else if ( point.y > kMiniMapScreenPos.y && button == kMouseLeftButton) {
            handleClickOnMinimap(point);
        }
    } else {
        // User clicked on the map
        handleClickOnMap(point, button);
    }

    return true;
}

/*!
 * The user has clicked on the weapon selector.
 * \param point Mouse coord
 * \param button Mouse button that was clicked
 */
void GameplayMenu::handleClickOnWeaponSelector(Point2D point, int button)
{
    uint8 w_num = ((point.y - (2 + 46 + 44 + 10 + 46 + 44 + 15)) / 32) * 4
            + point.x / 32;
    fs_knl::PedInstance *pLeader = selection_.leader();
    if (pLeader->isAlive()) {
        bool is_ctrl = g_System.isKeyModStatePressed(fs_eng::KMD_CTRL);
        if (w_num < pLeader->numWeapons()) {
            if (button == kMouseLeftButton) {
                // Button 1 : selection/deselection of weapon for all selection
                handleWeaponSelection(w_num, is_ctrl);
            } else {
                // Button 3 : drop weapon from selected agent inventory
                pLeader->addActionPutdown(w_num, is_ctrl);
            }
        }
    }
}

void GameplayMenu::setIPAForAgent(size_t slot, IPAStim::IPAType ipa_type, int percentage)
{
    fs_knl::PedInstance *ped = mission_->ped(slot);
    if (ped->isDead())
        return;

    switch(ipa_type)
    {
        case IPAStim::Adrenaline:
            ped->adrenaline_->setAmount(percentage);
            break;
        case IPAStim::Perception:
            ped->perception_->setAmount(percentage);
            break;
        case IPAStim::Intelligence:
            ped->intelligence_->setAmount(percentage);
            break;
    }
}

void GameplayMenu::updateIPALevelMeters(int elapsed)
{
    for (size_t agent = 0; agent < fs_knl::Squad::kMaxSlot; ++agent) {
        fs_knl::PedInstance *ped = mission_->getSquad()->member(agent);
        if (ped && ped->isAlive())
            ped->updtIPATime(elapsed);
    }
}

/*!
 * React to a click on the map
 * @param point Where the player clicked in screen coordinates
 * @param button The mouse button he clicked
 */
void GameplayMenu::handleClickOnMap(Point2D point, int button) {
    fs_knl::TilePoint mapPt = mission_->get_map()->screenToTilePoint(displayOriginPt_.x + point.x - 129,
                    displayOriginPt_.y + point.y);
#ifdef _DEBUG
    if (g_System.isKeyModStatePressed(fs_eng::KMD_ALT)) {
        printf("Tile x:%d, y:%d, z:%d, ox:%d, oy:%d\n",
            mapPt.tx, mapPt.ty, mapPt.tz, mapPt.ox, mapPt.oy);

        if (target_) {
            printf("   > target(%i) : %s\n",
                target_->id(), target_->natureName());
        }

        int tileid = mission_->get_map()->tileAt(mapPt.tx, mapPt.ty, mapPt.tz);
        printf("Tile id %d\n", tileid);
        return;
    }
#endif //_DEBUG

    bool ctrl = g_System.isKeyModStatePressed(fs_eng::KMD_CTRL);
    if (button == kMouseLeftButton) {
        if (target_) {
            switch (target_->nature()) {
            case fs_knl::MapObject::kNatureWeapon:
                selection_.pickupWeapon(dynamic_cast<fs_knl::WeaponInstance *>(target_), ctrl);
                break;
            case fs_knl::MapObject::kNaturePed:
                selection_.followPed(dynamic_cast<fs_knl::PedInstance *>(target_));
                break;
            case fs_knl::MapObject::kNatureVehicle:
                selection_.enterOrLeaveVehicle(dynamic_cast<fs_knl::Vehicle *>(target_), ctrl);
                break;
            default:
                break;
            }
        } else if (mission_->getWalkable(mapPt)) {
            selection_.moveTo(mapPt, ctrl);
        }
    } else if (button == kMouseRightButton) {
        fs_knl::WorldPoint aimedAtLocW;
        if (getAimedAt(point.x, point.y, &aimedAtLocW)) {
            isPlayerShooting_ = true;
            selection_.shootAt(aimedAtLocW);
        }
    }
}

/*!
 * User has clicked on the minimap. All selected agent go to destination.
 * Clicking on the minimap does not allow to shoot or to use objects.
 * \param point minimap coordinate
 */
void GameplayMenu::handleClickOnMinimap(Point2D point) {
    // convert minimap coordinate in map coordinate
    fs_knl::TilePoint pt = mm_renderer_.minimapToMapPoint(point.add(-kMiniMapScreenPos.x, -kMiniMapScreenPos.y));
    // As minimap is flat, we can't see the height. So take the Z coordinate
    // of the leader as a reference
    pt.tz = selection_.leader()->tileZ();
    if (mission_->getWalkableClosestByZ(pt))
    {
        // Destination is walkable so go
        selection_.moveTo(pt, false);
     }
}

/*!
 * Set the point on the map the player is aiming at.
 * It depends on whether the player has clicked on a shootable target
 * or a point on the ground.
 * \param x mouse X coord on screen
 * \param y mouse Y coord on screen
 * \param pLocWToSet Finale location
 * \return True if location has been set.
 */
bool GameplayMenu::getAimedAt(int x, int y, fs_knl::WorldPoint *pLocWToSet) {
    bool locationSet = false;

    if (target_) {
        //  Player has aimed an object
        pLocWToSet->convertFromTilePoint(target_->position());
        // z is set to half the size of the object
        pLocWToSet->z += target_->sizeZ() >> 1;
        locationSet = true;
    } else {
        // Player is shooting on the ground
        fs_knl::TilePoint mapLocT = mission_->get_map()->screenToTilePoint(displayOriginPt_.x + x - 129,
                    displayOriginPt_.y + y);
        mapLocT.tz = 0;
        if (mission_->getShootableTile(&mapLocT)) {
            locationSet = true;
            pLocWToSet->convertFromTilePoint(mapLocT);
        }
    }

    return locationSet;
}

/*!
 *
 */
void GameplayMenu::stopShootingEvent()
{
    isPlayerShooting_ = false;
    for (SquadSelection::Iterator it = selection_.begin(); it != selection_.end(); ++it) {
        fs_knl::PedInstance *pAgent = *it;

        pAgent->stopShooting();
    }
}


void GameplayMenu::handleMouseUp(Point2D point, int button)
{
    ipa_chng_.ipa_chng = -1;

    if (button == kMouseRightButton && isPlayerShooting_) {
        stopShootingEvent();
    }

}

bool GameplayMenu::handleUnMappedKey(const fs_eng::FS_Key key) {
    bool change = false;    // indicator whether menu should be redrawn 
    bool consumed = true;

    // Pause/unpause game
    if (key.keyCode == fs_eng::kKeyCode_P) {
        paused_ = !paused_;
        if (paused_) {
            stopShootingEvent();
            g_MusicMgr.pause();
        } else {
            g_MusicMgr.resume();
        }
        return true;
    }

    if (paused_)
        return true;

    bool ctrl = g_System.isKeyModStatePressed(fs_eng::KMD_CTRL);

    // SPACE is pressed when the mission failed or succeeded to return
    // to debrief menu
    if (key.keyCode == fs_eng::kKeyCode_Space) {
        if (mission_->completed() || mission_->failed()) {
            if (mission_->completed()) {
                // Display success animation
                menu_manager_->gotoMenu(fs_game_menus::kMenuIdFliSuccess);
            }
            else if (mission_->failed()) {
                menu_manager_->gotoMenu(fs_game_menus::kMenuIdFliFailedMission);
            }

            return true;
        }
    } else if (key.keyCode == fs_eng::kKeyCode_Escape) {
        if (mission_->isRunning()) {
            // Abort mission
            mission_->endWithStatus(fs_knl::Mission::kMissionStatusAborted);
        }
        // Display only the menu up animation
        menu_manager_->gotoMenu(fs_game_menus::kMenuIdFliMissionAborted);
        return true;
    }

    // Handle agent selection by numeric keys. Key 0 cycles between one agent
    // selection and all 4 agents.
    // Individual keys select the specified agent unless ctrl is pressed -
    // then they add/remove agent from current selection.
    if (key.keyCode == fs_eng::kKeyCode_0) {
        // This code is exactly the same as for clicking on "group-button"
        // as you can see above.
        selectAllAgents();
    }
    else if (key.keyCode == fs_eng::kKeyCode_1) {
        selectAgent(0, ctrl);
    }
    else if (key.keyCode == fs_eng::kKeyCode_2) {
        selectAgent(1, ctrl);
    }
    else if (key.keyCode == fs_eng::kKeyCode_3) {
        selectAgent(2, ctrl);
    }
    else if (key.keyCode == fs_eng::kKeyCode_4) {
        selectAgent(3, ctrl);
    } else if (key.keyCode == fs_eng::kKeyCode_Left) { // Scroll the map to the left
        scroll_x_ = -SCROLL_STEP;
    } else if (key.keyCode == fs_eng::kKeyCode_Right) { // Scroll the map to the right
        scroll_x_ = SCROLL_STEP;
    } else if (key.keyCode == fs_eng::kKeyCode_Up) { // Scroll the map to the top
        scroll_y_ = -SCROLL_STEP;
    } else if (key.keyCode == fs_eng::kKeyCode_Down) { // Scroll the map to the bottom
        scroll_y_ = SCROLL_STEP;
    }

#ifdef _DEBUG
    else if (key.keyCode == fs_eng::kKeyCode_F11) {
        mission_->endWithStatus(fs_knl::Mission::kMissionStatusCompleted);
        return true;
    } else if (key.keyCode == fs_eng::kKeyCode_F12) {
        mission_->endWithStatus(fs_knl::Mission::kMissionStatusFailed);
        return true;
    }
#endif
    else if (key.keyCode >= fs_eng::kKeyCode_F1 && key.keyCode <= fs_eng::kKeyCode_F8) {
        // Those keys are direct access to inventory
        uint8_t weapon_idx = (uint8_t) key.keyCode - (uint8_t) fs_eng::kKeyCode_F5;
        handleWeaponSelection(weapon_idx, ctrl);
        return true;
    } else if ((key.keyCode == fs_eng::kKeyCode_D) && ctrl && mission_->isRunning()) { // selected agents are killed with 'd'
        // save current selection as it will be modified when agents die
        std::vector<fs_knl::PedInstance *> agents_suicide;
        for (SquadSelection::Iterator it = selection_.begin();
                        it != selection_.end(); ++it) {
                agents_suicide.push_back(*it);
        }

        for (size_t i=0; i < agents_suicide.size(); i++) {
            agents_suicide[i]->commitSuicide();
        }
    } else {
        consumed = false;
    }

#ifdef _DEBUG
#if 0
    static int sound_num = 20;
    if (key.unicode == 'i') {
     g_SoundMgr.play((snd::InGameSample)sound_num);
     printf("sn %i\n", sound_num);
     sound_num++;
     if (sound_num == 33)
         sound_num = 20;
    }
#endif

#if 0
    if (key == KEY_i)
        mission_->ped(0)->setTileY(mission_->ped(0)->tileY() - 1);

    if (key == KEY_k)
        mission_->ped(0)->setTileY(mission_->ped(0)->tileY() + 1);

    if (key == KEY_j)
        mission_->ped(0)->setTileX(mission_->ped(0)->tileX() - 1);

    if (key == KEY_l)
        mission_->ped(0)->setTileX(mission_->ped(0)->tileX() + 1);

    if (key == KEY_PAGEUP) {
        if (mission_->ped(0)->tileZ() < mission_->mmax_z_)
            mission_->ped(0)->setTileZ(mission_->ped(0)->tileZ() + 1);
    }

    if (key == KEY_PAGEDOWN) {
        if (mission_->ped(0)->tileZ() > 0)
            mission_->ped(0)->setTileZ(mission_->ped(0)->tileZ() - 1);
    }

    if (key == KEY_o) {
        mission_->ped(0)->setOffZ(mission_->ped(0)->offZ() + 8);
        if (mission_->ped(0)->tileZ() >= mission_->mmax_z_) {
            mission_->ped(0)->setTileZ(mission_->mmax_z_ - 1);
            mission_->ped(0)->setOffZ(127);
        }
    }

    if (key == KEY_p) {
        mission_->ped(0)->setOffZ(mission_->ped(0)->offZ() - 8);
        if (mission_->ped(0)->tileZ() < 0) {
            mission_->ped(0)->setTileZ(0);
            mission_->ped(0)->setOffZ(0);
        }
    }
    if (mission_->ped(0)->offZ() != 0)
        mission_->ped(0)->setVisZ(mission_->ped(0)->tileZ() - 1);
    else
        mission_->ped(0)->setVisZ(mission_->ped(0)->tileZ());

    printf("%i %i %i\n", mission_->ped(0)->tileX(), mission_->ped(0)->tileY(),
        mission_->ped(0)->tileZ());
#endif

#ifdef ANIM_PLUS_FRAME_VIEW
    // used to see animations by number + frame
    if (key.unicode == 'a') {
        qanim--;
        if (qanim < 0)
            qanim = 0;
    }

    if (key.unicode == 's') {
        qanim++;
        if (qanim > 1969)
            qanim = 1969;
    }

    if (key.unicode == 'q') {
        qanim -= 8;
        if (qanim < 0)
            qanim = 0;
    }

    if (key.unicode == 'w') {
        qanim += 8;
        if (qanim > 1969)
            qanim = 1969;
    }

    if (key.unicode == 'x') {
        qframe++;
        if (qframe > 30)
            qframe = 0;
    }

    if (key.unicode == 'z') {
        qframe--;
        if (qframe < 0)
            qframe = 0;
    }

    if (key.unicode == ' ')
        printf("qanim %i qframe %i\n", qanim, qframe);
#endif

#if 0
    // when directional pathfinding will be implemented this will be used
    // for tests
    if (key == KEY_t) {
        mission_->vehicle(0)->setDirection(
                mission_->vehicle(0)->direction() + 1);
    }

    if (key == KEY_y) {
        mission_->vehicle(0)->setDirection(
                mission_->vehicle(0)->direction() - 1);
    }
#endif

#endif //_DEBUG

    return consumed;
}


void GameplayMenu::drawSelectAllButton() {
    // 64x10
    if(isButtonSelectAllPressed_) {
        g_SpriteMgr.drawSprite(1792, {0, 46 + 44});
        g_SpriteMgr.drawSprite(1793, {64, 46 + 44});
    } else {
        g_SpriteMgr.drawSprite(1796, {0, 46 + 44});
        g_SpriteMgr.drawSprite(1797, {64, 46 + 44});
    }
}

void GameplayMenu::drawMissionHint(uint32_t elapsed) {

    elapsed += mission_hint_ticks_;
    int inc = elapsed / 45;
    mission_hint_ticks_ = elapsed % 45;

    g_SpriteMgr.drawSprite(1798, {0, 46 + 44 + 10 + 46 + 44 - 1});
    g_SpriteMgr.drawSprite(1799, {64, 46 + 44 + 10 + 46 + 44 - 1});

    mission_hint_ += inc;

    bool inversed = false;
    bool text_pw = (target_ && target_->nature() == fs_knl::MapObject::kNatureWeapon
        && target_->isDrawable());

    std::string str;

    uint8 txtColor;

    if ((mission_hint_ > 20 && mission_hint_ < 41)
        || (mission_hint_ > 60))
    {
        for (SquadSelection::Iterator it = selection_.begin();
                            it != selection_.end(); ++it) {
            if ((*it)->isMoving()) {
                str = getMessage("HINT_GOING");
            } else {
                str = getMessage("HINT_OBSERVING");
            }
            if ((*it)->wePickupWeapon()) {
                str = getMessage("HINT_PICKUP_WEAPON");
            }
            if ((*it)->drawnAnim() == fs_knl::PedInstance::ad_HitAnim) {
                str = getMessage("HINT_HIT_BY_BULLET");
            }
        }
        txtColor = 14;

        if (mission_hint_ > 79) {
            mission_hint_ = 0;
            return;
        }
    } else {

        inversed = (mission_hint_ % 5) > 2;
        txtColor = inversed ? 0 : 11;

        if (mission_) {
            mission_->objectiveMsg(str);
            if (mission_->failed()) {
                str = getMessage("HINT_MISSION_FAILED");
                text_pw = false;
            }

            if (mission_->completed()) {
                str = getMessage("HINT_MISSION_COMPLETE");
                text_pw = false;
            }

            if (mission_hint_ > 40 && mission_hint_ < 61)
                if (mission_->completed() || mission_->failed()) {
                    str = getMessage("HINT_PRESS_SPACE");
                    text_pw = false;
                }
        }

        if (inversed && !text_pw) {
            g_System.drawFillRect({0, 46 + 44 + 10 + 46 + 44}, 128, 12, menu_manager_->kMenuColorYellow);
        } else {
            if (text_pw) {
                str = ((fs_knl::WeaponInstance *)target_)->name();
                txtColor = inversed ? 14 : 11;
            }
        }
    }

    int width = gameFont()->textWidth(str, false);
    int x = 64 - width / 2;
    fs_eng::FSColor aColor;
    menu_manager_->getColorFromMenuPalette(txtColor, aColor);
    gameFont()->drawText(x, 46 + 44 + 10 + 46 + 44 + 2 - 1, str, aColor);
}

void GameplayMenu::drawPausePanel() {
    std::string str_paused = getMessage("GAME_PAUSED");
    fs_eng::MenuFont *font_used = getMenuFont(fs_eng::FontManager::SIZE_1);
    int txt_width = font_used->textWidth(str_paused);
    int txt_posx = fs_eng::kScreenWidth / 2 - txt_width / 2;
    int txt_height = font_used->textHeight(false);
    int txt_posy = fs_eng::kScreenHeight / 2 - txt_height / 2;

    g_System.drawFillRect({txt_posx - 10, txt_posy - 5},
        txt_width + 20, txt_height + 10, menu_manager_->kMenuColorBlack);
    fs_eng::FSColor aColor;
    menu_manager_->getColorFromMenuPalette(11, aColor);
    gameFont()->drawText(txt_posx, txt_posy, str_paused, aColor);
}

void GameplayMenu::drawWeaponSelectors() {
    fs_knl::PedInstance *p = NULL;

    p = selection_.leader();

    if (p) {
        bool draw_pw = true;
        for (int j = 0; j < 2; j++) {
            for (int i = 0; i < 4; i++) {
                fs_knl::WeaponInstance *wi = NULL;
                int s = 1601;
                // NOTE: weapon selectors can be drawn by drawFrame instead
                // of using current draw(), animations are folowing:
                // 285,286 empty selector :: 287 persuadatron 289
                // 291 pistol 293 :: 295 gauss gun 297 :: 299 shotgun 301
                // 303 uzi 305 :: 307 minigun 309 :: 311 laser gun 313
                // 315 flamer 317 :: 319 long range 321 :: 323 scanner 325
                // 327 medikit 329 :: 331 time bomb 333 :: 343 access card 345
                // 351 energy shield 353

                if (i + j * 4 < p->numWeapons()) {
                    wi = p->weapon(i + j * 4);
                    s = wi->getClass()->selector();
                    if (p->selectedWeapon() && p->selectedWeapon() == wi)
                        s += 40;
                } else if (draw_pw) {
                    if (target_ && target_->nature() == fs_knl::MapObject::kNatureWeapon
                        && (mission_hint_ % 20) < 10
                        && target_->isDrawable())
                    {
                        // player is pointing a weapon on the ground and there's space
                        // in the inventory to display its icon
                        wi = (fs_knl::WeaponInstance *)target_;
                        draw_pw = false;
                        s = wi->getClass()->selector() + 40;
                    }
                }

                g_SpriteMgr.drawSprite(s, {32 * i, 2 + 46 + 44 + 10 + 46 + 44 + 15 + j * 32});

                // draw ammo bars
                if (wi) {
                    int n;

                    if (!wi->usesAmmo())
                        continue;
                    if (wi->ammoCapacity() == 0)
                        n = 25;
                    else
                        n = 25 * wi->ammoRemaining() / wi->ammoCapacity();

                    g_System.drawFillRect({32 * i + 3, 46 + 44 + 10 + 46 + 44 + 15 + j * 32 + 23 + 2},
                        n, 5, menu_manager_->kMenuColorWhiteBlue);
                }
            }
        }
    } else {
        for (int j = 0; j < 2; j++)
            for (int i = 0; i < 4; i++) {
                int s = 1601;

                g_SpriteMgr.drawSprite(s, {32 * i, 2 + 46 + 44 + 10 + 46 + 44 + 15 + j * 32});
            }

    }
}

/*!
 * Select the given agent.
 * \param agentNo Agent id
 * \param addToGroup If true, agent is added to the current selection.
 * If not, the selection is emptied and filled with the new agent.
 */
void GameplayMenu::selectAgent(size_t agentNo, bool addToGroup) {
    // TODO: when agent deselected and he was shooting, he should stop
    if (selection_.selectAgent(agentNo, addToGroup)) {
        updateSelectAll();
        centerMinimapOnLeader();
        highlightLeaderMarker();
        g_SoundMgr.play(fs_eng::SPEECH_SELECTED);
    }
}

/*!
 * Selects all agents.
 * \param invert If true, selects only the agents that were not already
 * selected.
 */
void GameplayMenu::selectAllAgents() {
    bool prv_state = isButtonSelectAllPressed_;
    isButtonSelectAllPressed_ = !isButtonSelectAllPressed_;
    selection_.selectAllAgents(isButtonSelectAllPressed_);
    updateSelectAll();
    if (isButtonSelectAllPressed_ != prv_state) {
        g_SoundMgr.play(fs_eng::SPEECH_SELECTED);
    }
}

/*!
 * Make the current leader marker blinks.
 * All other agents not.
 */
void GameplayMenu::highlightLeaderMarker()
{
    for (size_t i = fs_knl::Squad::kSlot1; i < fs_knl::Squad::kMaxSlot; i++) {
        // draw animation only for leader
        //mission_->sfxObjects(4 + i)->setDrawAllFrames(selection_.getLeaderSlot() == i);
        //mission_->sfxObjects(4 + i)->playMainAnimation();
        if (selection_.getLeaderSlot() == i) {
            mission_->sfxObjects(4 + i)->playMainAnimation();
        } else {
            mission_->sfxObjects(4 + i)->resetAnimation();
        }
    }
}

/**
 * Updating position for visual markers for all agents.
 * \return void
 *
 */
void GameplayMenu::updateMarkersPosition() {
    for (size_t i = 0; i < fs_knl::Squad::kMaxSlot; i++) {
        if (mission_->sfxObjects(i + 4)->isDrawable()) {
            fs_knl::PedInstance *pAgent = mission_->getSquad()->member(i);
            if (pAgent != NULL && pAgent->isAlive()) {
                fs_knl::TilePoint agentPos = pAgent->position();
                agentPos.ox -= 16;
                agentPos.oz += 256;

                mission_->sfxObjects(i + 4)->setPosition(agentPos);
            }
        }
    }
}

/*!
 * This method checks among the squad to see if an agent died and deselects him.
 */
void GameplayMenu::updateSelectionForDeadAgent(fs_knl::PedInstance *pPed) {
    // Deselects dead agent
    selection_.deselectAgent(pPed);
    // hide dead agent's marker
    mission_->sfxObjects(pPed->id() + 4)->setDrawable(false);

    // if selection is empty after agent's death
    // selects the first selectable agent
    if (selection_.size() == 0) {
        for (size_t i = fs_knl::Squad::kSlot1; i < fs_knl::Squad::kMaxSlot; i++) {
            if (selection_.selectAgent(i, false)) {
                // Agent has been selected -> quit
                break;
            }
        }
    }

    // anyway updates markers
    highlightLeaderMarker();
    updateSelectAll();
}

/*!
 * Updates the minimap.
 */
void GameplayMenu::centerMinimapOnLeader() {
    // Centers the minimap on the selection leader
    if (selection_.size() > 0) {
        fs_knl::PedInstance *pAgent = mission_->ped(selection_.getLeaderSlot());
        mm_renderer_.centerOn(pAgent);
    }
}

/*!
 * Update the select all button's state
 */
void GameplayMenu::updateSelectAll() {
    uint8 nbAgentAlive = 0;
    // count the number of remaining agents
    for (size_t indx = 0; indx < fs_knl::Squad::kMaxSlot; indx++) {
        fs_knl::PedInstance *pAgent = mission_->getSquad()->member(indx);
        if (pAgent && pAgent->isAlive()) {
            nbAgentAlive++;
        }
    }

    // if number of agents alive is the same as number of selected agents
    // then button is pressed.
    isButtonSelectAllPressed_ = ((nbAgentAlive == selection_.size()
        && nbAgentAlive != 0) || nbAgentAlive == 1);
}

/*!
 *
 */
void GameplayMenu::handleWeaponSelection(uint8 selectorIndex, bool ctrl) {
    fs_knl::PedInstance *pLeader = selection_.leader();

    if (selectorIndex < pLeader->numWeapons()) {
        fs_knl::WeaponInstance *wi = pLeader->weapon(selectorIndex);
        if (pLeader->selectedWeapon() == wi) {
            // Player clicked on an already selected weapon -> deselect
            selection_.deselectWeaponOfSameCategory(wi->getClass());
        } else {
            selection_.selectWeaponFromLeader(selectorIndex, ctrl);
        }
    }
    g_SoundMgr.play(fs_eng::SPEECH_SELECTED);
}

/**
 * Method to intercept game events.
 */
void GameplayMenu::onPoliceWarningEmittedEvent(fs_knl::PoliceWarningEmittedEvent *pEvt) {
    if (canPlayPoliceWarnSound_) {
        // warn
        g_SoundMgr.play(fs_eng::PUTDOWN_WEAPON);
        canPlayPoliceWarnSound_ = false;
    }
}

void GameplayMenu::onAgentDiedEvent(fs_knl::AgentDiedEvent *pEvt) {
    LOG(Log::k_FLG_GAME, "GameplayMenu", "onAgentDiedEvent", ("AgentDiedEvent received"))
    // checking agents, if all are dead -> mission failed
    if (mission_->getSquad()->isAllDead()) {
        mission_->endWithStatus(fs_knl::Mission::kMissionStatusFailed);
    }

    // Anyway update selection
    updateSelectionForDeadAgent(pEvt->pPed);
}

void GameplayMenu::onShootingWeaponSelectedEvent(fs_knl::ShootingWeaponSelectedEvent *pEvt) {
    if (pEvt->isSelected) {
        LOG(Log::k_FLG_GAME, "GameplayMenu", "onShootingWeaponSelectedEvent", ("shooting weapon was selected"))
        fs_knl::PedInstance *pPedSource = pEvt->pPed;
        mission_->addArmedPed(pPedSource);
        for (size_t i = 0; i < mission_->numPeds(); i++) {
            fs_knl::PedInstance *pPed = mission_->ped(i);
            if (pPed != pPedSource) {
                pPed->behaviour().handleBehaviourEvent(fs_knl::Behaviour::kBehvEvtWeaponOut, pPedSource);
            }
        }
    } else {
        LOG(Log::k_FLG_GAME, "GameplayMenu", "onShootingWeaponSelectedEvent", ("shooting weapon was deselected"))
        fs_knl::PedInstance *pPedSource = pEvt->pPed;
        mission_->removeArmedPed(pPedSource);
        for (size_t i = 0; i < mission_->numPeds(); i++) {
            fs_knl::PedInstance *pPed = mission_->ped(i);
            if (pPed != pPedSource) {
                pPed->behaviour().handleBehaviourEvent(fs_knl::Behaviour::kBehvEvtWeaponCleared, pPedSource);
            }
        }
    }
}

/*!
 * Call when receiving the event that tells the mission has ended.
 * @param pEvt MissionEndedEvent
 */
void GameplayMenu::onMissionEndedEvent(fs_knl::MissionEndedEvent *pEvt) {
    if (pEvt->status == fs_knl::Mission::kMissionStatusCompleted) {
        LOG(Log::k_FLG_GAME, "Mission", "endWithStatus()", ("Mission succeeded"));
        g_SoundMgr.play(fs_eng::SPEECH_MISSION_COMPLETED);
        g_MusicMgr.playSong(fs_eng::MusicManager::kMusicSongMissionCompleted);
    } else if (pEvt->status == fs_knl::Mission::kMissionStatusFailed) {
        LOG(Log::k_FLG_GAME, "Mission", "endWithStatus()", ("Mission failed"));
        g_SoundMgr.play(fs_eng::SPEECH_MISSION_FAILED);
        g_MusicMgr.playSong(fs_eng::MusicManager::kMusicSongMissionFailed);
    }
}

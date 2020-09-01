/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2007  Davor Ocelic <docelic@mail.inet.hr>            *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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
 *  project'sweb  site;  see <http://www.gnu.org/licenses/gpl.html>.    *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#include <stdio.h>
#include <assert.h>
#include "app.h"
#include "gameplaymenu.h"
#include "menus/gamemenuid.h"
#include "gfx/fliplayer.h"
#include "utils/file.h"
#include "model/vehicle.h"
#include "mission.h"
#include "model/shot.h"

#ifdef _WIN32
#include <windows.h>
#endif

// The number of pixel of a scroll
const int SCROLL_STEP = 16;

const int GameplayMenu::kMiniMapScreenX = 0;
const int GameplayMenu::kMiniMapScreenY = 46 + 44 + 10 + 46 + 44 + 15 + 2 * 32 + 2;

//#define ANIM_PLUS_FRAME_VIEW

GameplayMenu::GameplayMenu(MenuManager *m) :
Menu(m, fs_game_menus::kMenuIdGameplay, fs_game_menus::kMenuIdDebrief, "", "mscrenup.dat"),
tick_count_(0), last_animate_tick_(0), last_motion_tick_(0),
last_motion_x_(320), last_motion_y_(240), mission_hint_ticks_(0),
mission_hint_(0), mission_(NULL), selection_(),
target_(NULL),
mm_renderer_(), warningTimer_(20000)
{
    displayOriginPt_.x = 0;
    displayOriginPt_.y = 0;
    scroll_x_ = 0;
    scroll_y_ = 0;
    ipa_chng_.ipa_chng = -1;
    canPlayPoliceWarnSound_ = true;
    g_gameCtrl.addListener(this, GameEvent::kMission);
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

    TilePoint mpt = mission_->get_map()->screenToTilePoint(newOriginX, displayOriginPt_.y);

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

    TilePoint mpt = mission_->get_map()->screenToTilePoint(displayOriginPt_.x, newWorldY);

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
    PedInstance *p_leader = selection_.leader();
    Point2D start;
    mission_->get_map()->tileToScreenPoint(p_leader->tileX(),
        p_leader->tileY(), mission_->mmax_z_ + 1, 0, 0, &start);
    start.x -= (GAME_SCREEN_WIDTH - 129) / 2;
    start.y -= GAME_SCREEN_HEIGHT / 2;

    if (start.x < 0)
        start.x = 0;

    if (start.y < 0)
        start.y = 0;

    // Check if the position is within map borders
    TilePoint mpt = mission_->get_map()->screenToTilePoint(start.x, start.y);

    if (mpt.tx < mission_->minX())
        mpt.tx = mission_->minX();

    if (mpt.ty < mission_->minY())
        mpt.ty = mission_->minY();

    if (mpt.tx > mission_->maxX())
        mpt.tx = mission_->maxX();

    if (mpt.ty > mission_->maxY())
        mpt.ty = mission_->maxY();

    // recalculating new screen coords
    Point2D msp;
    mission_->get_map()->tileToScreenPoint(mpt.tx, mpt.ty,
        mission_->mmax_z_ + 1, 0, 0, &msp);
    displayOriginPt_.x = msp.x;
    displayOriginPt_.y = msp.y;
}

/*!
 * Called before the menu is first shown.
 * Place to do some initialising.
 */
void GameplayMenu::handleShow() {
    mission_ = g_Session.getMission();
    mission_->start();
    // init selection to the first selectable agent
    selection_.setSquad(mission_->getSquad());

    // init menu internal state
    isButtonSelectAllPressed_ = false;
    initWorldCoords();

    // set graphic palette
    menu_manager_->setPaletteForMission(g_Session.getSelectedBlock().mis_id);
    g_Screen.clear(0);

    highlightLeaderMarker();
    updateMarkersPosition();

    // Init renderers
    map_renderer_.init(mission_, &selection_);
    mm_renderer_.init(mission_, mission_->getSquad()->hasScanner());
    centerMinimapOnLeader();
    isPlayerShooting_ = false;

    // Change cursor to game cursor
    g_System.usePointerCursor();
    g_System.showCursor();

    // play game track
    g_App.music().playTrack(msc::TRACK_ASSASSINATE);
    menu_manager_->resetSinceMouseDown();
}

#ifdef ANIM_PLUS_FRAME_VIEW
int qanim = 1959, qframe = 0;
#endif

void GameplayMenu::handleTick(int elapsed)
{
    if (paused_)
        return;
    bool change = false;
    tick_count_ += elapsed;

    if (!mission_->completed() && !mission_->failed()) {
        // Update stats
        mission_->stats()->incrMissionDuration(elapsed);

        // Checks mission objectives
        mission_->checkObjectives();
    }

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
        int diff = tick_count_ - last_animate_tick_;
        last_animate_tick_ = tick_count_;

        for (size_t i = 0; i < mission_->numSfxObjects(); i++) {
            SFXObject *pSfx = mission_->sfxObjects(i);
            change |= pSfx->animate(diff);
            if (pSfx->sfxLifeOver()) {
                mission_->delSfxObject(i);
                i--;
            }
        }

        for (size_t i = 0; i < mission_->numPeds(); i++)
            change |= mission_->ped(i)->animate(diff, mission_);


        for (size_t i = 0; i < mission_->numVehicles(); i++)
            change |= mission_->vehicle(i)->animate(diff);

        for (size_t i = 0; i < mission_->numWeaponsOnGround(); i++)
            change |= mission_->weaponOnGround(i)->animate(diff);

        for (size_t i = 0; i < mission_->numStatics(); i++)
            change |= mission_->statics(i)->animate(diff, mission_);

        for (size_t i = 0; i < mission_->numPrjShots(); i++) {
            change |= mission_->prjShots(i)->animate(diff, mission_);
            if (mission_->prjShots(i)->isLifeOver()) {
                mission_->delPrjShot(i);
                i--;
            }
        }

        updateMarkersPosition();
    }

    updateMinimap(elapsed);

    updateIPALevelMeters(elapsed);

    if (change) {
        needRendering();
        // force target to update
        handleMouseMotion(last_motion_x_, last_motion_y_, 0, KMD_NONE);
    }

    drawMissionHint(elapsed);
}

void GameplayMenu::handleRender(DirtyList &dirtyList)
{
    g_Screen.clear(0);
    map_renderer_.render(displayOriginPt_);
    g_Screen.drawRect(0,0, 129, GAME_SCREEN_HEIGHT);
    agt_sel_renderer_.render(selection_, mission_->getSquad());
    drawSelectAllButton();
    drawMissionHint(0);
    drawWeaponSelectors();
    mm_renderer_.render(kMiniMapScreenX, kMiniMapScreenY);

#ifdef _DEBUG
    // drawing of different sprites
//    g_App.gameSprites().sprite(9 * 40 + 1)->draw(0, 0, 0, false, true);
#if 0
    // 1601 == start of weapons icons for sidebar
    // 1621 == start of selected weapons icons for sidebar
    // 1748 == start of agent selectors for sidebar
    // 1772 == start of selected agent selectors for sidebar
    g_Screen.clear(1);
    int x = 0, y = 0, my = 0;
    for (int i = 1756; i < g_App.gameSprites().spriteCount(); i++) {
        Sprite *s = g_App.gameSprites().sprite(i);

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
    g_App.gameSprites().drawFrame(qanim, qframe, 320, 200);
#endif
#endif

#ifdef TRACK_FPS
    char tmp[100];
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
    sprintf(tmp, "FPS : %.2f FRAMES PER SEC", fps);
    gameFont()->drawText(10, Screen::kScreenHeight - 15, tmp, 14);
#endif
}

void GameplayMenu::handleLeave()
{
    g_App.music().stopPlayback();

    g_System.hideCursor();
    menu_manager_->setDefaultPalette();
    mission_->end();
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

void GameplayMenu::handleMouseMotion(int x, int y, int state, const int modKeys)
{
    last_motion_tick_ = tick_count_;
    last_motion_x_ = x;
    last_motion_y_ = y;
    // locking mouse motion on ipa change until mouseup is recieved
    if (ipa_chng_.ipa_chng != -1 && menu_manager_->isMouseDragged()) {
        PedInstance *p = mission_->ped(ipa_chng_.agent_used);
        if (p->isAlive()) {
            int percent = agt_sel_renderer_.getPercentageAnyX(
                ipa_chng_.agent_used, x);

            // if agent is in selected group we will update all groups IPA
            if (selection_.isAgentSelected(ipa_chng_.agent_used)) {
                for (uint8 i = 0; i < AgentManager::kMaxSlot; ++i) {
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
    } else if (last_motion_x_ > GAME_SCREEN_WIDTH - 5) {
        scroll_x_ = SCROLL_STEP;
    }

    if (last_motion_y_ < 5) {
        scroll_y_ = - SCROLL_STEP;
    } else if (last_motion_y_ > GAME_SCREEN_HEIGHT - 5) {
        scroll_y_ = SCROLL_STEP;
    }

    bool inrange = false;
    target_ = NULL;

    if (x > 128) {
#ifdef _DEBUG
        // During debug our agents are included in possible targets
        for (size_t i = 0; mission_ && i < mission_->numPeds(); ++i) {
#else
        for (size_t i = mission_->getSquad()->size(); mission_ && i < mission_->numPeds(); ++i) {
#endif
            PedInstance *p = mission_->ped(i);
            if (p->isAlive() && p->map() != -1) {
                Point2D scPt;
                mission_->get_map()->tileToScreenPoint(p->position(), &scPt);
                int px = scPt.x - 10;
                int py = scPt.y - (1 + p->tileZ()) * TILE_HEIGHT/3
                    - (p->offZ() * TILE_HEIGHT/3) / 128;

                if (x - 129 + displayOriginPt_.x >= px && y + displayOriginPt_.y >= py &&
                    x - 129 + displayOriginPt_.x < px + 21 && y + displayOriginPt_.y < py + 34)
                {
                    // mouse pointer is on the object, so it's the new target
                    target_ = p;
                    inrange = selection_.isTargetInRange(mission_, target_);
                    break;
                }
            }
        }

        for (size_t i = 0; mission_ && i < mission_->numVehicles(); ++i) {
            Vehicle *v = mission_->vehicle(i);
            // TrainHead cannot be selected to prevent player from putting agents in it
            if (v->isAlive() && v->getType() != Vehicle::kVehicleTypeTrainHead) {
                Point2D scPt;
                mission_->get_map()->tileToScreenPoint(v->position(), &scPt);
                int px = scPt.x - 20;
                int py = scPt.y - 10 - v->tileZ() * TILE_HEIGHT/3;

                if (x - 129 + displayOriginPt_.x >= px && y + displayOriginPt_.y >= py &&
                    x - 129 + displayOriginPt_.x < px + 40 && y + displayOriginPt_.y < py + 32)
                {
                    target_ = v;
                    inrange = selection_.isTargetInRange(mission_, target_);
                    break;
                }
            }
        }

        for (size_t i = 0; mission_ && i < mission_->numWeaponsOnGround(); ++i) {
            WeaponInstance *w = mission_->weaponOnGround(i);

            if (w->map() != -1) {
                Point2D scPt;
                mission_->get_map()->tileToScreenPoint(w->position(), &scPt);
                int px = scPt.x - 10;
                int py = scPt.y + 4 - w->tileZ() * TILE_HEIGHT/3
                    - (w->offZ() * TILE_HEIGHT/3) / 128;

                if (x - 129 + displayOriginPt_.x >= px && y + displayOriginPt_.y >= py &&
                    x - 129 + displayOriginPt_.x < px + 20 && y + displayOriginPt_.y < py + 15)
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
                int py = scPt.y + 4 - s->tileZ() * TILE_HEIGHT/3
                    - (s->offZ() * TILE_HEIGHT/3) / 128;

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
        if (target_->nature() == MapObject::kNaturePed ||
            target_->nature() == MapObject::kNatureVehicle) {
            if (inrange)
                g_System.useTargetRedCursor();
            else
                g_System.useTargetCursor();
        } else if (target_->nature() == MapObject::kNatureWeapon) {
            g_System.usePickupCursor();
        }
    } else if (x > 128) {
            g_System.usePointerCursor();
    } else {
            g_System.usePointerYellowCursor();
    }

    if (x < 129 && isPlayerShooting_) {
        stopShootingEvent();
    }

    if (isPlayerShooting_) {
        // update direction for each shooting player
        WorldPoint aimedAtLocW;
        if (getAimedAt(x, y, &aimedAtLocW)) {
            for (SquadSelection::Iterator it = selection_.begin(); it != selection_.end(); ++it) {
                PedInstance *pAgent = *it;
                if (pAgent->isUsingWeapon()) {
                    // If ped is currently shooting
                    // then update the action with new shooting target
                    pAgent->updateShootingTarget(aimedAtLocW);
                }
            }
        }
    }
}

bool GameplayMenu::handleMouseDown(int x, int y, int button, const int modKeys)
{
    if (paused_)
        return true;

    if (x < 129) {
        bool ctrl = false;  // Is control button pressed
        if (modKeys & KMD_CTRL) {
            ctrl = true;
        }

        // First check if player has clicked on agent selectors
        SelectorEvent selEvt;
        if (agt_sel_renderer_.hasClickedOnAgentSelector(x, y, selEvt)) {
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
        } else if (y >= 42 + 48 && y < 42 + 48 + 10) {
            // User clicked on the select all button
            selectAllAgents();
        }
        else if (y >= 2 + 46 + 44 + 10 + 46 + 44 + 15
                 && y < 2 + 46 + 44 + 10 + 46 + 44 + 15 + 64)
        {
            // user clicked on the weapon selector
            handleClickOnWeaponSelector(x, y, button, modKeys);
        } else if ( y > kMiniMapScreenY && button == kMouseLeftButton) {
            handleClickOnMinimap(x, y);
        }
    } else {
        // User clicked on the map
        handleClickOnMap(x, y, button, modKeys);
    }

    return true;
}

/*!
 * The user has clicked on the weapon selector.
 * \param x Mouse X coord
 * \param y Mouse Y coord
 * \param button Mouse button that was clicked
 * \param modKeys System keys states
 */
void GameplayMenu::handleClickOnWeaponSelector(int x, int y, int button,
    const int modKeys)
{
    uint8 w_num = ((y - (2 + 46 + 44 + 10 + 46 + 44 + 15)) / 32) * 4
            + x / 32;
    PedInstance *pLeader = selection_.leader();
    if (pLeader->isAlive()) {
        bool is_ctrl = (modKeys & KMD_CTRL) != 0;
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
    // redraw weapon selector
    addDirtyRect(0, 207, 128, 64);
}

void GameplayMenu::setIPAForAgent(size_t slot, IPAStim::IPAType ipa_type, int percentage)
{
    PedInstance *ped = mission_->ped(slot);
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
    for (size_t agent = 0; agent < AgentManager::kMaxSlot; ++agent) {
        PedInstance *ped = mission_->getSquad()->member(agent);
        if (ped && ped->isAlive())
            ped->updtIPATime(elapsed);
    }
}

void GameplayMenu::handleClickOnMap(int x, int y, int button, const int modKeys) {
    TilePoint mapPt = mission_->get_map()->screenToTilePoint(displayOriginPt_.x + x - 129,
                    displayOriginPt_.y + y);
#ifdef _DEBUG
    if ((modKeys & KMD_ALT) != 0) {
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

    bool ctrl = (modKeys & KMD_CTRL) != 0;
    if (button == kMouseLeftButton) {
        if (target_) {
            switch (target_->nature()) {
            case MapObject::kNatureWeapon:
                selection_.pickupWeapon(dynamic_cast<WeaponInstance *>(target_), ctrl);
                break;
            case MapObject::kNaturePed:
                selection_.followPed(dynamic_cast<PedInstance *>(target_), ctrl);
                break;
            case MapObject::kNatureVehicle:
                selection_.enterOrLeaveVehicle(dynamic_cast<Vehicle *>(target_), ctrl);
                break;
            default:
                break;
            }
        } else if (mission_->getWalkable(mapPt)) {
            selection_.moveTo(mapPt, ctrl);
        }
    } else if (button == kMouseRightButton) {
        WorldPoint aimedAtLocW;
        if (getAimedAt(x, y, &aimedAtLocW)) {
            isPlayerShooting_ = true;
            selection_.shootAt(aimedAtLocW);
        }
    }
}

/*!
 * User has clicked on the minimap. All selected agent go to destination.
 * Clicking on the minimap does not allow to shoot or to use objects.
 * \param x minimap coordinate
 * \param y minimap coordinate
 */
void GameplayMenu::handleClickOnMinimap(int x, int y) {
    // convert minimap coordinate in map coordinate
    TilePoint pt = mm_renderer_.minimapToMapPoint(x - kMiniMapScreenX, y - kMiniMapScreenY);
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
bool GameplayMenu::getAimedAt(int x, int y, WorldPoint *pLocWToSet) {
    bool locationSet = false;

    if (target_) {
        //  Player has aimed an object
        pLocWToSet->convertFromTilePoint(target_->position());
        // z is set to half the size of the object
        pLocWToSet->z += target_->sizeZ() >> 1;
        locationSet = true;
    } else {
        // Player is shooting on the ground
        TilePoint mapLocT = mission_->get_map()->screenToTilePoint(displayOriginPt_.x + x - 129,
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
        PedInstance *pAgent = *it;

        pAgent->stopShooting();
    }
}


void GameplayMenu::handleMouseUp(int x, int y, int button, const int modKeys)
{
    ipa_chng_.ipa_chng = -1;

    if (button == kMouseRightButton && isPlayerShooting_) {
        stopShootingEvent();
    }

}

bool GameplayMenu::handleUnknownKey(Key key, const int modKeys) {
    bool change = false; /* indicator whether menu should be redrawn */
    bool consumed = true;

    // Pause/unpause game
    if (isLetterP(key.unicode)) {
        if (paused_) {
            paused_ = false;
        } else {
            paused_ = true;
            // TODO: translate all paused texts
            std::string str_paused = getMessage("GAME_PAUSED");
            MenuFont *font_used = getMenuFont(FontManager::SIZE_1);
            int txt_width = font_used->textWidth(str_paused.c_str(), false);
            int txt_posx = Screen::kScreenWidth / 2 - txt_width / 2;
            int txt_height = font_used->textHeight(false);
            int txt_posy = Screen::kScreenHeight / 2 - txt_height / 2;

            g_Screen.drawRect(txt_posx - 10, txt_posy - 5,
                txt_width + 20, txt_height + 10);
            gameFont()->drawText(txt_posx, txt_posy, str_paused.c_str(), 11);
            stopShootingEvent();
        }
        return true;
    }

    if (paused_)
        return true;

    bool ctrl = false;
    if (modKeys & KMD_CTRL) {
        ctrl = true;
    }

    // SPACE is pressed when the mission failed or succeeded to return
    // to menu
    if (key.unicode == K_SPACE) {
        if (mission_->completed() || mission_->failed()) {
            // Do not display default leaving animation because
            // a success/failed animation will be played
            leaveAnim_ = "";
            if (mission_->completed()) {
                // Display success animation
                menu_manager_->gotoMenu(fs_game_menus::kMenuIdFliSuccess);
            }
            else if (mission_->failed()) {

                menu_manager_->gotoMenu(fs_game_menus::kMenuIdFliFailedMission);
            }

            return true;
        }
    } else if (key.keyFunc == KFC_ESCAPE) {
        // Abort mission
        mission_->endWithStatus(Mission::kMissionStatusAborted);
        // Return false so when can still go to parent menu with escape
        return false;
    }

    /* Handle agent selection by numeric keys. Key 0 cycles between one agent
     * selection and all 4 agents.
     * Individual keys select the specified agent unless ctrl is pressed -
     * then they add/remove agent from current selection. */
    if (key.keyVirt == KVT_NUMPAD0) {
        /* This code is exactly the same as for clicking on "group-button"
         * as you can see above. */
        selectAllAgents();
    }
    else if (key.keyVirt == KVT_NUMPAD1) {
        selectAgent(0, ctrl);
    }
    else if (key.keyVirt == KVT_NUMPAD2) {
        selectAgent(1, ctrl);
    }
    else if (key.keyVirt == KVT_NUMPAD3) {
        selectAgent(2, ctrl);
    }
    else if (key.keyVirt == KVT_NUMPAD4) {
        selectAgent(3, ctrl);
    } else if (key.keyFunc == KFC_LEFT) { // Scroll the map to the left
        scroll_x_ = -SCROLL_STEP;
    } else if (key.keyFunc == KFC_RIGHT) { // Scroll the map to the right
        scroll_x_ = SCROLL_STEP;
    } else if (key.keyFunc == KFC_UP) { // Scroll the map to the top
        scroll_y_ = -SCROLL_STEP;
    } else if (key.keyFunc == KFC_DOWN) { // Scroll the map to the bottom
        scroll_y_ = SCROLL_STEP;
    } else if (key.keyFunc == KFC_F1) { // Music Control
        g_App.music().toggleMusic();
    } else if (key.keyFunc == KFC_F2) { // Sound Control
        g_App.gameSounds().toggleSound();
    }

#ifdef _DEBUG
    else if (key.keyFunc == KFC_F3) {
        mission_->endWithStatus(Mission::kMissionStatusCompleted);
        return true;
    } else if (key.keyFunc == KFC_F4) {
        mission_->endWithStatus(Mission::kMissionStatusFailed);
        return true;
    }
#endif
    else if (key.keyFunc >= KFC_F5 && key.keyFunc <= KFC_F12) {
        // Those keys are direct access to inventory
        uint8 weapon_idx = (uint8) key.keyFunc - (uint8) KFC_F5;
        handleWeaponSelection(weapon_idx, ctrl);
        return true;
    } else if ((isLetterD(key.unicode)) && ctrl) { // selected agents are killed with 'd'
        // save current selection as it will be modified when agents die
        std::vector<PedInstance *> agents_suicide;
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
     g_App.gameSounds().play((snd::InGameSample)sound_num);
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

    if (change)
        needRendering();

    return consumed;
}


void GameplayMenu::drawSelectAllButton() {
    // 64x10
    if(isButtonSelectAllPressed_) {
        g_App.gameSprites().sprite(1792)->draw(0, 46 + 44, 0);
        g_App.gameSprites().sprite(1793)->draw(64, 46 + 44, 0);
    } else {
        g_App.gameSprites().sprite(1796)->draw(0, 46 + 44, 0);
        g_App.gameSprites().sprite(1797)->draw(64, 46 + 44, 0);
    }
}

void GameplayMenu::drawMissionHint(int elapsed) {

    elapsed += mission_hint_ticks_;
    int inc = elapsed / 45;
    mission_hint_ticks_ = elapsed % 45;

    g_App.gameSprites().sprite(1798)->draw(
        0, 46 + 44 + 10 + 46 + 44 - 1, 0);
    g_App.gameSprites().sprite(1799)->draw(
        64, 46 + 44 + 10 + 46 + 44 - 1, 0);

    mission_hint_ += inc;

    bool inversed = false;
    bool text_pw = (target_ && target_->nature() == MapObject::kNatureWeapon
        && target_->map() != -1);

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
            if ((*it)->drawnAnim() == PedInstance::ad_HitAnim) {
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
            g_Screen.drawRect(0, 46 + 44 + 10 + 46 + 44, 128, 12, 11);
        } else {
            if (text_pw) {
                str = ((WeaponInstance *)target_)->name();
                txtColor = inversed ? 14 : 11;
            }
        }
    }

    int width = gameFont()->textWidth(str.c_str(), false, false);
    int x = 64 - width / 2;
    gameFont()->drawText(x, 46 + 44 + 10 + 46 + 44 + 2 - 1, str.c_str(), txtColor);
}

void GameplayMenu::drawWeaponSelectors() {
    PedInstance *p = NULL;

    p = selection_.leader();

    if (p) {
        bool draw_pw = true;
        for (int j = 0; j < 2; j++) {
            for (int i = 0; i < 4; i++) {
                WeaponInstance *wi = NULL;
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
                    if (target_ && target_->nature() == MapObject::kNatureWeapon
                        && (mission_hint_ % 20) < 10
                        && target_->map() != -1)
                    {
                        // player is pointing a weapon on the ground and there's space
                        // in the inventory to display its icon
                        wi = (WeaponInstance *)target_;
                        draw_pw = false;
                        s = wi->getClass()->selector() + 40;
                    }
                }

                g_App.gameSprites().sprite(s)->draw(
                        32 * i, 2 + 46 + 44 + 10 + 46 + 44 + 15 + j * 32, 0);

                // draw ammo bars
                if (wi) {
                    int n;

                    if (wi->ammo() == -1)
                        continue;
                    if (wi->ammo() == 0)
                        n = 25;
                    else
                        n = 25 * wi->ammoRemaining() / wi->ammo();

                    g_Screen.drawRect(32 * i + 3, 46 + 44 + 10 + 46 + 44 + 15 + j * 32 + 23 + 2,
                        n, 5, 12);
                }
            }
        }
    } else {
        for (int j = 0; j < 2; j++)
            for (int i = 0; i < 4; i++) {
                int s = 1601;

                g_App.gameSprites().sprite(s)->draw(
                        32 * i, 2 + 46 + 44 + 10 + 46 + 44 + 15 + j * 32, 0);
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
        g_App.gameSounds().play(snd::SPEECH_SELECTED);

        // redraw agent selectors
        addDirtyRect((agentNo % 2) * 65, (agentNo / 2) * 90 , 64, 46);
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
        g_App.gameSounds().play(snd::SPEECH_SELECTED);
        // redraw all agent selectors
        addDirtyRect(0, 0, 128, 180);
    }
}

/*!
 * Make the current leader marker blinks.
 * All other agents not.
 */
void GameplayMenu::highlightLeaderMarker()
{
    for (size_t i = AgentManager::kSlot1; i < AgentManager::kMaxSlot; i++) {
        // draw animation only for leader
        mission_->sfxObjects(4 + i)->setDrawAllFrames(selection_.getLeaderSlot() == i);
    }
}

/**
 * Updating position for visual markers for all agents.
 * \return void
 *
 */
void GameplayMenu::updateMarkersPosition() {
    for (size_t i = 0; i < AgentManager::kMaxSlot; i++) {
        if (mission_->sfxObjects(i + 4)->isDrawable()) {
            PedInstance *pAgent = mission_->getSquad()->member(i);
            if (pAgent != NULL && pAgent->isAlive()) {
                TilePoint agentPos = pAgent->position();
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
void GameplayMenu::updateSelectionForDeadAgent(PedInstance *pPed) {
    // Deselects dead agent
    selection_.deselectAgent(pPed);
    // hide dead agent's marker
    mission_->sfxObjects(pPed->id() + 4)->setDrawable(false);

    // if selection is empty after agent's death
    // selects the first selectable agent
    if (selection_.size() == 0) {
        for (size_t i = AgentManager::kSlot1; i < AgentManager::kMaxSlot; i++) {
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
        PedInstance *pAgent = mission_->ped(selection_.getLeaderSlot());
        mm_renderer_.centerOn(pAgent->tileX(), pAgent->tileY(), pAgent->offX(), pAgent->offY());
    }
}

/*!
 * Updates the minimap.
 */
void GameplayMenu::updateMinimap(int elapsed) {
    centerMinimapOnLeader();
    mm_renderer_.handleTick(elapsed);
}

/*!
 * Update the select all button's state
 */
void GameplayMenu::updateSelectAll() {
    uint8 nbAgentAlive = 0;
    // count the number of remaining agents
    for (size_t indx = 0; indx < AgentManager::kMaxSlot; indx++) {
        PedInstance *pAgent = mission_->getSquad()->member(indx);
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
    PedInstance *pLeader = selection_.leader();

    if (selectorIndex < pLeader->numWeapons()) {
        WeaponInstance *wi = pLeader->weapon(selectorIndex);
        if (pLeader->selectedWeapon() == wi) {
            // Player clicked on an already selected weapon -> deselect
            selection_.deselectWeaponOfSameCategory(wi->getClass());
        } else {
            selection_.selectWeaponFromLeader(selectorIndex, ctrl);
        }
    }
    g_App.gameSounds().play(snd::SPEECH_SELECTED);
}

/**
 * Method to intercept game events.
 */
void GameplayMenu::handleGameEvent(GameEvent evt) {
    if (evt.type == GameEvent::kAgentDied) {
        // checking agents, if all are dead -> mission failed
        if (mission_->getSquad()->isAllDead()) {
            mission_->endWithStatus(Mission::kMissionStatusFailed);
            // clear signal on minimap
            GameEvent sigEvt;
            sigEvt.type = GameEvent::kObjFailed;
            mm_renderer_.handleGameEvent(sigEvt);
        }

        // Anyway update selection
        PedInstance *p_ped = static_cast<PedInstance *> (evt.pCtxt);
        updateSelectionForDeadAgent(p_ped);
    } else if (evt.type == GameEvent::kEvtShootingWeaponSelected) {
        PedInstance *pPedSource = static_cast<PedInstance *> (evt.pCtxt);
        mission_->addArmedPed(pPedSource);
        for (size_t i = 0; i < mission_->numPeds(); i++) {
            PedInstance *pPed = mission_->ped(i);
            if (pPed != pPedSource) {
                pPed->behaviour().handleBehaviourEvent(Behaviour::kBehvEvtWeaponOut, pPedSource);
            }
        }
    } else if (evt.type == GameEvent::kEvtShootingWeaponDeselected) {
        PedInstance *pPedSource = static_cast<PedInstance *> (evt.pCtxt);
        mission_->removeArmedPed(pPedSource);
        for (size_t i = 0; i < mission_->numPeds(); i++) {
            PedInstance *pPed = mission_->ped(i);
            if (pPed != pPedSource) {
                pPed->behaviour().handleBehaviourEvent(Behaviour::kBehvEvtWeaponCleared, pPedSource);
            }
        }
    } else if (evt.type == GameEvent::kEvtWarnAgent) {
        if (canPlayPoliceWarnSound_) {
            // warn
            g_App.gameSounds().play(snd::PUTDOWN_WEAPON);
            canPlayPoliceWarnSound_ = false;
        }
    }
}

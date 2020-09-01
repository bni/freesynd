/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
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

#include <stdio.h>
#include <assert.h>
#include "app.h"
#include "utils/file.h"
#include "briefmenu.h"
#include "menus/gamemenuid.h"
#include "core/missionbriefing.h"

const int BriefMenu::kMiniMapScreenX = 504;
const int BriefMenu::kMiniMapScreenY = 220;
const int BriefMenu::kMiniMapWidth = 120;
const int BriefMenu::kMiniMapHeight = 120;
const int BriefMenu::kMaxLinePerPage = 14;

#if 0
#define EXECUTION_SPEED_TIME
#endif
BriefMenu::BriefMenu(MenuManager * m)
    : Menu(m, fs_game_menus::kMenuIdBrief, fs_game_menus::kMenuIdMap, "mbrief.dat", "mbrieout.dat"),
        start_line_(0), p_briefing_(NULL), mm_renderer_() {
    addStatic(85, 35, 545, "#BRIEF_TITLE", FontManager::SIZE_4, false);
    txtTimeId_ = addStatic(500, 9, "", FontManager::SIZE_2, true);       // Time

    // Briefing scroll button
    nextButId_ = addImageOption(461, 316, Sprite::MSPR_RIGHT_ARROW2_D, Sprite::MSPR_RIGHT_ARROW2_L);
    registerHotKey(KFC_RIGHT, nextButId_);
    prevButId_ = addImageOption(427, 316, Sprite::MSPR_LEFT_ARROW2_D, Sprite::MSPR_LEFT_ARROW2_L);
    registerHotKey(KFC_LEFT, prevButId_);

    // Accept button
    addOption(17, 347, 128, 25, "#MENU_ACC_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdSelect);
    // Map button
    addOption(148, 347, 99, 25, "#BRIEF_MAP", FontManager::SIZE_2, fs_game_menus::kMenuIdMap);
    // Main menu button
    addOption(500, 347,  128, 25, "#MENU_MAIN_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdMain);

    // Money
    txtMoneyId_ = addStatic(500, 87, 127, "0", FontManager::SIZE_2, true);     // textfield
    // Info
    infosButId_ = addOption(500, 118, 127, 10, "#BRIEF_INFO", FontManager::SIZE_2); // info button
    txtInfoId_ = addStatic(500, 140, 127, "0", FontManager::SIZE_2, true);
    // Enhancement
    enhButId_ = addOption(500, 169, 127, 10, "#BRIEF_ENH", FontManager::SIZE_2);
    txtEnhId_ = addStatic(500, 195, 127, "0", FontManager::SIZE_2, true);

    a_page_ = new std::string[kMaxLinePerPage];
}

BriefMenu::~BriefMenu() {
    delete [] a_page_;
    a_page_ = NULL;
}

void BriefMenu::handleTick(int elapsed)
{
    if (g_Session.updateTime(elapsed)) {
        updateClock();
    }

    if (mm_renderer_.handleTick(elapsed)) {
        redrawMiniMap();
    }
}

/*!
 * Update the game time display
 */
void BriefMenu::updateClock() {
    char tmp[100];

    g_Session.getTimeAsStr(tmp);
    getStatic(txtTimeId_)->setText(tmp);

    // update money
    getStatic(txtMoneyId_)->setTextFormated("%d", g_Session.getMoney());
}

/*!
 * Return the zoom level in the minimap corresponding to the enhancement
 * level.
 * \param enh_lvl Level of enhancement already bought by player
 * \return The zoom level in the minimap
 */
MinimapRenderer::EZoom BriefMenu::toZoomLevel(uint8 enh_lvl) {
    if (enh_lvl == 0) {
        return MinimapRenderer::ZOOM_X4;
    } else if (enh_lvl == 1) {
        return MinimapRenderer::ZOOM_X3;
    } else if (enh_lvl == 2) {
        return MinimapRenderer::ZOOM_X2;
    } else {
        return MinimapRenderer::ZOOM_X1;
    }
}

void BriefMenu::handleShow() {

    menu_manager_->saveBackground();

    // grab mission info
    int cur_miss = g_Session.getSelectedBlock().mis_id;

    // Loads mission briefing
    p_briefing_ = g_gameCtrl.missions().loadBriefing(cur_miss);
    assert(p_briefing_ != NULL);

    start_line_ = 0;
    getOption(prevButId_)->setVisible(false);
    getOption(nextButId_)->setVisible(false);
    update_briefing_text();

    // reset minimap renderer with current mission
    uint8 enh_lvl = g_Session.getSelectedBlock().enhanceLevel;
    MinimapRenderer::EZoom zoom =
        toZoomLevel(enh_lvl);

    bool drawEnemies = enh_lvl == p_briefing_->nb_enhts();

    mm_renderer_.init(p_briefing_, zoom, drawEnemies);

    updateClock();

    // Initialize the informations label with current cost
    if (g_Session.getSelectedBlock().infoLevel < p_briefing_->nb_infos()) {
        getStatic(txtInfoId_)->setTextFormated("%d",
            p_briefing_->infoCost(g_Session.getSelectedBlock().infoLevel));
        getOption(infosButId_)->setWidgetEnabled(true);
    } else {
        getStatic(txtInfoId_)->setText("");
        getOption(infosButId_)->setWidgetEnabled(false);
    }

    // Initialize the enhancements label with current cost
    if (enh_lvl < p_briefing_->nb_enhts()) {
        getStatic(txtEnhId_)->setTextFormated("%d",
            p_briefing_->enhanceCost(enh_lvl));
        getOption(enhButId_)->setWidgetEnabled(true);
    } else {
        getStatic(txtEnhId_)->setText("");
        getOption(enhButId_)->setWidgetEnabled(false);
    }

    g_System.showCursor();
}

/*!
 * Helpfull method to order minimap redraw by inserting a dirty rect.
 */
void BriefMenu::redrawMiniMap() {
    addDirtyRect(kMiniMapScreenX, kMiniMapScreenY, kMiniMapWidth, kMiniMapHeight);
}

/*!
 * Helpfull method to order briefing redraw by inserting a dirty rect.
 */
void BriefMenu::redrawBriefing() {
    addDirtyRect(22, 86, 474, 256);
}

/*!
 * Draws the text stored in the a_page field.
 */
void BriefMenu::render_briefing_text() {
    for (int i = 0; i < kMaxLinePerPage; i++) {
        getMenuFont(FontManager::SIZE_2)->
            drawTextCp437(24, 88 + i * 16, a_page_[i].c_str(), true);
    }
}

/*!
 * Reads a word (up to next separator : white space, new line or end of text)
 * and adds the word to the given line if it fits whithin the limit.
 * \return true if the line is complete
 */
bool BriefMenu::read_next_word(std::string & brief, std::string & line)
{
    std::string new_line(line);
    size_t idx = 0;
    char last_char;

    // reads until a separator is met
    do {
        last_char = brief.at(idx++);
        if (last_char != '\n') {
            new_line.push_back(last_char);
        }
    } while ((idx < brief.size()) && (last_char != ' ' && last_char != '\n'));

    if (getMenuFont(FontManager::SIZE_2)->textWidth(new_line.c_str(), true) > 470) {
        // space added to end of word will produce wrong width,
        // we remove it and check again
        if (last_char == ' ') {
            new_line.erase(new_line.size() - 1, 1);
            if (getMenuFont(FontManager::SIZE_2)->textWidth(new_line.c_str(), true) > 470)
                // new line is too big so we can add the line as it is
                return true;
        } else
            // new line is too big so we can add the line as it is
            return true;
    }

    // we didn't reached the limit so add the word to the current line
    line.assign(new_line);
    // remove what has been read from briefing text
    brief.erase(0, idx);

    if (brief.size() == 0 || last_char == '\n') {
        // we reach the end of text or a new line so we can add the line
        return true;
    }

    return false;
}

/*!
 * Called when next or previous button was clicked or when info was bought.
 * Updates the array of lines to be displayed.
 */
void BriefMenu::update_briefing_text()
{
    int line_count = 0;

    // first, clear array
    for (int i = 0; i < kMaxLinePerPage; i++) {
        a_page_[i].erase();
    }

    for (int lvl = 0; lvl <= g_Session.getSelectedBlock().infoLevel; lvl++) {
        std::string brief(p_briefing_->briefing(lvl));
        std::string line;

        while (brief.size() != 0 &&
                line_count <= (start_line_ + kMaxLinePerPage)) {
            bool add = read_next_word(brief, line);

            if (add) {
                // first line should not be empty
                if (line_count % kMaxLinePerPage == 0 && line.size() == 0) {
                    continue;
                }
                if (line_count >= start_line_ && line_count < (start_line_ + kMaxLinePerPage)) {
                    a_page_[line_count - start_line_].assign(line);
                }
                line_count++;
                line.erase();
            }
        }
    }

    // Previous button is visible only if not on the first page
    getOption(prevButId_)->setVisible(start_line_ != 0);
    // Next button is visible only if there are line after the lines currently displayed
    getOption(nextButId_)->setVisible(line_count >= (start_line_ + kMaxLinePerPage));
    redrawBriefing();
}

void BriefMenu::handleRender(DirtyList &dirtyList) {

    g_Screen.drawLogo(18, 14, g_Session.getLogo(), g_Session.getLogoColour());

    // write briefing
#ifdef EXECUTION_SPEED_TIME
    printf("---------------------------");
    printf("start time %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif
    if (dirtyList.intersectsList(22, 86, 460, 220)) {
        render_briefing_text();
    }
#ifdef EXECUTION_SPEED_TIME
    printf("+++++++++++++++++++++++++++");
    printf("end time %i.%i\n", SDL_GetTicks()/1000, SDL_GetTicks()%1000);
#endif
    // NOTE: enhance levels: 0 = 10px(5), 1 = 8px(4), 2 = 6px(3), 3 - 4px(2),
    // 4 - 2px(1); x = 502(251), y = 218(109), 124x124(62x62)
    // enemy peds are at maximum enhance lvl
    // 640x400(320x200), (504, 220) = (252, 110)
    // g_Screen.scale2x(10, 100, pMission->mmax_x_, pMission->mmax_y_,
    //     pMission->minimap_overlay_,0, false);

    // draw minimap
    mm_renderer_.render(kMiniMapScreenX, kMiniMapScreenY);
}

void BriefMenu::handleLeave() {
    delete p_briefing_;
    p_briefing_ = NULL;
    g_System.hideCursor();
}

void BriefMenu::handleAction(const int actionId, void *ctx, const int modKeys) {
    if (actionId == infosButId_) {
        // Buy some informations
        if (g_Session.getSelectedBlock().infoLevel < p_briefing_->nb_infos()) {
            g_Session.decreaseMoney(p_briefing_->infoCost(g_Session.getSelectedBlock().infoLevel));
            g_Session.getSelectedBlock().infoLevel += 1;

            getStatic(txtMoneyId_)->setTextFormated("%d", g_Session.getMoney());
            if (g_Session.getSelectedBlock().infoLevel < p_briefing_->nb_infos()) {
                getStatic(txtInfoId_)->setTextFormated("%d",
                    p_briefing_->infoCost(g_Session.getSelectedBlock().infoLevel));
            } else {
                getOption(infosButId_)->setWidgetEnabled(false);
                getStatic(txtInfoId_)->setText("");
            }
            update_briefing_text();
        }
    }

    if (actionId == enhButId_) {
        // Buy some map enhancement
        if (g_Session.getSelectedBlock().enhanceLevel < p_briefing_->nb_enhts()) {
            g_Session.decreaseMoney(p_briefing_->enhanceCost(g_Session.getSelectedBlock().enhanceLevel));
            g_Session.getSelectedBlock().enhanceLevel += 1;
            // zoom in the minimap
            mm_renderer_.zoomOut();
            if (g_Session.getSelectedBlock().enhanceLevel == p_briefing_->nb_enhts()) {
                mm_renderer_.setDrawEnemies(true);
                getOption(enhButId_)->setWidgetEnabled(false);
            }
            redrawMiniMap();

            getStatic(txtMoneyId_)->setTextFormated("%d", g_Session.getMoney());
            if (g_Session.getSelectedBlock().enhanceLevel < p_briefing_->nb_enhts()) {
                getStatic(txtEnhId_)->setTextFormated("%d",
                    p_briefing_->enhanceCost(g_Session.getSelectedBlock().enhanceLevel));
            } else
                getStatic(txtEnhId_)->setText("");
        }
    }

    if (actionId == nextButId_) {
        // Next page
        start_line_ += kMaxLinePerPage;
        update_briefing_text();
    }

    if (actionId == prevButId_) {
        // Previous page
        start_line_ -= kMaxLinePerPage;
        update_briefing_text();
    }
}

/*!
 * Handles the mouse click.
 * If the user clicks on the minimap with the left button, the minimap scrolls
 * in the corresponding direction.
 * \return True if the user clicked on the minimap so event is consumed
 */
bool BriefMenu::handleMouseDown(int x, int y, int button, const int modKeys) {
    if (button == 1 && x >= kMiniMapScreenX
        && x < (kMiniMapScreenX + kMiniMapWidth)
        && y >= kMiniMapScreenY && y < (kMiniMapScreenY + kMiniMapHeight)) {
        if (x >= kMiniMapScreenX && x < 544) {
            mm_renderer_.scrollLeft();
        } else if (x >= 584 && x < 624) {
             mm_renderer_.scrollRight();
        }

        if (y >= kMiniMapScreenY && y < 260) {
             mm_renderer_.scrollUp();
        } else if (y >= 300 && y < 340) {
             mm_renderer_.scrollDown();
        }
        // Redraw map
        redrawMiniMap();
        return true;
    }

    return false;
}

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "utils/file.h"
#include "menus/mapmenu.h"
#include "menus/gamemenuid.h"
#include "core/gamesession.h"
#include "gfx/screen.h"
#include "system.h"
#include "menus/menumanager.h"

struct pos {
    int x;
    int y;
};

/*!
 * This structure holds the position of the various
 * blocks on the map.
 * The order in the array is the same as the one in
 * the block file.
 */
struct BlockDisplay {
    struct pos pos;
    struct pos logo_pos;
    struct pos line_start;
    struct pos line_end;
} g_BlocksDisplay[50] = {
    { {46, 18}, {14, 12}, {48, 32}, {76, 36}},
    { {90, 16}, {14, 12}, {48, 32}, {107, 40}},
    { {122, 6}, {226, 78}, {226, 78}, {181, 33}},
    { {218, 16}, {226, 78}, {245, 78}, {250, 40}},
    { {320, 22}, {289, 24}, {322, 52}, {344, 60}},
    { {370, 20}, {353, 12}, {387, 45}, {404, 62}},
    { {434, 10}, {353, 12}, {388, 34}, {458, 48}},
    { {482, 30}, {585, 69}, {583, 79}, {527, 53}},
    { {70, 36}, {13, 59}, {49, 73}, {100, 63}},
    { {290, 70}, {243, 93}, {279, 107}, {316, 97}},
    { {334, 54}, {289, 22}, {325, 57}, {360, 88}},
    { {372, 58}, {353, 12}, {381, 47}, {406, 90}},
    { {436, 74}, {353, 12}, {389, 45}, {452, 102}},
    { {434, 62}, {585, 69}, {584, 85}, {487, 81}},
    { {532, 76}, {585, 69}, {584, 89}, {541, 93}},
    { {160, 52}, {225, 77}, {224, 87}, {177, 69}},
    { {56, 76}, {13, 59}, {49, 83}, {74, 91}},
    { {76, 76}, {13, 59}, {49, 83}, {96, 93}},
    { {110, 76}, {149, 123}, {149, 123}, {117, 91}},
    { {140, 76}, {149, 123}, {163, 122}, {157, 91}},
    { {278, 124}, {243, 93}, {279, 123}, {300, 135}},
    { {288, 124}, {243, 93}, {279, 123}, {326, 153}},
    { {344, 116}, {421, 185}, {423, 183}, {385, 137}},
    { {398, 106}, {421, 185}, {435, 184}, {427, 129}},
    { {474, 98}, {565, 133}, {564, 143}, {511, 119}},
    { {56, 98}, {13, 127}, {49, 135}, {86, 113}},
    { {100, 106}, {149, 123}, {148, 133}, {107, 117}},
    { {198, 130}, {207, 165}, {217, 164}, {207, 137}},
    { {266, 140}, {207, 165}, {243, 177}, {284, 163}},
    { {336, 156}, {243, 209}, {279, 215}, {358, 167}},
    { {378, 146}, {421, 185}, {421, 184}, {401, 159}},
    { {440, 118}, {421, 185}, {449, 184}, {468, 143}},
    { {492, 136}, {565, 133}, {564, 149}, {509, 147}},
    { {66, 122}, {13, 127}, {49, 141}, {80, 135}},
    { {120, 172}, {61, 179}, {97, 195}, {138, 189}},
    { {280, 170}, {243, 209}, {279, 211}, {312, 181}},
    { {326, 166}, {285, 251}, {317, 250}, {352, 205}},
    { {370, 172}, {421, 185}, {420, 203}, {385, 203}},
    { {118, 206}, {89, 257}, {115, 256}, {131, 217}},
    { {142, 184}, {207, 165}, {206, 191}, {159, 211}},
    { {180, 196}, {243, 209}, {242, 225}, {195, 223}},
    { {328, 226}, {285, 251}, {321, 259}, {342, 249}},
    { {354, 212}, {412, 249}, {412, 259}, {363, 243}},
    { {502, 250}, {459, 253}, {495, 278}, {522, 285}},
    { {542, 246}, {597, 259}, {596, 275}, {569, 271}},
    { {542, 282}, {597, 259}, {596, 283}, {567, 293}},
    { {152, 230}, {215, 265}, {214, 269}, {173, 243}},
    { {148, 242}, {215, 265}, {214, 283}, {159, 281}},
    { {170, 252}, {215, 265}, {214, 277}, {179, 269}},
    { {504, 160}, {471, 197}, {507, 211}, {536, 207}}
};


/*!
 * Class constructor.
 * \param m The menu manager.
 */
MapMenu::MapMenu(MenuManager * m)
    :  Menu(m, fs_game_menus::kMenuIdMap, fs_game_menus::kMenuIdMain, "mmap.dat", "mmapout.dat"),
mapblk_data_(NULL), select_tick_count_(0) {
    //
    briefButId_ = addOption(17, 347, 128, 25, "#MAP_BRIEF_BUT",
        FontManager::SIZE_2, fs_game_menus::kMenuIdBrief);
    addOption(500, 347,  128, 25, "#MENU_MAIN_BUT",
        FontManager::SIZE_2, fs_game_menus::kMenuIdMain);

    // Country name
    txtCountryId_ = addStatic(188, 312, 270, "", FontManager::SIZE_1,
        true);
    // Pop label
    addStatic(194, 332, "#MAP_POP", FontManager::SIZE_2, true);
    // Pop value
    txtPopId_ = addStatic(268, 332, "", FontManager::SIZE_1, true);
    // Tax label
    addStatic(194, 346, "#MAP_TAX", FontManager::SIZE_2, true);
    // Tax value
    txtTaxValueId_ = addStatic(268, 346, "", FontManager::SIZE_1, true);
    // Own label
    txtOwnLblId_ = addStatic(194, 360, "#MAP_OWN", FontManager::SIZE_2, true);
    // Own status
    txtOwnId_ = addStatic(268, 360, "", FontManager::SIZE_1, true);

    txtTimeId_ = addStatic(500, 9, "", FontManager::SIZE_2, true);  // Time

    // Tax cursors
    txtTaxPctId_ = addStatic(350, 346, "@   30%", FontManager::SIZE_1, true);
    decrTaxButId_ = addImageOption(375, 346, Sprite::MSPR_TAX_DECR,
        Sprite::MSPR_TAX_DECR, false);
    registerHotKey(K_MINUS, decrTaxButId_);
    incrTaxButId_ = addImageOption(435, 346, Sprite::MSPR_TAX_INCR,
        Sprite::MSPR_TAX_INCR, false);
    registerHotKey(K_PLUS, incrTaxButId_);

    // 64 x 44 x 50
    // Load map block informations
    mapblk_data_ = File::loadOriginalFile("mmapblk.dat", mapblk_size_);

    blk_tick_count_ = 0;
    blink_status_ = true;
}

MapMenu::~MapMenu() {
    delete[] mapblk_data_;
    mapblk_data_ = NULL;
}

/*!
 * Update map informations depending on
 * the currently selected mission.
 */
void MapMenu::handleBlockSelected() {
    Block blk = g_Session.getBlock(g_Session.getSelectedBlockId());

    if (blk.status == BLK_FINISHED) {  // A mission is finished
        // Brief is available only if replay mission cheat is set
        if (g_Session.canReplayMission()) {
            getOption(briefButId_)->setVisible(true);
        } else {
            getOption(briefButId_)->setVisible(false);
        }
    } else if (blk.status == BLK_UNAVAIL) {  // A mission is unavailable
        getOption(briefButId_)->setVisible(false);
    } else {
        // Brief is available because mission is either available
        // or on rebellion
        getOption(briefButId_)->setVisible(true);
    }

    // Update the country informations
    getStatic(txtCountryId_)->setText(blk.name);

    char tmp[100];

    // Population
    sprintf(tmp, "%i", blk.population);
    getStatic(txtPopId_)->setText(tmp);

    // Mission is finished
    if (blk.status == BLK_FINISHED) {
        // Status
        getStatic(txtOwnLblId_)->setText("#MAP_STAT");
        switch (blk.popStatus) {
            case STAT_VERY_HAPPY:
                getStatic(txtOwnId_)->setText("#MAP_STAT_VHAPPY");
                break;
            case STAT_HAPPY:
                getStatic(txtOwnId_)->setText("#MAP_STAT_HAPPY");
                break;
            case STAT_CONTENT:
                getStatic(txtOwnId_)->setText("#MAP_STAT_CTNT");
                break;
            case STAT_UNHAPPY:
                getStatic(txtOwnId_)->setText("#MAP_STAT_UNHAPPY");
                break;
            case STAT_DISCONTENT:
                getStatic(txtOwnId_)->setText("#MAP_STAT_DISCTNT");
                break;
            default:
                // should never happend
                getStatic(txtOwnId_)->setText("");
        }

        // Tax
        int tax = blk.tax + blk.addToTax;
        sprintf(tmp, "%i", g_Session.getTaxRevenue(blk.population, tax));
        getStatic(txtTaxValueId_)->setText(tmp);

        getOption(decrTaxButId_)->setVisible(true);
        getOption(incrTaxButId_)->setVisible(true);
        sprintf(tmp, "@   %d%%", tax);
        getStatic(txtTaxPctId_)->setText(tmp);

    } else {
        int tax = blk.tax + blk.addToTax;
        // Status
        getStatic(txtOwnLblId_)->setText("#MAP_OWN");
        getStatic(txtOwnId_)->setText("");
        // Tax
        if (blk.status == BLK_REBEL) {
            getStatic(txtTaxValueId_)->setText("#MAP_STAT_REBEL");
            sprintf(tmp, "@   %d%%", tax);
            getStatic(txtTaxPctId_)->setText(tmp);
        } else {
            getStatic(txtTaxValueId_)->setText("#MAP_TAX_UNKWN");
            getStatic(txtTaxPctId_)->setText("");
        }

        getOption(decrTaxButId_)->setVisible(false);
        getOption(incrTaxButId_)->setVisible(false);
    }

    /*addDirtyRect(192, 310, 260, 70);*/
}

void MapMenu::handleTick(int elapsed) {
    // This a count to refresh the blinking line of the selector
    select_tick_count_ += elapsed;
    if (select_tick_count_ > 200) {
        static int count = 0;
        select_tick_count_ = count++;
        needRendering();
    }

    blk_tick_count_ += elapsed;
    if (blk_tick_count_ > 500) {
        blk_tick_count_ = 0;
        blink_status_ = !blink_status_;
        needRendering();
    }

    if (g_Session.updateTime(elapsed)) {
        handleBlockSelected();
        updateClock();
    }
}

/*! 
 * Update the game time display
 */
void MapMenu::updateClock() {
    char tmp[100];
    g_Session.getTimeAsStr(tmp);
    getStatic(txtTimeId_)->setText(tmp);
}

/*!
 * Utility method to draw the mission selector on the map
 * depending on the current selection.<br/>
 * The selector consists of the player logo inside a box and
 * a line that links the logo to the selected region.
 */
void MapMenu::drawSelector() {
    uint8 selId = g_Session.getSelectedBlockId();
    int logo_x = g_BlocksDisplay[selId].logo_pos.x;
    int logo_y = g_BlocksDisplay[selId].logo_pos.y;
    g_Screen.drawLogo(logo_x, logo_y, g_Session.getLogo(),
        g_Session.getLogoColour(), true);

    // Draw box enclosing logo
    uint8 box[18 * 18];
    memset(box, 255, 18 * 18);
    for (int i = 0; i < 18; i++) {
        box[i] = 252;
        box[i + 17 * 18] = 252;
    }
    for (int j = 0; j < 18; j++) {
        box[j * 18] = 252;
        box[j * 18 + 17] = 252;
    }
    g_Screen.scale2x(logo_x - 2, logo_y - 2, 18, 18, box);

    // Draw line between country and logobox
    int blk_line_end_x = g_BlocksDisplay[selId].line_end.x;
    int blk_line_end_y = g_BlocksDisplay[selId].line_end.y;
    int blk_line_start_x = g_BlocksDisplay[selId].line_start.x;
    int blk_line_start_y = g_BlocksDisplay[selId].line_start.y;
    g_Screen.drawLine(blk_line_start_x, blk_line_start_y, blk_line_end_x,
                      blk_line_end_y, 252, 5, select_tick_count_ % 10);
    g_Screen.drawLine(blk_line_start_x, blk_line_start_y - 1,
                      blk_line_end_x, blk_line_end_y - 1, 252, 5,
                      select_tick_count_ % 10);
    g_Screen.drawLine(blk_line_start_x, blk_line_start_y, blk_line_end_x,
                      blk_line_end_y, 4, 5, select_tick_count_ % 10 + 5);
    g_Screen.drawLine(blk_line_start_x, blk_line_start_y - 1,
                      blk_line_end_x, blk_line_end_y - 1, 4, 5,
                      select_tick_count_ % 10 + 5);

    // Reset the counter
    select_tick_count_ = 0;
}

void MapMenu::handleShow() {
    // save a background without country colour
    menu_manager_->saveBackground();

    // Show the mouse
    g_System.showCursor();

    // State of the briefing button
    handleBlockSelected();

    // Update the time
    updateClock();
}

void MapMenu::handleRender(DirtyList &dirtyList) {
    // Draws all countries
    for (int i = 0; i < GameSession::NB_MISSION; i++) {
        Block blk = g_Session.getBlock(i);
        if ((i == g_Session.getSelectedBlockId()) ||
            (blk.status == BLK_AVAIL && blink_status_) ||
            blk.status != BLK_AVAIL) {
            uint8 data[64 * 44];
            memcpy(data, mapblk_data_ + i * 64 * 44, 64 * 44);
            for (int j = 0; j < 64 * 44; j++)
                if (data[j] == 0)
                    data[j] = 255;
                else
                    data[j] = g_Session.get_owner_color(blk);
            g_Screen.scale2x(g_BlocksDisplay[i].pos.x,
                g_BlocksDisplay[i].pos.y, 64, 44, data, 64);
        }
    }

    // Draws the selector
    drawSelector();
}

void MapMenu::handleLeave() {
    g_System.hideCursor();
}

bool MapMenu::handleMouseDown(int x, int y, int button, const int modKeys) {
    // Checks among the missions which one has been clicked on
    for (int i = 0; i < 50; i++) {
        if (x > g_BlocksDisplay[i].pos.x && x < g_BlocksDisplay[i].pos.x + 64 &&
            y > g_BlocksDisplay[i].pos.y && y < g_BlocksDisplay[i].pos.y + 44) {
            if (mapblk_data_
                [i * 64 * 44 + (y - g_BlocksDisplay[i].pos.y) / 2 * 64 +
                 (x - g_BlocksDisplay[i].pos.x) / 2] != 0) {
                     // Do something only if the selected block is new
                     // ie the user did not click on the same mission
                     if (g_Session.getSelectedBlockId() != i) {
                        g_Session.setSelectedBlockId(i);

                        handleBlockSelected();
                        needRendering();
                     }
                return true;
            }
        }
    }

    return false;
}

void MapMenu::handleAction(const int actionId, void *ctx, const int modKeys) {
    bool refresh = false;
    if ( actionId == incrTaxButId_ ) {
        if (modKeys & KMD_CTRL) {
            refresh = g_Session.addToTaxRate(10);
        } else {
            refresh = g_Session.addToTaxRate(1);
        }
    } else if ( actionId == decrTaxButId_ ) {
        if (modKeys & KMD_CTRL) {
            refresh = g_Session.addToTaxRate(-10);
        } else {
            refresh = g_Session.addToTaxRate(-1);
        }
    }

    if (refresh) {
        handleBlockSelected();
        needRendering();
    }
}

bool MapMenu::handleUnknownKey(Key key, const int modKeys) {
    bool consumed = false;
    if (key.keyFunc == KFC_LEFT) {
        // navigate among available missions by decreasing index
        int start = g_Session.getSelectedBlockId();
        for (int i = 1; i < GameSession::NB_MISSION; i++) {
            int index = start - i;
            if (index < 0) {
                index = GameSession::NB_MISSION + index;
            }
            Block blk = g_Session.getBlock(index);
            if (blk.status == BLK_AVAIL) {
                g_Session.setSelectedBlockId(index);
                consumed = true;
                break;
            }
        }
    } else if (key.keyFunc == KFC_RIGHT) {
        // navigate among available missions by increasing index
        int start = g_Session.getSelectedBlockId();
        for (int i = 1; i < GameSession::NB_MISSION; i++) {
            int index = (start + i) % GameSession::NB_MISSION;
            Block blk = g_Session.getBlock(index);
            if (blk.status == BLK_AVAIL) {
                g_Session.setSelectedBlockId(index);
                consumed = true;
                break;
            }
        }
    } else if (key.keyFunc == KFC_PAGEUP) {
        // Pressing PageUp increase tax of 10 percents
        Block blk = g_Session.getBlock(g_Session.getSelectedBlockId());
        if (blk.status == BLK_FINISHED) {
            consumed = g_Session.addToTaxRate(10);
        }
    } else if ( key.keyFunc == KFC_PAGEDOWN ) {
        // Pressing PageDown decrease tax of 10 percents
        Block blk = g_Session.getBlock(g_Session.getSelectedBlockId());
        if (blk.status == BLK_FINISHED) {
            consumed = g_Session.addToTaxRate(-10);
        }
    }

    if (consumed) {
        needRendering();
    }

    handleBlockSelected();

    return consumed;
}

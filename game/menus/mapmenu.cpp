/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "menus/mapmenu.h"

#include "fs-utils/io/file.h"
#include "fs-utils/log/log.h"
#include "menus/gamemenuid.h"
#include "core/gamesession.h"
#include "fs-engine/system/system.h"
#include "fs-engine/menus/menumanager.h"

using fs_eng::FontManager;

/*!
 * This structure holds the position of the various
 * blocks on the map.
 * The order in the array is the same as the one in
 * the block file.
 */
struct BlockDisplay {
    //! Position of the block on the map
    Point2D pos;
    //! Position of the logo when the block is selected
    Point2D logo_pos;
    //! Start position of the line that links the logo to the selected block
    Point2D line_start;
    //! End position of the line that links the logo to the selected block
    Point2D line_end;
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

const int MapMenu::kSegmentSize = 5;
const int MapMenu::kIntervalSize = 5;
const int MapMenu::kCountrySpriteWidth = 64;
const int MapMenu::kCountrySpriteHeight = 44;
const int MapMenu::kCountryTextureSize = 512;
const int MapMenu::kCountrySpritePerRow = 8;

/*!
 * Class constructor.
 * \param m The menu manager.
 */
MapMenu::MapMenu(fs_eng::MenuManager * m)
        : fs_eng::Menu(m, fs_game_menus::kMenuIdMap, fs_game_menus::kMenuIdMain, true),
          mapblk_data_(NULL), timerBlinkLine_(200), timerBlinkCountry_(500, true) {
    cursorOnShow_ = kMenuCursor;
    offsetLine_ = 0;
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
    decrTaxButId_ = addImageOption(375, 346, fs_eng::Sprite::MSPR_TAX_DECR,
        fs_eng::Sprite::MSPR_TAX_DECR, false);
    registerHotKey(fs_eng::kKeyCode_Down, decrTaxButId_);
    incrTaxButId_ = addImageOption(435, 346, fs_eng::Sprite::MSPR_TAX_INCR,
        fs_eng::Sprite::MSPR_TAX_INCR, false);
    registerHotKey(fs_eng::kKeyCode_Up, incrTaxButId_);
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
}

bool MapMenu::handleTick(uint32_t elapsed) {
    // This a count to refresh the blinking line of the selector
    if (timerBlinkLine_.update(elapsed)) {
        offsetLine_ = (offsetLine_ + 1) % kIntervalSize;
    }

   // This counter is used for blinking available countries
   timerBlinkCountry_.update(elapsed);

    if (g_Session.updateTime(elapsed)) {
        handleBlockSelected();
        updateClock();
    }

    return true;
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
    BlockDisplay block = g_BlocksDisplay[g_Session.getSelectedBlockId()];
    g_LogoMgr.draw(block.logo_pos, g_Session.getLogo(),
        g_Session.getLogoColour(), false);

    // Draw box enclosing logo
    g_System.drawRect(block.logo_pos.add(-2, -2), 36, 36, menu_manager_->kMenuColorDarkGreen);

    // Draw line between country and logobox
    // Draw 2 lines for thickness    
    g_System.drawLine(block.line_start, block.line_end, menu_manager_->kMenuColorDarkGreen);
    g_System.drawLine(block.line_start.add(0, -1), block.line_end.add(0, -1), menu_manager_->kMenuColorDarkGreen);
    drawDottedline(block.line_start, block.line_end);
}

/*!
 * Draw spaced lines using an adaptation of Bresenham algorithm.
 * @param start Start of the whole line
 * @param end  End of the whole line
 */
void MapMenu::drawDottedline(Point2D start, Point2D end) {
    std::vector<Point2D> points;
    Point2D currentPoint = start;
    int dx = abs(end.x - start.x);
    int dy = abs(end.y - start.y);
    int sx = start.x < end.x ? 1 : -1;
    int sy = start.y < end.y ? 1 : -1;
    int err = dx - dy;
    int distanceDone = -offsetLine_;
    bool inSeg = false;

    while(currentPoint.x != end.x || currentPoint.y != end.y) {
        if (distanceDone >= 0 && distanceDone < kSegmentSize) {
            if (!inSeg) {
                // we start a new segment so add start point
                points.push_back(currentPoint);
                inSeg = true;
            }
        }
        distanceDone += 1;

        if (distanceDone >= kSegmentSize + kIntervalSize) {
            distanceDone = 0;
        } else if (distanceDone >= kSegmentSize && inSeg) {
            // end of the segment
            points.push_back(currentPoint);
            inSeg = false;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            currentPoint.x += sx;
        }
        if (e2 < dx) {
            err += dx;
            currentPoint.y += sy;
        }
    }

    for (size_t i=0; i+1 < points.size(); i += 2) {
        g_System.drawLine(points[i], points[i+1], menu_manager_->kMenuColorLightGreen);
        g_System.drawLine(points[i].add(0,-1), points[i+1].add(0,-1), menu_manager_->kMenuColorLightGreen);
    }
}

bool MapMenu::handleBeforeShow() {
    if (!countriesTexture_) {
        initCountriesTexture();
    }
    // State of the briefing button
    handleBlockSelected();

    // Update the time
    updateClock();

    buildMapOfCountriesPerColor();

    return true;
}

void MapMenu::buildMapOfCountriesPerColor() {
    countriesPerColor_.clear();

    for (int i = 0; i < GameSession::NB_MISSION; i++) {
        Block blk = g_Session.getBlock(i);
        fs_eng::FSColor color;
        getMenuManager()->getColorFromMenuPalette(g_Session.get_owner_color(blk), color);

        if (countriesPerColor_.contains(color)) {
            countriesPerColor_[color].push_back(i);
        } else {
            std::list<int> newList;
            newList.push_back(i);
            countriesPerColor_[color] = newList;
        }
    }
}

void MapMenu::handleRender() {
    // Draws all countries
    for (auto const& [color, countries] : countriesPerColor_) {
        // We are using color modulation to draw each country with the right color
        countriesTexture_->setColorModulation(color);
        // Countries are grouped by color
        for(int countryId : countries ) {
            Block block = g_Session.getBlock(countryId);
            // Available countries blink to show they are available
            // but the selected country does not blink
            if ((countryId == g_Session.getSelectedBlockId()) ||
                (block.status == BLK_AVAIL && timerBlinkCountry_.state()) ||
                block.status != BLK_AVAIL) {
                countriesTexture_->renderStretch(*countrySpritePositions_[countryId], 
                                                g_BlocksDisplay[countryId].pos,
                                                kCountrySpriteWidth,
                                                kCountrySpriteHeight,
                                                2);
            }
        }
    }

    // Draws the selector
    drawSelector();
}


bool MapMenu::handleMouseDown(Point2D point, int button) {
    // Checks among the missions which one has been clicked on
    for (int i = 0; i < 50; i++) {
        if (point.x > g_BlocksDisplay[i].pos.x && point.x < g_BlocksDisplay[i].pos.x + 64 &&
            point.y > g_BlocksDisplay[i].pos.y && point.y < g_BlocksDisplay[i].pos.y + 44) {
            if (mapblk_data_
                [i * 64 * 44 + (point.y - g_BlocksDisplay[i].pos.y) / 2 * 64 +
                 (point.x - g_BlocksDisplay[i].pos.x) / 2] != 0) {
                     // Do something only if the selected block is new
                     // ie the user did not click on the same mission
                     if (g_Session.getSelectedBlockId() != i) {
                        g_Session.setSelectedBlockId(i);

                        handleBlockSelected();
                     }
                return true;
            }
        }
    }

    return false;
}

void MapMenu::handleAction(const ActionDesc &action) {
    bool refresh = false;
    if ( action.id == incrTaxButId_ ) {
        if (g_System.isKeyModStatePressed(fs_eng::KMD_CTRL)) {
            refresh = g_Session.addToTaxRate(10);
        } else {
            refresh = g_Session.addToTaxRate(1);
        }
    } else if ( action.id == decrTaxButId_ ) {
        if (g_System.isKeyModStatePressed(fs_eng::KMD_CTRL)) {
            refresh = g_Session.addToTaxRate(-10);
        } else {
            refresh = g_Session.addToTaxRate(-1);
        }
    }

    if (refresh) {
        handleBlockSelected();
    }
}

bool MapMenu::handleUnMappedKey(const fs_eng::FS_Key key) {
    bool consumed = false;
    if (key.keyCode == fs_eng::kKeyCode_Left) {
        // navigate among available missions by decreasing index
        int start = g_Session.getSelectedBlockId();
        for (int i = 1; i < GameSession::NB_MISSION; i++) {
            int index = start - i;
            if (index < 0) {
                index = GameSession::NB_MISSION + index;
            }
            Block blk = g_Session.getBlock(index);
            if (blk.status != BLK_UNAVAIL) {
                g_Session.setSelectedBlockId(index);
                consumed = true;
                break;
            }
        }
    } else if (key.keyCode == fs_eng::kKeyCode_Right) {
        // navigate among available missions by increasing index
        int start = g_Session.getSelectedBlockId();
        for (int i = 1; i < GameSession::NB_MISSION; i++) {
            int index = (start + i) % GameSession::NB_MISSION;
            Block blk = g_Session.getBlock(index);
            if (blk.status != BLK_UNAVAIL) {
                g_Session.setSelectedBlockId(index);
                consumed = true;
                break;
            }
        }
    } else if (key.keyCode == fs_eng::kKeyCode_PageUp) {
        // Pressing PageUp increase tax of 10 percents
        Block blk = g_Session.getBlock(g_Session.getSelectedBlockId());
        if (blk.status == BLK_FINISHED) {
            consumed = g_Session.addToTaxRate(10);
        }
    } else if ( key.keyCode == fs_eng::kKeyCode_PageDown ) {
        // Pressing PageDown decrease tax of 10 percents
        Block blk = g_Session.getBlock(g_Session.getSelectedBlockId());
        if (blk.status == BLK_FINISHED) {
            consumed = g_Session.addToTaxRate(-10);
        }
    }

    handleBlockSelected();

    return consumed;
}

/*!
 * Reads the file containing map data and build a spriteset with all countries.
 * We then modify the palette of the texture to use the color white as it is
 * not in the menu palette by default so it's easier for color modulation
 */
void MapMenu::initCountriesTexture() {
    LOG(Log::k_FLG_GFX, "MapMenu", "initCountriesTexture", ("Initiate countries texture\n"))
    
    // 64 x 44 x 50
    // Load map block informations
    size_t mapblk_size;
    mapblk_data_ = fs_utl::File::loadOriginalFile("mmapblk.dat", mapblk_size);
    if (mapblk_data_ == nullptr) {
        FSERR(Log::k_FLG_GFX, "MapMenu", "initCountriesTexture", ("Could not read file: mmapblk.dat"))
        return;
    }

    uint8_t *countriesBuffer = new uint8_t[kCountryTextureSize * kCountryTextureSize];
    countrySpritePositions_ = new Point2D*[GameSession::NB_MISSION];

    copyCountriesPixelsToBuffer(mapblk_data_, countriesBuffer);

    countriesTexture_ = g_System.createTexture();
    // In the mmapblk.dat, each visible pixel that composes a country is set to index 1 
    // All remaining pixels are set to index 0. So we set the color key to be the one at index 0
    bool res = countriesTexture_->create8bitsSurfaceFromData(countriesBuffer, 
                                            kCountryTextureSize, 
                                            kCountryTextureSize, 0);

    if (!res) {
        FSERR(Log::k_FLG_GFX, "MapMenu", "initCountriesTexture", ("Could not create texture for countries"))
    }

    if (res) {
        countriesTexture_->setPalette(getMenuManager()->getMenuPalette());
        // We also change the color at index 1 to be white for easier color modulation
        fs_eng::FSColor white {0xFF, 0xFF, 0xFF, 0xFF};
        countriesTexture_->setColorInPalette(1, white);
        countriesTexture_->loadTextureFromSurface();
    }
    delete [] countriesBuffer;
}

/*!
 * @brief 
 * @param mapblkData 
 * @param countriesBuffer 
 */
void MapMenu::copyCountriesPixelsToBuffer(const uint8_t *mapblkData, uint8_t *countriesBuffer) {
    for (int i=0; i < GameSession::NB_MISSION; i++) {
        // Coords in the destination surface
        int row = i / kCountrySpritePerRow;
        int col = i - (row * kCountrySpritePerRow);

        // start of the country pixels in the destination texture (upper left corner of logo)
        int logoOffsetDest = (col * kCountrySpriteWidth) + (row * kCountrySpritePerRow * kCountrySpriteWidth * kCountrySpriteHeight);
        int logoOffsetSrc = i * kCountrySpriteWidth * kCountrySpriteHeight;
        
        // Copy pixels line by line
        for (int j=0; j < kCountrySpriteHeight; j++) {
            int lineOffsetDest = j * kCountrySpriteWidth * kCountrySpritePerRow;
            int lineOffsetSrc = j * kCountrySpriteWidth;
            memcpy(countriesBuffer + logoOffsetDest + lineOffsetDest, mapblkData + logoOffsetSrc + lineOffsetSrc, kCountrySpriteWidth);
        }

        countrySpritePositions_[i] = new Point2D();
        countrySpritePositions_[i]->x = col * kCountrySpriteWidth;
        countrySpritePositions_[i]->y = row * kCountrySpriteHeight;
    }
}


/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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
#include "app.h"
#include "model/research.h"
#include "researchmenu.h"
#include "menus/gamemenuid.h"

ResearchMenu::ResearchMenu(MenuManager * m):Menu(m, fs_game_menus::kMenuIdResearch, fs_game_menus::kMenuIdSelect, "mresrch.dat", "mresout.dat")
{
    tab_ = TAB_EQUIPS;
    pSelectedWeapon_ = NULL;
    pSelectedMod_ = NULL;
    pSelectedRes_ = NULL;
    pResForGraph_ = NULL;

    addStatic(228, 35, "#RES_TITLE", FontManager::SIZE_4, false);          //Title
    txtTimeId_ = addStatic(500, 9, "", FontManager::SIZE_2, true);     // Time
    moneyTxtId_ = addStatic(500, 87, 128, "0", FontManager::SIZE_2, true);     // Money

    modsButId_ = addToggleAction(16, 290, 129, 25, "#MENU_MODS_BUT", FontManager::SIZE_2, tab_ == TAB_MODS);
    equipButId_ = addToggleAction(16, 318, 129, 25,  "#MENU_EQUIP_BUT", FontManager::SIZE_2, tab_ == TAB_EQUIPS);
    addOption(16, 346, 129, 25,  "#MENU_ACC_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdSelect);
    addOption(500, 347,  128, 25, "#MENU_MAIN_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdMain);

    pFieldEquipLBox_ = addListBox(20, 84,  122, 120, tab_ == TAB_EQUIPS);
    pFieldEquipLBox_->setModel(g_Session.researchManager().getAvailableWeaponsSearch());
    pFieldModsLBox_ = addListBox(20, 84,  122, 120, tab_ == TAB_MODS);
    pFieldModsLBox_->setModel(g_Session.researchManager().getAvailableModsSearch());

    pEquipsLBox_ = addListBox(504, 110,  122, 230, tab_ == TAB_EQUIPS);
    pEquipsLBox_->setModel(g_gameCtrl.weaponManager().getAvailableWeapons());
    pModsLBox_ = addListBox(504, 110,  122, 230, tab_ == TAB_MODS);
    pModsLBox_->setModel(g_gameCtrl.mods().getAvalaibleMods());

    // Close Mods/Equips details button
    cancelDescId_ = addOption(500, 320,  127, 22,  "#MENU_CANCEL_BUT", FontManager::SIZE_2, MENU_NO_MENU, false);
    // Start research button
    researchId_ = addOption(16, 158, 129, 25,  "#RES_RES_BUT", FontManager::SIZE_2, MENU_NO_MENU, false);
    // Close search details button
    cancelSearchId_ = addOption(16, 184, 129, 25,  "#MENU_CANCEL_BUT", FontManager::SIZE_2, MENU_NO_MENU, false);

    fieldTxtId_ = addStatic(20, 86, "", FontManager::SIZE_1, true);    // Search name
    fundMinLblId_ = addStatic(20, 105, "#RES_MIN_FUND_LBL", FontManager::SIZE_1, false);    // Funding Minimun label
    getStatic(fundMinLblId_)->setVisible(false);
    fundMinTxtId_ = addStatic(20, 117, "", FontManager::SIZE_1, true);    // Funding Minimun
    fundMaxLblId_ = addStatic(20, 130, "#RES_MAX_FUND_LBL", FontManager::SIZE_1, false);    // Funding max label
    getStatic(fundMaxLblId_)->setVisible(false);
    fundMaxTxtId_ = addStatic(20, 143, "", FontManager::SIZE_1, true);    // Search maximum

    incrFundId_ = addOption(16, 210, 129, 25,  "#RES_INC_FUND_BUT", FontManager::SIZE_2, MENU_NO_MENU, false);
    registerHotKey(K_PLUS, incrFundId_);
    decrFundId_ = addOption(16, 260, 129, 25,  "#RES_DEC_FUND_BUT", FontManager::SIZE_2, MENU_NO_MENU, false);
    registerHotKey(K_MINUS, decrFundId_);
    fundCurrLblId_ = addStatic(16, 242, 129, "", FontManager::SIZE_2, true);    // Current Funding label
    searchTitleLblId_ = addStatic(158, 86, "", FontManager::SIZE_2, true);    // Current search title

    g_gameCtrl.addListener(this, GameEvent::kGame);
}

/*!
 * Shows either the mods list box or equips list box.
 */
void ResearchMenu::showDetailsList() {
    addDirtyRect(500, 105,  125, 235);
    getOption(cancelDescId_)->setVisible(false);
    pSelectedWeapon_ = NULL;
    pSelectedMod_ = NULL;

    if (tab_ == TAB_MODS) {
        pModsLBox_->setVisible(true);
        pEquipsLBox_->setVisible(false);
    } else {
        pEquipsLBox_->setVisible(true);
        pModsLBox_->setVisible(false);
    }
}

/*!
 * Hides the mods list box or equips list box.
 */
void ResearchMenu::hideDetailsList() {
    pEquipsLBox_->setVisible(false);
    pModsLBox_->setVisible(false);
}

/*!
 * Shows either the search field list box for Equips or Mods.
 */
void ResearchMenu::showFieldList() {
    addDirtyRect(15, 80,  130, 130);

    if (tab_ == TAB_EQUIPS) {
        pFieldEquipLBox_->setVisible(true);
        pFieldModsLBox_->setVisible(false);
    } else {
        pFieldEquipLBox_->setVisible(false);
        pFieldModsLBox_->setVisible(true);
    }

    pSelectedRes_ = NULL;
    getOption(researchId_)->setVisible(false);
    getOption(cancelSearchId_)->setVisible(false);
    getStatic(fieldTxtId_)->setVisible(false);

    getStatic(fundMinTxtId_)->setVisible(false);
    getStatic(fundMaxTxtId_)->setVisible(false);
    getStatic(fundMinLblId_)->setVisible(false);
    getStatic(fundMaxLblId_)->setVisible(false);
    if (!pResForGraph_) {
        getOption(incrFundId_)->setVisible(false);
        getOption(decrFundId_)->setVisible(false);

        getStatic(fundCurrLblId_)->setVisible(false);
    }
}

/*!
 * Hides the search field list box for Equips and Mods.
 */
void ResearchMenu::hideFieldList() {
    pFieldEquipLBox_->setVisible(false);
    pFieldModsLBox_->setVisible(false);
}

/*!
 * Displays max and min fundings for selected research.
 */
void ResearchMenu::showResInfo() {
    if (pSelectedRes_) {
        MenuText * pTxt = getStatic(fundMinTxtId_);
        pTxt->setVisible(true);
        pTxt->setTextFormated("%d", pSelectedRes_->getMinFunding());

        pTxt = getStatic(fundMaxTxtId_);
        pTxt->setVisible(true);
        pTxt->setTextFormated("%d", pSelectedRes_->getMaxFunding());

        pTxt = getStatic(fieldTxtId_);
        pTxt->setVisible(true);
        pTxt->setText(pSelectedRes_->getName().c_str());

        getStatic(fundMinLblId_)->setVisible(true);
        getStatic(fundMaxLblId_)->setVisible(true);

        if (pSelectedRes_->getStatus() == Research::STARTED) {
            showResGraph(pSelectedRes_);
        } else {
            // Show start research button only if selected research
            // has not started yet
            getOption(researchId_)->setVisible(true);
        }
        // Show cancel button
        getOption(cancelSearchId_)->setVisible(true);
    }
}

void ResearchMenu::showResGraph(Research *pRes) {
    redrawGraph();
    pResForGraph_ = pRes;
    getOption(researchId_)->setVisible(false);
    getOption(incrFundId_)->setVisible(true);
    getOption(decrFundId_)->setVisible(true);
    MenuText *pTxt = getStatic(fundCurrLblId_);
    pTxt->setVisible(true);
    pTxt->setTextFormated("%d", pResForGraph_->getCurrFunding());
    getStatic(searchTitleLblId_)->setTextFormated("#RES_GRAP_TITLE", pResForGraph_->getName().c_str());
}

void ResearchMenu::drawSelectedWeaponInfos(int x, int y) {
    char tmp[100];

    getMenuFont(FontManager::SIZE_1)->drawText(x, y, pSelectedWeapon_->getName(), false);
    sprintf(tmp, "COST   :%d", pSelectedWeapon_->cost());
    getMenuFont(FontManager::SIZE_1)->drawText(x, y + 12, tmp, false);
    y += 24;

    if (pSelectedWeapon_->ammo() >= 0) {
        sprintf(tmp, "AMMO   :%d", pSelectedWeapon_->ammo());
        getMenuFont(FontManager::SIZE_1)->drawText(x, y, tmp, false);
        y += 12;
    }

    if (pSelectedWeapon_->range() >= 0) {
        sprintf(tmp, "RANGE  :%d", pSelectedWeapon_->range());
        getMenuFont(FontManager::SIZE_1)->drawText(x, y, tmp, false);
        y += 12;
    }

    if (pSelectedWeapon_->damagePerShot() >= 0 && pSelectedWeapon_->ammo() >= 0) {
        sprintf(tmp, "SHOT   :%d", pSelectedWeapon_->damagePerShot());
        getMenuFont(FontManager::SIZE_1)->drawText(x, y, tmp, false);
        y += 12;
    }
}

void ResearchMenu::drawSelectedModInfos(int x, int y)
{
    getMenuFont(FontManager::SIZE_1)->drawText(x, y, pSelectedMod_->getName(), false);
    char tmp[100];
    sprintf(tmp, "COST   :%d", pSelectedMod_->cost());
    getMenuFont(FontManager::SIZE_1)->drawText(504, y + 14, tmp, false);
    getMenuFont(FontManager::SIZE_1)->drawText(504, y + 28, pSelectedMod_->desc(), false);
}

void ResearchMenu::handleTick(int elapsed)
{
    if (g_Session.updateTime(elapsed)) {
        updateClock();
        if (pResForGraph_) {
            redrawGraph();
        }
    }
}

/*!
 * Update the game time display
 */
void ResearchMenu::updateClock() {
    char tmp[100];
    g_Session.getTimeAsStr(tmp);
    getStatic(txtTimeId_)->setText(tmp);

    // update money
    getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
}

void ResearchMenu::handleShow() {

    menu_manager_->saveBackground();

    // Show the mouse
    g_System.showCursor();

    // Update the time
    updateClock();

    // Display current active research
    if (g_Session.researchManager().getActiveSearch()) {
        showResGraph(g_Session.researchManager().getActiveSearch());
    }
}

void ResearchMenu::handleRender(DirtyList &dirtyList)
{
    g_Screen.drawLogo(18, 14, g_Session.getLogo(), g_Session.getLogoColour());

    if (pSelectedWeapon_) {
        uint8 ldata[62];
        memset(ldata, 16, sizeof(ldata));
        g_Screen.scale2x(502, 318, sizeof(ldata), 1, ldata);

        menuSprites().drawSpriteXYZ(pSelectedWeapon_->getBigIconId(), 502, 108, 0, false, true);
        drawSelectedWeaponInfos(504, 196);
    }

    if (pSelectedMod_) {
        uint8 ldata[62];
        memset(ldata, 16, sizeof(ldata));
        g_Screen.scale2x(502, 318, sizeof(ldata), 1, ldata);

        drawSelectedModInfos(504, 108);
    }

    if (pSelectedRes_) {
        uint8 ldata[63];
        memset(ldata, 16, sizeof(ldata));
        g_Screen.scale2x(18, 102, sizeof(ldata), 1, ldata);
        g_Screen.scale2x(18, 158, sizeof(ldata), 1, ldata);
        g_Screen.scale2x(18, 182, sizeof(ldata), 1, ldata);
    }

    if (pResForGraph_) {
        int lastX = 205;
        int lastY = 350;
        int projectedX = pResForGraph_->getProjectedHour() + 205;
        int projectedY = 110;

        // draw progression
        std::list < Research::ProgressPoint > list = pResForGraph_->getProgressList();
        for (std::list < Research::ProgressPoint >::iterator it = list.begin();
                it != list.end(); it++) {
            Research::ProgressPoint p = *it;
            int x = p.hours + 205;
            int y = 350 - (int)(p.percentage *2.4f);

            g_Screen.drawLine(lastX, lastY, x, y, 252);
            g_Screen.drawLine(lastX - 1, lastY, x - 1, y, 252);

            lastX = x;
            lastY = y;
        }

        // draw projection line
        if (pResForGraph_->getCurrFunding() != 0) {
            g_Screen.drawLine(lastX, lastY, projectedX, projectedY, 16);
            g_Screen.drawLine(lastX - 1, lastY, projectedX - 1, projectedY, 16);
        }
    }
}

void ResearchMenu::handleLeave() {
    g_System.hideCursor();
    // reset window presentation
    pResForGraph_ = NULL;
    showFieldList();
    showDetailsList();
    getStatic(searchTitleLblId_)->setText("");
}

void ResearchMenu::handleAction(const int actionId, void *ctx, const int modKeys)
{
    // Field list box : Equips or Mods
    if (actionId == pFieldEquipLBox_->getId() || actionId == pFieldModsLBox_->getId()) {
        // get selected field
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (ctx);
        pSelectedRes_ = static_cast<Research *> (pPair->second);

        // Hide list
        hideFieldList();
        // Show Research and Cancel buttons
        showResInfo();

    } else if (actionId == pModsLBox_->getId()) {  // Selection of an avalaible mod
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (ctx);
        pSelectedMod_ = static_cast<Mod *> (pPair->second);
        hideDetailsList();
        getOption(cancelDescId_)->setVisible(true);

    } else if (actionId == pEquipsLBox_->getId()) { // Selection of an avalaible weapon
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (ctx);
        pSelectedWeapon_ = static_cast<Weapon *> (pPair->second);
        hideDetailsList();
        getOption(cancelDescId_)->setVisible(true);

    } else if (actionId == modsButId_) {
        if (tab_ != TAB_MODS) {
            tab_ = TAB_MODS;
            showFieldList();
            showDetailsList();
        }
    } else if (actionId == equipButId_) {
        if (tab_ != TAB_EQUIPS) {
            tab_ = TAB_EQUIPS;
            showFieldList();
            showDetailsList();
        }
    } else if (actionId == cancelDescId_) {
        showDetailsList();
    } else if (actionId == cancelSearchId_) {
        showFieldList();
    } else if (actionId == researchId_) {
        g_Session.researchManager().start(pSelectedRes_);
        showResGraph(pSelectedRes_);
    } else if (actionId == incrFundId_) {
        if (pResForGraph_->incrFunding()) {
            getStatic(fundCurrLblId_)->setTextFormated("%d", pResForGraph_->getCurrFunding());
            // redraw graph
            redrawGraph();
        }
    } else if (actionId == decrFundId_) {
        if (pResForGraph_->decrFunding()) {
            getStatic(fundCurrLblId_)->setTextFormated("%d", pResForGraph_->getCurrFunding());
            // redraw graph
            redrawGraph();
        }
    }
}

void ResearchMenu::handleGameEvent(GameEvent evt) {
    if (evt.type == GameEvent::kResearch) {
        // A research has ended
        Research *pRes = static_cast<Research *> (evt.pCtxt);

        // If current graph was for this research, make it disappear
        if (pResForGraph_ && pResForGraph_->getId() == pRes->getId()) {
            pResForGraph_ = NULL;
            redrawGraph();
            getStatic(fundCurrLblId_)->setVisible(false);
            getStatic(searchTitleLblId_)->setText("");
        }

        // If there was a research info panel opened -> close it
        if (pSelectedRes_ && pSelectedRes_->getId() == pRes->getId()) {
            showFieldList();
        }
    }
}

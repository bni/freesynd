/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2011  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2011  Mark <mentor66@users.sourceforge.net>          *
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

#include "selectmenu.h"
#include "menus/menumanager.h"
#include "menus/gamemenuid.h"
#include "core/gamecontroller.h"
#include "core/gamesession.h"
#include "gfx/screen.h"
#include "system.h"
#include "model/mod.h"

SelectMenu::SelectMenu(MenuManager * m):Menu(m, fs_game_menus::kMenuIdSelect, fs_game_menus::kMenuIdBrief, "mselect.dat",
    "mselout.dat"),
cur_agent_(0), tick_count_(0), rnd_(0), sel_all_(false)
{
    tab_ = TAB_EQUIPS;
    pSelectedWeap_ = NULL;
    selectedWInstId_ = 0;
    pSelectedMod_ = NULL;
    weapon_dragged_ = NULL;

    addStatic(85, 35, 545, "#SELECT_TITLE", FontManager::SIZE_4, false);
    txtTimeId_ = addStatic(500, 9, "", FontManager::SIZE_2, true);       // Time
    moneyTxtId_ = addStatic(500, 87, 128, "0", FontManager::SIZE_2, true);     // Money

    addOption(16, 234, 129, 25, "#SELECT_RES_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdResearch);
    teamButId_ = addToggleAction(16, 262, 129, 25, "#SELECT_TEAM_BUT", FontManager::SIZE_2, false);
    modsButId_ = addToggleAction(16, 290, 129, 25, "#MENU_MODS_BUT", FontManager::SIZE_2, false);
    equipButId_ = addToggleAction(16, 318, 129, 25, "#MENU_EQUIP_BUT", FontManager::SIZE_2, true);
    acceptButId_ = addOption(16, 346, 129, 25, "#MENU_ACC_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdLoading);
    addOption(500, 347,  128, 25, "#MENU_MAIN_BUT", FontManager::SIZE_2, fs_game_menus::kMenuIdMain);

    // Team list
    pTeamLBox_ = addTeamListBox(502, 106, 124, 236, false);
    pTeamLBox_->setModel(g_gameCtrl.agents().getAgents());
    // Available weapons list
    pWeaponsLBox_ = addListBox(504, 110,  122, 230, tab_ == TAB_EQUIPS);
    pWeaponsLBox_->setModel(g_gameCtrl.weaponManager().getAvailableWeapons());
    // Available mods list
    pModsLBox_ = addListBox(504, 110,  122, 230, tab_ == TAB_MODS);
    pModsLBox_->setModel(g_gameCtrl.mods().getAvalaibleMods());

    cancelButId_ = addOption(500, 270,  127, 22, "#MENU_CANCEL_BUT",
        FontManager::SIZE_2, MENU_NO_MENU, false);
    reloadButId_ = addOption(500, 295,  127, 22, "#SELECT_RELOAD_BUT",
        FontManager::SIZE_2, MENU_NO_MENU, false);
    purchaseButId_ = addOption(500, 320,  127, 22, "#SELECT_BUY_BUT",
        FontManager::SIZE_2, MENU_NO_MENU, false);
    sellButId_ = addOption(500, 320,  127, 22, "#SELECT_SELL_BUT",
        FontManager::SIZE_2, MENU_NO_MENU, false);

    // Agent name selected
    txtAgentId_ = addStatic(158, 86, "", FontManager::SIZE_2, true);
}

SelectMenu::~SelectMenu()
{
}

/*!
 * Draws a dashed line around the currently selected agent selector.
 * \param x Coordinates of the top left corner
 * \param y Coordinates of the top left corner
 */
void SelectMenu::drawAgentSelector(int x, int y) {
    // First create a transparent sprite
    uint8 cdata[30 * 33];
    int cwidth = 30;
    int cheight = 33;
    memset(cdata, 255, sizeof(cdata));

    // Draws the upper and lower lines
    for (int i = 0; i < cwidth; i++) {
        cdata[i] = ((rnd_ + i) % 8 <= 4) ? 252 : 16;
        cdata[i + (cheight - 1) * cwidth] =
            ((rnd_ + i) % 8 >= 4) ? 252 : 16;
    }

    // Draws the right and left line
    for (int j = 0; j < cheight; j++) {
        cdata[j * cwidth] = ((rnd_ + j) % 8 >= 4) ? 252 : 16;
        cdata[j * cwidth + cwidth - 1] = ((rnd_ + j) % 8 <= 4) ? 252 : 16;
    }

    // blits the sprite at given position
    g_Screen.scale2x(x, y, cwidth, cheight, cdata);
}

void SelectMenu::drawAgent()
{
    Agent *selected = g_gameCtrl.agents().squadMember(cur_agent_);
    if (selected == NULL)
        return;

    int torso, arms, legs;
    int armsx = 188;
    int armsy = 152;
    int torsoy = 116;
    int legsy = 218;
    if (selected->isMale()) {
        torso = 30;
        arms = 40;
        legs = 32;
    } else {
        torso = 31;
        arms = 44;
        legs = 36;
        armsx += 10;
        armsy += 6;
        torsoy += 2;
        legsy -= 4;
    }

    if (selected->slot(Mod::MOD_LEGS)) {
        legs = selected->slot(Mod::MOD_LEGS)->icon(selected->isMale());
        getMenuFont(FontManager::SIZE_1)->drawText(366, 250,
            selected->slot(Mod::MOD_LEGS)->getName(),
            false);
    }
    if (selected->slot(Mod::MOD_ARMS)) {
        arms = selected->slot(Mod::MOD_ARMS)->icon(selected->isMale());
        getMenuFont(FontManager::SIZE_1)->drawText(366, 226,
            selected->slot(Mod::MOD_ARMS)->getName(),
            false);
    }

    menuSprites().drawSpriteXYZ(arms, armsx, armsy, 0, false, true);
    menuSprites().drawSpriteXYZ(torso, 224, torsoy, 0, false, true);
    menuSprites().drawSpriteXYZ(legs, 224, legsy, 0, false, true);

    if (selected->slot(Mod::MOD_CHEST)) {
        int chest = selected->slot(Mod::MOD_CHEST)->icon(selected->isMale());
        getMenuFont(FontManager::SIZE_1)->drawText(366, 202,
            selected->slot(Mod::MOD_CHEST)->getName(), false);
        int chestx = 216;
        int chesty = 146;
        if (!selected->isMale()) {
            chestx += 8;
            chesty += 2;
        }
        menuSprites().drawSpriteXYZ(chest, chestx, chesty, 0, false,
                                          true);
    }

    if (selected->slot(Mod::MOD_HEART)) {
        int heart = selected->slot(Mod::MOD_HEART)->icon(selected->isMale());
        getMenuFont(FontManager::SIZE_1)->drawText(366, 160,
            selected->slot(Mod::MOD_HEART)->getName(), false);
        menuSprites().drawSpriteXYZ(heart, 254, 166, 0, false, true);
    }

    if (selected->slot(Mod::MOD_EYES)) {
        int eyes = selected->slot(Mod::MOD_EYES)->icon(selected->isMale());
        getMenuFont(FontManager::SIZE_1)->drawText(366, 136,
            selected->slot(Mod::MOD_EYES)->getName(), false);
        int eyesx = 238;
        if (!selected->isMale()) {
            eyesx += 2;
        }
        menuSprites().drawSpriteXYZ(eyes, eyesx, 116, 0, false,
                                          true);
    }

    if (selected->slot(Mod::MOD_BRAIN)) {
        int brain = selected->slot(Mod::MOD_BRAIN)->icon(selected->isMale());
        getMenuFont(FontManager::SIZE_1)->drawText(366, 112,
            selected->slot(Mod::MOD_BRAIN)->getName(), false);
        int brainx = 238;
        if (!selected->isMale()) {
            brainx += 2;
        }
        menuSprites().drawSpriteXYZ(brain, brainx, 114, 0, false, true);
    }
    // restore lines over agent
    menu_manager_->blitFromBackground(254, 124, 30, 2);
    menu_manager_->blitFromBackground(264, 132, 30, 2);
    menu_manager_->blitFromBackground(266, 174, 36, 2);
    menu_manager_->blitFromBackground(252, 210, 56, 2);
    menu_manager_->blitFromBackground(302, 232, 10, 2);
    menu_manager_->blitFromBackground(264, 256, 30, 2);

    // draw inventory
    screenPoint pos[8];
    WeaponInstance * draw_weapons[8];
    for (uint8 i = 0; i < 8; ++i) {
        draw_weapons[i] = NULL;
    }
    for (int j = 0, k = 0; j < 2; ++j)
        for (int i = 0; i < 4 && (j * 4 + i < selected->numWeapons()); ++i)
        {
            // TODO: make weapon to be drawn at end
            WeaponInstance *wi = selected->weapon(j * 4 + i);
            if (wi == weapon_dragged_ && menu_manager_->isMouseDragged()) {
                pos[7] = weapon_pos_;
                draw_weapons[7] = wi;
            } else {
                draw_weapons[k] = wi;
                screenPoint pos_l = {366 + i * 32, 308 + j * 32};
                pos[k] = pos_l;
                ++k;
            }

        }

    for (uint8 i = 0; i < 8; ++i) {
        WeaponInstance *wi = draw_weapons[i];

        if (wi) {
            Weapon *pW = wi->getClass();
            menuSprites().drawSpriteXYZ(pW->getSmallIconId(),
                pos[i].x, pos[i].y, 0, false, true);
            uint8 data[3] = {204, 204, 204};
            if (pW->ammo() != -1) {
                int n = wi->ammoRemaining();
                if (pW->ammo() == 0)
                    n = 24;
                else {
                    n *= 24;
                    n /= pW->ammo();
                }
                for (int k = 0; k < n; k++)
                    g_Screen.scale2x(pos[i].x + k + 4, pos[i].y + 22, 1, 3, data);
            }
        }
    }
}

void SelectMenu::handleTick(int elapsed)
{
    tick_count_ += elapsed;
    // Updates the moving agent selector
    if (tick_count_ > 300) {
        rnd_ = (rnd_ + 1) % 8;
        tick_count_ = 0;
        needRendering();
    }

    // Update the clock
    if (g_Session.updateTime(elapsed)) {
        updateClock();
    }
}

/*!
 * Update the game time display
 */
void SelectMenu::updateClock() {
    char tmp[100];
    g_Session.getTimeAsStr(tmp);
    getStatic(txtTimeId_)->setText(tmp);

    // update money
    getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
}

void SelectMenu::drawSelectedWeaponInfos(int x, int y) {
    char tmp[100];

    // Draw a border around cancel button
    g_Screen.drawRect(502, 268, 124, 2, fs_cmn::kColorDarkGreen);
    g_Screen.drawRect(502, 292, 124, 2, fs_cmn::kColorDarkGreen);
    g_Screen.drawRect(502, 318, 124, 2, fs_cmn::kColorDarkGreen);

    // Draw the selected weapon big icon
    menuSprites().drawSpriteXYZ(pSelectedWeap_->getBigIconId(), 502, 106, 0, false, true);

    const int shifted_x = x + 52;
    getMenuFont(FontManager::SIZE_1)->drawText(x, y, pSelectedWeap_->getName(), true);
    y += 12;
    sprintf(tmp, ":%d", pSelectedWeap_->cost());
    getMenuFont(FontManager::SIZE_1)->drawText(x, y,
        getMessage("SELECT_WPN_COST").c_str(), true);
    getMenuFont(FontManager::SIZE_1)->drawText(shifted_x, y, tmp, true);
    y += 12;

    if (pSelectedWeap_->usesAmmo()) {
        sprintf(tmp, ":%d", pSelectedWeap_->ammo());
        getMenuFont(FontManager::SIZE_1)->drawText(x, y,
            getMessage("SELECT_WPN_AMMO").c_str(), true);
        getMenuFont(FontManager::SIZE_1)->drawText(shifted_x, y, tmp, true);
        y += 12;
    }

    if (pSelectedWeap_->range() > 0) {
        sprintf(tmp, ":%d", pSelectedWeap_->range());
        getMenuFont(FontManager::SIZE_1)->drawText(x, y,
            getMessage("SELECT_WPN_RANGE").c_str(), true);
        getMenuFont(FontManager::SIZE_1)->drawText(shifted_x, y, tmp, true);
        y += 12;
    }

    if (pSelectedWeap_->usesAmmo()) {
        sprintf(tmp, ":%d", pSelectedWeap_->ammoCost());
        getMenuFont(FontManager::SIZE_1)->drawText(x, y,
            getMessage("SELECT_WPN_SHOT").c_str(), true);
        getMenuFont(FontManager::SIZE_1)->drawText(shifted_x, y, tmp, true);
        y += 12;
    }

    if (selectedWInstId_ > 0 && g_gameCtrl.weaponManager().isAvailable(pSelectedWeap_)) {
        WeaponInstance *wi = g_gameCtrl.agents().squadMember(cur_agent_)->weapon(selectedWInstId_ - 1);
        if (wi->needsReloading()) {
            int rldCost = pSelectedWeap_->calculateReloadingCost(wi->ammoRemaining());

            sprintf(tmp, ":%d", rldCost);
            getMenuFont(FontManager::SIZE_1)->drawText(x, y,
                getMessage("SELECT_WPN_RELOAD").c_str(), true);
            getMenuFont(FontManager::SIZE_1)->drawText(shifted_x, y, tmp, true);
            y += 12;
        }
    }
}

void SelectMenu::drawSelectedModInfos(int x, int y)
{
    // Draw a border around cancel button
    g_Screen.drawRect(502, 268, 124, 2, fs_cmn::kColorDarkGreen);
    g_Screen.drawRect(502, 292, 124, 2, fs_cmn::kColorDarkGreen);
    g_Screen.drawRect(502, 318, 124, 2, fs_cmn::kColorDarkGreen);

    getMenuFont(FontManager::SIZE_1)->drawText(x, y, pSelectedMod_->getName(), true);
    char tmp[100];
    sprintf(tmp, "COST   :%d", pSelectedMod_->cost());
    getMenuFont(FontManager::SIZE_1)->drawText(504, y + 14, tmp, true);
    getMenuFont(FontManager::SIZE_1)->drawText(504, y + 28, pSelectedMod_->desc(), true);
}

void SelectMenu::handleShow() {

    menu_manager_->saveBackground();

    // Show the mouse
    g_System.showCursor();

    // Update the time
    updateClock();

    if (g_gameCtrl.agents().squadMember(cur_agent_)) {
        getStatic(txtAgentId_)->setTextFormated("#SELECT_SUBTITLE", g_gameCtrl.agents().squadMember(cur_agent_)->getName());
    } else {
        getStatic(txtAgentId_)->setText("");
    }

    for (int iAgnt=0; iAgnt<AgentManager::MAX_AGENT; iAgnt++) {
        Agent *pAgentFromCryo = g_gameCtrl.agents().agent(iAgnt);
        pTeamLBox_->setSquadLine(g_gameCtrl.agents().getSquadSlotForAgent(pAgentFromCryo), iAgnt);
    }
    showItemList();

    updateAcceptEnabled();
    menu_manager_->resetSinceMouseDown();
}

void SelectMenu::handleRender(DirtyList &dirtyList) {
    g_Screen.drawLogo(18, 14, g_Session.getLogo(), g_Session.getLogoColour());

    // write team member icons and health
    Agent *t1 = g_gameCtrl.agents().squadMember(AgentManager::kSlot1);
    Agent *t2 = g_gameCtrl.agents().squadMember(AgentManager::kSlot2);
    Agent *t3 = g_gameCtrl.agents().squadMember(AgentManager::kSlot3);
    Agent *t4 = g_gameCtrl.agents().squadMember(AgentManager::kSlot4);
    if (t1) {
        if (t1->isActive()) {
            menuSprites().drawSpriteXYZ(Sprite::MSPR_SELECT_1, 20, 84, 0, false, true);
            g_Screen.drawRect(68, 88, 6, 36, 204);
        } else {
            g_Screen.drawRect(68, 88, 6, 36, 10);
        }
    }
    if (t2) {
        if (t2->isActive()) {
            menuSprites().drawSpriteXYZ(Sprite::MSPR_SELECT_2, 82, 84, 0, false, true);
            g_Screen.drawRect(132, 88, 6, 36, 204);
        } else {
            g_Screen.drawRect(132, 88, 6, 36, 10);
        }
    }
    if (t3) {
        if (t3->isActive()) {
            menuSprites().drawSpriteXYZ(Sprite::MSPR_SELECT_3, 20, 162, 0, false, true);
            g_Screen.drawRect(68, 166, 6, 36, 204);
        } else {
            g_Screen.drawRect(68, 166, 6, 36, 10);
        }
    }
    if (t4) {
        if (t4->isActive()) {
            menuSprites().drawSpriteXYZ(Sprite::MSPR_SELECT_4, 82, 162, 0, false, true);
            g_Screen.drawRect(132, 166, 6, 36, 204);
        } else {
            g_Screen.drawRect(132, 166, 6, 36, 10);
        }
    }
    if (sel_all_) {
        menuSprites().drawSpriteXYZ(77, 20, 152, 0, false, true);
    }

    // Draw the selector around the selected agent
    switch (cur_agent_) {
        case 0:
            drawAgentSelector(20, 84);
            break;
        case 1:
            drawAgentSelector(82, 84);
            break;
        case 2:
            drawAgentSelector(20, 162);
            break;
        case 3:
            drawAgentSelector(82, 162);
            break;
    }

    if (pSelectedWeap_) {
        drawSelectedWeaponInfos(504, 194);
    } else if (pSelectedMod_) {
        drawSelectedModInfos(504, 108);
    }

    drawAgent();
}

void SelectMenu::handleLeave() {
    g_System.hideCursor();
    // resetting menu, all other variables are reset in handleShow
    // with showItemList()
    tab_ = TAB_EQUIPS;
    rnd_ = 0;
    cur_agent_ = 0;
    sel_all_ = false;
    weapon_dragged_ = NULL;
}

/*!
 * Invert the active/inactive status of an agent in the squad.
 * Cannot inactivate an agent if he's the last active agent.
 */
void SelectMenu::toggleAgent(int n)
{
    int nactive = 0;
    // count the number of active agents
    for (size_t i = 0; i < AgentManager::kMaxSlot; i++)
        if (g_gameCtrl.agents().isSquadSlotActive(i))
            nactive++;
    Agent *a = g_gameCtrl.agents().squadMember(n);
    if (a) {
        // prevent from inactiving the last active agent
        if (a->isActive() && nactive == 1)
            return;
        a->setActive(!a->isActive());
        dirtyAgentSelector();
    }
}

/*!
 * Update the accept button : enable if there is one agent active.
 */
void SelectMenu::updateAcceptEnabled() {
    // Player cannot start mission if no agent has been activated
    bool found = false;
    for (size_t i=0; i<AgentManager::kMaxSlot; i++) {
        if (g_gameCtrl.agents().isSquadSlotActive(i)) {
            found = true;
            break;
        }
    }
    getOption(acceptButId_)->setWidgetEnabled(found);
}

void SelectMenu::handleMouseMotion(int x, int y, int state, const int modKeys)
{
    if (weapon_dragged_) {
        weapon_pos_.x = x;
        weapon_pos_.y = y;
        needRendering();
    }
}

void SelectMenu::handleMouseUp(int x, int y, int button, const int modKeys)
{
    if (button == 3) {
        Agent *selected = g_gameCtrl.agents().squadMember(cur_agent_);
        if (selected == NULL)
            weapon_dragged_ = NULL;
        if (weapon_dragged_) {
            int target = -1;
            if (x >= 20 && x <= 140) {
                if (y >= 84 && y <= 150) {
                    if (x >= 82) {
                        target = 1;
                    } else {
                        target = 0;
                    }
                }
                if (y >= 162 && y <= 228) {
                    if (x >= 82) {
                        target = 3;
                    } else {
                        target = 2;
                    }
                }
            }
            Agent *reciever = target != -1
                ? g_gameCtrl.agents().squadMember(target) : NULL;
            if (target != cur_agent_ && reciever
                && reciever->numWeapons() < 8)
            {
                selected->removeWeapon(weapon_dragged_);
                reciever->addWeapon(weapon_dragged_);

                pSelectedWeap_ = NULL;
                selectedWInstId_ = 0;
                showItemList();
            }
            weapon_dragged_ = NULL;
        }
    }
}

bool SelectMenu::handleMouseDown(int x, int y, int button, const int modKeys)
{
    if (x >= 20 && x <= 140) {
        if (y >= 84 && y <= 150) {
            if (x >= 82) {
                handleClickOnAgentSelector(AgentManager::kSlot2, button);
            } else {
                handleClickOnAgentSelector(AgentManager::kSlot1, button);
            }
        }
        if (y > 150 && y < 162) {
            sel_all_ = !sel_all_;
            needRendering();
        }
        if (y >= 162 && y <= 228) {
            if (x >= 82) {
                handleClickOnAgentSelector(AgentManager::kSlot4, button);
            } else {
                handleClickOnAgentSelector(AgentManager::kSlot3, button);
            }
        }
    }

    // Checks if the user clicked on item in the current agent inventory
    Agent *selected = g_gameCtrl.agents().squadMember(cur_agent_);
    if (selected) {
        if (x >= 366 && x < 366 + 4 * 32
            && y >= 308 && y < 308 + 2 * 32)
        {
            int newId = (x - 366) / 32 + ((y - 308) / 32) * 4;
            if (newId < selected->numWeapons())
            {
                // The user has actually selected a weapon from the inventory :
                // 1/ selects the EQUIPS toggle button
                tab_ = TAB_EQUIPS;
                pSelectedMod_ = NULL;
                selectToggleAction(equipButId_);
                // 2/ computes the id of the selected weapon and selects it
                newId++;

                WeaponInstance *wi = selected->weapon(newId - 1);
                if (button == 3) {
                    weapon_dragged_ = wi;
                    weapon_pos_.x = x;
                    weapon_pos_.y = y;
                }

                if (newId != selectedWInstId_) { // Do something only if a different weapon is selected
                    selectedWInstId_ = newId;
                    pSelectedWeap_ = wi->getClass();
                    addDirtyRect(500, 105,  125, 235);
                    // 3/ see if reload button should be displayed,
                    // if weapon is not researched it will not be reloadable
                    bool displayReload = wi->needsReloading()
                        && g_gameCtrl.weaponManager().isAvailable(pSelectedWeap_);
                    getOption(reloadButId_)->setVisible(displayReload);

                    // 4/ hides the purchase button for the sell button
                    getOption(purchaseButId_)->setVisible(false);
                    getOption(cancelButId_)->setVisible(true);
                    getOption(sellButId_)->setVisible(true);
                    pTeamLBox_->setVisible(false);
                    pModsLBox_->setVisible(false);
                    pWeaponsLBox_->setVisible(false);
                }
            }
        }
    }

    return false;
}

/**
 * Handles when the player clicks on a agent selector.
 */
void SelectMenu::handleClickOnAgentSelector(const int agent_no, int button) {
    if (button == 3) {
        toggleAgent(agent_no);
    } else if (cur_agent_ != agent_no) {
        if (selectedWInstId_ != 0)
            updateSelectedWeapon();
        cur_agent_ = agent_no;
        if (g_gameCtrl.agents().squadMember(agent_no)) {
            getStatic(txtAgentId_)->setTextFormated("#SELECT_SUBTITLE",
                g_gameCtrl.agents().squadMember(agent_no)->getName());
        } else {
            getStatic(txtAgentId_)->setText("");
        }
        needRendering();
    }
}

/*!
 * Hides the list of Mods or Weapon and shows the purchase and cancel buttons
 * that appear on the detail panel.
 */
void SelectMenu::showModWeaponPanel() {
    getOption(purchaseButId_)->setVisible(true);
    getOption(cancelButId_)->setVisible(true);
    if (tab_ == TAB_MODS) {
        pModsLBox_->setVisible(false);
    } else {
        pWeaponsLBox_->setVisible(false);
    }
}

void SelectMenu::showItemList() {
    addDirtyRect(500, 105,  125, 235);
    pSelectedMod_ = NULL;
    pSelectedWeap_ = NULL;
    selectedWInstId_ = 0;
    getOption(cancelButId_)->setVisible(false);
    getOption(reloadButId_)->setVisible(false);
    getOption(purchaseButId_)->setVisible(false);
    getOption(sellButId_)->setVisible(false);

    if (tab_ == TAB_MODS) {
        pModsLBox_->setVisible(true);
        pWeaponsLBox_->setVisible(false);
        pTeamLBox_->setVisible(false);
    } else if (tab_ == TAB_EQUIPS) {
        pModsLBox_->setVisible(false);
        pWeaponsLBox_->setVisible(true);
        pTeamLBox_->setVisible(false);
    } else {
        pModsLBox_->setVisible(false);
        pWeaponsLBox_->setVisible(false);
        pTeamLBox_->setVisible(true);
    }
}

void SelectMenu::handleAction(const int actionId, void *ctx, const int modKeys)
{
    if (actionId == teamButId_) {
        tab_ = TAB_TEAM;
        showItemList();
    } else if (actionId == modsButId_) {
        tab_ = TAB_MODS;
        showItemList();
    } else if (actionId == equipButId_) {
        tab_ = TAB_EQUIPS;
        showItemList();
    } else if (actionId == pTeamLBox_->getId()) {
        // get the selected agent from the team listbox
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (ctx);
        Agent *pNewAgent = static_cast<Agent *> (pPair->second);

        bool found = false;
        // check if selected agent is already part of the mission squad
        for (size_t j = 0; j < AgentManager::kMaxSlot; j++) {
            if (g_gameCtrl.agents().squadMember(j) == pNewAgent) {
                found = true;
                break;
            }
        }

        // Agent was not part of the squad
        if (!found) {
            // adds him to the squad
            g_gameCtrl.agents().setSquadMember(cur_agent_, pNewAgent);
            // Update current agent name
            getStatic(txtAgentId_)->setTextFormated("#SELECT_SUBTITLE", pNewAgent->getName());
            pTeamLBox_->setSquadLine(cur_agent_, pPair->first);
            updateAcceptEnabled();

            // redraw agent display
            addDirtyRect(158, 110, 340, 260);
            // redraw agent buttons
            dirtyAgentSelector();
        }

    } else if (actionId == pModsLBox_->getId()) {
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (ctx);
        pSelectedMod_ = static_cast<Mod *> (pPair->second);
        showModWeaponPanel();
    } else if (actionId == pWeaponsLBox_->getId()) {
        std::pair<int, void *> * pPair = static_cast<std::pair<int, void *> *> (ctx);
        pSelectedWeap_ = static_cast<Weapon *> (pPair->second);
        showModWeaponPanel();
    } else if (actionId == cancelButId_) {
        showItemList();
    } else if (actionId == reloadButId_) {
        Agent *selected = g_gameCtrl.agents().squadMember(cur_agent_);
        WeaponInstance *wi = selected->weapon(selectedWInstId_ - 1);
        int rldCost = pSelectedWeap_->calculateReloadingCost(wi->ammoRemaining());

        if (g_Session.canAfford(rldCost)) {
            g_Session.decreaseMoney(rldCost);
            wi->reload();
            getOption(reloadButId_)->setVisible(false);
            getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
        }
    } else if (actionId == purchaseButId_) {
        // Buying weapon
        if (pSelectedWeap_) {
            if (sel_all_) {
                for (size_t n = 0; n < AgentManager::kMaxSlot; n++) {
                    Agent *selected = g_gameCtrl.agents().squadMember(n);
                    if (selected && selected->numWeapons() < 8
                        && g_Session.canAfford(pSelectedWeap_->cost())) {
                        g_Session.decreaseMoney(pSelectedWeap_->cost());
                        selected->addWeapon(WeaponInstance::createInstance(pSelectedWeap_));
                        getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
                    }
                }
            } else {
                Agent *selected = g_gameCtrl.agents().squadMember(cur_agent_);
                if (selected && selected->numWeapons() < 8
                    && g_Session.canAfford(pSelectedWeap_->cost())) {
                    g_Session.decreaseMoney(pSelectedWeap_->cost());
                    selected->addWeapon(WeaponInstance::createInstance(pSelectedWeap_));
                    getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
                }
            }
            needRendering();
        } else if (pSelectedMod_) {
            if (sel_all_) {
                for (size_t n = 0; n < AgentManager::kMaxSlot; n++) {
                    Agent *selected = g_gameCtrl.agents().squadMember(n);
                    if (selected && selected->canHaveMod(pSelectedMod_)
                        && g_Session.canAfford(pSelectedMod_->cost())) {
                        selected->addMod(pSelectedMod_);
                        g_Session.decreaseMoney(pSelectedMod_->cost());
                        getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
                    }
                }
            } else {
                Agent *selected = g_gameCtrl.agents().squadMember(cur_agent_);
                if (selected && selected->canHaveMod(pSelectedMod_)
                    && g_Session.canAfford(pSelectedMod_->cost())) {
                    selected->addMod(pSelectedMod_);
                    g_Session.decreaseMoney(pSelectedMod_->cost());
                    getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
                }
            }
            showItemList();
        }

    } else if (actionId == sellButId_ && selectedWInstId_) {
        addDirtyRect(360, 305, 135, 70);
        Agent *selected = g_gameCtrl.agents().squadMember(cur_agent_);
        WeaponInstance *pWi = selected->removeWeaponAtIndex(selectedWInstId_ - 1);
        g_Session.increaseMoney(pWi->getClass()->cost());
        getStatic(moneyTxtId_)->setTextFormated("%d", g_Session.getMoney());
        delete pWi;
        showItemList();
    }
}

void SelectMenu::updateSelectedWeapon() {
    assert(selectedWInstId_ != 0);

    Agent *selected = g_gameCtrl.agents().squadMember(cur_agent_);
    if (selected == NULL) {
        tab_ = TAB_EQUIPS;
        showItemList();
        return;
    }

    WeaponInstance *wi = selected->weapon(selectedWInstId_ - 1);

    // if weapon is researched it can be bought
    if (g_gameCtrl.weaponManager().isAvailable(wi->getClass())) {
        selectedWInstId_ = 0;
        getOption(sellButId_)->setVisible(false);
        getOption(purchaseButId_)->setVisible(true);
        if (wi->needsReloading()) {
            getOption(reloadButId_)->setVisible(false);
        }
        getOption(cancelButId_)->setVisible(true);
    } else {
        tab_ = TAB_EQUIPS;
        showItemList();
    }
}

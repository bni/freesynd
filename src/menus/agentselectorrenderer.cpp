/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2013  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "menus/agentselectorrenderer.h"
#include "gfx/screen.h"
#include "app.h"
#include "ped.h"

const int AgentSelectorRenderer::kIpaBarWidth = 56;
const int AgentSelectorRenderer::kIpaBarHeight = 10;
const int AgentSelectorRenderer::kIpaBarLeft13 = 4;
const int AgentSelectorRenderer::kIpaBarLeft24 = 68;
const int AgentSelectorRenderer::kIpaBarTop12  = 48;
const int AgentSelectorRenderer::kIpaBarTop34  = 148;
const int AgentSelectorRenderer::kIpaYOffset = 14;

/*!
 *
 */
bool AgentSelectorRenderer::hasClickedOnAgentSelector(int x, int y, SelectorEvent & evt) {
    evt.eventType = SelectorEvent::kNone;
    if (y < 46) {
        evt.eventType = SelectorEvent::kSelectAgent;
        if (x < 64) {
            evt.agentSlot = 0;
        } else {
            evt.agentSlot = 1;
        }
    }

    else if (y >= 42 + 48 + 10 && y < 42 + 48 + 10 + 46) {
        evt.eventType = SelectorEvent::kSelectAgent;
        if (x < 64) {
            evt.agentSlot = 2;
        } else {
            evt.agentSlot = 3;
        }
    }
    else
    {
        scanCoordsForIPA(x,y, evt);
    }

    return evt.eventType != SelectorEvent::kNone;
}

void AgentSelectorRenderer::scanCoordsForIPA(int x, int y, SelectorEvent & evt)
{
    IPAStim::IPAType types[] = {IPAStim::Adrenaline, IPAStim::Perception, IPAStim::Intelligence};
    // For each agent
    for(size_t a = 0; a < AgentManager::kMaxSlot; ++a)
    {
        int barLeft = getIpaBarLeftForAgent(a);
        if(x >= barLeft && x <= barLeft + kIpaBarWidth)
        {
            // For each type of IPA
            for (int i = 0; i < 3; ++i) {
                int barTop = getIpaBarTop(a, types[i]);
                if( y >= barTop && y <= barTop + kIpaBarHeight) {
                    evt.eventType = SelectorEvent::kSelectIpa;
                    evt.agentSlot = a;
                    evt.IpaType = types[i];
                    evt.percentage = getPercentage(barLeft, x);
                    return;
                }
            }
        }
    }
}

/*!
 * Draws all the elements
 * for one bar
 */
void AgentSelectorRenderer::drawIPABar(int agent, IPAStim *stim)
{
    // Convert those percentages to pixels
    int amount_x = (float)kIpaBarWidth * ((float)stim->getAmount()/100.0);
    int effect_x = (float)kIpaBarWidth * ((float)stim->getEffect()/100.0);
    int dependency_x = (float)kIpaBarWidth * ((float)stim->getDependency()/100.0);

    IPAStim::IPAType type = stim->getType();
    
    // Draw a bar between the current level and the dependency marker
    // x needs to be leftmost...
    int left, width;
    boxify(left, width, amount_x, dependency_x);
    if(width > 0) {
        g_Screen.drawRect(getIpaBarLeftForAgent(agent) + left,
                          getIpaBarTop(agent, type),
                          width, kIpaBarHeight, colourForIpaType(type));
    }
    
    // NB: this bar stops rendering when it's neck-a-neck with 'amount'
    if(amount_x != effect_x)
    {
        boxify(left, width, effect_x, dependency_x);
        if(width > 0) {
            g_Screen.drawRect(getIpaBarLeftForAgent(agent) + left,
                              getIpaBarTop(agent, type),
                              width, kIpaBarHeight, dim_colour(type));
        }
    }
    
    // Draw a vertical white line to mark the dependency level
    g_Screen.drawVLine(getIpaBarLeftForAgent(agent) + dependency_x,
                      getIpaBarTop(agent, type), kIpaBarHeight, 12);
}

/*!
 * Draw the complete selector for an agent.
 * \param agentSlot
 * \param pAgent
 * \param isSelected
 */
void AgentSelectorRenderer::drawSelectorForAgent(size_t agentSlot,
    PedInstance *pAgent, bool isSelected)
{
    // parity check
    int topX = (agentSlot & 0x01) * 64;
    // 2,3 should be drawn at (46 + 44 + 10)
    int topY = (agentSlot >> 1) * (46 + 44 + 10);
    int spriteSelected = 1772 + agentSlot;
    int springUnselected = 1748 + (agentSlot > 1 ? agentSlot + 2 : agentSlot);

    // Draw the background of selector
    g_App.gameSprites().sprite(isSelected ? spriteSelected : springUnselected)->draw(
            topX, topY, 0);
    g_App.gameSprites().sprite(isSelected ? 1778 : 1754)->draw(
            topX, topY + 46, 0);

    if (pAgent) {
        // draw health bar
        int ydiff = 36 * pAgent->health() / pAgent->startHealth();
        g_Screen.drawRect(topX + 51,
            topY + 6 + 36 - ydiff, 7, ydiff, 12);

        //draw animation within selectors
        pAgent->drawSelectorAnim(topX + 32, topY + 38);

        // draw IPA, for alive only agents
        if (pAgent->isAlive()) {
            drawIPABar(agentSlot, pAgent->adrenaline_);
            drawIPABar(agentSlot, pAgent->perception_);
            drawIPABar(agentSlot, pAgent->intelligence_);
        }
    }
}

/*!
 * Draw all elements for the agent selectors.
 */
void AgentSelectorRenderer::render(SquadSelection & selection, Squad * pSquad) {
    for (size_t a = 0; a < AgentManager::kMaxSlot; a++) {
        PedInstance * pAgent = pSquad->member(a);
        drawSelectorForAgent(a, pAgent, selection.isAgentSelected(a));
    }
}

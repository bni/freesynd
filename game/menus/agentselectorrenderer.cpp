/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2013, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#include "menus/agentselectorrenderer.h"

#include "fs-engine/system/system.h"
#include "fs-engine/gfx/animationmanager.h"
#include "fs-kernel/model/ped.h"

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
    for(size_t a = 0; a < fs_knl::Squad::kMaxSlot; ++a)
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
 * Returns the color for drawing the IPA bar of given type.
 */
fs_eng::FSColor AgentSelectorRenderer::colourForIpaType(IPAStim::IPAType type, const fs_eng::Palette &palette)
{
    switch (type)
    {
        case IPAStim::Adrenaline:
            return palette[fs_eng::kPaletteGameColorLightRed];
        case IPAStim::Perception:
            return palette[fs_eng::kPaletteGameColorBlue];
        case IPAStim::Intelligence:
            return palette[fs_eng::kPaletteGameColorLightBrown];
        default:
            assert(false);
    }
    return palette[fs_eng::kPaletteGameColorBlack];
}

/*!
 * @brief 
 * @param type 
 * @return 
 */
fs_eng::FSColor AgentSelectorRenderer::dim_colour(IPAStim::IPAType type, const fs_eng::Palette &palette)
{
    switch (type)
    {
        case IPAStim::Adrenaline:
            return palette[fs_eng::kPaletteGameColorDarkRed];
        case IPAStim::Perception:
            return palette[fs_eng::kPaletteGameColorBlueGrey];
        case IPAStim::Intelligence:
            return palette[fs_eng::kPaletteGameColorDarkBrown];
        default:
            assert(false);
    }
    return palette[fs_eng::kPaletteGameColorBlack];
}

/*!
 * Draws all the elements
 * for one bar
 */
void AgentSelectorRenderer::drawIPABar(size_t agent, IPAStim *stim, const fs_eng::Palette &palette)
{
    // Convert those percentages to pixels
    int amount_x = (float)kIpaBarWidth * ((float)stim->getAmount()/100.0);
    int effect_x = (float)kIpaBarWidth * ((float)stim->getEffect()/100.0);
    int dependency_x = (float)kIpaBarWidth * ((float)stim->getDependency()/100.0);

    IPAStim::IPAType type = stim->getType();

    // Draw a bar between the current level and the dependency marker
    // x needs to be leftmost...
    int left, width;
    fs_utl::boxify(left, width, amount_x, dependency_x);
    if(width > 0) {
        g_System.drawFillRect({getIpaBarLeftForAgent(agent) + left,
                          getIpaBarTop(agent, type)},
                          width, kIpaBarHeight, colourForIpaType(type, palette));
    }

    // NB: this bar stops rendering when it's neck-a-neck with 'amount'
    if(amount_x != effect_x)
    {
        fs_utl::boxify(left, width, effect_x, dependency_x);
        if(width > 0) {
            g_System.drawFillRect({getIpaBarLeftForAgent(agent) + left,
                              getIpaBarTop(agent, type)},
                              width, kIpaBarHeight, dim_colour(type, palette));
        }
    }

    // Draw a vertical white line to mark the dependency level
    g_System.drawVLine({getIpaBarLeftForAgent(agent) + dependency_x,
                      getIpaBarTop(agent, type)}, kIpaBarHeight, palette[fs_eng::kPaletteGameColorWhite]);
}

/*!
 * Draw the complete selector for an agent.
 * \param agentSlot
 * \param pAgent
 * \param isSelected
 */
void AgentSelectorRenderer::drawSelectorForAgent(size_t agentSlot,
    fs_knl::PedInstance *pAgent, bool isSelected, const fs_eng::Palette &palette)
{
    // parity check
    int topX = (agentSlot & 0x01) * 64;
    // 2,3 should be drawn at (46 + 44 + 10)
    int topY = (agentSlot >> 1) * (46 + 44 + 10);
    int spriteSelected = 1772 + agentSlot;
    int springUnselected = 1748 + (agentSlot > 1 ? agentSlot + 2 : agentSlot);

    // Draw the background of selector
    g_SpriteMgr.drawSprite(isSelected ? spriteSelected : springUnselected, {topX, topY});
    g_SpriteMgr.drawSprite(isSelected ? 1778 : 1754, {topX, topY + 46});

    if (pAgent) {
        // draw health bar
        int ydiff = 36 * pAgent->health() / pAgent->startHealth();
        g_System.drawFillRect({topX + 51,
            topY + 6 + 36 - ydiff}, 7, ydiff, palette[fs_eng::kPaletteGameColorWhite]);

        //draw animation within selectors
        Point2D top = {topX + 32, topY + 38};
        pAgent->drawSelectorAnim(top);

        // draw IPA, for alive only agents
        if (pAgent->isAlive()) {
            drawIPABar(agentSlot, pAgent->adrenaline_, palette);
            drawIPABar(agentSlot, pAgent->perception_, palette);
            drawIPABar(agentSlot, pAgent->intelligence_, palette);
        }
    }
}

/*!
 * Draw all elements for the agent selectors.
 */
void AgentSelectorRenderer::render(SquadSelection & selection, fs_knl::Squad * pSquad, const fs_eng::Palette &palette) {
    for (size_t a = 0; a < fs_knl::Squad::kMaxSlot; a++) {
        fs_knl::PedInstance * pAgent = pSquad->member(a);
        drawSelectorForAgent(a, pAgent, selection.isAgentSelected(a), palette);
    }
}

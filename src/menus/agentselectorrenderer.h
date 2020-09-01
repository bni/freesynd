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

#ifndef MENUS_AGENTSELECTORRENDERER_H_
#define MENUS_AGENTSELECTORRENDERER_H_

#include "ipastim.h"
#include "menus/squadselection.h"

/*!
 * The SelectorEvent class is the event that is used
 * to tell where the player has clicked on the control panel.
 */
class SelectorEvent {
public:
    /*!
     * This enumeration lists the different types of event.
     */
    enum EType {
        kNone,
        kSelectAgent,
        kSelectIpa
    };

    /*! The type of the event.*/
    EType eventType;
    /*! What agent was clicked.*/
    size_t agentSlot;
    /*! The type of IPA selected for eventType = kSelectIpa.*/
    IPAStim::IPAType IpaType;
    /*! for eventType = kSelectIpa.*/
    int percentage;
};

/*!
 * This renderer manages the drawing of agent selector on the control panel
 * in the gameplay menu.
 */
class AgentSelectorRenderer
{
public:
    //! Check if player has clicked on a agent selector
    bool hasClickedOnAgentSelector(int x, int y, SelectorEvent & evt);
    //! Renders the agent's selectors
    void render(SquadSelection & selection, Squad * pSquad);

private:
    static const int kIpaBarWidth;
    static const int kIpaBarHeight;
    static const int kIpaBarLeft13;
    static const int kIpaBarLeft24;
    static const int kIpaBarTop12;
    static const int kIpaBarTop34;
    static const int kIpaYOffset;

    /*!
     * Checks if the player has clicked on an IPA bar for an agent.
     * \return NULL if the player didn't click on a IPA meter.
     */
    void scanCoordsForIPA(int x, int y, SelectorEvent & evt);

    //! Draws all the elements for one bar
    void drawIPABar(int agent, IPAStim *stim);

    /*!
     * Returns the Y coord of the top left corner of the IPA bar for a given agent slot and IPA Type.
     */
    int getIpaBarTop(size_t agent, IPAStim::IPAType type)
    {
        int top;
        if(agent > 1)
            top = kIpaBarTop34;
        else
            top = kIpaBarTop12;
        
        switch (type) {
            case IPAStim::Adrenaline:
                break;
            case IPAStim::Perception:
                top += kIpaYOffset;
                break;
            case IPAStim::Intelligence:
                top += 2 * kIpaYOffset;
                break;
            default:
                assert(false);
        }
        return top;
    }
    
    /*!
     * Returns the X coord for all IPA bars for a given agent slot.
     */
    int getIpaBarLeftForAgent(size_t agent)
    {
        return (agent % 2 == 1) ? kIpaBarLeft24 : kIpaBarLeft13;
    }
    
    /*!
     * Returns the color for drawing the IPA bar of given type.
     */
    int colourForIpaType(IPAStim::IPAType type)
    {
        switch (type)
        {
            case IPAStim::Adrenaline:
                return fs_cmn::kColorLightRed;
            case IPAStim::Perception:
                return fs_cmn::kColorBlue;
            case IPAStim::Intelligence:
                return fs_cmn::kColorLightBrown;
            default:
                assert(false);
        }
        return -1;
    }

    int dim_colour(IPAStim::IPAType type)
    {
        switch (type)
        {
            case IPAStim::Adrenaline:
                return fs_cmn::kColorDarkRed;
            case IPAStim::Perception:
                return fs_cmn::kColorBlueGrey;
            case IPAStim::Intelligence:
                return fs_cmn::kColorDarkBrown;
            default:
                assert(false);
        }
        return -1;
    }
    
    
    /*!
     *
     */
    int getPercentage(int left, int x)
    {
        int offset = x - left;
        return (int)(((float)offset / (float) kIpaBarWidth) * 100.0);
    }

    void drawSelectorForAgent(size_t agentSlot, PedInstance *pAgent, bool isSelected);

public:
    //! gets percentage for any x coord
    int getPercentageAnyX(size_t agent, int x) {
        int barLeft = getIpaBarLeftForAgent(agent);
        if (x < barLeft)
            x = barLeft;
        if (x > barLeft + kIpaBarWidth)
            x = barLeft + kIpaBarWidth;

        return getPercentage(barLeft, x);
    }
};

#endif  // MENUS_AGENTSELECTORRENDERER_H_

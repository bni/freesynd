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

#ifndef BRIEFMENU_H
#define BRIEFMENU_H

#include "menus/minimaprenderer.h"

class MissionBriefing;

/*!
 * Brief Menu class. 
 */
class BriefMenu : public Menu {
public:
    BriefMenu(MenuManager *m);
    ~BriefMenu();

    void handleTick(int elapsed);
    void handleShow();
    void handleRender(DirtyList &dirtyList);
    void handleLeave();
    void handleAction(const int actionId, void *ctx, const int modKeys);
    
protected:
    /*! Origin of the minimap on the screen.*/
    static const int kMiniMapScreenX;
    /*! Origin of the minimap on the screen.*/
    static const int kMiniMapScreenY;
    static const int kMiniMapWidth;
    static const int kMiniMapHeight;
    /*! Number of line on a briefing page.*/
    static const int kMaxLinePerPage;

    void updateClock();
    void redrawMiniMap();
    void redrawBriefing();

    bool handleMouseDown(int x, int y, int button, const int modKeys);
    //! Return the zoom level from the enhancement level
    MinimapRenderer::EZoom toZoomLevel(uint8 enh_lvl);

    //! Renders the current page of briefing
    void render_briefing_text();
    //! Update the current briefing page
    void update_briefing_text();
    //! Convenience methode to read a word in the briefing text
    bool read_next_word(std::string & brief, std::string & line);

protected:
    /*!
     * Index of the first line in the briefing text to display
     */
    int start_line_;
    /*!
     * This array stores only the lines that are currently displayed.
     */
    std::string *a_page_;
    /*! Id of the text widget for time.*/
    int txtTimeId_;
    /*! Id of the text widget for money.*/
    int txtMoneyId_;
    /*! Id of the text widget for info.*/
    int txtInfoId_;
    /*! Id of the text widget for enhance.*/
    int txtEnhId_;
    /*! Id of the infos button.*/
    int infosButId_;
    /*! Id of the enhance button.*/
    int enhButId_;
    /*! Id of the scroll next button.*/
    int nextButId_;
    /*! Id of the scroll previous button.*/
    int prevButId_;
    /*! Briefing informations on current mission.*/
    MissionBriefing *p_briefing_;
    /*! Class that draws the minimap.*/
    BriefMinimapRenderer mm_renderer_;
};

#endif

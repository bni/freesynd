/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#ifndef KERNEL_SFXOBJECT_H
#define KERNEL_SFXOBJECT_H

#include "fs-kernel/model/mapobject.h"
#include "fs-kernel/model/map.h"

/*!
 * SFXObject map object class.
 */
class SFXObject : public MapObject {
public:
    /*!
     * Type of SfxObject.
     */
    enum SfxTypeEnum {
        sfxt_Unknown = 0,
        sfxt_BulletHit = 1,
        sfxt_FlamerFire = 2,
        sfxt_Smoke = 3,
        sfxt_Fire_LongSmoke = 4,
        sfxt_ExplosionFire = 5,
        sfxt_ExplosionBall = 6,
        sfxt_LargeFire = 7,
        sfxt_SelArrow = 8,
        sfxt_AgentFirst = 9,
        sfxt_AgentSecond = 10,
        sfxt_AgentThird = 11,
        sfxt_AgentFourth = 12
    };

    SFXObject(Map *pMap, SfxTypeEnum type, bool drawable = true, int t_show = 0, bool managed = false);
    virtual ~SFXObject() {}

    bool sfxLifeOver() { return sfx_life_over_; }
    //! Return true if object is managed by another object
    bool isManaged() { return managed_; }
    //! Set whether animation should loop or not
    void setLoopAnimation(bool flag) { loopAnimation_ = flag; }
    //! Reset animation
    void reset();

    void draw(const Point2D &screenPos) override;
    bool animate(int elapsed) override;
    //!
    void correctZ(int mapMaxZ);
    void setDrawAllFrames(bool daf) {
        if (daf != draw_all_frames_) {
            draw_all_frames_ = daf;
            frame_ = 0;
        }
    }
protected:
    static uint16 sfxIdCnt;
    /*! The type of SfxObject.*/
    SfxTypeEnum type_;
    int anim_;
    bool sfx_life_over_;
    // to draw all frames or first frame only
    bool draw_all_frames_;
    //! Tells if the animation should restart automatically after ending
    bool loopAnimation_;
    int elapsed_left_;
    //! True means the life of the object is managed by something else than gameplaymenu
    bool managed_;
};

#endif  //KERNEL_SFXOBJECT_H

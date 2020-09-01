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

#ifndef MODEL_SHOT_H_
#define MODEL_SHOT_H_

#include <vector>
#include <list>

#include "mapobject.h"

class Mission;
class WeaponInstance;
class PedInstance;

/*!
 * A shot is the result of the action of a weapon.
 * It's the shot that inflicts damage.
 */
class Shot {
 public:
    explicit Shot(const fs_dmg::DamageToInflict &dmg) {
        dmg_ = dmg;
    }
    virtual ~Shot() {}

    virtual void inflictDamage(Mission *pMission) = 0;

    fs_dmg::DamageToInflict & getAttributes() { return dmg_; }
 protected:
    //! The damage that will be inflicted by this shot
    fs_dmg::DamageToInflict dmg_;
};

/*!
 * This class represents a shot where the impacts are immediatly
 * made. Weapons who use this type of shot are pistol, minigun, Shotgun,
 * uzi, laser, long range.
 */
class InstantImpactShot : public Shot {
 public:
    explicit InstantImpactShot(
        const fs_dmg::DamageToInflict &dmg) : Shot(dmg) {}

    void inflictDamage(Mission *pMission);
 private:
    //! Spread the impact on the ground
    void diffuseImpact(Mission *m, const WorldPoint &originLocW,
                       WorldPoint *pImpactPosW);

    //!
    void createImpactAnimation(Mission *pMission,
                               ShootableMapObject * pTargetHit,
                               const WorldPoint impactPosW);
};

/*!
 * This class represents an explosion caused by a time bomb, an agent's
 * suicide or a car explosion.
 */
class Explosion : public Shot {
 public:
    //! Creates an explosion at the location of owner
    static void createExplosion(Mission *pMission, ShootableMapObject *pOwner,
                                double range, int dmgValue = 16);
    //! Creates an explosion at the given location
    static void createExplosion(Mission *pMission, ShootableMapObject *pOwner,
                                const WorldPoint &location, double range,
                                int dmgValue);

    explicit Explosion(const fs_dmg::DamageToInflict &dmg);

    void inflictDamage(Mission *pMission);
 private:
    void generateFlameWaves(Mission *pMission, WorldPoint *pOrigin, double dmg_rng);
    //!
    void getAllShootablesWithinRange(
        Mission *pMission,
        const WorldPoint &originLocW,
        std::vector<ShootableMapObject *> &objInRangeLst);

 private:
    /*! Type of animation for the explosion.*/
    SFXObject::SfxTypeEnum rngDmgAnim_;
};

/*!
 * Base class of shots whose path is drawn.
 */
class ProjectileShot: public Shot {
 public:
    //! Constructor
    explicit ProjectileShot(const fs_dmg::DamageToInflict &dmg);

    //! Animate the shot
    virtual bool animate(int elapsed, Mission *m);

    //! Returns true if shot can be destroyed
    bool isLifeOver() { return lifeOver_; }

 protected:
    //! Update projectile position
    virtual bool moveProjectile(int elapsed, Mission *pMission);
    virtual void drawTrace(Mission *pMission) = 0;
 protected:
    /*! This tells if the shot object shot be destroyed.*/
    bool lifeOver_;
    int elapsed_;
    /*! Projectile speed.*/
    double speed_;

    /*! Current position of projectile.*/
    WorldPoint curPosW_;
    /*! Position of the target.*/
    WorldPoint targetLocW_;

    /*! Projectile position incrementation on X axis.*/
    double incX_;
    /*! Projectile position incrementation on Y axis.*/
    double incY_;
    /*! Projectile position incrementation on Z axis.*/
    double incZ_;

    /*! Maximum distance the projectile can go.*/
    double distanceMax_;
    /*! Updated distance from origin to current projectile's position.*/
    double currentDistance_;
    /*! flag to know if we can draw the impact of the shot.*/
    bool drawImpact_;
    /*! Not null if the projectile has to touch smth destroyable.*/
    ShootableMapObject * pShootableHit_;
};

/*!
 * Shot made by Gauss Gun.
 * The Gauss Gun shoots a projectile that flies until it hist its target
 * then it explodes burning everything around.
 * When flying, the projectile leaves a trace of smoke behind it.
 */
class GaussGunShot: public ProjectileShot {
 public:
    //! Constructor
    explicit GaussGunShot(const fs_dmg::DamageToInflict &dmg);
    //! Destructor
    ~GaussGunShot() {}

    void inflictDamage(Mission *pMission);
 protected:
    //! Update projectile position
    void drawTrace(Mission *pMission);
 protected:
    /*! Position of the last trace animation.*/
    double lastAnimDist_;
};

/*!
 * Shot made with the flame thrower.
 */
class FlamerShot: public ProjectileShot {
 public:
    //! Constructor
    explicit FlamerShot(Mission *pMission, const fs_dmg::DamageToInflict &dmg);
    //! Desctructor
    ~FlamerShot();

    void inflictDamage(Mission *pMission);

 protected:
    void drawTrace(Mission *pMission);
 protected:
    //! The flame that represents the shot
    SFXObject *pFlame_;
};

#endif  // MODEL_SHOT_H_

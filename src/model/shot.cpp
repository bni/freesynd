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

#include <map>

#include "app.h"
#include "model/shot.h"
#include "mission.h"
#include "ped.h"
#include "vehicle.h"

void InstantImpactShot::inflictDamage(Mission *pMission) {
    WorldPoint originLocW(dmg_.d_owner->position()); // origin of shooting
    /*printf("Origin loc %d %d %d\n", originLocW.x, originLocW.y, originLocW.z);*/
    // get how much impacts does the weapon generate
    int nbImpacts = dmg_.pWeapon->getClass()->impactsPerAmmo();

    // If there are many impacts, a target can be hit by several impacts
    // so this map stores number of impacts for a target
    std::map<ShootableMapObject *, int> hitsByObject;

    for (int i = 0; i < nbImpacts; ++i) {
        WorldPoint impactPosW = dmg_.aimedLocW;

        if (nbImpacts > 1) {
            // When multiple impacts, they're spread
            diffuseImpact(pMission, originLocW, &impactPosW);
            /*if (impactPosW.z < originLocW.z) {
                printf("Impact %d below Z\n", i);
            }
            printf("Impact %d apres diffuse %d %d %d\n", i, impactPosW.x, impactPosW.y, impactPosW.z);*/
        }

        // Verify if shot hit something or was blocked by a tile
        ShootableMapObject *pTargetHit = NULL;
        pMission->checkIfBlockersInShootingLine(
            originLocW, &pTargetHit, &impactPosW, true, false, dmg_.pWeapon->range(), NULL, dmg_.d_owner);
        /*printf("Impact %d apres checkIfBlockers %d %d %d\n", i, impactPosW.x, impactPosW.y, impactPosW.z);*/

        if (pTargetHit != NULL) {
            hitsByObject[pTargetHit] = hitsByObject[pTargetHit] + 1;
        }
        // creates impact sprite
        createImpactAnimation(pMission, pTargetHit, impactPosW);
    }

    // finally distribute damage
    std::map<ShootableMapObject *, int>::iterator it;
    for (it = hitsByObject.begin(); it != hitsByObject.end(); it++) {
        dmg_.dvalue = (*it).second * dmg_.pWeapon->getClass()->damagePerShot();
        (*it).first->handleHit(dmg_);
    }
}

void InstantImpactShot::diffuseImpact(Mission *pMission, const WorldPoint &originLocW, WorldPoint *pImpactPosW) {
    double angle = dmg_.pWeapon->getClass()->shotAngle();
    if (angle == 0)
        return;

    angle *= (double)(69 + (rand() & 0x1F)) / 100.0;
    int cx = originLocW.x;
    int cy = originLocW.y;
    int cz = originLocW.z;

    int tx = pImpactPosW->x;
    int ty = pImpactPosW->y;
    int tz = pImpactPosW->z;
    double dtx = (double)(tx - cx);
    double dty = (double)(ty - cy);
    double dtz = (double)(tz - cz);
    double dist_cur = 0.0;
    dist_cur = sqrt(dtx * dtx + dty * dty + dtz * dtz);
    if (dist_cur == 0.0)
        return;

    angle /= (180.0 / PI);
    double angx = acos(dtx/dist_cur);
    double angy = acos(dty/dist_cur);
    double angz = acos(dtz/dist_cur);

    double set_sign = 1.0;
    if (rand() % 100 < 50)
        set_sign = -1.0;
    double diff_ang = (angle * (double)(rand() % 100) / 200.0) * set_sign;
    angx += diff_ang;
    angle -= fabs(diff_ang);
    int gtx = cx + (int)(cos(angx) * dist_cur);

    set_sign = 1.0;
    if (rand() % 100 < 50)
        set_sign = -1.0;
    diff_ang = (angle * (double)(rand() % 100) / 200.0) * set_sign;
    angy += diff_ang;
    angle -= fabs(diff_ang);
    int gty = cy + (int)(cos(angy) * dist_cur);

    set_sign = 1.0;
    if (rand() % 100 < 50)
        set_sign = -1.0;
    angz += (angle * (double)(rand() % 100) / 200.0) * set_sign;

    int gtz = cz + (int)(cos(angz) * dist_cur);

    if (gtx < 0) {
        if (cos(angx) == 0.0) {
            gtx = 0;
        } else {
            dist_cur -= fabs((double)gtx / cos(angx));
            gtx = 0;
            gty = cy + (int)(cos(angy) * dist_cur);
            gtz = cz + (int)(cos(angz) * dist_cur);
        }
    }
    if (gty < 0) {
        if (cos(angy) == 0.0) {
            gty = 0;
        } else {
            dist_cur -= fabs((double)gty / cos(angy));
            gty = 0;
            gtx = cx + (int)(cos(angx) * dist_cur);
            gtz = cz + (int)(cos(angz) * dist_cur);
        }
    }
    if (gtz < 0) {
        if (cos(angz) == 0.0) {
            gtz = 0;
        } else {
            dist_cur -= fabs((double)gtz / cos(angz));
            gtz = 0;
            gtx = cx + (int)(cos(angx) * dist_cur);
            gty = cy + (int)(cos(angy) * dist_cur);
        }
    }

    int max_x = (pMission->mmax_x_ - 1) * 256;
    int max_y = (pMission->mmax_y_ - 1) * 256;
    int max_z = (pMission->mmax_z_ - 1) * 128;
    if (gtx > max_x) {
        if (cos(angx) == 0.0) {
            gtx = max_x;
        } else {
            dist_cur -= fabs((double)(gtx - max_x) / cos(angx));
            gtx = max_x;
            gty = cy + (int)(cos(angy) * dist_cur);
            gtz = cz + (int)(cos(angz) * dist_cur);
        }
    }
    if (gty > max_y) {
        if (cos(angy) == 0.0) {
            gty = max_y;
        } else {
            dist_cur -= fabs((double)(gty - max_y) / cos(angy));
            gty = max_y;
            gtx = cx + (int)(cos(angx) * dist_cur);
            gtz = cz + (int)(cos(angz) * dist_cur);
        }
    }
    if (gtz > max_z) {
        if (cos(angx) == 0.0) {
            gtz = max_z;
        } else {
            dist_cur -= fabs((double)(gtz - max_z) / cos(angz));
            gtz = max_z;
            gtx = cx + (int)(cos(angx) * dist_cur);
            gty = cy + (int)(cos(angy) * dist_cur);
        }
    }
    assert(gtx >= 0 && gty >= 0 && gtz >= 0);
    assert(gtx <= max_x && gty <= max_y && gtz <= max_z);

    pImpactPosW->x = gtx;
    pImpactPosW->y = gty;
    pImpactPosW->z = gtz;
}

/*!
 * Creates the impact animation based on the type of target hit.
 * Some weapons can have no animations for an impact.
 */
void InstantImpactShot::createImpactAnimation(Mission *pMission, ShootableMapObject * pTargetHit, const WorldPoint impactPosW) {
    SFXObject::SfxTypeEnum impactAnimId =
        (pTargetHit != NULL ?
            dmg_.pWeapon->getClass()->impactAnims()->objectHit :
            dmg_.pWeapon->getClass()->impactAnims()->groundHit);

    if (impactAnimId != SFXObject::sfxt_Unknown) {
        SFXObject *so = new SFXObject(pMission->map(), impactAnimId);
        so->setPosition(impactPosW);
        so->correctZ();
        pMission->addSfxObject(so);
    }
}

/*!
 * Convenient method to create explosions.
 * \param pMission Mission data
 * \param pOwner What's at the origin of the explosion
 * \param The range for damage
 * \param The value of the damage
 */
void Explosion::createExplosion(Mission *pMission, ShootableMapObject *pOwner, double range, int dmgValue) {
    WorldPoint location(pOwner->position());

    createExplosion(pMission, pOwner, location, range, dmgValue);
}

void Explosion::createExplosion(Mission *pMission, ShootableMapObject *pOwner, const WorldPoint &location, double range, int dmgValue) {
    fs_dmg::DamageToInflict dmg;
    if (pOwner && pOwner->is(MapObject::kNatureWeapon)) {
        // It's a bomb that exploded (other waepons do not explode)
        dmg.pWeapon = dynamic_cast<WeaponInstance *>(pOwner);
    } else {
        dmg.pWeapon = NULL;
    }

    dmg.d_owner = pOwner;
    dmg.dtype = fs_dmg::kDmgTypeExplosion;
    dmg.range = range;
    dmg.dvalue =  dmgValue;
    dmg.originLocW = location;
    dmg.originLocW.z += 8;

    Explosion explosion(dmg);
    explosion.inflictDamage(pMission);
}

Explosion::Explosion(const fs_dmg::DamageToInflict &dmg) : Shot(dmg) {
    // GaussGun has a different animation for explosion
    if (dmg_.pWeapon && dmg_.pWeapon->isInstanceOf(Weapon::GaussGun)) {
        rngDmgAnim_ = SFXObject::sfxt_LargeFire;
    } else {
        rngDmgAnim_ = SFXObject::sfxt_ExplosionFire;
    }
};

/*!
 *
 */
void Explosion::inflictDamage(Mission *pMission) {
    std::vector<ShootableMapObject *> objInRangeLst;
    // Get all destructible objects in range
    getAllShootablesWithinRange(pMission, dmg_.originLocW, objInRangeLst);

    for (std::vector<ShootableMapObject *>::iterator it = objInRangeLst.begin();
        it != objInRangeLst.end(); it++)
    {
        ShootableMapObject *smo = *it;
        // distribute damage
        smo->handleHit(dmg_);
        // draw a explosion ball above each object that was hit
        SFXObject *so = new SFXObject(pMission->map(), SFXObject::sfxt_ExplosionBall);
        so->setPosition(smo->tileX(), smo->tileY(), smo->tileZ(), smo->offX(),
            smo->offY(), smo->offZ());
        so->correctZ();
        pMission->addSfxObject(so);
    }
    // create the ring of fire around the origin of explosion
    generateFlameWaves(pMission, &(dmg_.originLocW), dmg_.range);

    g_App.gameSounds().play(snd::EXPLOSION_BIG);
}

/*! Draws animation of impact/explosion
 * \param pMission Mission data
 * \param pOrigin center of explosion
 * \param dmg_rng effective range for drawing
 */
void Explosion::generateFlameWaves(Mission *pMission, WorldPoint *pOrigin, double dmg_rng)
{
    WorldPoint base_pos = *pOrigin;
    pOrigin->z += 4;
    if (pOrigin->z > (pMission->mmax_z_ - 1) * 128)
        pOrigin->z = (pMission->mmax_z_ - 1) * 128;
    // TODO: exclude flames on water, put these flames to the ground,
    // don't draw in air(, stairs problem?)
    double angle_inc = PI;
    const uint8 waves = (int)dmg_rng / 144 + 1;

    for (uint8 i = 0; i < waves; i++) {
        double base_angle = 0.0;
        if (rand() % 100 > 74)
            base_angle += angle_inc;

        for (int j = 0; j < (4 << i); j++) {
            double x = (double)(144 * i) * cos(base_angle);
            double y = (double)(144 * i) * sin(base_angle);
            base_angle += angle_inc;
            WorldPoint flamePosW;
            flamePosW.x = base_pos.x + (int)x;
            flamePosW.y = base_pos.y + (int)y;
            flamePosW.z = base_pos.z;

            uint8 block_mask = pMission->checkBlockedByTile(*pOrigin, &flamePosW, true, dmg_rng);
            if (block_mask != 32) {
                SFXObject *so = new SFXObject(pMission->map(), rngDmgAnim_,
                                100 * (rand() % 16));
                so->setPosition(flamePosW);
                pMission->addSfxObject(so);
            }
        }
        angle_inc /= 2.0;
    }
}

/*!
 * Returns all ShootableMapObject that are alive and in range of
 * weapon who generated this explosion.
 * Every object found is added to the objInRangeVec vector.
 * \param pMission Mission data
 * \param originLocW Origin of explosion
 * \param objInRangeVec Result list
 */
void Explosion::getAllShootablesWithinRange(Mission *pMission,
                                       const WorldPoint &originLocW,
                                       std::vector<ShootableMapObject *> &objInRangeVec) {
    // Look at all peds alive, in range of explosion and not in a vehicle
    for (size_t i = 0; i < pMission->numPeds(); ++i) {
        PedInstance *p = pMission->ped(i);
        if (p->isAlive() && p->isCloseTo(originLocW, dmg_.range) && p->inVehicle() == NULL) {
            WorldPoint pedPosW(p->position());
            if (pMission->checkBlockedByTile(originLocW, &pedPosW, false, dmg_.range) == 1) {
                objInRangeVec.push_back(p);
            }
        }
    }

    for (size_t i = 0; i < pMission->numStatics(); ++i) {
        Static *st = pMission->statics(i);
        if (!st->isExcludedFromBlockers() && st->isAlive() && st->isCloseTo(originLocW, dmg_.range)) {
            WorldPoint staticPosW(st->position());
            if (pMission->checkBlockedByTile(originLocW, &staticPosW, false, dmg_.range) == 1) {
                objInRangeVec.push_back(st);
            }
        }
    }

    // look at all vehicles
    for (size_t i = 0; i < pMission->numVehicles(); ++i) {
        ShootableMapObject *v = pMission->vehicle(i);
        if (v->isAlive() && v->isCloseTo(originLocW, dmg_.range)) {
            WorldPoint vehiclePosW(v->position());
            if (pMission->checkBlockedByTile(originLocW, &vehiclePosW, false, dmg_.range) == 1) {
                objInRangeVec.push_back(v);
            }
        }
    }

    // look at all bombs on the ground except the weapon that generated the shot
    for (size_t i = 0; i < pMission->numWeaponsOnGround(); ++i) {
        WeaponInstance *w = pMission->weaponOnGround(i);
        if (w->isInstanceOf(Weapon::TimeBomb) && w != dmg_.pWeapon && !w->hasOwner() && w->isAlive()) {
            WorldPoint weaponPosW(w->position());
            if (pMission->checkBlockedByTile(originLocW, &weaponPosW, false, dmg_.range) == 1) {
                objInRangeVec.push_back(w);
            }
        }
    }
}

ProjectileShot::ProjectileShot(const fs_dmg::DamageToInflict &dmg) : Shot(dmg) {
    elapsed_ = -1;
    curPosW_ = dmg.originLocW;
    currentDistance_ = 0;
    lifeOver_ = false;
    drawImpact_ = false;
    pShootableHit_ = NULL;

    speed_ = dmg.pWeapon->getClass()->shotSpeed();
    // distance from origin of shoot to target on each axis
    targetLocW_.x = dmg.aimedLocW.x;
    targetLocW_.y = dmg.aimedLocW.y;
    targetLocW_.z = dmg.aimedLocW.z;

    double diffx = (double)(targetLocW_.x - curPosW_.x);
    double diffy = (double)(targetLocW_.y - curPosW_.y);
    double diffz = (double)(targetLocW_.z - curPosW_.z);

    double distanceToTarget  = sqrt(diffx * diffx + diffy * diffy + diffz * diffz);
    if (distanceToTarget != 0) {
        incX_ = diffx / distanceToTarget;
        incY_ = diffy / distanceToTarget;
        incZ_ = diffz / distanceToTarget;
    }

    distanceMax_ = dmg.pWeapon->getClass()->range();
    if (distanceToTarget < distanceMax_) {
        distanceMax_ = distanceToTarget;
    }
}

bool ProjectileShot::animate(int elapsed, Mission *pMission) {
    if (elapsed_ == -1) {
        // It's the first time the animate method is called since shot
        // was created : start counting
        elapsed_ = 0;
        return false;
    }

    if (moveProjectile(elapsed, pMission)) {
        inflictDamage(pMission);
    }

    return true;
}

/*!
 * Updates the projectile position and animation of trace.
 * \param elapsed Time elapsed since last frame.
 * \param pMission Mission data
 * \return True if projectile has reached its target or max distance.
 */
bool ProjectileShot::moveProjectile(int elapsed, Mission *pMission) {
    bool endMove = false;
    elapsed_ += elapsed;

    // Distance crossed in the elapsed time
    double inc_dist = speed_ * (double)elapsed / 1000;
    if ((currentDistance_ + inc_dist) > distanceMax_) {
        // Projectile reached the maximum distance
        if (currentDistance_ > distanceMax_) {
            currentDistance_ = distanceMax_;
        }
        inc_dist = distanceMax_ - currentDistance_;
        endMove = true;
    }

    // This is the distance after the move
    double nextDist = currentDistance_ + inc_dist;
    WorldPoint nextPosW; // This is the position of projectile after this move
    bool do_recalc = false;

    nextPosW.x = dmg_.originLocW.x + (int)(incX_ * nextDist);
    if (nextPosW.x < 0) {
        nextPosW.x = 0;
        endMove = true;
        do_recalc = true;
    } else if (nextPosW.x > (pMission->mmax_x_ - 1) * 256) {
        nextPosW.x = (pMission->mmax_x_ - 1) * 256;
        endMove = true;
        do_recalc = true;
    }
    if (do_recalc) {
        do_recalc = false;
        if (incX_ != 0) {
            nextDist = (double)(nextPosW.x - dmg_.originLocW.x) / incX_;
        }
    }

    nextPosW.y = dmg_.originLocW.y + (int)(incY_ * nextDist);
    if (nextPosW.y < 0) {
        nextPosW.y = 0;
        endMove = true;
        do_recalc = true;
    } else if (nextPosW.y > (pMission->mmax_y_ - 1) * 256) {
        nextPosW.y = (pMission->mmax_y_ - 1) * 256;
        endMove = true;
        do_recalc = true;
    }
    if (do_recalc) {
        do_recalc = false;
        if (incY_ != 0) {
            nextDist = (double)(nextPosW.y - dmg_.originLocW.y) / incY_;
            nextPosW.x = dmg_.originLocW.x + (int)(incX_ * nextDist);
        }
    }

    nextPosW.z = dmg_.originLocW.z + (int)(incZ_ * nextDist);
    if (nextPosW.z < 0) {
        nextPosW.z = 0;
        endMove = true;
        do_recalc = true;
    } else if (nextPosW.z > (pMission->mmax_z_ - 1) * 128) {
        nextPosW.z = (pMission->mmax_z_ - 1) * 128;
        endMove = true;
        do_recalc = true;
    }
    if (do_recalc) {
        if (incZ_ != 0) {
            nextDist = (double)(nextPosW.z - dmg_.originLocW.z) / incZ_;
            nextPosW.x = dmg_.originLocW.x + (int)(incX_ * nextDist);
            nextPosW.y = dmg_.originLocW.y + (int)(incY_ * nextDist);
        }
    }

    // maxr here is set to maximum that projectile can fly from its
    // current position
    uint8 block_mask = pMission->checkIfBlockersInShootingLine(
        curPosW_, &pShootableHit_, &nextPosW, true, false, distanceMax_ - currentDistance_, NULL, dmg_.d_owner);

    if (block_mask == 1) {
        // Projectile has reached initial target
        if (nextPosW.equals(targetLocW_)) {
            // we can stop the move and draw the explosion
            drawImpact_ = true;
            endMove = true;
        }
    } else if (block_mask == 32) {
        // projectile is out of map : do not draw explosion
        // not sure if necessary
        endMove = true;
    } else {
        // projectile has hit something
        drawImpact_ = true;
        endMove = true;
    }

    curPosW_ = nextPosW;
    currentDistance_ = nextDist;
    drawTrace(pMission);

    return endMove;
}

GaussGunShot::GaussGunShot(const fs_dmg::DamageToInflict &dmg) : ProjectileShot(dmg) {
    lastAnimDist_ = 0;
}

void GaussGunShot::inflictDamage(Mission *pMission) {
    lifeOver_ = true;

    if (drawImpact_) {
        int dmgRange = dmg_.pWeapon->getClass()->rangeDmg();
        Explosion::createExplosion(pMission, dmg_.pWeapon, curPosW_, dmgRange, dmg_.dvalue);
    }
}

/*!
 * Draws the animation of smoke behind the projectile.
 * \param pMission Mission data
 */
void GaussGunShot::drawTrace(Mission *pMission) {
    // distance between 2 animations
    double anim_d = 64;

    double diffx = (double) (dmg_.originLocW.x - curPosW_.x);
    double diffy = (double) (dmg_.originLocW.y - curPosW_.y);
    double diffz = (double) (dmg_.originLocW.z - curPosW_.z);
    double d = sqrt(diffx * diffx + diffy * diffy
        + diffz * diffz);

    if (d > lastAnimDist_) {
        int diff_dist = (int) ((d - lastAnimDist_) / anim_d);
        if (diff_dist != 0) {
            for (int i = 1; i <= diff_dist; i++) {
                WorldPoint t;
                lastAnimDist_ += anim_d;
                t.x = dmg_.originLocW.x + (int)(lastAnimDist_ * incX_);
                t.y = dmg_.originLocW.y + (int)(lastAnimDist_ * incY_);
                t.z = dmg_.originLocW.z + (int)(lastAnimDist_ * incZ_);

                t.z += 128;
                if (t.z > (pMission->mmax_z_ - 1) * 128)
                    t.z = (pMission->mmax_z_ - 1) * 128;

                SFXObject *so = new SFXObject(pMission->map(),
                    dmg_.pWeapon->getClass()->impactAnims()->trace_anim);
                so->setPosition(t);
                pMission->addSfxObject(so);
            }
        }
    }
}

FlamerShot::FlamerShot(Mission *pMission, const fs_dmg::DamageToInflict &dmg) :
        ProjectileShot(dmg) {
    // We create a SFXObjet that we keep in memory to updateits position
    pFlame_ = new SFXObject(pMission->map(),
                            dmg_.pWeapon->getClass()->impactAnims()->trace_anim);
    // The sfxObject will loop to keep it alive
    pFlame_->setLoopAnimation(true);
    pFlame_->setPosition(dmg.originLocW);
    pMission->addSfxObject(pFlame_);
}

FlamerShot::~FlamerShot() {
    if(pFlame_ != NULL) {
        delete pFlame_;
    }
}

/*!
 * Draws the animation of smoke behind the projectile.
 * \param pMission Mission data
 */
void FlamerShot::drawTrace(Mission *pMission) {

    pFlame_->setPosition(curPosW_);
}

void FlamerShot::inflictDamage(Mission *pMission) {
    lifeOver_ = true;
    // target was hit (or shot reached an end)  so we
    // can get rid of sfxobject
    // it will be destroyed by the GameplayMenu loop
    pFlame_->setLoopAnimation(false);
    pFlame_ = NULL;
    if (pShootableHit_ != NULL) {
        pShootableHit_->handleHit(dmg_);
    }
}


/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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
#include <string.h>
#include <assert.h>

#include "app.h"
#include "utils/log.h"
#include "weapon.h"
#include "ped.h"
#include "mission.h"
#include "model/shot.h"
#include "core/gamesession.h"

#define Z_SHIFT_TO_AIR   4

uint16 WeaponInstance::weaponIdCnt = 0;

Weapon::Weapon(WeaponType w_type, ConfigFile &conf)
{
    type_ = w_type;
    submittedToSearch_ = false;

    switch(w_type) {
        case Weapon::Pistol:
            idx_ = Weapon::Pistol_Anim;
            sample_ = snd::PISTOL;
            dmg_type_ = fs_dmg::kDmgTypeBullet;
            shot_property_ = Weapon::wspt_Pistol;
            impactAnims_.groundHit = SFXObject::sfxt_BulletHit;
            impactAnims_.objectHit = SFXObject::sfxt_BulletHit;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::Minigun:
            idx_ = Weapon::Minigun_Anim;
            sample_ = snd::MINIGUN;
            dmg_type_ = fs_dmg::kDmgTypeBullet;
            shot_property_ = Weapon::wspt_Minigun;
            impactAnims_.groundHit = SFXObject::sfxt_BulletHit;
            impactAnims_.objectHit = SFXObject::sfxt_BulletHit;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::Flamer:
            idx_ = Weapon::Flamer_Anim;
            sample_ = snd::FLAME;
            dmg_type_ = fs_dmg::kDmgTypeBurn;
            shot_property_ = Weapon::wspt_Flamer;
            impactAnims_.groundHit = SFXObject::sfxt_FlamerFire;
            impactAnims_.objectHit = SFXObject::sfxt_FlamerFire;
            impactAnims_.trace_anim = SFXObject::sfxt_FlamerFire;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::LongRange:
            idx_ = Weapon::LongRange_Anim;
            sample_ = snd::LONGRANGE;
            dmg_type_ = fs_dmg::kDmgTypeBullet;
            shot_property_ = Weapon::wspt_LongRange;
            impactAnims_.groundHit = SFXObject::sfxt_BulletHit;
            impactAnims_.objectHit = SFXObject::sfxt_BulletHit;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::EnergyShield:
            idx_ = Weapon::EnergyShield_Anim;
            sample_ = snd::NO_SOUND;
            dmg_type_ = fs_dmg::kDmgTypeNone;
            shot_property_ = Weapon::wspt_EnergyShield;
            impactAnims_.groundHit = SFXObject::sfxt_Unknown;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::Uzi:
            idx_ = Weapon::Uzi_Anim;
            sample_ = snd::UZI;
            dmg_type_ = fs_dmg::kDmgTypeBullet;
            shot_property_ = Weapon::wspt_Uzi;
            impactAnims_.groundHit = SFXObject::sfxt_BulletHit;
            impactAnims_.objectHit = SFXObject::sfxt_BulletHit;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::Laser:
            idx_ = Weapon::Laser_Anim;
            sample_ = snd::LASER;
            dmg_type_ = fs_dmg::kDmgTypeLaser;
            shot_property_ = Weapon::wspt_Laser;
            impactAnims_.groundHit = SFXObject::sfxt_Fire_LongSmoke;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::GaussGun:
            idx_ = Weapon::Gauss_Anim;
            sample_ = snd::GAUSSGUN;
            dmg_type_ = fs_dmg::kDmgTypeExplosion;
            shot_property_ = Weapon::wspt_GaussGun;
            impactAnims_.groundHit = SFXObject::sfxt_ExplosionFire;
            impactAnims_.objectHit = SFXObject::sfxt_ExplosionBall;
            impactAnims_.trace_anim = SFXObject::sfxt_Smoke;
            impactAnims_.rd_anim = SFXObject::sfxt_LargeFire;
            break;
        case Weapon::Shotgun:
            idx_ = Weapon::Shotgun_Anim;
            sample_ = snd::SHOTGUN;
            dmg_type_ = fs_dmg::kDmgTypeBullet;
            shot_property_ = Weapon::wspt_Shotgun;
            impactAnims_.groundHit = SFXObject::sfxt_BulletHit;
            impactAnims_.objectHit = SFXObject::sfxt_BulletHit;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::MediKit:
            idx_ = Weapon::Unarmed_Anim;
            sample_ = snd::NO_SOUND;
            dmg_type_ = fs_dmg::kDmgTypeNone;
            shot_property_ = Weapon::wspt_MediKit;
            impactAnims_.groundHit = SFXObject::sfxt_Unknown;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::Scanner:
            idx_ = Weapon::Unarmed_Anim;
            sample_ = snd::NO_SOUND;
            dmg_type_ = fs_dmg::kDmgTypeNone;
            shot_property_ = Weapon::wspt_Scanner;
            impactAnims_.groundHit = SFXObject::sfxt_Unknown;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::AccessCard:
            idx_ = Weapon::Unarmed_Anim;
            sample_ = snd::NO_SOUND;
            dmg_type_ = fs_dmg::kDmgTypeNone;
            shot_property_ = Weapon::wspt_AccessCard;
            impactAnims_.groundHit = SFXObject::sfxt_Unknown;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        case Weapon::TimeBomb:
            idx_ = Weapon::Unarmed_Anim;
            sample_ = snd::EXPLOSION;
            dmg_type_ = fs_dmg::kDmgTypeExplosion;
            shot_property_ = Weapon::wspt_TimeBomb;
            impactAnims_.groundHit = SFXObject::sfxt_ExplosionFire;
            impactAnims_.objectHit = SFXObject::sfxt_ExplosionBall;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_ExplosionFire;
            break;
        case Weapon::Persuadatron:
            idx_ = Weapon::Unarmed_Anim;
            sample_ = snd::PERSUADE;
            dmg_type_ = fs_dmg::kDmgTypePersuasion;
            shot_property_ = Weapon::wspt_Persuadatron;
            impactAnims_.groundHit = SFXObject::sfxt_Unknown;
            impactAnims_.objectHit = SFXObject::sfxt_Unknown;
            impactAnims_.trace_anim = SFXObject::sfxt_Unknown;
            impactAnims_.rd_anim = SFXObject::sfxt_Unknown;
            break;
        default:
#if _DEBUG
            printf("unknown weapon loaded(%i), NULL passed", w_type);
#endif
            break;
    }

    // initialize other properties
    initFromConfig(w_type, conf);
}

void Weapon::initFromConfig(WeaponType w_type, ConfigFile &conf) {
    const char *pattern = "weapon.%d.%s";
    char propName[25];

    try {
        sprintf(propName, pattern, w_type, "name");
        name_ = g_Ctx.getMessage(conf.read<std::string>(propName));

        sprintf(propName, pattern, w_type, "icon.small");
        small_icon_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "icon.big");
        big_icon_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "cost");
        cost_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "ammo.nb");
        ammo_ = conf.read<int>(propName, 0);
        sprintf(propName, pattern, w_type, "ammo.price");
        ammo_cost_ = conf.read<int>(propName, 0);
        sprintf(propName, pattern, w_type, "range");
        range_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "rank");
        rank_ = conf.read<int>(propName, -1);
        sprintf(propName, pattern, w_type, "anim");
        anim_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "ammopershot");
        ammo_per_shot_ = conf.read<int>(propName, 0);
        sprintf(propName, pattern, w_type, "timeforshot");
        time_for_shot_ = conf.read<int>(propName, 0);
        sprintf(propName, pattern, w_type, "timereload");
        time_reload_ = conf.read<int>(propName, 0);
        sprintf(propName, pattern, w_type, "damagerange");
        range_dmg_ = conf.read<int>(propName, 0);
        sprintf(propName, pattern, w_type, "shotangle");
        shot_angle_ = conf.read<double>(propName, 0.0);
        sprintf(propName, pattern, w_type, "shotaccuracy");
        shot_accuracy_ = conf.read<double>(propName, 0.0);
        sprintf(propName, pattern, w_type, "shotspeed");
        shot_speed_ = conf.read<int>(propName, 0);
        sprintf(propName, pattern, w_type, "dmg_per_shot");
        dmg_per_shot_ = conf.read<int>(propName, 0);
        sprintf(propName, pattern, w_type, "ammo.impactNb");
        impactsPerAmmo_ = conf.read<int>(propName, 0);
        sprintf(propName, pattern, w_type, "weight");
        weight_ = conf.read<int>(propName);
        sprintf(propName, pattern, w_type, "auto.fire_rate");
        fireRate_ = conf.read<int>(propName, 0);
    } catch (...) {
        FSERR(Log::k_FLG_GAME, "Weapon", "initFromConfig", ("Cannot load weapon %d : %s\n", w_type, propName))
    }
}

int Weapon::calculateReloadingCost(int remaingAmmo) {
    return (ammo_ - remaingAmmo) * ammo_cost_;
}

/*!
 * Creates a new instance of Weapon instance for the given weapon class.
 * \param pWeaponClass Class of weapon
 * \param Current amount of ammo. If value is -1, ammo is full
 * \return an instance of WeaponInstance
 */
WeaponInstance *WeaponInstance::createInstance(Weapon *pWeaponClass, int remainingAmmo) {
    return new WeaponInstance(pWeaponClass, weaponIdCnt++, remainingAmmo);
}

WeaponInstance::WeaponInstance(Weapon * pWeaponClass, uint16 anId, int remainingAmmo) :
        ShootableMapObject(anId, -1, MapObject::kNatureWeapon),
        bombSoundTimer(pWeaponClass->reloadTime()), bombExplosionTimer(pWeaponClass->timeForShot()),
        flamerTimer_(180) {
    pWeaponClass_ = pWeaponClass;
    ammo_remaining_ = remainingAmmo == -1 ? pWeaponClass->ammo() : remainingAmmo;
    pOwner_ = NULL;
    activated_ = false;
    if (pWeaponClass->getType() == Weapon::TimeBomb
        || pWeaponClass->getType() == Weapon::Flamer)
    {
        size_x_ = 32;
        size_y_ = 32;
        size_z_ = 32;
    }
    health_ = 1;
    start_health_ = 1;
    pFlamerShot_ = NULL;
}

bool WeaponInstance::animate(int elapsed) {

    if (activated_) {
        if (isInstanceOf(Weapon::TimeBomb)) {
            if (bombSoundTimer.update(elapsed)) {
                g_App.gameSounds().play(snd::TIMEBOMB);
            }

            if (bombExplosionTimer.update(elapsed)) {
                fs_dmg::DamageToInflict dmg;
                fire(g_Session.getMission(), dmg, elapsed);
                return true;
            }
        }
    }

    if (isDrawable()) {
        return MapObject::animate(elapsed);
    }

    return false;
}

/**
 * Calculate amount of ammo consummed in the elapsed time
 * \param elapsed int
 * \return bool return true if there is no more ammo
 *
 */
bool WeaponInstance::consumeAmmoForEnergyShield(int elapsed) {
    int timeForShot = pWeaponClass_->timeForShot();
    shieldTimeUsed_ += elapsed;

    if (ammo_remaining_ > 0 && shieldTimeUsed_ >= timeForShot) {
        // here time for shot is the unit of time for spending ammo
        // there's no time for reloading

        int remainingShots = ammo_remaining_ / pWeaponClass_->ammoPerShot();
        if (ammo_remaining_ % pWeaponClass_->ammoPerShot()) {
            remainingShots++;
        }

        // effective shots is the number of shot we have to do due to elapsed time
        int effectiveShots = shieldTimeUsed_ / timeForShot;
        shieldTimeUsed_ %= timeForShot;

        if (effectiveShots > remainingShots) {
            effectiveShots = remainingShots;
            shieldTimeUsed_ = 0;
        }

        ammo_remaining_ -= effectiveShots * pWeaponClass_->ammoPerShot();
        if (ammo_remaining_ < 0) {
            ammo_remaining_ = 0;
        }
    }
    return ammo_remaining_ == 0;
}

void WeaponInstance::draw(int x, int y) {
    addOffs(x, y);
    g_App.gameSprites().drawFrame(pWeaponClass_->anim(), frame_, x, y);
}

/*!
 * Plays the sound associated with that weapon.
 */
void WeaponInstance::playSound() {
    g_App.gameSounds().play(pWeaponClass_->getSound());
}

void WeaponInstance::activate() {
    activated_ = true;
}

void WeaponInstance::deactivate() {
    activated_ = false;
}

/*!
 * Use weapon and decrease ammo.
 * This method is used only for shooting weapons and Medikit.
 * \param pMission Mission data
 * \param dmg Information on the damage to perform
 * \param elapsed Time since last frame
 */
void WeaponInstance::fire(Mission *pMission, fs_dmg::DamageToInflict &dmg, int elapsed) {
    bool updateStats = true;
    if (isInstanceOf(Weapon::MediKit)) {
        dmg.d_owner->resetHealth();
        updateStats = false;
    } else if (isInstanceOf(Weapon::GaussGun)) {
        GaussGunShot *pShot = new GaussGunShot(dmg);
        pMission->addPrjShot(pShot);
    } else if (isInstanceOf(Weapon::Flamer)) {
        // when targeting a point with the flamer, the point of impact
        // circles around the target.
        // We use the weapon's direction field to store a logical direction
        // which will give the current moving point of impact
        switch(direction()) {
        case 0:
            dmg.aimedLocW.y += 160;
            break;
        case 1:
            dmg.aimedLocW.x += 96;
            dmg.aimedLocW.y += 96;
            break;
        case 2:
            dmg.aimedLocW.x += 160;
            break;
        case 3:
            dmg.aimedLocW.x += 96;
            dmg.aimedLocW.y -= 96;
            break;
        case 4:
            dmg.aimedLocW.y -= 160;
            break;
        case 5:
            dmg.aimedLocW.x -= 96;
            dmg.aimedLocW.y -= 96;
            break;
        case 6:
            dmg.aimedLocW.x -= 160;
            break;
        case 7:
            dmg.aimedLocW.x -= 96;
            dmg.aimedLocW.y += 96;
            break;
        }

        FlamerShot *pFlamerShot = new FlamerShot(pMission, dmg);
        pMission->addPrjShot(pFlamerShot);

        // Change direction for next time
        if (flamerTimer_.update(elapsed)) {
            setDirection((direction() + 1) % 8);
        }
    }  else if (isInstanceOf(Weapon::TimeBomb)) {
        updateStats = false;
        setDrawable(false);
        health_ = 0;
        deactivate();
        Explosion::createExplosion(pMission, this,
            (double)pWeaponClass_->rangeDmg(), pWeaponClass_->damagePerShot());
    } else if (isInstanceOf(Weapon::EnergyShield)) {
        pOwner_->setEnergyActivated(true);
        shieldTimeUsed_ = 0;
        // return now because ammo is decreased in UseEnergyShieldAction
        return;
    } else {
        // For other weapons, damage are done immediatly because projectile speed
        // is too high to draw them
        InstantImpactShot shot(dmg);
        shot.inflictDamage(pMission);
    }

    int ammoUsed = pWeaponClass_->ammoPerShot();
    ammo_remaining_ -= ammoUsed;
    if (ammo_remaining_ < 0) {
        ammo_remaining_ = 0;
    }

    if (updateStats) {
        if (pOwner_ && pOwner_->isOurAgent()) {
            pMission->stats()->incrShots(ammoUsed);
        }
    }
}

void WeaponInstance::handleHit(fs_dmg::DamageToInflict & d)
{
    // When a bomb is hit, it explodes
    if (isInstanceOf(Weapon::TimeBomb) && health_ > 0 &&
        (d.dtype == fs_dmg::kDmgTypeLaser || d.dtype == fs_dmg::kDmgTypeBullet || d.dtype == fs_dmg::kDmgTypeExplosion)) {
        // we pass the given DamageToInflict just for the compiler
        // as it is not used by the fire method for a Bomb
        // same for elapsed
        fire(g_Session.getMission(), d, 0);
    }
}

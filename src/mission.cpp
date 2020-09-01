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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string>

#include "mission.h"
#include "gfx/screen.h"
#include "app.h"
#include "model/objectivedesc.h"
#include "utils/log.h"
#include "model/vehicle.h"
#include "model/squad.h"
#include "model/shot.h"

const uint8 Mission::kBMaskBlockerTargetOutOfMap = 0x20;
const uint8 Mission::kBMaskBlockerTargetObjectUpdated = 0x02;
const uint8 Mission::kBMaskBlockerTargetPosUpdated = 0x04;

/*!
 * Initialize the statistics.
 * \param nbAgents Number of agents for the mission
 */
void MissionStats::init(int nbAgents) {
    agents_ = nbAgents;
    missionDuration_ = 0;
    agentCaptured_ = 0;
    enemyKilled_ = 0;
    criminalKilled_ = 0;
    civilKilled_ = 0;
    policeKilled_ = 0;
    guardKilled_ = 0;
    convinced_ = 0;
    nbOfShots_ = 0;
    nbOfHits_ = 0;
}

Mission::Mission(const LevelData::MapInfos & map_infos)
{
    status_ = kMissionStatusRunning;

    mtsurfaces_ = NULL;
    mdpoints_ = NULL;
    mdpoints_cp_ = NULL;
    i_map_id_ = READ_LE_UINT16(map_infos.map);
    p_map_ = NULL;
    min_x_= READ_LE_UINT16(map_infos.min_x) / 2;
    min_y_ = READ_LE_UINT16(map_infos.min_y) / 2;
    max_x_ = READ_LE_UINT16(map_infos.max_x) / 2;
    max_y_ = READ_LE_UINT16(map_infos.max_y) / 2;
    cur_objective_ = 0;
    p_minimap_ = NULL;
    p_squad_ = new Squad();
}

Mission::~Mission()
{
    for (unsigned int i = 0; i < vehicles_.size(); i++)
        delete vehicles_[i];
    for (unsigned int i = 0; i < peds_.size(); i++)
        delete peds_[i];
    for (unsigned int i = 0; i < weaponsOnGround_.size(); i++)
        delete weaponsOnGround_[i];
    while (sfx_objects_.size() != 0) {
        delSfxObject(0);
    }
    for (unsigned int i = 0; i < prj_shots_.size(); i++)
        delete prj_shots_[i];
    for (unsigned int i = 0; i < statics_.size(); i++)
        delete statics_[i];
    for (unsigned int i = 0; i < objectives_.size(); i++)
        delete objectives_[i];
    armedPedsVec_.clear();
    clrSurfaces();

    if (p_minimap_) {
        delete p_minimap_;
    }

    if (p_squad_) {
        delete p_squad_;
    }
}

void Mission::delPrjShot(size_t i) {
    delete prj_shots_[i];
    prj_shots_.erase((prj_shots_.begin() + i));
}

/*!
 * Removes given ped from the list of armed peds.
 * \param pPed The ped to remove
 */
void Mission::removeArmedPed(PedInstance *pPed) {
    for (size_t i = 0; i < armedPedsVec_.size();  i++) {
        if (pPed == armedPedsVec_[i]) {
            armedPedsVec_.erase(armedPedsVec_.begin() + i);
            break;
        }
    }
}

/*!
 * Sets the given message with the current objective label.
 */
void Mission::objectiveMsg(std::string& msg) {
    if (cur_objective_ < objectives_.size()) {
        msg = objectives_[cur_objective_]->msg;
    } else {
        msg = "";
    }
}

/*!
 * Sets the given map for the mission.
 * Creates a minimap from it.
 * \param p_map The map to set.
 */
void Mission::set_map(Map *p_map) {
    if (p_map) {
        p_map_ = p_map;
        p_map_->mapDimensions(&mmax_x_, &mmax_y_, &mmax_z_);

        if (p_minimap_) {
            delete p_minimap_;
        }
        p_minimap_ = new MiniMap(p_map_);
    }
}

int Mission::mapWidth()
{
    return p_map_->width();
}

int Mission::mapHeight()
{
    return p_map_->height();
}

void Mission::start()
{
    LOG(Log::k_FLG_GAME, "Mission", "start()", ("Start mission"));
    // Reset mission statistics
    stats_.init(p_squad_->size());

    cur_objective_ = 0;

    // creating a list of available weapons
    // TODO: consider weight of weapons when adding?
    std::vector <Weapon *> wpns;
    g_gameCtrl.weaponManager().getAvailable(fs_dmg::kDmgTypeBullet, wpns);
    int indx_best = -1;
    int indx_second = -1;
    for (int i = 0, sz = wpns.size(), rank_best = -1, rank_second = -1;
        i < sz; ++i)
    {
        if (wpns[i]->rank() > rank_best) {
            rank_second = rank_best;
            indx_second = indx_best;
            rank_best = wpns[i]->rank();
            indx_best = i;
        } else if (wpns[i]->rank() > rank_second) {
            rank_second = wpns[i]->rank();
            indx_second = i;
        }
    }
    Weapon *bomb = g_gameCtrl.weaponManager().getAvailable(Weapon::TimeBomb);

    // TODO: check whether enemy agents weapons are equal to best two
    // if not set them
    for (uint16 i = p_squad_->size(), sz = peds_.size(); i < sz; ++i) {
        PedInstance *p = peds_[i];
        if (p->objGroupDef() == PedInstance::og_dmAgent &&
            p->numWeapons() == 0)
        {
            int index_give = indx_best;
            if (indx_second != -1 && (rand() & 0xFF) > 200)
                index_give = indx_second;
            WeaponInstance *wi = WeaponInstance::createInstance(wpns[index_give]);
            p->addWeapon(wi);
            wi->setOwner(p);
            if (bomb) {
                WeaponInstance *w_bomb = WeaponInstance::createInstance(bomb);
                p->addWeapon(w_bomb);
                w_bomb->setOwner(p);
            }
        }
    }
}

/*!
 * Checks if objectives are completed or failed and updates
 * mission status.
 */
void Mission::checkObjectives() {
    // We only check the current objective
    if (cur_objective_ < objectives_.size()) {
        ObjectiveDesc * pObj = objectives_[cur_objective_];

        // If it's the first time the objective is checked,
        // declares it started
        if (pObj->status == kNotStarted) {
            LOG(Log::k_FLG_GAME, "Mission", "checkObjectives()", ("Start objective : %d", cur_objective_));
            // An objective has just started, warn all listeners
            pObj->start(this);
        }

        // Checks if the objective is completed
        pObj->evaluate(this);

        if (pObj->isTerminated()) {
            if (pObj->status == kFailed) {
                endWithStatus(kMissionStatusFailed);
            } else {
                // Objective is completed -> go to next one
                cur_objective_++;
                if (cur_objective_ >= objectives_.size()) {
                    // the last objective has been completed : mission succeeded
                    endWithStatus(kMissionStatusCompleted);
                }
            }
        }
    }
}

/*!
 * Ends the mission with the given status.
 * \param status The ending status
 */
void Mission::endWithStatus(Status status) {
    switch (status) {
    case kMissionStatusCompleted:
        g_App.gameSounds().play(snd::SPEECH_MISSION_COMPLETED);
        break;
    case kMissionStatusFailed:
        g_App.gameSounds().play(snd::SPEECH_MISSION_FAILED);
        break;
    case kMissionStatusAborted:
        break;
    default:
        // leave without changing status
        return;
    }

    status_ = status;
}

/**
 * When mission is over, weapons from agents are saved for
 * reuse in other missions. So they are transfered to Agent.
 * \param p PedInstance* The agent in the mission
 * \param pAg Agent* The Agent in cryo chamber
 *
 */
void Mission::transferWeaponsFromPedInstanceToAgent(PedInstance* p, Agent* pAg)
{
    while (p->numWeapons()) {
        WeaponInstance *wi = p->removeWeaponAtIndex(0);
        wi->deactivate();
        // auto-reload for pistol
        if (wi->isInstanceOf(Weapon::Pistol))
            wi->reload();
        pAg->addWeapon(wi);
    }
}


void Mission::end()
{
    LOG(Log::k_FLG_GAME, "Mission", "end()", ("End mission"));
    for (unsigned int i = p_squad_->size(); i < peds_.size(); i++) {
        PedInstance *p = peds_[i];
        // TODO: influence country happiness with number of killed overall
        // civilians+police, more killed less happy
        // TODO: add money per every persuaded non-agent ped
        if (p->isDead()) {
            switch (p->type()) {
                case PedInstance::kPedTypeAgent:
                    stats_.incrEnemyKilled();
                    break;
                case PedInstance::kPedTypeCriminal:
                    stats_.incrCriminalKilled();
                    break;
                case PedInstance::kPedTypeCivilian:
                    stats_.incrCivilKilled();
                    break;
                case PedInstance::kPedTypeGuard:
                    stats_.incrGuardKilled();
                    break;
                case PedInstance::kPedTypePolice:
                    stats_.incrPoliceKilled();
                    break;
            }
        } else if (p->isPersuaded()) {
            if (p->objGroupDef() == PedInstance::og_dmAgent) {
                stats_.incrAgentCaptured();
                if (completed()) {
                    Agent *pAg = g_gameCtrl.agents().createAgent(false);
                    if (pAg) {
                        transferWeaponsFromPedInstanceToAgent(p, pAg);
                        *((ModOwner *)pAg) = *((ModOwner *)p);
                    }
                }
            } else
                stats_.incrConvinced();
        }
    }

    // synch ped agents with agent from cryo chamber
    for (size_t i = AgentManager::kSlot1; i < AgentManager::kMaxSlot; i++) {
        PedInstance *p_pedAgent = p_squad_->member(i);
        if (p_pedAgent) {
            Agent *pAg = g_gameCtrl.agents().squadMember(i);
            if (p_pedAgent->isDead()) {
                // an agent died -> remove him from cryo
                g_gameCtrl.agents().destroyAgentSlot(i);
            } else {
                // synch only weapons
                transferWeaponsFromPedInstanceToAgent(p_pedAgent, pAg);
            }
        }
    }

    // reset squad
    p_squad_->clear();
}

void Mission::addWeaponToGround(WeaponInstance * w)
{
    for (unsigned int i = 0; i < weaponsOnGround_.size(); i++) {
        // TODO : check if == operator is used correctly (see  == in WeaponInstance)
        if (weaponsOnGround_[i] == w)
            return;
    }
    weaponsOnGround_.push_back(w);
}

void Mission::removeWeaponOnGround(WeaponInstance *pWeapon) {
    for (unsigned int i = 0; i < weaponsOnGround_.size(); i++) {
        if (weaponsOnGround_[i] == pWeapon) {
            weaponsOnGround_.erase(weaponsOnGround_.begin() + i);
        }
    }
}

MapObject * Mission::findObjectWithNatureAtPos(int tilex, int tiley, int tilez,
                            MapObject::ObjectNature *nature, int *searchIndex,
                            bool only) {

    const TilePoint position(tilex, tiley, tilez);
    switch(*nature) {
        case MapObject::kNaturePed:
            for (unsigned int i = *searchIndex; i < peds_.size(); i++) {
                // dead peds are included because doors stay opened even with dead corpses
                // it also prevents glitches with a closed door over a dead body
                if (peds_[i]->sameTile(position)) {
                    *searchIndex = i + 1;
                    *nature = MapObject::kNaturePed;
                    return peds_[i];
                }
            }
            if(only)
                return NULL;
            *searchIndex = 0;
        case MapObject::kNatureVehicle:
            for (unsigned int i = *searchIndex; i < vehicles_.size(); i++)
                if (vehicles_[i]->sameTile(position))
                {
                    *searchIndex = i + 1;
                    *nature = MapObject::kNatureVehicle;
                    return vehicles_[i];
                }
            break;
        default:
            FSERR(Log::k_FLG_GAME, "Mission", "findObjectWithNatureAtPos", ("Undefined nature %i\n", *nature));
            break;
    }
    return NULL;
}

// Surface walkable
bool Mission::sWalkable(char thisTile, char upperTile) {

    return (
            // checking surface
            (((thisTile >= 0x05 && thisTile <= 0x09) ||
            thisTile == 0x0B || (thisTile >= 0x0D && thisTile <= 0x0F)
            || (thisTile == 0x11 || thisTile == 0x12)))
            // or checking stairs
            || ((thisTile > 0x00 && thisTile < 0x05))
        ) && (upperTile == 0x00 || upperTile == 0x10);
}

bool Mission::isSurface(char thisTile) {
    return (thisTile >= 0x05 && thisTile <= 0x09) ||
        thisTile == 0x0B || (thisTile >= 0x0D && thisTile <= 0x0F)
        || (thisTile == 0x11 || thisTile == 0x12);
}

bool Mission::isStairs(char thisTile) {
    return thisTile > 0x00 && thisTile < 0x05;
}

bool Mission::setSurfaces() {

    // Description: creates map of walkable surfaces, also
    // defines directions where movement is possible

    // NOTE: tiles walkdata type 0x0D are quiet special, and they
    // are not handled correctly, these correction and andjustings
    // can create additional speed drain, as such I didn't
    // implemented them as needed. To make it possible a patch
    // required to walkdata and a lot of changes which I don't
    // want to do.
    // 0x10 appear above walking tile where train stops
    clrSurfaces();
    int mmax_m_all = mmax_x_ * mmax_y_ * mmax_z_;
    mtsurfaces_ = (uint8 *)malloc(mmax_m_all * sizeof(uint8));
    mdpoints_ = (floodPointDesc *)malloc(mmax_m_all * sizeof(floodPointDesc));
    mdpoints_cp_ = (floodPointDesc *)malloc(mmax_m_all * sizeof(floodPointDesc));
    if(mtsurfaces_ == NULL || mdpoints_ == NULL || mdpoints_cp_ == NULL) {
        clrSurfaces();
        FSERR(Log::k_FLG_GAME, "Mission", "setSurfaces", ("Memory allocation error\n"));
        return false;
    }
    mmax_m_xy = mmax_x_ * mmax_y_;
    memset((void *)mtsurfaces_, 0, mmax_m_all * sizeof(uint8));
    memset((void *)mdpoints_, 0, mmax_m_all * sizeof(floodPointDesc));
    for (int ix = 0; ix < mmax_x_; ++ix) {
        for (int iy = 0; iy < mmax_y_; ++iy) {
            for (int iz = 0; iz < mmax_z_; ++iz) {
                mtsurfaces_[ix + iy * mmax_x_ + iz * mmax_m_xy] =
                    p_map_->getTileAt(ix, iy, iz)->getWalkData();
            }
        }
    }

    // to make surfaces where large doors are located walkable
    for (std::vector<Static *>::iterator it = statics_.begin();
        it != statics_.end(); ++it)
    {
        Static *s = *it;
        if (s->type() == Static::smt_LargeDoor) {
            printf("Large door detected\n");
            int indx = s->tileX() + s->tileY() * mmax_x_
                + s->tileZ() * mmax_m_xy;
            mtsurfaces_[indx] = 0x00;
            if (s->orientation() == Static::kStaticOrientation1) {
                if (indx - 1 >= 0)
                    mtsurfaces_[indx - 1] = 0x00;
                if (indx + 1 < mmax_m_all)
                    mtsurfaces_[indx + 1] = 0x00;
            } else if (s->orientation() == Static::kStaticOrientation2) {
                if (indx - mmax_x_ >= 0)
                    mtsurfaces_[indx - mmax_x_] = 0x00;
                if (indx + mmax_x_ < mmax_m_all)
                    mtsurfaces_[indx + mmax_x_] = 0x00;
            }
        }
    }

    //printf("surface data size %i\n", sizeof(surfaceDesc) * mmax_m_all);
    //printf("flood data size %i\n", sizeof(floodPointDesc) * mmax_m_all);

    for (unsigned int i = 0; i < peds_.size(); ++i) {
        PedInstance *p = peds_[i];
        int x = p->tileX();
        int y = p->tileY();
        int z = p->tileZ();
        if (z >= mmax_z_ || z < 0 || p->isDead()) {
            // TODO : check on all maps those peds correct position
            p->setTileZ(mmax_z_ - 1);
            continue;
        }
        if (mdpoints_[x + y * mmax_x_ + z * mmax_m_xy].bfNodeDesc == m_fdNotDefined) {
            WorldPoint stodef;
            std::vector<WorldPoint> vtodefine;
            mdpoints_[x + y * mmax_x_ + z * mmax_m_xy].bfNodeDesc = m_fdDefReq;
            stodef.x = x;
            stodef.y = y * mmax_x_;
            stodef.z = z * mmax_m_xy;
            vtodefine.push_back(stodef);
            do {
                stodef = vtodefine.back();
                vtodefine.pop_back();
                x = stodef.x;
                y = stodef.y;
                z = stodef.z;
                //if (x == 50 && y / mmax_x_ == 27 && z / mmax_m_xy == 2)
                    //x = 50;
                uint8 this_s = mtsurfaces_[x + y + z];
                uint8 upper_s = 0;
                floodPointDesc *cfp = &(mdpoints_[x + y + z]);
                int zm = z - mmax_m_xy;
                // if current is 0x00 or 0x10 tile we will use lower tile
                // to define it
                if (this_s == 0x00 || this_s == 0x10) {
                    if (zm < 0) {
                        cfp->bfNodeDesc = m_fdNonWalkable;
                        continue;
                    }
                    z = zm;
                    zm -= mmax_m_xy;
                    upper_s = this_s;
                    this_s = mtsurfaces_[x + y + z];
                    if (!sWalkable(this_s, upper_s))
                        continue;
                } else if (this_s == 0x11 || this_s == 0x12) {
                    int zp_tmp = z + mmax_m_xy;
                    if (zp_tmp < mmax_m_all) {
                        // we are defining tile above current
                        cfp = &(mdpoints_[x + y + zp_tmp]);
                    } else
                        cfp->bfNodeDesc = m_fdNonWalkable;
                }
                int xm = x - 1;
                int ym = y - mmax_x_;
                int xp = x + 1;
                int yp = y + mmax_x_;
                int zp = z + mmax_m_xy;
                floodPointDesc *nxtfp;
                if (zp < mmax_m_all) {
                    upper_s = mtsurfaces_[x + y + zp];
                    if(!sWalkable(this_s, upper_s)) {
                        cfp->bfNodeDesc = m_fdNonWalkable;
                        continue;
                    }
                } else {
                    cfp->bfNodeDesc = m_fdNonWalkable;
                    continue;
                }
                unsigned char sdirm = 0x00;
                unsigned char sdirh = 0x00;
                unsigned char sdirl = 0x00;
                unsigned char sdirmr = 0x00;

                switch (this_s) {
                    case 0x00:
                        cfp->bfNodeDesc = m_fdNonWalkable;
                        break;
                    case 0x01:
                        cfp->bfNodeDesc = m_fdWalkable;
                        cfp->bfNodeDesc |= m_fdSafeWalk;
                        if (zm >= 0) {
                            mdpoints_[x + y + zm].bfNodeDesc = m_fdNonWalkable;
                            if (yp < mmax_m_xy) {
                                this_s = mtsurfaces_[x + yp + zm];
                                upper_s = mtsurfaces_[x + yp + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    sdirm |= 0x01;
                                    nxtfp = &(mdpoints_[x + yp + z]);
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = yp;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (this_s == 0x01) {
                                    nxtfp = &(mdpoints_[x + yp + zm]);
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x01;
                                        nxtfp = &(mdpoints_[x + yp + zm]);
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = yp;
                                            stodef.z = zm;
                                            vtodefine.push_back(stodef);
                                        }
                                    } else
                                        nxtfp->bfNodeDesc = m_fdNonWalkable;
                                }
                            }
                            if (xm >= 0) {
                                this_s = mtsurfaces_[xm + y + zm];
                                upper_s = mtsurfaces_[xm + y + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    nxtfp = &(mdpoints_[xm + y + z]);
                                    sdirm |= 0x40;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xm;
                                        stodef.y = y;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (isStairs(this_s)) {
                                    nxtfp = &(mdpoints_[xm + y + zm]);
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xm;
                                        stodef.y = y;
                                        stodef.z = zm;
                                        vtodefine.push_back(stodef);
                                    }
                                }
                            }
                            if (xp < mmax_x_) {
                                this_s = mtsurfaces_[xp + y + zm];
                                upper_s = mtsurfaces_[xp + y + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    nxtfp = &(mdpoints_[xp + y + z]);
                                    sdirm |= 0x04;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xp;
                                        stodef.y = y;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (isStairs(this_s)) {
                                    nxtfp = &(mdpoints_[xp + y + zm]);
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xp;
                                        stodef.y = y;
                                        stodef.z = zm;
                                        vtodefine.push_back(stodef);
                                    }
                                }
                            }
                        }

                        if (ym >= 0) {
                            nxtfp = &(mdpoints_[x + ym + zp]);
                            this_s = mtsurfaces_[x + ym + z];
                            upper_s = mtsurfaces_[x + ym + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                sdirh |= 0x10;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = ym;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if(upper_s == 0x01 && (zp + mmax_m_xy) < mmax_m_all) {
                                if(sWalkable(upper_s, mtsurfaces_[
                                    x + ym + (zp + mmax_m_xy)]))
                                {
                                    sdirh |= 0x10;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = ym;
                                        stodef.z = zp;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }

                        if (xm >= 0) {
                            this_s = mtsurfaces_[xm + y + z];
                            upper_s = mtsurfaces_[xm + y + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                nxtfp = &(mdpoints_[xm + y + zp]);
                                sdirh |= 0x40;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xm;
                                    stodef.y = y;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (this_s == 0x01) {
                                nxtfp = &(mdpoints_[xm + y + z]);
                                if (sWalkable(this_s, upper_s)) {
                                    sdirm |= 0x40;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xm;
                                        stodef.y = y;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }

                        if (xp < mmax_x_) {
                            this_s = mtsurfaces_[xp + y + z];
                            upper_s = mtsurfaces_[xp + y + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                nxtfp = &(mdpoints_[xp + y + zp]);
                                sdirh |= 0x04;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = y;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (this_s == 0x01) {
                                nxtfp = &(mdpoints_[xp + y + z]);
                                if (sWalkable(this_s, upper_s)) {
                                    sdirm |= 0x04;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xp;
                                        stodef.y = y;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }
                        cfp->dirm = sdirm;
                        cfp->dirh = sdirh;
                        cfp->dirl = sdirl;

                        break;
                    case 0x02:
                        cfp->bfNodeDesc = m_fdWalkable;
                        cfp->bfNodeDesc |= m_fdSafeWalk;
                        if (zm >= 0) {
                            mdpoints_[x + y + zm].bfNodeDesc = m_fdNonWalkable;
                            if (ym >= 0) {
                                this_s = mtsurfaces_[x + ym + zm];
                                upper_s = mtsurfaces_[x + ym + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    nxtfp = &(mdpoints_[x + ym + z]);
                                    sdirm |= 0x10;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = ym;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (this_s == 0x02) {
                                    nxtfp = &(mdpoints_[x + ym + zm]);
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x10;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = ym;
                                            stodef.z = zm;
                                            vtodefine.push_back(stodef);
                                        }
                                    } else
                                        nxtfp->bfNodeDesc = m_fdNonWalkable;
                                }
                            }
                            if (xm >= 0) {
                                this_s = mtsurfaces_[xm + y + zm];
                                upper_s = mtsurfaces_[xm + y + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    nxtfp = &(mdpoints_[xm + y + z]);
                                    sdirm |= 0x40;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xm;
                                        stodef.y = y;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (isStairs(this_s)) {
                                    nxtfp = &(mdpoints_[xm + y + zm]);
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xm;
                                        stodef.y = y;
                                        stodef.z = zm;
                                        vtodefine.push_back(stodef);
                                    }
                                }
                            }
                            if (xp < mmax_x_) {
                                this_s = mtsurfaces_[xp + y + zm];
                                upper_s = mtsurfaces_[xp + y + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    nxtfp = &(mdpoints_[xp + y + z]);
                                    sdirm |= 0x04;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xp;
                                        stodef.y = y;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (isStairs(this_s)) {
                                    nxtfp = &(mdpoints_[xp + y + zm]);
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xp;
                                        stodef.y = y;
                                        stodef.z = zm;
                                        vtodefine.push_back(stodef);
                                    }
                                }
                            }
                        }

                        if (yp < mmax_m_xy) {
                            nxtfp = &(mdpoints_[x + yp + zp]);
                            this_s = mtsurfaces_[x + yp + z];
                            upper_s = mtsurfaces_[x + yp + zp];
                            if(isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                sdirh |= 0x01;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = yp;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if(upper_s == 0x02 && (zp + mmax_m_xy) < mmax_m_all) {
                                if(sWalkable(upper_s,  mtsurfaces_[
                                    x + yp + (zp + mmax_m_xy)]))
                                {
                                    sdirh |= 0x01;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = yp;
                                        stodef.z = zp;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }

                        if (xm >= 0) {
                            this_s = mtsurfaces_[xm + y + z];
                            upper_s = mtsurfaces_[xm + y + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                nxtfp = &(mdpoints_[xm + y + zp]);
                                sdirh |= 0x40;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xm;
                                    stodef.y = y;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (this_s == 0x02) {
                                nxtfp = &(mdpoints_[xm + y + z]);
                                if (sWalkable(this_s, upper_s)) {
                                    sdirm |= 0x40;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xm;
                                        stodef.y = y;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }

                        if (xp < mmax_x_) {
                            this_s = mtsurfaces_[xp + y + z];
                            upper_s = mtsurfaces_[xp + y + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                nxtfp = &(mdpoints_[xp + y + zp]);
                                sdirh |= 0x04;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = y;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (this_s == 0x02) {
                                nxtfp = &(mdpoints_[xp + y + z]);
                                if (sWalkable(this_s, upper_s)) {
                                    sdirm |= 0x04;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xp;
                                        stodef.y = y;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }
                        cfp->dirm = sdirm;
                        cfp->dirh = sdirh;
                        cfp->dirl = sdirl;

                        break;
                    case 0x03:
                        cfp->bfNodeDesc = m_fdWalkable;
                        cfp->bfNodeDesc |= m_fdSafeWalk;
                        if (zm >= 0) {
                            mdpoints_[x + y + zm].bfNodeDesc = m_fdNonWalkable;
                            if (xm >= 0) {
                                this_s = mtsurfaces_[xm + y + zm];
                                upper_s = mtsurfaces_[xm + y + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    nxtfp = &(mdpoints_[xm + y + z]);
                                    sdirm |= 0x40;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xm;
                                        stodef.y = y;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (this_s == 0x03) {
                                    nxtfp = &(mdpoints_[xm + y + zm]);
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x40;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xm;
                                            stodef.y = y;
                                            stodef.z = zm;
                                            vtodefine.push_back(stodef);
                                        }
                                    } else
                                        nxtfp->bfNodeDesc = m_fdNonWalkable;
                                }
                            }
                            if (ym >= 0) {
                                this_s = mtsurfaces_[x + ym + zm];
                                upper_s = mtsurfaces_[x + ym + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    nxtfp = &(mdpoints_[x + ym + z]);
                                    sdirm |= 0x10;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = ym;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (isStairs(this_s)) {
                                    nxtfp = &(mdpoints_[x + ym + zm]);
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = ym;
                                        stodef.z = zm;
                                        vtodefine.push_back(stodef);
                                    }
                                }
                            }
                            if (yp < mmax_m_xy) {
                                this_s = mtsurfaces_[x + yp + zm];
                                upper_s = mtsurfaces_[x + yp + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    nxtfp = &(mdpoints_[x + yp + z]);
                                    sdirm |= 0x01;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = yp;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (isStairs(this_s)) {
                                    nxtfp = &(mdpoints_[x + yp + zm]);
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = yp;
                                        stodef.z = zm;
                                        vtodefine.push_back(stodef);
                                    }
                                }
                            }
                        }

                        if (xp < mmax_x_) {
                            nxtfp = &(mdpoints_[xp + y + zp]);
                            this_s = mtsurfaces_[xp + y + z];
                            upper_s = mtsurfaces_[xp + y + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                sdirh |= 0x04;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = y;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if(upper_s == 0x03 && (zp + mmax_m_xy) < mmax_m_all) {
                                if(sWalkable(upper_s,
                                    mtsurfaces_[xp + y + (zp + mmax_m_xy)]))
                                {
                                    sdirh |= 0x04;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xp;
                                        stodef.y = y;
                                        stodef.z = zp;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }

                        if (ym >= 0) {
                            this_s = mtsurfaces_[x + ym + z];
                            upper_s = mtsurfaces_[x + ym + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                nxtfp = &(mdpoints_[x + ym + zp]);
                                sdirh |= 0x10;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = ym;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (this_s == 0x03) {
                                nxtfp = &(mdpoints_[x + ym + z]);
                                if (sWalkable(this_s, upper_s)) {
                                    sdirm |= 0x10;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = ym;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }

                        if (yp < mmax_m_xy) {
                            this_s = mtsurfaces_[x + yp + z];
                            upper_s = mtsurfaces_[x + yp + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                nxtfp = &(mdpoints_[x + yp + zp]);
                                sdirh |= 0x01;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = yp;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (this_s == 0x03) {
                                nxtfp = &(mdpoints_[x + yp + z]);
                                if (sWalkable(this_s, upper_s)) {
                                    sdirm |= 0x01;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = yp;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }
                        cfp->dirm = sdirm;
                        cfp->dirh = sdirh;
                        cfp->dirl = sdirl;

                        break;
                    case 0x04:
                        cfp->bfNodeDesc = m_fdWalkable;
                        cfp->bfNodeDesc |= m_fdSafeWalk;
                        if (zm >= 0) {
                            mdpoints_[x + y + zm].bfNodeDesc = m_fdNonWalkable;
                            if (xp < mmax_x_) {
                                this_s = mtsurfaces_[xp + y + zm];
                                upper_s = mtsurfaces_[xp + y + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    nxtfp = &(mdpoints_[xp + y + z]);
                                    sdirm |= 0x04;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xp;
                                        stodef.y = y;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (this_s == 0x04) {
                                    nxtfp = &(mdpoints_[xp + y + zm]);
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x04;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xp;
                                            stodef.y = y;
                                            stodef.z = zm;
                                            vtodefine.push_back(stodef);
                                        }
                                    } else
                                        nxtfp->bfNodeDesc = m_fdNonWalkable;
                                }
                            }
                            if (ym >= 0) {
                                this_s = mtsurfaces_[x + ym + zm];
                                upper_s = mtsurfaces_[x + ym + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    nxtfp = &(mdpoints_[x + ym + z]);
                                    sdirm |= 0x10;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = ym;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (isStairs(this_s)) {
                                    nxtfp = &(mdpoints_[x + ym + zm]);
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = ym;
                                        stodef.z = zm;
                                        vtodefine.push_back(stodef);
                                    }
                                }
                            }
                            if (yp < mmax_m_xy) {
                                this_s = mtsurfaces_[x + yp + zm];
                                upper_s = mtsurfaces_[x + yp + z];
                                if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                    nxtfp = &(mdpoints_[x + yp + z]);
                                    sdirm |= 0x01;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = yp;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else if (isStairs(this_s)) {
                                    nxtfp = &(mdpoints_[x + yp + zm]);
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = yp;
                                        stodef.z = zm;
                                        vtodefine.push_back(stodef);
                                    }
                                }
                            }
                        }

                        if (xm >= 0) {
                            nxtfp = &(mdpoints_[xm + y + zp]);
                            this_s = mtsurfaces_[xm + y + z];
                            upper_s = mtsurfaces_[xm + y + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                sdirh |= 0x40;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xm;
                                    stodef.y = y;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if(upper_s == 0x04 && (zp + mmax_m_xy) < mmax_m_all) {
                                if(sWalkable(upper_s, mtsurfaces_[
                                    xm + y + (zp + mmax_m_xy)]))
                                {
                                    sdirh |= 0x40;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = xm;
                                        stodef.y = y;
                                        stodef.z = zp;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }

                        if (ym >= 0) {
                            this_s = mtsurfaces_[x + ym + z];
                            upper_s = mtsurfaces_[x + ym + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                nxtfp = &(mdpoints_[x + ym + zp]);
                                sdirh |= 0x10;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = ym;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (this_s == 0x04) {
                                nxtfp = &(mdpoints_[x + ym + z]);
                                if (sWalkable(this_s, upper_s)) {
                                    sdirm |= 0x10;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = ym;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }

                        if (yp < mmax_m_xy) {
                            this_s = mtsurfaces_[x + yp + z];
                            upper_s = mtsurfaces_[x + yp + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s)) {
                                nxtfp = &(mdpoints_[x + yp + zp]);
                                sdirh |= 0x01;
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = yp;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (this_s == 0x04) {
                                nxtfp = &(mdpoints_[x + yp + z]);
                                if (sWalkable(this_s, upper_s)) {
                                    sdirm |= 0x01;
                                    if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                        nxtfp->bfNodeDesc = m_fdDefReq;
                                        stodef.x = x;
                                        stodef.y = yp;
                                        stodef.z = z;
                                        vtodefine.push_back(stodef);
                                    }
                                } else
                                    nxtfp->bfNodeDesc = m_fdNonWalkable;
                            }
                        }
                        cfp->dirm = sdirm;
                        cfp->dirh = sdirh;
                        cfp->dirl = sdirl;

                        break;
                    case 0x05:
                    case 0x06:
                    case 0x07:
                    case 0x08:
                    case 0x09:
                    case 0x0B:
                    case 0x0D:
                    case 0x0E:
                    case 0x0F:
                        cfp->bfNodeDesc = m_fdWalkable;
                        if (!((this_s > 0x05 && this_s < 0x0A) || this_s == 0x0B
                            || this_s == 0x0F))
                        {
                            cfp->bfNodeDesc |= m_fdSafeWalk;
                        }
                        if (xm >= 0) {
                            this_s = mtsurfaces_[xm + y + z];
                            upper_s = mtsurfaces_[xm + y + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s))
                            {
                                sdirm |= (0x20 | 0x40 | 0x80);
                                nxtfp = &(mdpoints_[xm + y + zp]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xm;
                                    stodef.y = y;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (isStairs(this_s) && sWalkable(this_s,
                                upper_s))
                            {
                                sdirmr |= (0x20 | 0x80);
                                if (this_s == 0x01 || this_s == 0x02
                                    || this_s == 0x03)
                                {
                                    sdirl |= 0x40;
                                }
                                nxtfp = &(mdpoints_[xm + y + z]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xm;
                                    stodef.y = y;
                                    stodef.z = z;
                                    vtodefine.push_back(stodef);
                                }
                            } else {
                                sdirmr |= (0x20 | 0x80);
                                if ((zp + mmax_m_xy) < mmax_m_all
                                    && (upper_s == 0x01 || upper_s == 0x02 || upper_s == 0x04
                                    || upper_s == 0x12)) {
                                    if (sWalkable(upper_s,
                                        mtsurfaces_[xm + y + (zp + mmax_m_xy)]))
                                    {
                                        if (upper_s == 0x12)
                                            sdirh |= 0x40;
                                        else
                                            sdirm |= 0x40;
                                        nxtfp = &(mdpoints_[xm + y + zp]);
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xm;
                                            stodef.y = y;
                                            stodef.z = zp;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        } else
                            sdirmr |= (0x20 | 0x80);

                        if (xp < mmax_x_) {
                            this_s = mtsurfaces_[xp + y + z];
                            upper_s = mtsurfaces_[xp + y + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s))
                            {
                                sdirm |= (0x02 | 0x04 | 0x08);
                                nxtfp = &(mdpoints_[xp + y + zp]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = y;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (isStairs(this_s) && sWalkable(this_s,
                                upper_s))
                            {
                                sdirmr |= (0x02 | 0x08);
                                if (this_s == 0x01 || this_s == 0x02
                                    || this_s == 0x04)
                                {
                                    sdirl |= 0x04;
                                }
                                nxtfp = &(mdpoints_[xp + y + z]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = y;
                                    stodef.z = z;
                                    vtodefine.push_back(stodef);
                                }
                            } else {
                                sdirmr |= (0x02 | 0x08);
                                if ((zp + mmax_m_xy) < mmax_m_all
                                    && (upper_s == 0x01 || upper_s == 0x02
                                    || upper_s == 0x03 || upper_s == 0x11))
                                {
                                    if (sWalkable(upper_s,
                                        mtsurfaces_[xp + y + (zp + mmax_m_xy)]))
                                    {
                                        if (upper_s == 0x11)
                                            sdirh |= 0x04;
                                        else
                                            sdirm |= 0x04;
                                        nxtfp = &(mdpoints_[xp + y + zp]);
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xp;
                                            stodef.y = y;
                                            stodef.z = zp;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        } else
                            sdirmr |= (0x02 | 0x08);

                        if(ym >= 0) {
                            this_s = mtsurfaces_[x + ym + z];
                            upper_s = mtsurfaces_[x + ym + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s))
                            {
                                sdirm |= (0x08 | 0x10 | 0x20);
                                nxtfp = &(mdpoints_[x + ym + zp]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = ym;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (isStairs(this_s) && sWalkable(this_s,
                                upper_s))
                            {
                                sdirmr |= (0x08 | 0x20);
                                if (this_s == 0x02 || this_s == 0x03 || this_s == 0x04){
                                    sdirl |= 0x10;
                                }
                                nxtfp = &(mdpoints_[x + ym + z]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = ym;
                                    stodef.z = z;
                                    vtodefine.push_back(stodef);
                                }
                            } else {
                                sdirmr |= (0x08 | 0x20);
                                if ((zp + mmax_m_xy) < mmax_m_all
                                    && (upper_s == 0x01 || upper_s == 0x03
                                    || upper_s == 0x04 || upper_s == 0x11))
                                {
                                    if (sWalkable(upper_s,
                                        mtsurfaces_[x + ym + (zp + mmax_m_xy)]))
                                    {
                                        if (upper_s == 0x11)
                                            sdirh |= 0x10;
                                        else
                                            sdirm |= 0x10;
                                        nxtfp = &(mdpoints_[x + ym + zp]);
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = ym;
                                            stodef.z = zp;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        } else
                            sdirmr |= (0x08 | 0x20);

                        if (yp < mmax_m_xy) {
                            this_s = mtsurfaces_[x + yp + z];
                            upper_s = mtsurfaces_[x + yp + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s))
                            {
                                sdirm |= (0x80 | 0x01 | 0x02);
                                nxtfp = &(mdpoints_[x + yp + zp]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = yp;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (isStairs(this_s) && sWalkable(this_s,
                                upper_s))
                            {
                                sdirmr |= (0x80 | 0x02);
                                if (this_s == 0x01 || this_s == 0x03
                                    || this_s == 0x04)
                                {
                                    sdirl |= 0x01;
                                }
                                nxtfp = &(mdpoints_[x + yp + z]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = yp;
                                    stodef.z = z;
                                    vtodefine.push_back(stodef);
                                }
                            } else {
                                sdirmr |= (0x80 | 0x02);
                                if ((zp + mmax_m_xy) < mmax_m_all
                                    && (upper_s == 0x02 || upper_s == 0x03
                                    || upper_s == 0x04 || upper_s == 0x12))
                                {
                                    if (sWalkable(upper_s,
                                        mtsurfaces_[x + yp + (zp + mmax_m_xy)]))
                                    {
                                        if (upper_s == 0x12)
                                            sdirh |= 0x01;
                                        else
                                            sdirm |= 0x01;
                                        nxtfp = &(mdpoints_[x + yp + zp]);
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = yp;
                                            stodef.z = zp;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        } else
                            sdirmr |= (0x80 | 0x02);
                        sdirm &= (0xFF ^ sdirmr);

                        // edges

                        if (xm >= 0) {
                            if (ym >= 0 && (sdirm & 0x20) != 0) {
                                nxtfp = &(mdpoints_[xm + ym + zp]);
                                this_s = mtsurfaces_[xm + ym + z];
                                upper_s = mtsurfaces_[xm + ym + zp];
                                if (!(isSurface(this_s) && sWalkable(this_s,
                                    upper_s)))
                                {
                                    sdirm &= (0xFF ^ 0x20);
                                } else if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xm;
                                    stodef.y = ym;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            }

                            if (yp < mmax_m_xy && (sdirm & 0x80) != 0) {
                                nxtfp = &(mdpoints_[xm + yp + zp]);
                                this_s = mtsurfaces_[xm + yp + z];
                                upper_s = mtsurfaces_[xm + yp + zp];
                                if (!(isSurface(this_s) && sWalkable(this_s,
                                    upper_s)))
                                {
                                    sdirm &= (0xFF ^ 0x80);
                                } else if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xm;
                                    stodef.y = yp;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            }
                        }

                        if (xp < mmax_x_) {
                            if (ym >= 0 && (sdirm & 0x08) != 0) {
                                nxtfp = &(mdpoints_[xp + ym + zp]);
                                this_s = mtsurfaces_[xp + ym + z];
                                upper_s = mtsurfaces_[xp + ym + zp];
                                if (!(isSurface(this_s) && sWalkable(this_s,
                                    upper_s)))
                                {
                                    sdirm &= (0xFF ^ 0x08);
                                } else if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = ym;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            }

                            if (yp < mmax_m_xy && (sdirm & 0x02) != 0) {
                                nxtfp = &(mdpoints_[xp + yp + zp]);
                                this_s = mtsurfaces_[xp + yp + z];
                                upper_s = mtsurfaces_[xp + yp + zp];
                                if (!(isSurface(this_s) && sWalkable(this_s,
                                    upper_s)))
                                {
                                    sdirm &= (0xFF ^ 0x02);
                                } else if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = yp;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            }
                        }
                        cfp->dirm = sdirm;
                        cfp->dirh = sdirh;
                        cfp->dirl = sdirl;

                        break;
                    case 0x0A:
                    case 0x0C:
                    case 0x10:
                        cfp->bfNodeDesc = m_fdNonWalkable;
                        break;
                    case 0x11:
                        cfp->bfNodeDesc = m_fdWalkable;
                        cfp->bfNodeDesc |= m_fdSafeWalk;
                        if (zm >= 0) {
                            mdpoints_[x + y + zm].bfNodeDesc = m_fdNonWalkable;
                            if (xm >= 0) {
                                this_s = mtsurfaces_[xm + y + zm];
                                upper_s = mtsurfaces_[xm + y + z];
                                if (isSurface(this_s)) {
                                    nxtfp = &(mdpoints_[xm + y + z]);
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x40;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xm;
                                            stodef.y = y;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                } else if (isStairs(upper_s) && upper_s != 0x04) {
                                    nxtfp = &(mdpoints_[xm + y + z]);
                                    this_s = upper_s;
                                    upper_s = mtsurfaces_[xm + y + zp];
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x40;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xm;
                                            stodef.y = y;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                            if (ym >= 0) {
                                this_s = mtsurfaces_[x + ym + zm];
                                upper_s = mtsurfaces_[x + ym + z];
                                if (isSurface(this_s)) {
                                    nxtfp = &(mdpoints_[x + ym + z]);
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x10;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = ym;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                } else if (isStairs(upper_s) && upper_s != 0x01) {
                                    nxtfp = &(mdpoints_[x + ym + z]);
                                    this_s = upper_s;
                                    upper_s = mtsurfaces_[x + ym + zp];
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x10;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = ym;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                            if (yp < mmax_m_xy) {
                                this_s = mtsurfaces_[x + yp + zm];
                                upper_s = mtsurfaces_[x + yp + z];
                                if (isSurface(this_s)) {
                                    nxtfp = &(mdpoints_[x + yp + z]);
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x01;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = yp;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                } else if (isStairs(upper_s) && upper_s != 0x02) {
                                    nxtfp = &(mdpoints_[x + yp + z]);
                                    this_s = upper_s;
                                    upper_s = mtsurfaces_[x + yp + zp];
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x01;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = yp;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        }

                        if (xp < mmax_x_) {
                            this_s = mtsurfaces_[xp + y + z];
                            upper_s = mtsurfaces_[xp + y + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s))
                            {
                                sdirm |= (0x02 | 0x04 | 0x08);
                                nxtfp = &(mdpoints_[xp + y + zp]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = y;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (isStairs(this_s) && sWalkable(this_s,
                                upper_s))
                            {
                                sdirmr |= (0x02 | 0x08);
                                if (this_s == 0x01 || this_s == 0x02 || this_s == 0x04){
                                    sdirl |= 0x04;
                                }
                                nxtfp = &(mdpoints_[xp + y + z]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = y;
                                    stodef.z = z;
                                    vtodefine.push_back(stodef);
                                }
                            } else {
                                sdirmr |= (0x02 | 0x08);
                                if ((zp + mmax_m_xy) < mmax_m_all
                                    && (upper_s == 0x01 || upper_s == 0x02
                                    || upper_s == 0x03))
                                {
                                    if (sWalkable(upper_s,
                                        mtsurfaces_[xp + y + (zp + mmax_m_xy)]))
                                    {
                                        sdirm |= 0x04;
                                        nxtfp = &(mdpoints_[xp + y + zp]);
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xp;
                                            stodef.y = y;
                                            stodef.z = zp;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        } else
                            sdirmr |= (0x02 | 0x08);

                        if(ym >= 0) {
                            this_s = mtsurfaces_[x + ym + z];
                            upper_s = mtsurfaces_[x + ym + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s))
                            {
                                sdirm |= (0x08 | 0x10);
                                nxtfp = &(mdpoints_[x + ym + zp]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = ym;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (isStairs(this_s) && sWalkable(this_s,
                                upper_s))
                            {
                                sdirmr |= (0x08 | 0x20);
                                if (this_s == 0x02 || this_s == 0x03 || this_s == 0x04) {
                                    sdirl |= 0x10;
                                }
                                nxtfp = &(mdpoints_[x + ym + z]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = ym;
                                    stodef.z = z;
                                    vtodefine.push_back(stodef);
                                }
                            } else {
                                sdirmr |= (0x08 | 0x20);
                                if ((zp + mmax_m_xy) < mmax_m_all
                                    && (upper_s == 0x01 || upper_s == 0x03 || upper_s == 0x04)) {
                                    if (sWalkable(upper_s,
                                        mtsurfaces_[x + ym + (zp + mmax_m_xy)]))
                                    {
                                        sdirm |= 0x10;
                                        nxtfp = &(mdpoints_[x + ym + zp]);
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = ym;
                                            stodef.z = zp;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        } else
                            sdirmr |= (0x08);

                        if (yp < mmax_m_xy) {
                            this_s = mtsurfaces_[x + yp + z];
                            upper_s = mtsurfaces_[x + yp + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s))
                            {
                                sdirm |= (0x01 | 0x02);
                                nxtfp = &(mdpoints_[x + yp + zp]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = yp;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (isStairs(this_s) && sWalkable(this_s,
                                upper_s))
                            {
                                sdirmr |= (0x80 | 0x02);
                                if (this_s == 0x01 || this_s == 0x03 || this_s == 0x04) {
                                    sdirl |= 0x01;
                                }
                                nxtfp = &(mdpoints_[x + yp + z]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = yp;
                                    stodef.z = z;
                                    vtodefine.push_back(stodef);
                                }
                            } else {
                                sdirmr |= (0x80 | 0x02);
                                if ((zp + mmax_m_xy) < mmax_m_all
                                    && (upper_s == 0x02 || upper_s == 0x03
                                    || upper_s == 0x04))
                                {
                                    if (sWalkable(upper_s,
                                        mtsurfaces_[x + yp + (zp + mmax_m_xy)]))
                                    {
                                        sdirm |= 0x01;
                                        nxtfp = &(mdpoints_[x + yp + zp]);
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = yp;
                                            stodef.z = zp;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        } else
                            sdirmr |= (0x80 | 0x02);
                        sdirm &= (0xFF ^ sdirmr);

                        // edges
                        if (xp < mmax_x_) {
                            if (ym >= 0 && (sdirm & 0x08) != 0) {
                                nxtfp = &(mdpoints_[xp + ym + zp]);
                                this_s = mtsurfaces_[xp + ym + z];
                                upper_s = mtsurfaces_[xp + ym + zp];
                                if (!(isSurface(this_s) && sWalkable(this_s,
                                    upper_s)))
                                {
                                    sdirm &= (0xFF ^ 0x08);
                                } else if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = ym;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            }

                            if (yp < mmax_m_xy && (sdirm & 0x02) != 0) {
                                nxtfp = &(mdpoints_[xp + yp + zp]);
                                this_s = mtsurfaces_[xp + yp + z];
                                upper_s = mtsurfaces_[xp + yp + zp];
                                if (!(isSurface(this_s) && sWalkable(this_s,
                                    upper_s)))
                                {
                                    sdirm &= (0xFF ^ 0x02);
                                } else if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = yp;
                                    stodef.z = z;
                                    vtodefine.push_back(stodef);
                                }
                            }
                        }
                        cfp->dirm = sdirm;
                        cfp->dirh = sdirh;
                        cfp->dirl = sdirl;

                        break;
                    case 0x12:
                        cfp->bfNodeDesc = m_fdWalkable;
                        cfp->bfNodeDesc |= m_fdSafeWalk;
                        if (zm >= 0) {
                            mdpoints_[x + y + zm].bfNodeDesc = m_fdNonWalkable;
                            if (ym >= 0) {
                                this_s = mtsurfaces_[x + ym + zm];
                                upper_s = mtsurfaces_[x + ym + z];
                                if (isSurface(this_s)) {
                                    nxtfp = &(mdpoints_[x + ym + z]);
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x10;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = ym;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                } else if (isStairs(upper_s) && upper_s != 0x01) {
                                    nxtfp = &(mdpoints_[x + ym + z]);
                                    this_s = upper_s;
                                    upper_s = mtsurfaces_[x + ym + zp];
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x10;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = ym;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                            if (xm >= 0) {
                                this_s = mtsurfaces_[xm + y + zm];
                                upper_s = mtsurfaces_[xm + y + z];
                                if (isSurface(this_s)) {
                                    nxtfp = &(mdpoints_[xm + y + z]);
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x40;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xm;
                                            stodef.y = y;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                } else if (isStairs(upper_s) && upper_s != 0x04) {
                                    nxtfp = &(mdpoints_[xm + y + z]);
                                    this_s = upper_s;
                                    upper_s = mtsurfaces_[xm + y + zp];
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x40;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xm;
                                            stodef.y = y;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                            if (xp < mmax_x_) {
                                this_s = mtsurfaces_[xp + y + zm];
                                upper_s = mtsurfaces_[xp + y + z];
                                if (isSurface(this_s)) {
                                    nxtfp = &(mdpoints_[xp + y + z]);
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x04;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xp;
                                            stodef.y = y;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                } else if (isStairs(upper_s) && upper_s != 0x03) {
                                    nxtfp = &(mdpoints_[xp + y + z]);
                                    this_s = upper_s;
                                    upper_s = mtsurfaces_[xp + y + zp];
                                    if (sWalkable(this_s, upper_s)) {
                                        sdirl |= 0x04;
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xp;
                                            stodef.y = y;
                                            stodef.z = z;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        }

                        if (xm >=0) {
                            this_s = mtsurfaces_[xm + y + z];
                            upper_s = mtsurfaces_[xm + y + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s))
                            {
                                sdirm |= (0x40 | 0x80);
                                nxtfp = &(mdpoints_[xm + y + zp]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xm;
                                    stodef.y = y;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (isStairs(this_s) && sWalkable(this_s,
                                upper_s))
                            {
                                sdirmr |= (0x20 | 0x80);
                                if (this_s == 0x01 || this_s == 0x02 || this_s == 0x03){
                                    sdirl |= 0x40;
                                }
                                nxtfp = &(mdpoints_[xm + y + z]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xm;
                                    stodef.y = y;
                                    stodef.z = z;
                                    vtodefine.push_back(stodef);
                                }
                            } else {
                                sdirmr |= (0x20 | 0x80);
                                if ((zp + mmax_m_xy) < mmax_m_all
                                    && (upper_s == 0x01 || upper_s == 0x02
                                    || upper_s == 0x04))
                                {
                                    if (sWalkable(upper_s,
                                        mtsurfaces_[xm + y + (zp + mmax_m_xy)]))
                                    {
                                        sdirm |= 0x40;
                                        nxtfp = &(mdpoints_[xm + y + zp]);
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xm;
                                            stodef.y = y;
                                            stodef.z = zp;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        } else
                            sdirmr |= (0x20 | 0x80);

                        if (xp < mmax_x_) {
                            this_s = mtsurfaces_[xp + y + z];
                            upper_s = mtsurfaces_[xp + y + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s))
                            {
                                sdirm |= (0x02 | 0x04);
                                nxtfp = &(mdpoints_[xp + y + zp]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = y;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (isStairs(this_s) && sWalkable(this_s,
                                upper_s))
                            {
                                sdirmr |= (0x02 | 0x08);
                                if (this_s == 0x01 || this_s == 0x02
                                    || this_s == 0x04)
                                {
                                    sdirl |= 0x04;
                                }
                                nxtfp = &(mdpoints_[xp + y + z]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = y;
                                    stodef.z = z;
                                    vtodefine.push_back(stodef);
                                }
                            } else {
                                sdirmr |= (0x02 | 0x08);
                                if ((zp + mmax_m_xy) < mmax_m_all
                                    && (upper_s == 0x01 || upper_s == 0x02
                                    || upper_s == 0x03))
                                {
                                    if (sWalkable(upper_s,
                                        mtsurfaces_[xp + y + (zp + mmax_m_xy)]))
                                    {
                                        sdirm |= 0x04;
                                        nxtfp = &(mdpoints_[xp + y + zp]);
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = xp;
                                            stodef.y = y;
                                            stodef.z = zp;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        } else
                            sdirmr |= (0x02 | 0x08);

                        if (yp < mmax_m_xy) {
                            this_s = mtsurfaces_[x + yp + z];
                            upper_s = mtsurfaces_[x + yp + zp];
                            if (isSurface(this_s) && sWalkable(this_s, upper_s))
                            {
                                sdirm |= (0x80 | 0x01 | 0x02);
                                nxtfp = &(mdpoints_[x + yp + zp]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = yp;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            } else if (isStairs(this_s) && sWalkable(this_s,
                                upper_s))
                            {
                                sdirmr |= (0x80 | 0x02);
                                if (this_s == 0x01 || this_s == 0x03 || this_s == 0x04) {
                                    sdirl |= 0x01;
                                }
                                nxtfp = &(mdpoints_[x + yp + z]);
                                if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = x;
                                    stodef.y = yp;
                                    stodef.z = z;
                                    vtodefine.push_back(stodef);
                                }
                            } else {
                                sdirmr |= (0x80 | 0x02);
                                if ((zp + mmax_m_xy) < mmax_m_all
                                    && (upper_s == 0x02 || upper_s == 0x03
                                    || upper_s == 0x04))
                                {
                                    if (sWalkable(upper_s,
                                        mtsurfaces_[x + yp + (zp + mmax_m_xy)]))
                                    {
                                        sdirm |= 0x01;
                                        nxtfp = &(mdpoints_[x + yp + zp]);
                                        if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                            nxtfp->bfNodeDesc = m_fdDefReq;
                                            stodef.x = x;
                                            stodef.y = yp;
                                            stodef.z = zp;
                                            vtodefine.push_back(stodef);
                                        }
                                    }
                                }
                            }
                        } else
                            sdirmr |= (0x80 | 0x02);
                        sdirm &= (0xFF ^ sdirmr);

                        // edges
                        if (yp < mmax_m_xy) {
                            if (xm >= 0 && (sdirm & 0x80) != 0) {
                                nxtfp = &(mdpoints_[xm + yp + zp]);
                                this_s = mtsurfaces_[xm + yp + z];
                                upper_s = mtsurfaces_[xm + yp + zp];
                                if (!(isSurface(this_s) && sWalkable(this_s,
                                    upper_s)))
                                {
                                    sdirm &= (0xFF ^ 0x80);
                                } else if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xm;
                                    stodef.y = yp;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            }
                            if (xp < mmax_x_ && (sdirm & 0x02) != 0) {
                                nxtfp = &(mdpoints_[xp + yp + zp]);
                                this_s = mtsurfaces_[xp + yp + z];
                                upper_s = mtsurfaces_[xp + yp + zp];
                                if (!(isSurface(this_s) && sWalkable(this_s,
                                    upper_s)))
                                {
                                    sdirm &= (0xFF ^ 0x02);
                                } else if (nxtfp->bfNodeDesc == m_fdNotDefined) {
                                    nxtfp->bfNodeDesc = m_fdDefReq;
                                    stodef.x = xp;
                                    stodef.y = yp;
                                    stodef.z = zp;
                                    vtodefine.push_back(stodef);
                                }
                            }
                        }

                        cfp->dirm = sdirm;
                        cfp->dirh = sdirh;
                        cfp->dirl = sdirl;

                        break;
                }
            } while (vtodefine.size());
        }
    }
#if 0
    unsigned int cw = 0;
    for (int iz = 0; iz < mmax_z_; iz++) {
        for (int iy = 0; iy < mmax_y_; iy++) {
            for (int ix = 0; ix < mmax_x_; ix++) {
                floodPointDesc *cfpp = &(mdpoints_[ix + iy * mmax_x_ + iz * mmax_m_xy]);

                if ((cfpp->bfNodeDesc & m_fdWalkable) == m_fdWalkable)
                    cw++;
            }
        }
    }

    printf("flood walkables %i\n", cw);
#endif
    return true;
}

void Mission::clrSurfaces() {

    if(mtsurfaces_ != NULL) {
        free(mtsurfaces_);
        mtsurfaces_ = NULL;
    }
    if(mdpoints_ != NULL) {
        free(mdpoints_);
        mdpoints_ = NULL;
    }
    if(mdpoints_cp_ != NULL) {
        free(mdpoints_cp_);
        mdpoints_cp_ = NULL;
    }
}

/*!
 * Uses isometric coordinates transformation to find walkable tile,
 * starting from top.
 * \param mtp
 * \return True if walkable tile found
 */
bool Mission::getWalkable(TilePoint &mtp) {
    bool gotit = false;
    int bx, by, box, boy;
    int bz = mmax_z_;
    unsigned int cindx;
    unsigned char twd;
    do {
        bz--;
        // using lowered Z, at start postion is at top of tile not at bottom
        bx = mtp.tx * 256 + mtp.ox + 128 * (bz - 1);
        box = bx % 256;
        bx = bx / 256;
        by = mtp.ty * 256 + mtp.oy + 128 * (bz - 1);
        boy = by % 256;
        by = by / 256;
        if (bz >= mmax_z_ || bx >= mmax_x_ || by >= mmax_y_)
            continue;
        if (bz < 0 || bx < 0 || by < 0)
            break;
        cindx = bx + by * mmax_x_ + bz * mmax_m_xy;
        if ((mdpoints_[cindx].bfNodeDesc & m_fdWalkable) == m_fdWalkable) {
            twd = mtsurfaces_[cindx];
            int dx = 0;
            int dy = 0;
            switch (twd) {
                case 0x01:
                    dy = ((boy + 128) * 2) / 3;
                    dx = box + 128 - dy / 2;
                    if (dx < 256) {
                        gotit = true;
                        box = dx;
                        boy = dy;
                    } else {
                        if ((bx + 1) < mmax_x_) {
                            cindx = (bx + 1) + by * mmax_x_ + bz * mmax_m_xy;
                            if ((mdpoints_[cindx].bfNodeDesc & m_fdWalkable) == m_fdWalkable
                                && mtsurfaces_[cindx] == 0x01)
                            {
                                gotit = true;
                                bx++;
                                box = dx - 256;
                                boy = dy;
                            }
                        }
                    }
                    break;
                case 0x02:
                    if (boy < 128) {
                        dy = boy * 2;
                        dx = box + boy;
                        if (dx < 256) {
                            gotit = true;
                            box = dx;
                            boy = dy;
                        } else {
                            if ((bx + 1) < mmax_x_) {
                                cindx = (bx + 1) + by * mmax_x_ + bz * mmax_m_xy;
                                if ((mdpoints_[cindx].bfNodeDesc & m_fdWalkable) == m_fdWalkable
                                    && mtsurfaces_[cindx] == 0x02)
                                {
                                    gotit = true;
                                    bx++;
                                    box = dx - 256;
                                    boy = dy;
                                }
                            }
                        }
                    } else {
                        // TODO : add check 0x01?
                    }
                    break;
                case 0x03:
                    if (box < 128) {
                        dx = box * 2;
                        dy = box + boy;
                        if (dy < 256) {
                            gotit = true;
                            box = dx;
                            boy = dy;
                        } else {
                            if ((by + 1) < mmax_y_) {
                                cindx = bx + (by + 1) * mmax_x_ + bz * mmax_m_xy;
                                if ((mdpoints_[cindx].bfNodeDesc & m_fdWalkable) == m_fdWalkable
                                    && mtsurfaces_[cindx] == 0x03)
                                {
                                    gotit = true;
                                    by++;
                                    box = dx;
                                    boy = dy - 256;
                                }
                            }
                        }
                    } else {
                        // TODO : 0x04?
                    }
                    break;
                case 0x04:
                    dx = ((box + 128) * 2) / 3;
                    dy = boy + 128 - dx / 2;
                    if (dy < 256) {
                        gotit = true;
                        box = dx;
                        boy = dy;
                    } else {
                        if ((by + 1) < mmax_y_) {
                            cindx = bx + (by + 1) * mmax_x_ + bz * mmax_m_xy;
                            if ((mdpoints_[cindx].bfNodeDesc & m_fdWalkable) == m_fdWalkable
                                && mtsurfaces_[cindx] == 0x04)
                            {
                                gotit = true;
                                by++;
                                box = dx;
                                boy = dy - 256;
                            }
                        }
                    }
                    break;
                default:
                    gotit = true;
                    // TODO: 0x01 0x02 0x03 0x04?
                break;
            }
        } else if (bz - 1 >= 0) {
            cindx = bx + by * mmax_x_ + (bz - 1) * mmax_m_xy;
            if ((mdpoints_[cindx].bfNodeDesc & m_fdWalkable) == m_fdWalkable) {
                twd = mtsurfaces_[cindx];
                int dx = 0;
                int dy = 0;
                switch (twd) {
                    case 0x01:
                        dy = (boy * 2) / 3;
                        dx = box - dy / 2;
                        if (dx >= 0) {
                            gotit = true;
                            box = dx;
                            boy = dy;
                            bz--;
                        }
                        break;
                    case 0x02:
                        dy = (boy - 128) * 2;
                        dx = (box + dy / 2) - 128;
                        if (dy >= 0 && dx >= 0 && dx < 256) {
                            gotit = true;
                            box = dx;
                            boy = dy;
                            bz--;
                        }
                        break;
                    case 0x03:
                        dx = (box - 128) * 2;
                        dy = (boy + dx / 2) - 128;
                        if (dx >= 0 && dy >= 0 && dy < 256) {
                            gotit = true;
                            box = dx;
                            boy = dy;
                            bz--;
                        }
                        break;
                    case 0x04:
                        dx = (box * 2) / 3;
                        dy = boy - dx / 2;
                        if (dy >= 0) {
                            gotit = true;
                            box = dx;
                            boy = dy;
                            bz--;
                        }
                        break;
                    default:
                        break;
                }

            }
        }
    } while (bz != 0 && !gotit);
    if (gotit) {
        mtp.tx = bx;
        mtp.ty = by;
        mtp.tz = bz;
        mtp.ox = box;
        mtp.oy = boy;
    }
    return gotit;
}

bool Mission::getWalkableClosestByZ(TilePoint &mtp) {
    // NOTE: using z from mtp as start to find closest z
    int inc_z = mtp.tz;
    int dec_z = mtp.tz;
    bool found = false;

    do {
        if (inc_z < mmax_z_) {
            if ((mdpoints_[mtp.tx + mtp.ty * mmax_x_
                + inc_z * mmax_m_xy].bfNodeDesc & m_fdWalkable) == m_fdWalkable)
            {
                mtp.tz = inc_z;
                found = true;
                break;
            }
            inc_z++;
        }
        if (dec_z >= 0) {
            if ((mdpoints_[mtp.tx + mtp.ty * mmax_x_
                + dec_z * mmax_m_xy].bfNodeDesc & m_fdWalkable) == m_fdWalkable)
            {
                mtp.tz = dec_z;
                found = true;
                break;
            }
            dec_z--;
        }
    } while (inc_z < mmax_z_ || dec_z >= 0);

    return found;
}

/*!
* This function looks for blockers - statics, vehicles, peds, weapons
*/
MapObject * Mission::checkBlockedByObject(WorldPoint * pStartPt, WorldPoint * pEndPt,
        double *dist, const ShootableMapObject *pOrigin) {
    // TODO: calculating closest blocker first? (start point can be closer though)
    double inc_xyz[3];
    inc_xyz[0] = (pEndPt->x - pStartPt->x) / (*dist);
    inc_xyz[1] = (pEndPt->y - pStartPt->y) / (*dist);
    inc_xyz[2] = (pEndPt->z - pStartPt->z) / (*dist);
    WorldPoint copyStartPt = *pStartPt;
    WorldPoint copyEndPt = *pEndPt;
    WorldPoint blockStartPt;
    WorldPoint blockEndPt;
    double closest = *dist;
    MapObject *pBlocker = NULL;

    for (unsigned int i = 0; i < statics_.size(); ++i) {
        Static * s_blocker = statics_[i];
        if (s_blocker->isExcludedFromBlockers())
            continue;
        if (s_blocker->isBlocker(&copyStartPt, &copyEndPt, inc_xyz)) {
            int cx = pStartPt->x - copyStartPt.x;
            int cy = pStartPt->y - copyStartPt.y;
            int cz = pStartPt->z - copyStartPt.z;
            double dist_blocker = sqrt((double) (cx * cx + cy * cy + cz * cz));
            if (closest == -1 || dist_blocker < closest) {
                closest = dist_blocker;
                pBlocker = s_blocker;
                blockStartPt = copyStartPt;
                blockEndPt = copyEndPt;
            }
            copyStartPt = *pStartPt;
            copyEndPt = *pEndPt;
        }
    }
    // if shooter is a Ped and is shooting from a vehicle,
    // then skip that vehicle in the search
    Vehicle *pShooterVehicle = NULL;
    if (pOrigin && pOrigin->is(MapObject::kNaturePed)) {
        const PedInstance *pPed = static_cast<const PedInstance *>(pOrigin);
        pShooterVehicle = pPed->inVehicle(); // can be null
    }
    for (unsigned int i = 0; i < vehicles_.size(); ++i) {
        Vehicle * pVehicle = vehicles_[i];
        if (pVehicle != pShooterVehicle) {
            if (pVehicle->isBlocker(&copyStartPt, &copyEndPt, inc_xyz)) {
                int cx = pStartPt->x - copyStartPt.x;
                int cy = pStartPt->y - copyStartPt.y;
                int cz = pStartPt->z - copyStartPt.z;
                double dist_blocker = sqrt((double) (cx * cx + cy * cy + cz * cz));
                if (closest == -1 || dist_blocker < closest) {
                    closest = dist_blocker;
                    pBlocker = pVehicle;
                    blockStartPt = copyStartPt;
                    blockEndPt = copyEndPt;
                }
                copyStartPt = *pStartPt;
                copyEndPt = *pEndPt;
            }
        }
    }

    for (unsigned int i = 0; i < peds_.size(); ++i) {
        PedInstance * p_blocker = peds_[i];
        if (p_blocker->isAlive() && p_blocker != pOrigin && p_blocker->inVehicle() == NULL) {
            if (p_blocker->isBlocker(&copyStartPt, &copyEndPt, inc_xyz)) {
                int cx = pStartPt->x - copyStartPt.x;
                int cy = pStartPt->y - copyStartPt.y;
                int cz = pStartPt->z - copyStartPt.z;
                double dist_blocker = sqrt((double) (cx * cx + cy * cy + cz * cz));
                if (closest == -1 || dist_blocker < closest) {
                    closest = dist_blocker;
                    pBlocker = p_blocker;
                    blockStartPt = copyStartPt;
                    blockEndPt = copyEndPt;
                }
                copyStartPt = *pStartPt;
                copyEndPt = *pEndPt;
            }
        }
    }

    for (unsigned int i = 0; i < weaponsOnGround_.size(); ++i) {
        WeaponInstance *pWeapon = weaponsOnGround_[i];
        if (!pWeapon->hasOwner()) {
            if (pWeapon->isBlocker(&copyStartPt, &copyEndPt, inc_xyz)) {
                int cx = pStartPt->x - copyStartPt.x;
                int cy = pStartPt->y - copyStartPt.y;
                int cz = pStartPt->z - copyStartPt.z;
                double dist_blocker = sqrt((double) (cx * cx + cy * cy + cz * cz));
                if (closest == -1 || dist_blocker < closest) {
                    closest = dist_blocker;
                    pBlocker = pWeapon;
                    blockStartPt = copyStartPt;
                    blockEndPt = copyEndPt;
                }
                copyStartPt = *pStartPt;
                copyEndPt = *pEndPt;
            }
        }
    }
    if (pBlocker != NULL) {
        *pStartPt = blockStartPt;
        *pEndPt = blockEndPt;
        *dist = closest;
    }

    return pBlocker;
}

/*!
 * Verify that the path from originPosW to pTargetPosW is not blocked by a tile.
 * If such a tile exists, pTargetPosW is updated with the position of the blocking tile.
 * \param originPosW Path starting point
 * \param pTargetPosW Path end point
 * \param updateLoc Set to true to update pTargetPosW when blocking tile is found
 * \param distanceMax Maximum distance we cannot cross. If distanceMax is
 *   reached before pTargetPosW, then path is stopped.
 * \param pInitialDistance This is the distance between origin and initial target position
 * \return a bitmask indicating the type of result:
 *      - 0b(1) : target in range
 *      - 3b(8) : distanceMax is reached
 *      - 4b(16): blocker tile, "pTargetLoc" is set
 *      - 5b(32): out of visible reach
 */
uint8 Mission::checkBlockedByTile(const WorldPoint & originPosW, WorldPoint *pTargetPosW,
                                  bool updateLoc, double distanceMax, double *pInitialDistance) {
    // TODO: some objects mid point is higher then map z
    assert(distanceMax >= 0);

    int cx = originPosW.x;
    int cy = originPosW.y;
    int cz = originPosW.z;
    if (cz > (mmax_z_ - 1) * 128)
        return kBMaskBlockerTargetOutOfMap;

    // This variable will store the target location as it may moves if
    // a tile blocks the path.
    WorldPoint tmpTargetWLoc = *pTargetPosW;

    if (tmpTargetWLoc.z > (mmax_z_ - 1) * 128)
        return kBMaskBlockerTargetOutOfMap;

    // This is the distance between the origin and the target
    double distanceToTarget = 0;
    distanceToTarget = sqrt((double)((tmpTargetWLoc.x - cx) * (tmpTargetWLoc.x - cx) + (tmpTargetWLoc.y - cy) * (tmpTargetWLoc.y - cy)
        + (tmpTargetWLoc.z - cz) * (tmpTargetWLoc.z - cz)));
    uint8 block_mask = 1;

    if (pInitialDistance)
        *pInitialDistance = distanceToTarget;
    if (distanceToTarget == 0)
        return block_mask;

    double sx = (double) cx;
    double sy = (double) cy;
    double sz = (double) cz;

    if (distanceToTarget >= distanceMax) {
        // the distance we have to cross (distanceToTarget) is higher than the maximum
        // distance we are allowed to cross (distanceMax)

        // update target position according to distanceMax
        double dist_k = (double)distanceMax / distanceToTarget;
        tmpTargetWLoc.x = cx + (int)((tmpTargetWLoc.x - cx) * dist_k);
        tmpTargetWLoc.y = cy + (int)((tmpTargetWLoc.y - cy) * dist_k);
        tmpTargetWLoc.z = cz + (int)((tmpTargetWLoc.z - cz) * dist_k);
        // set mask to indicate distanceMax is reached
        block_mask = 8;
        if (updateLoc) {
            *pTargetPosW = tmpTargetWLoc;
        }
        distanceToTarget = distanceMax;
    }

    // NOTE: these values are less then 1.
    // If they are incremented, time required to check range will be shorter but less precise check,
    // If decremented longer but more precise.
    // Increment is (n * 8)
    double incrX = ((tmpTargetWLoc.x - cx) * 8) / distanceToTarget;
    double incrY = ((tmpTargetWLoc.y - cy) * 8) / distanceToTarget;
    double incrZ = ((tmpTargetWLoc.z - cz) * 8) / distanceToTarget;

    int oldx = cx / 256;
    int oldy = cy / 256;
    int oldz = cz / 128;
    double dist_close = distanceToTarget;
    // look note before, should be same increment
    double dist_dec = 1.0 * 8;

    while (dist_close > dist_dec) {
        int nx = (int)sx / 256;
        int ny = (int)sy / 256;
        int nz = (int)sz / 128;
        unsigned char twd = mtsurfaces_[nx + ny * mmax_x_
            + nz * mmax_m_xy];
        if (oldx != nx || oldy != ny || oldz != nz
            || (twd >= 0x01 && twd <= 0x04))
        {
            if (!(twd == 0x00 || twd == 0x0C || twd == 0x10)) {
                bool is_blocked = false;
                int offz = (int)sz % 128;
                switch (twd) {
                    case 0x01:
                        if (offz <= (127 - (((int)sy % 256) >> 1)))
                            is_blocked = true;
                        break;
                    case 0x02:
                        if (offz <= (((int)sy % 256) >> 1))
                            is_blocked = true;
                        break;
                    case 0x03:
                        if (offz <= (((int)sx % 256) >> 1))
                            is_blocked = true;
                        break;
                    case 0x04:
                        if (offz <= (127 - (((int)sx % 256) >> 1)))
                            is_blocked = true;
                        break;
                    default:
                        is_blocked = true;
                }
                if (is_blocked) {
                    sx -= incrX;
                    sy -= incrY;
                    sz -= incrZ;
                    double dsx = sx - (double)cx;
                    double dsy = sy - (double)cy;
                    double dsz = sz - (double)cz;
                    tmpTargetWLoc.x = (int)sx;
                    tmpTargetWLoc.y = (int)sy;
                    tmpTargetWLoc.z = (int)sz;
                    dist_close = sqrt(dsx * dsx + dsy * dsy + dsz * dsz);
                    // set mask to indicate path is blocked by a tile
                    if (block_mask == 1)
                        block_mask = 16;
                    else
                        block_mask |= 16;
                    if (updateLoc) {
                        pTargetPosW->x = (int)sx;
                        pTargetPosW->y = (int)sy;
                        pTargetPosW->z = (int)sz;
                    }
                    break;
                }
            }
            oldx = nx;
            oldy = ny;
            oldz = nz;
        }
        sx += incrX;
        sy += incrY;
        sz += incrZ;
        dist_close -= dist_dec;
    } // end while

    return block_mask;
}

/*!
 * \param originLoc
 * \param pTarget
 * \param pTargetPosW
 * \param setBlocker
 * \param checkTileOnly Check blockers only for map elements not objects.
 * \param maxr maximum distance we can run
 * \param distTo
 * \param pOrigin
 * \return mask where bits are:
 * - 0b : target in range(1)
 * - 1b : blocker is object, "t" is set(2)
 * - 2b : blocker object, "pn" is set(4)
 * - 3b : reachable point set (8)
 * - 4b : blocker tile, "pn" is set(16)
 * - 5b : out of visible reach(32)
 * NOTE: only if "pn" or "t" are not null, variables are set

*/
uint8 Mission::checkIfBlockersInShootingLine(const WorldPoint & originLoc, ShootableMapObject ** pTarget,
    WorldPoint *pTargetPosW, bool setBlocker, bool checkTileOnly, double maxr,
    double * distTo, const ShootableMapObject *pOrigin)
{
    // search for a tile blocking the path towards the target
    // tmp will hold the updated position after that search
    WorldPoint tmpPosW;
    if (pTarget && *pTarget) {
        tmpPosW.convertFromTilePoint((*pTarget)->position());
    } else {
        tmpPosW = *pTargetPosW;
    }

    uint8 bfBlockerFound = checkBlockedByTile(originLoc, &tmpPosW, true, maxr, distTo);
    if (bfBlockerFound == kBMaskBlockerTargetOutOfMap) {
        // coords are out of map limits
        return bfBlockerFound;
    }

    if (setBlocker) {
        *pTargetPosW = tmpPosW;
    }

    if (checkTileOnly)
        return bfBlockerFound;

    WorldPoint tmpOrigin = originLoc;
    WorldPoint tmpEnd = tmpPosW;

    // We search for a possible object blocking the way on the path
    // between origin and the reached position
    int dx = tmpPosW.x - originLoc.x;
    int dy = tmpPosW.y - originLoc.y;
    int dz = tmpPosW.z - originLoc.z;
    double distToBlocker = sqrt((double)(dx * dx + dy * dy + dz * dz));
    MapObject *blockerObj = checkBlockedByObject(&tmpOrigin, &tmpEnd, &distToBlocker, pOrigin);

    if (blockerObj) {
        if (bfBlockerFound == 1)
            bfBlockerFound = 0;

        if (setBlocker) {
            if (pTargetPosW) {
                *pTargetPosW = tmpOrigin;
                bfBlockerFound |= kBMaskBlockerTargetPosUpdated;
            }
            if (pTarget) {
                *pTarget = (ShootableMapObject *)blockerObj;
                bfBlockerFound |= kBMaskBlockerTargetObjectUpdated;
            }
        } else {
            if (pTarget && *pTarget) {
                if (*pTarget != blockerObj)
                    bfBlockerFound |= 6;
                else
                    bfBlockerFound = 1;
            } else
                bfBlockerFound |= 6;
        }
    } else {
        if (setBlocker) {
            if (bfBlockerFound != 1 && pTarget)
                *pTarget = NULL;
        }
    }

    return bfBlockerFound;
}

/*!
 * Returns the length of the path between a ped and a object if such a path exists and it is
 * shorter than the maximum length allowed.
 * \param pPed The origin of the path
 * \param objectToReach The end of the path
 * \param maxLength The length of the path must not exceed this value
 * \param length The returned length if the path exists
 * \return 0 if a path exists, else path does not exist so length is not set.
 */
uint8 Mission::getPathLengthBetween(PedInstance *pPed, ShootableMapObject* objectToReach, double maxLength, double *length) {
    WorldPoint cur_xyz(pPed->position());
    cur_xyz.z += (pPed->sizeZ() >> 1);
    // TODO : it's not inRangeCPos that must be called but a method for path calculation
    uint8 res = checkIfBlockersInShootingLine(cur_xyz, &objectToReach, NULL, false, true, maxLength, length, pPed);
    return res == 1 ? 0 : 1;
}

bool Mission::getShootableTile(TilePoint *pLocT)
{
    // TODO: review later
    bool gotit = false;
    int bx, by, box, boy;
    int bz = mmax_z_;
    unsigned char twd;
    unsigned int cindx;
    do {
        --bz;
        int bzm = bz - 1;
        bx = pLocT->tx * 256 + pLocT->ox + 128 * bzm;
        box = bx % 256;
        bx = bx / 256;
        by = pLocT->ty * 256 + pLocT->oy + 128 * bzm;
        boy = by % 256;
        by = by / 256;
        if (bz >= mmax_z_ || bx >= mmax_x_ || by >= mmax_y_)
            continue;
        if (bz < 0 || bx < 0 || by < 0)
            break;
        twd = mtsurfaces_[bx + by * mmax_x_ + bzm * mmax_m_xy];
        int dx = 0;
        int dy = 0;
        switch (twd) {
            case 0x01:
                dy = (boy * 2) / 3;
                dx = box - dy / 2;
                if (dx >= 0) {
                    gotit = true;
                    box = dx;
                    boy = dy;
                } else {
                    if ((bx - 1) >= 0) {
                        cindx = (bx - 1) + by * mmax_x_ + bzm * mmax_m_xy;
                        if (mtsurfaces_[cindx] == 0x01) {
                            gotit = true;
                            --bx;
                            box = dx + 256;
                            boy = dy;
                        }
                    }
                }
                break;
            case 0x02:
                dy = (boy - 128) * 2;
                dx = (box + dy / 2) - 128;
                if (dy >= 0) {
                    if (dx >= 0) {
                        if (dx < 256) {
                            gotit = true;
                            box = dx;
                            boy = dy;
                        } else {
                            if ((bx + 1) < mmax_x_) {
                                cindx = (bx + 1) + by * mmax_x_ + bzm * mmax_m_xy;
                                if (mtsurfaces_[cindx] == 0x02) {
                                    gotit = true;
                                    ++bx;
                                    box = dx - 256;
                                    boy = dy;
                                }
                            }
                        }
                    } else {
                        if ((bx - 1) >= 0) {
                            cindx = (bx - 1) + by * mmax_x_ + bzm * mmax_m_xy;
                            if (mtsurfaces_[cindx] == 0x02) {
                                gotit = true;
                                --bx;
                                box = dx + 256;
                                boy = dy;
                            }
                        }
                    }
                }
                break;
            case 0x03:
                dx = (box - 128) * 2;
                dy = (boy + dx / 2) - 128;
                if (dx >= 0) {
                    if (dy >= 0) {
                        if (dy < 256) {
                            gotit = true;
                            box = dx;
                            boy = dy;
                        } else {
                            if ((by + 1) < mmax_y_) {
                                cindx = bx + (by + 1) * mmax_x_ + bzm * mmax_m_xy;
                                if (mtsurfaces_[cindx] == 0x03) {
                                    gotit = true;
                                    by++;
                                    box = dx;
                                    boy = dy - 256;
                                }
                            }
                        }
                    } else {
                        if ((by - 1) >= 0) {
                            cindx = bx + (by - 1) * mmax_x_ + bzm * mmax_m_xy;
                            if (mtsurfaces_[cindx] == 0x03) {
                                gotit = true;
                                --by;
                                box = dx;
                                boy = dy + 256;
                            }
                        }
                    }
                }
                break;
            case 0x04:
                dx = (box * 2) / 3;
                dy = boy - dx / 2;
                if (dy >= 0) {
                    gotit = true;
                    box = dx;
                    boy = dy;
                } else {
                    if ((by - 1) >= 0) {
                        cindx = bx + (by - 1) * mmax_x_ + bzm * mmax_m_xy;
                        if (mtsurfaces_[cindx] == 0x04) {
                            gotit = true;
                            --by;
                            box = dx;
                            boy = dy + 256;
                        }
                    }
                }
                break;
            default:
                if (!(twd == 0x00 || twd == 0x0C || twd == 0x10))
                    gotit = true;
            break;
        }
       if (!gotit) {
            if (box < 128 && (bx - 1) >= 0) {
                cindx = (bx - 1) + by * mmax_x_ + bzm * mmax_m_xy;
                twd = mtsurfaces_[cindx];
                if (twd == 0x01) {
                    dy = (boy * 2) / 3;
                    dx = (box + 256) - dy / 2;
                    if (dx < 256) {
                        --bx;
                        gotit = true;
                        box = dx;
                        boy = dy;
                    }
                } else if (twd == 0x04) {
                    dx = ((box + 256) * 2) / 3;
                    dy = boy - dx / 2;
                    if (dy >= 0) {
                        --bx;
                        gotit = true;
                        box = dx;
                        boy = dy;
                    }
                }
            }
            if (!gotit && boy < 128 && (by - 1) >= 0) {
                cindx = bx + (by - 1) * mmax_x_ + bzm * mmax_m_xy;
                twd = mtsurfaces_[cindx];
                if (twd == 0x01) {
                    dy = ((boy + 256) * 2) / 3;
                    dx = box - dy / 2;
                    if (dx >= 0) {
                        --by;
                        gotit = true;
                        box = dx;
                        boy = dy;
                    }
                } else if (twd == 0x04) {
                    dx = (box * 2) / 3;
                    dy = (boy + 256) - dx / 2;
                    if (dy < 256) {
                        --by;
                        gotit = true;
                        box = dx;
                        boy = dy;
                    }
                }
                if(!gotit && box < 128 && (bx - 1) >= 0) {
                    --cindx;
                    int dx2 = 0;
                    int dy2 = 0;
                    twd = mtsurfaces_[cindx];
                    if (twd == 0x01) {
                        dy2 = ((boy + 256) * 2) / 3;
                        dx2 = (box + 256) - dy2 / 2;
                        if (dx2 < 256 && dy2 < 256) {
                            --bx;
                            --by;
                            gotit = true;
                            box = dx2;
                            boy = dy2;
                        }
                    } else if (twd == 0x04) {
                        dx2 = ((box + 256) * 2) / 3;
                        dy2 = (boy + 256) - dx2 / 2;
                        if (dx2 < 256 && dy2 < 256) {
                            --bx;
                            --by;
                            gotit = true;
                            box = dx2;
                            boy = dy2;
                        }
                    }
                }
            }
       }
    } while (bz != 0 && !gotit);
    if (gotit) {
        twd = mtsurfaces_[bx + by * mmax_x_ + (bz - 1) * mmax_m_xy];
        switch (twd) {
            case 0x01:
                pLocT->oz = 127 - (boy >> 1);
                --bz;
                break;
            case 0x02:
                pLocT->oz = boy >> 1;
                --bz;
                break;
            case 0x03:
                pLocT->oz = box >> 1;
                --bz;
                break;
            case 0x04:
                pLocT->oz = 127 - (box >> 1);
                --bz;
                break;
            default:
                twd = mtsurfaces_[bx + by * mmax_x_ + bz * mmax_m_xy];
                if (!(twd == 0x00 || twd == 0x0C || twd == 0x10)) {
                    // recalculating point of collision
                    if (box > 192 || boy > 192) {
                        if (box >= boy)
                            pLocT->oz = (256 - box) << 1;
                        else if (boy > box)
                            pLocT->oz = (256 - boy) << 1;
                    } else
                        pLocT->oz = 128;

                    bx = pLocT->tx * 256 + pLocT->ox + 128 * (bz - 1) + pLocT->oz;
                    box = bx % 256;
                    bx = bx / 256;
                    by = pLocT->ty * 256 + pLocT->oy + 128 * (bz - 1) + pLocT->oz;
                    boy = by % 256;
                    by = by / 256;
                    bz += pLocT->oz / 128;
                    pLocT->oz %= 128;
                } else
                    pLocT->oz = 0;
        }
        pLocT->tx = bx;
        pLocT->ty = by;
        pLocT->tz = bz;
        pLocT->ox = box;
        pLocT->oy = boy;
        assert(bz >= 0);
    }
    return gotit;
}

bool Mission::isTileSolid(int x, int y, int z, int ox, int oy, int oz) {
    bool solid = true;
    uint8 twd = mtsurfaces_[x + y * mmax_x_ + z * mmax_m_xy];
    switch (twd) {
        case 0x00:
        case 0x0C:
        case 0x10:
            solid = false;
            break;
        case 0x01:
            if (oz > (127 - (oy >> 1)))
                solid = false;
            break;
        case 0x02:
            if (oz > (oy >> 1))
                solid = false;
            break;
        case 0x03:
            if (oz > (ox >> 1))
                solid = false;
            break;
        case 0x04:
            if (oz > (127 - (ox >> 1)))
                solid = false;
            break;
        default:
            break;
    }

    return solid;
}

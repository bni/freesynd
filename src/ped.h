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

#ifndef PED_H
#define PED_H

#include <map>
#include <set>

#include "common.h"
#include "mapobject.h"
#include "modowner.h"
#include "gfx/spritemanager.h"
#include "model/weaponholder.h"
#include "model/weapon.h"
#include "ipastim.h"
#include "ia/actions.h"
#include "ia/behaviour.h"

class Agent;
class Mission;
class Squad;
class GenericCar;
class Vehicle;

#define NUM_ANIMS 10

/*!
 * Pedestrian class.
 */
class Ped {
public:
    Ped();
    virtual ~Ped() {}

    void setStandAnim(Weapon::WeaponAnimIndex weapon, int anim) {
        assert(weapon < NUM_ANIMS);
        stand_anims_[weapon] = anim;
    }

    void setWalkAnim(Weapon::WeaponAnimIndex weapon, int anim) {
        assert(weapon < NUM_ANIMS);
        walk_anims_[weapon] = anim;
    }

    void setStandFireAnim(Weapon::WeaponAnimIndex weapon, int anim) {
        assert(weapon < NUM_ANIMS);
        stand_fire_anims_[weapon] = anim;
    }

    void setWalkFireAnim(Weapon::WeaponAnimIndex weapon, int anim) {
        assert(weapon < NUM_ANIMS);
        walk_fire_anims_[weapon] = anim;
    }

    void setDieAgentAnim(int anim) { die_agent_anim_ = anim; }
    void setDeadAgentAnim(int anim) { dead_agent_anim_ = anim; }
    void setDieAnim(int anim) { die_anim_ = anim; }
    void setDeadAnim(int anim) { dead_anim_ = anim; }
    void setHitAnim(int anim) { hit_anim_ = anim; }
    void setPickupAnim(int anim) { pickup_anim_ = anim; }

    void setVaporizeAnim(int anim) { vaporize_anim_ = anim; }
    void setSinkAnim(int anim) { sink_anim_ = anim; }
    void setStandBurnAnim(int anim) { stand_burn_anim_ = anim; }
    void setWalkBurnAnim(int anim) { walk_burn_anim_ = anim; }
    void setDieBurnAnim(int anim) { die_burn_anim_ = anim; }
    void setSmokeBurnAnim(int anim) { smoke_burn_anim_ = anim; }
    void setDeadBurnAnim(int anim) { dead_burn_anim_ = anim; }

    void setPersuadeAnim(int anim) { persuade_anim_ = anim; }

    bool drawStandFrame(int x, int y, int dir, int frame,
            Weapon::WeaponAnimIndex weapon = Weapon::Unarmed_Anim);
    int lastStandFrame(int dir, Weapon::WeaponAnimIndex weapon);
    bool drawWalkFrame(int x, int y, int dir, int frame,
            Weapon::WeaponAnimIndex weapon = Weapon::Unarmed_Anim);
    int lastWalkFrame(int dir, Weapon::WeaponAnimIndex weapon);
    bool drawStandFireFrame(int x, int y, int dir, int frame,
            Weapon::WeaponAnimIndex weapon);
    int lastStandFireFrame(int dir, Weapon::WeaponAnimIndex weapon);
    bool drawWalkFireFrame(int x, int y, int dir, int frame,
            Weapon::WeaponAnimIndex weapon);
    int lastWalkFireFrame(int dir, Weapon::WeaponAnimIndex weapon);
    bool drawDieFrame(int x, int y, int frame);
    int lastDieFrame();

    void drawDeadFrame(int x, int y, int frame);
    void drawDeadAgentFrame(int x, int y, int frame);

    void drawHitFrame(int x, int y, int dir, int frame);
    int lastHitFrame(int dir);
    void drawPickupFrame(int x, int y, int frame);
    int lastPickupFrame();
    void drawVaporizeFrame(int x, int y, int dir, int frame);
    int lastVaporizeFrame(int dir);
    void drawSinkFrame(int x, int y, int frame);
    int lastSinkFrame();

    void drawStandBurnFrame(int x, int y, int frame);
    void drawWalkBurnFrame(int x, int y, int frame);
    void drawDieBurnFrame(int x, int y, int frame);
    int lastDieBurnFrame();
    void drawSmokeBurnFrame(int x, int y, int frame);
    void drawDeadBurnFrame(int x, int y, int frame);

    void drawPersuadeFrame(int x, int y, int frame);
    int lastPersuadeFrame();

protected:
    int stand_anims_[NUM_ANIMS];
    int walk_anims_[NUM_ANIMS];
    int stand_fire_anims_[NUM_ANIMS];
    int walk_fire_anims_[NUM_ANIMS];
    int die_agent_anim_;
    int dead_agent_anim_;
    int die_anim_;
    int dead_anim_;
    // has 4 directions
    int hit_anim_;
    int pickup_anim_;

    // has 4 directions
    int vaporize_anim_;
    int sink_anim_;
    int stand_burn_anim_;
    int walk_burn_anim_;
    int die_burn_anim_;
    int smoke_burn_anim_;
    int dead_burn_anim_;

    int persuade_anim_;
};

/*!
 * Pedestrian instance class.
 */
class PedInstance : public ShootableMovableMapObject, public WeaponHolder,
    public ModOwner
{
public:
    //! starting health for agents
    static const int kAgentMaxHealth;
    //! Default time for a ped between two shoots
    static const int kDefaultShootReactionTime;
    //! Id of the group for the player's agents
    static const uint32 kPlayerGroupId;
    /*!
     * Type of Ped.
     */
    enum PedType {
        kPedTypeCivilian = 0x01,
        kPedTypeAgent = 0x02,
        kPedTypePolice = 0x04,
        kPedTypeGuard = 0x08,
        kPedTypeCriminal = 0x10
    } ;

    PedInstance(Ped *ped, uint16 id, int m, bool isOur);
    ~PedInstance();

    //*************************************
    // Properties
    //*************************************
    //! Returns true if the agent is one of us.
    bool isOurAgent() { return is_our_; }
    //! Return the type of Ped
    PedType type() { return type_; }
    void setTypeFromValue(uint8 value);
    //! Returns the ped's behaviour
    Behaviour & behaviour() { return behaviour_; }
    //! Return true if ped has escaped the map
    bool hasEscaped() { return fs_cmn::isBitsOnWithMask(desc_state_, pd_smEscaped); }
    //! Indicate that the ped has escaped
    void escape() { fs_cmn::setBitsWithMask(&desc_state_, pd_smEscaped); }
    //! Return true if ped don't panic
    bool isPanicImmuned() { return panicImmuned_; }
    //! Tells the ped not to panic
    void setPanicImmuned() { panicImmuned_ = true; }

    typedef enum {
        ad_NoAnimation,
        ad_HitAnim,
        ad_DieAnim,
        ad_DeadAnim,
        ad_DeadAgentAnim,
        ad_PickupAnim,
        ad_PutdownAnim,
        ad_WalkAnim,
        ad_StandAnim,
        ad_WalkFireAnim,
        ad_StandFireAnim,
        ad_VaporizeAnim,
        ad_SinkAnim,
        ad_StandBurnAnim,
        ad_WalkBurnAnim,
        ad_DieBurnAnim,
        ad_SmokeBurnAnim,
        ad_DeadBurnAnim,
        ad_PersuadedAnim
    } AnimationDrawn;

    //! MapObject::state_
    enum pedActionStateMasks {
        pa_smNone = 0x0,
        pa_smStanding = 0x0001,
        pa_smWalking = 0x0002,
        pa_smHit = 0x0004,
        pa_smFiring = 0x0008,
        pa_smFollowing = 0x0010,
        pa_smPickUp = 0x0020,
        pa_smPutDown = 0x0040,
        pa_smBurning = 0x0080,
        pa_smInCar = 0x0400,
        pa_smHitByPersuadotron = 0x0800,
        pa_smDead = 0x1000,
        // this object should be ignored in all Ai procedures
        pa_smUnavailable = 0x2000,
        //! When a ped is hit by a laser
        pa_smHitByLaser = 0x4000,
        //! When a ped is walking and burning
        pa_smWalkingBurning = 0x8000,
        pa_smCheckExcluded = pa_smDead | pa_smUnavailable,
        pa_smAll = 0xFFFF
    };

    void draw(int x, int y);

    void setFrame(int f) { frame_ = f; }

    void setSightRange(int new_sight_range) { sight_range_ = new_sight_range; }
    int sightRange() { return sight_range_; }

    void showPath(int scrollX, int scrollY);

    bool switchActionStateTo(uint32 as);
    bool switchActionStateFrom(uint32 as);
    void synchDrawnAnimWithActionState(void);
    bool animate(int elapsed, Mission *mission);

    void drawSelectorAnim(int x, int y);
    //! Update frame to render
    bool updateAnimation(int elapsed);
    //! Set state for ped (replace switchActionStateTo)
    void goToState(uint32 as);
    //! Quit state for ped (replace switchActionStateFrom)
    void leaveState(uint32 as);

    //*************************************
    // Action management
    //*************************************
    //! Adds the given action to the list of actions
    void addMovementAction(MovementAction *pAction, bool appendAction);
    //! Adds the given action to the list of default scripted actions
    void addToDefaultActions(MovementAction *pToAdd);
    //! Adds the given action to the list of alternative scripted actions
    void addToAltActions(MovementAction *pToAdd);
    //! Returns the ped's current movement action
    MovementAction * currentAction() { return currentAction_; }
    //! Returns the ped's first default action (can be null)
    MovementAction * defaultAction() { return defaultAction_; }
    //! Returns true if ped's current action is from given source
    bool isCurrentActionFromSource(Action::ActionSource source) {
        return currentAction_ != NULL &&
                currentAction_->source() == source;
    }
    //! Returns the ped's first alternative action (can be null)
    MovementAction * altAction() { return altAction_; }
    //! Removes all ped's actions : current + scripted
    void destroyAllActions(bool includeScripted = true);
    //! Removes ped's action of using weapon
    void destroyUseWeaponAction();
    //! Execute the current action if any
    bool executeAction(int elapsed, Mission *pMission);
    //! Execute a weapon action if any
    bool executeUseWeaponAction(int elapsed, Mission *pMission);
    //! Restart the actions of given source and set as current action
    void resetActions(Action::ActionSource source);
    //! Switch to the given source of action
    void setCurrentActionWithSource(Action::ActionSource source);

    //! Adds action to walk to a given destination
    void addActionWalk(const TilePoint &destPosT, bool appendAction);

    //! Adds action to follow a ped
    void addActionFollowPed(PedInstance *pPed);
    //! Adds action to put down weapon on the ground
    void addActionPutdown(uint8 weaponIndex, bool appendAction);
    //! Adds action to pick up weapon from the ground
    MovementAction * createActionPickup(WeaponInstance *pWeapon);
    //! Creates actions to walk and enter a given vehicle
    MovementAction * createActionEnterVehicle(Vehicle *pVehicle);
    //! Adds action to drive vehicle to destination
    void addActionDriveVehicle(
           GenericCar *pVehicle, const TilePoint &destination, bool appendAction);
    //! Return true if ped can use a weapon
    bool canAddUseWeaponAction(WeaponInstance *pWeapon = NULL);
    //! Adds action to shoot somewhere
    uint8 addActionShootAt(const WorldPoint &aimedPt);
    //! Adds action to use medikit
    void addActionUseMedikit(WeaponInstance *pMedikit);
    //! Adds action to use Energy Shield
    void addActionUseEnergyShield(WeaponInstance *pMedikit);
    //! Creates and insert a HitAction for the ped
    void insertHitAction(fs_dmg::DamageToInflict &d);

    //*************************************
    // Movement management
    //*************************************
    //! See ShootableMovableMapObject::initMovementToDestination()
    bool initMovementToDestination(Mission *m, const TilePoint &destinationPt, int newSpeed = -1);

    //! See ShootableMovableMapObject::doMove()
    bool doMove(int elapsed, Mission *pMission);

    //*************************************
    // Weapon management
    //*************************************
    WeaponInstance * dropWeapon(uint8 index);
    void dropAllWeapons();
    bool wePickupWeapon();

    //*************************************
    // Shoot & Damage management
    //*************************************
    //! Return true if ped is currently using a weapon (ie there's an active action)
    bool isUsingWeapon() { return pUseWeaponAction_ != NULL; }
    //! Make the ped stop using weapon
    void stopUsingWeapon();
    //! Make the ped stop using a weapon that shoots
    void stopShooting();
    //! Update the ped's shooting target
    void updateShootingTarget(const WorldPoint &aimedPt);
    //! Adjust aimed point with user accuracy and weapon max range
    void adjustAimedPtWithRangeAndAccuracy(Weapon *pWeaponClass, WorldPoint *pAimedLocW);
    //! Gets the time before a ped can shoot again
    int getTimeBetweenShoots(WeaponInstance *pWeapon);

    //! Forces agent to kill himself
    void commitSuicide();

    //! Return true if ped has activated his energy shield
    bool isEnergyShieldActivated() { return fs_cmn::isBitsOnWithMask(desc_state_, pd_smShieldProtected); }
    void setEnergyActivated(bool status);

    //! Return the damage after applying protection of Mod
    int getRealDamage(fs_dmg::DamageToInflict &d);
    //! Method called when object is hit by a weapon shot.
    void handleHit(fs_dmg::DamageToInflict &d);
    //! Method called to check if ped has died
    bool handleDeath(Mission *pMission, fs_dmg::DamageToInflict &d);

    //*************************************
    // Persuasion
    //*************************************
    //! Return true if ped is persuaded
    bool isPersuaded() { return fs_cmn::isBitsOnWithMask(desc_state_, pd_smControlled); }
    //! Returns true if this ped can persuade that ped
    bool canPersuade(PedInstance *pOtherPed, const int persuadotronRange);
    //! Return owner of persuaded
    PedInstance * owner() { return owner_; }
    //! Adds given ped to the list of persuaded peds by this agent
    void addPersuaded(PedInstance *p);
    //! Removes given ped to the list of persuaded peds by this agent
    void rmvPersuaded(PedInstance *p);
    //! Method called when an agent persuads this ped
    void handlePersuadedBy(PedInstance *pAgent);
    //! Change the owner of the ped
    void setNewOwner(PedInstance *pPed);
    //!
    void informPersuadedToEnterVehicle(Vehicle *pVehicle);
    void dropPersuadedFromCar(Vehicle *pCar);

    bool inSightRange(MapObject *t);
    Vehicle * inVehicle() const;

    void putInVehicle(Vehicle *v);
    void leaveVehicle();

    AnimationDrawn drawnAnim(void);
    void setDrawnAnim(AnimationDrawn drawn_anim);
    bool handleDrawnAnim(int elapsed);

    uint8 moveToDir(Mission *m, int elapsed, DirMoveType &dir_move,
        int dir = -1, int t_posx = -1, int t_posy = -1, int *dist = NULL,
        bool set_dist = false);

    void setAllAdrenaLevels(uint8 amount, uint8 depend, uint8 effect) {
        adrenaline_->setLevels256(amount, depend, effect);
    }

    void setAllInteliLevels(uint8 amount, uint8 depend, uint8 effect) {
        intelligence_->setLevels256(amount, depend, effect);
    }

    void setAllPercepLevels(uint8 amount, uint8 depend, uint8 effect) {
        perception_->setLevels256(amount, depend, effect);
    }
    void updtIPATime(int elapsed) {
        adrenaline_->processTicks(elapsed);
        perception_->processTicks(elapsed);
        intelligence_->processTicks(elapsed);
    }

    void setDescStateMasks(unsigned int desc_state) {
        desc_state_ = desc_state;
    }
    unsigned int descStateMasks() { return desc_state_; }

    void setHostileDesc(unsigned int hostile_desc) {
        hostile_desc_ = hostile_desc;
    }
    unsigned int hostileDesc() { return hostile_desc_; }

    void setObjGroupDef(unsigned int obj_group_def) {
        obj_group_def_ = obj_group_def;
    }
    unsigned int objGroupDef() { return obj_group_def_; }

    void setObjGroupID(unsigned int obj_group_id) {
        obj_group_id_ = obj_group_id;
    }
    unsigned int objGroupID() { return obj_group_id_; }

    void setTimeBeforeCheck(int32 tm) { tm_before_check_ = tm; }
    void setBaseModAcc(double mod_acc) { base_mod_acc_ = mod_acc; }

    class Mmuu32_t: public std::multimap<uint32, uint32> {
    public:
        Mmuu32_t() {}
        ~Mmuu32_t() {}
        void rm(uint32 first, uint32 second = 0) {
            Mmuu32_t::iterator it = this->find(first);
            if (it == this->end())
                return;
            if (second == 0) {
                // removing all of this id
                Mmuu32_t::iterator its = it;
                do {
                    ++it;
                } while (it->first == first && it != this->end());
                this->erase(its, it);
            } else {
                do {
                    if (it->second == second) {
                        this->erase(it);
                        break;
                    }
                    ++it;
                } while (it->first == first && it != this->end());
            }
        }
        void add(uint32 first, uint32 second = 0) {
            Mmuu32_t::iterator it = this->find(first);
            if (it != this->end()) {
                if (second == 0) {
                    if (it->second != 0) {
                        // non-zeros should be removed, second value equal zero
                        // should be the only present
                        this->rm(first);
                        this->insert(std::pair<uint32, uint32>(first, second));
                    }
                } else {
                    bool found = false;
                    do {
                        if (it->first != first)
                            break;
                        if (it->second == second) {
                            found = true;
                            break;
                        }
                        ++it;
                    } while (it != this->end());
                    if (!found)
                        this->insert(std::pair<uint32, uint32>(first, second));
                }
            } else
                this->insert(std::pair<uint32, uint32>(first, second));
        }
        bool isIn(uint32 first, uint32 second = 0) {
            Mmuu32_t::iterator it = this->find(first);
            bool found = false;
            if (it != this->end()) {
                if (second == 0 || it->second == 0)
                    found = true;
                else {
                    do {
                        if (it->second == second) {
                            found = true;
                            break;
                        }
                        ++it;
                    } while (it != this->end() && it->first == first);
                }
            }
            return found;
        }
        bool isIn_KeyOnly(Mmuu32_t &mm) {
            if (mm.empty() || this->empty())
                return false;
            bool found = false;
            Mmuu32_t::iterator it_this = this->begin();
            uint32 first_value = it_this->first;
            do {
                Mmuu32_t::iterator it_mm = mm.find(first_value);
                found = it_mm != mm.end();
                if (found)
                    break;
                if (it_this->first != first_value)
                    first_value = it_this->first;
                else {
                    ++it_this;
                    continue;
                }
            } while (it_this != this->end());
            return found;
        }
        bool isIn_All(Mmuu32_t &mm) {
            if (mm.empty() || this->empty())
                return false;
            bool found = false;
            Mmuu32_t::iterator it_this = this->begin();
            uint32 first_value = it_this->first;
            uint32 second_value = it_this->second;
            do {
                Mmuu32_t::iterator it_mm = mm.find(first_value);
                found = it_mm != mm.end();
                if (found) {
                    if (second_value == 0 || it_mm->second == 0)
                        break;
                    std::pair<Mmuu32_t::iterator, Mmuu32_t::iterator> rng =
                        mm.equal_range(first_value);
                    found = false;
                    for (Mmuu32_t::iterator it = rng.first;
                        it != rng.second; ++it)
                    {
                        if (it->second == second_value) {
                            found = true;
                            break;
                        }
                    }
                }
                if (!found) {
                    if (it_this->first != first_value) {
                        first_value = it_this->first;
                        second_value = it_this->second;
                    } else {
                        if (it_this->second != second_value)
                            second_value = it_this->second;
                        ++it_this;
                        continue;
                    }
                }
            } while (it_this != this->end());
            return found;
        }
    };

    void addEnemyGroupDef(uint32 eg_id, uint32 eg_def = 0);
    void rmEnemyGroupDef(uint32 eg_id, uint32 eg_def = 0);
    bool isInEnemyGroupDef(uint32 eg_id, uint32 eg_def = 0);

    void addEmulatedGroupDef(uint32 eg_id, uint32 eg_def = 0);
    void rmEmulatedGroupDef(uint32 eg_id, uint32 eg_def = 0);
    bool isInEmulatedGroupDef(uint32 eg_id, uint32 eg_def = 0);
    bool isInEmulatedGroupDef(Mmuu32_t &r_egd,
        bool id_only = true);
    bool emulatedGroupDefsEmpty() { return emulated_group_defs_.size() == 0; }

    typedef std::pair<ShootableMapObject *, double> Pairsmod_t;
    typedef std::map <ShootableMapObject *, double> Msmod_t;
    bool isInHostilesFound(ShootableMapObject * hostile_found) {
        return hostiles_found_.find(hostile_found)
            != hostiles_found_.end();
    }
    void verifyHostilesFound(Mission *m);
    bool getHostilesFoundIt(Msmod_t::iterator &it_s, Msmod_t::iterator &it_e)
    {
        if (hostiles_found_.empty())
            return false;
        it_s = hostiles_found_.begin();
        it_e = hostiles_found_.end();
        return true;
    }
    //! Verify hostility between this Ped and the object
    bool isHostileTo(ShootableMapObject *obj,
        unsigned int hostile_desc_alt = 0);
    //! Verify if this ped is friend with the given ped
    bool isFriendWith(PedInstance *p);

    typedef enum {
        og_dmUndefined = 0x0,
        og_dmPedestrian = 0x01,
        og_dmCivilian = 0x01,
        og_dmAgent = 0x02,
        og_dmPolice = 0x04,
        og_dmGuard = 0x08,
        og_dmCriminal = 0x10
    } objGroupDefMasks;

    //! Returns ped's speed under normal conditions
    int getDefaultSpeed();
    int getSpeedOwnerBoost();

    void getAccuracy(double &base_acc);
    bool hasAccessCard();

    void cpyEnemyDefs(Mmuu32_t &eg_defs) { eg_defs = enemy_group_defs_; }
    bool isArmed() { return selectedWeapon() != NULL; }

    IPAStim *adrenaline_;
    IPAStim *perception_;
    IPAStim *intelligence_;
protected:
    /*!
     * Called before a weapon is selected to check if weapon can be selected.
     * \param wi The weapon to select
     */
    bool canSelectWeapon(WeaponInstance *pNewWeapon);
    /*!
     * Called when a weapon has been deselected.
     * \param wi The deselected weapon
     */
    void handleWeaponDeselected(WeaponInstance * wi);
    //! See WeaponHolder::handleWeaponSelected()
    void handleWeaponSelected(WeaponInstance * wi, WeaponInstance * previousWeapon);

    //! Called when a weapon has no ammo to select another one
    void handleSelectedWeaponHasNoAmmo();

    //! Returns the number of points an agent must have to persuade a ped of given type
    uint16 getRequiredPointsToPersuade(PedType type);
    //! When a ped dies, changes the persuaded owner/persuaded_group relation.
    void updatePersuadedRelations(Squad *pSquad);

private:
    inline int getClosestDirs(int dir, int& closest, int& closer);
    bool floodMap(Mission *m, const TilePoint &clippedDestPt, floodPointDesc *mdpmirror);
    void removeTilesWithNoChildsFromBase(Mission *m, unsigned short blvl, std::vector <toSetDesc> &bv, std::vector <lvlNodesDesc> &bn, floodPointDesc *mdpmirror);
    void removeTilesWithNoChildsFromTarget(Mission *m, unsigned short tlvl, std::vector <toSetDesc> &tv, std::vector <lvlNodesDesc> &tn, floodPointDesc *mdpmirror);
    void createPath(Mission *m, floodPointDesc *mdpmirror, std::vector<TilePoint> &cdestpath);
    void buildFinalDestinationPath(Mission *m, std::vector<TilePoint> &cdestpath, const TilePoint &destinationPt);

protected:
    enum pedDescStateMasks {
        pd_smUndefined = 0x0,
        //! Set when a ped has been persuaded
        pd_smControlled = 0x0001,
        //! Energy shield protects the ped
        pd_smShieldProtected = 0x0010,
        /*! When a mission's objective is to kill a ped and this ped has
        escaped, this value is used to indicate he's escaped.*/
        pd_smEscaped = 0x0080,
        pd_smAll = 0xFFFF
    };

    Ped *ped_;
    //! Type of Ped
    PedType type_;

    /*! Ped's behaviour.*/
    Behaviour behaviour_;
    /*! Current action*/
    MovementAction *currentAction_;
    /*!
     * Default and Alternative actions define the behaviour of non player controlled peds.
     * Default actions come from a mission file and alternative actions are used by ped
     * to react in certain situations (like fight).
     * Those actions are not deleted when finished.
     */
    MovementAction *defaultAction_;
    MovementAction *altAction_;
    /*! Current action of using a weapon.*/
    UseWeaponAction *pUseWeaponAction_;

    // (pedDescStateMasks)
    uint32 desc_state_;
    // this inherits definition from desc_state_
    // ((target checked)desc_state_ & hostile_desc_) != 0 kill him
    uint32 hostile_desc_;
    Mmuu32_t enemy_group_defs_;
    // if object is not hostile here, enemy_group_defs_ check
    // is skipped, but not hostiles_found_ or desc_state_
    Mmuu32_t emulated_group_defs_;
    // not set anywhere but used
    Mmuu32_t friend_group_defs_;
    //! dicovered hostiles are set here, only within sight range
    Msmod_t hostiles_found_;
    //! used by police officers, for now friends forever mode
    std::set <ShootableMapObject *> friends_found_;
    //! from this list they will move to friends_found_ if in sight
    std::set <ShootableMapObject *> friends_not_seen_;
    //! defines group obj belongs to (objGroupDefMasks), not unique
    uint32 obj_group_def_;
    uint32 old_obj_group_def_;

    //! a unique group identification number, 0 - all group IDs
    uint32 obj_group_id_;
    uint32 old_obj_group_id_;

    //! time wait before checking environment (enemies, friends etc)
    int32 tm_before_check_;

    //! base value that influences accuracy during fire
    double base_mod_acc_;

    AnimationDrawn drawn_anim_;

    int sight_range_;
    Vehicle *in_vehicle_;
    //! This flag tells if this is our agent, assuming it's an agent.
    bool is_our_;
    //! controller of ped - for persuaded
    PedInstance *owner_;
    //! Points obtained by agents for persuading peds
    uint16 totalPersuasionPoints_;
    //! The group of peds that this ped has persuaded
    std::set <PedInstance *> persuadedSet_;
    //! Tells whether the panic can react to panic or not
    bool panicImmuned_;
    //! This field is used to select a weapon after medikit was used
    WeaponInstance *pSelectedWeaponBeforeMedikit_;
};

#endif

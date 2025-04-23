/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net>
 *   Copyright (C) 2013, 2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef PED_H
#define PED_H

#include <map>
#include <set>

#include "fs-utils/common.h"
#include "fs-engine/enginecommon.h"
#include "fs-engine/gfx/spritemanager.h"
#include "fs-kernel/model/static.h"
#include "fs-kernel/model/modowner.h"
#include "fs-kernel/model/weapon.h"
#include "fs-kernel/model/weaponholder.h"
#include "fs-kernel/model/ipastim.h"
#include "fs-kernel/ia/actions.h"
#include "fs-kernel/ia/behaviour.h"

namespace fs_knl {

class Agent;
class Mission;
class Squad;
class GenericCar;
class Vehicle;

#define NUM_ANIMS 10

/*!
 * Structure to hold all animations for a PedInstance
 */
struct PedAnimations {
    //! Animation when standing with or without a weapon
    uint16_t standAnimations[NUM_ANIMS];
    //! Animation when walking with or without a weapon
    uint16_t walkAnimations[NUM_ANIMS];
    //! Animation when standing and shooting with a weapon
    uint16_t standShootAnimations[NUM_ANIMS];
    //! Animation when walking and shooting with a weapon
    uint16_t walkShootAnimations[NUM_ANIMS];
    //! Animation when ped pick up or drop object
    uint16_t pickAnimation;
    //! Animation when ped is hit by weapon
    uint16_t hitAnimation;
    uint16_t dyingAnimation;
    //! Single frame animation for dead ped
    uint16_t deadAnimation;
    //! Hit by laser
    uint16_t vaporizeAnimation;
    uint16_t deadAgentAnimation;
    //! Hit by flame
    uint16_t walkBurnAnimation;
    //! when dying from flame
    uint16_t dyingBurnAnimation;
    //! end of burning animation
    uint16_t smokeBurnAnimation;
    //! dead by burning
    uint16_t deadBurnAnimation;
    //! Persuaded animation
    uint16_t persuadedAnimation;
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
    static const uint32_t kPlayerGroupId;
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

    PedInstance(uint16_t id, Map *pMap, bool isOur);
    ~PedInstance();

    /*!
     * @name Properties
     */
    ///@{

    //! Returns true if the agent is one of us.
    bool isOurAgent() { return is_our_; }
    //! Return the type of Ped
    PedType type() { return type_; }

    ///@}

    void setTypeFromValue(uint8_t value);
    //! Returns the ped's behaviour
    Behaviour & behaviour() { return behaviour_; }
    //! Return true if ped has escaped the map
    bool hasEscaped() { return fs_utl::isBitsOnWithMask(desc_state_, pd_smEscaped); }
    //! Indicate that the ped has escaped
    void escape() { fs_utl::setBitsWithMask(&desc_state_, pd_smEscaped); }
    //! Return true if a ped can go in panic mode
    bool isImmunedToPanic();
    //! Tells the ped not to panic
    void setPanicImmuned() { panicImmuned_ = true; }

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
        pa_smDying = 0x0100,
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

    /**
     * @name Animation
     */
    ///@{
    /*!
     * @brief Init PedInstance animations using the base animation.
     */
    void setAnimations(uint16_t baseSpriteAnimationId);

    void draw(const Point2D &screenPos) override;

    void playStandOrWalkAnimation();
    void playStandAndShootAnimation();
    void playWalkAndShootAnimation();
    void playPickupOrDropAnimation();
    void playHitAnimation();
    void playDyingAnimation();
    void playDeadAnimation();
    void playVaporizeAnimation();
    void playDeadAgentAnimation();
    void playWalkBurnAnimation();
    void playDyingBurnAnimation();
    void playSmokeBurnAnimation();
    void playDeadBurnAnimation();
    void playPersuadedAnimation();
    ///@}

    void setSightRange(int new_sight_range) { sight_range_ = new_sight_range; }
    int sightRange() { return sight_range_; }

    void showPath(int scrollX, int scrollY, fs_eng::FSColor color);

    /**
     * @name State management
     */
    ///@{
    bool switchActionStateTo(uint32_t as);
    bool switchActionStateFrom(uint32_t as);
    void synchDrawnAnimWithActionState(void);

    void drawSelectorAnim(const Point2D &screenPos);
    //! Set state for ped (replace switchActionStateTo)
    void goToState(uint32_t as);
    //! Quit state for ped (replace switchActionStateFrom)
    void leaveState(uint32_t as);
    //! Return is current state is equal to given state
    bool isState(uint32_t as);
    ///@}

    /**
     * @name Action management
     */
    ///@{
    //! Return true if the given type of action can be created in current state
    bool canTakeAction(Action::ActionType type);
    //! Adds the given action to the list of actions
    void addMovementAction(MovementAction *pAction, bool appendAction);
    //! Adds the given action to the list of default scripted actions
    void addToDefaultActions(MovementAction *pToAdd);
    //! Adds the given action to the list of alternative scripted actions
    void addToAltActions(MovementAction *pToAdd);
    //! Returns the ped's current movement action
    MovementAction * currentAction() { return currentAction_; }
    //! Return true if current movement action is non null and has given type
    bool isCurrentActionOfType(Action::ActionType type) {
        return currentAction_ != NULL &&
                currentAction_->type() == type;
    }
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
    bool executeAction(uint32_t elapsed, Mission *pMission);
    //! Execute a weapon action if any
    bool executeUseWeaponAction(uint32_t elapsed, Mission *pMission);
    //! Restart the actions of given source and set as current action
    void resetActions(Action::ActionSource source);
    //! Switch to the given source of action
    void setCurrentActionWithSource(Action::ActionSource source);

    //! Adds action to walk to a given destination
    void addActionWalk(const TilePoint &destPosT, bool appendAction);

    //! Adds action to follow a ped
    void addActionFollowPed(PedInstance *pPed);
    //! Adds action to put down weapon on the ground
    void addActionPutdown(uint8_t weaponIndex, bool appendAction);
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
    uint8_t addActionShootAt(const WorldPoint &aimedPt);
    //! Adds action to use medikit
    void addActionUseMedikit(WeaponInstance *pMedikit);
    //! Adds action to use Energy Shield
    void addActionUseEnergyShield(WeaponInstance *pMedikit);
    //! Creates and insert a HitAction for the ped
    void insertHitAction(DamageToInflict &d);

    ///@}

    //*************************************
    // Movement management
    //*************************************
    //! See ShootableMovableMapObject::initMovementToDestination()
    bool initMovementToDestination(Mission *m, const TilePoint &destinationPt, int newSpeed = -1);

    //! See ShootableMovableMapObject::doMove()
    bool doMove(uint32_t elapsed, Mission *pMission) override;

    //*************************************
    // Weapon management
    //*************************************
    WeaponInstance * dropWeapon(uint8_t index);
    void dropAllWeapons();
    bool wePickupWeapon();

    /**
     * @name Shooting management
     */
    ///@{
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

    //! Return true if ped has activated his energy shield
    bool isEnergyShieldActivated() { return fs_utl::isBitsOnWithMask(desc_state_, pd_smShieldProtected); }
    void setEnergyActivated(bool status);
    ///@}

    /**
     * @name Damage management
     */
    ///@{
    //! Forces agent to kill himself
    void commitSuicide();
    //! Take the damage inflicted
    bool takeDamage(DamageToInflict &d);
    //! Method called when object is hit by a weapon shot.
    void handleHit(DamageToInflict &d) override;
    //! Method when a Ped is dead
    void handleDeath(const DamageToInflict &damage);
    ///@}

    //*************************************
    // Persuasion
    //*************************************
    //! Return true if ped is persuaded
    bool isPersuaded() { return fs_utl::isBitsOnWithMask(desc_state_, pd_smControlled); }
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
    //! Return true if this ped is inside a vehicle
    bool isInVehicle() { return in_vehicle_ != nullptr; }

    void putInVehicle(Vehicle *v);
    void leaveVehicle();

    uint8_t moveToDir(Mission *m, uint32_t elapsed, DirMoveType &dir_move,
        int dir = -1, int t_posx = -1, int t_posy = -1, int *dist = NULL,
        bool set_dist = false);

    void setAllAdrenaLevels(uint8_t amount, uint8_t depend, uint8_t effect) {
        adrenaline_->setLevels256(amount, depend, effect);
    }

    void setAllInteliLevels(uint8_t amount, uint8_t depend, uint8_t effect) {
        intelligence_->setLevels256(amount, depend, effect);
    }

    void setAllPercepLevels(uint8_t amount, uint8_t depend, uint8_t effect) {
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

    void setTimeBeforeCheck(int32_t tm) { tm_before_check_ = tm; }
    void setBaseModAcc(double mod_acc) { base_mod_acc_ = mod_acc; }

    class Mmuu32_t: public std::multimap<uint32_t, uint32_t> {
    public:
        Mmuu32_t() {}
        ~Mmuu32_t() {}
        void rm(uint32_t first, uint32_t second = 0) {
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
        void add(uint32_t first, uint32_t second = 0) {
            Mmuu32_t::iterator it = this->find(first);
            if (it != this->end()) {
                if (second == 0) {
                    if (it->second != 0) {
                        // non-zeros should be removed, second value equal zero
                        // should be the only present
                        this->rm(first);
                        this->insert(std::pair<uint32_t, uint32_t>(first, second));
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
                        this->insert(std::pair<uint32_t, uint32_t>(first, second));
                }
            } else
                this->insert(std::pair<uint32_t, uint32_t>(first, second));
        }
        bool isIn(uint32_t first, uint32_t second = 0) {
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
            uint32_t first_value = it_this->first;
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
            uint32_t first_value = it_this->first;
            uint32_t second_value = it_this->second;
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

    void addEnemyGroupDef(uint32_t eg_id, uint32_t eg_def = 0);
    void rmEnemyGroupDef(uint32_t eg_id, uint32_t eg_def = 0);
    bool isInEnemyGroupDef(uint32_t eg_id, uint32_t eg_def = 0);

    void addEmulatedGroupDef(uint32_t eg_id, uint32_t eg_def = 0);
    void rmEmulatedGroupDef(uint32_t eg_id, uint32_t eg_def = 0);
    bool isInEmulatedGroupDef(uint32_t eg_id, uint32_t eg_def = 0);
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

    enum objGroupDefMasks {
        og_dmUndefined = 0x0,
        og_dmPedestrian = 0x01,
        og_dmCivilian = 0x01,
        og_dmAgent = 0x02,
        og_dmPolice = 0x04,
        og_dmGuard = 0x08,
        og_dmCriminal = 0x10
    };

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
    //! Called before a weapon is selected to check if weapon can be selected.
    bool canSelectWeapon(WeaponInstance *pNewWeapon) override;
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
    uint16_t getRequiredPointsToPersuade(PedType type);
    //! When a ped dies, changes the persuaded owner/persuaded_group relation.
    void updatePersuadedRelations(Squad *pSquad);

    //! Subclasses reimplement this to update their internal state
    void doUpdateState(uint32_t elapsed) override;

    void handleAnimationEnded() override;

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
    //! The list of animations for the ped
    PedAnimations animations_;
    //! State of the ped
    uint32_t state_;
    // (pedDescStateMasks)
    uint32_t desc_state_;
    // this inherits definition from desc_state_
    // ((target checked)desc_state_ & hostile_desc_) != 0 kill him
    uint32_t hostile_desc_;
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
    uint32_t obj_group_def_;
    uint32_t old_obj_group_def_;

    //! a unique group identification number, 0 - all group IDs
    uint32_t obj_group_id_;
    uint32_t old_obj_group_id_;

    //! time wait before checking environment (enemies, friends etc)
    int32_t tm_before_check_;

    //! base value that influences accuracy during fire
    double base_mod_acc_;

    int sight_range_;
    Vehicle *in_vehicle_;
    //! This flag tells if this is our agent, assuming it's an agent.
    bool is_our_;
    //! controller of ped - for persuaded
    PedInstance *owner_;
    //! Points obtained by agents for persuading peds
    uint16_t totalPersuasionPoints_;
    //! The group of peds that this ped has persuaded
    std::set <PedInstance *> persuadedSet_;
    //! Tells whether the panic can react to panic or not
    bool panicImmuned_;
    //! This field is used to select a weapon after medikit was used
    WeaponInstance *pSelectedWeaponBeforeMedikit_;
};

/** \brief Event sent when our agent has died
 */
struct AgentDiedEvent {
    PedInstance *pPed;
};

/** \brief Event sent when an agent has selected or deselected a shooting weapon.
 */
struct ShootingWeaponSelectedEvent {
    //! The agent who has selected/deselected the weapon
    PedInstance *pPed;
    //! True means it was a selection, false a deselection
    bool isSelected;
};

/** \brief Event sent when a policeman warns an armed agent.
 */
struct PoliceWarningEmittedEvent {
    // for compilation
    int id;
};

}
#endif

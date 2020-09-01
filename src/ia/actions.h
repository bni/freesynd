/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2013  Bohdan Stelmakh <chamel@users.sourceforge.net> *
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

#ifndef IA_ACTIONS_H_
#define IA_ACTIONS_H_

#include "path.h"
#include "mapobject.h"
#include "utils/timer.h"

class Mission;
class PedInstance;
class WeaponInstance;
class Vehicle;
class GenericCar;
class TrainHead;


/*!
 * The Action class is an abstract class representing an action that a ped can do
 * like walking, driving a car, pick up someting or shoot.
 * Every action has following attributes :
 *   - status (from not started to finished),
 *   - origin : can be created from game file or after a player action. When
 *      an action is finished it is removed from the list except for scripted actions
 *      which are replayed.
 * Subclasses must implement the execute() method.
 */
class Action {
public:
    /*!
     * The source of an action is whether the action is scripted
     * or not.
     */
    enum ActionSource {
        kActionNotScripted,
        kActionDefault,
        kActionAlt
    };
    /*!
     * What type of action.
     * Only usefull types are defined.
     * Other actions have type kActTypeUndefined.
     */
    enum ActionType {
        //! Action of following a ped
        kActTypeFollow,
        //! For all type of hit action
        kActTypeHit,
        //! Action of resetting scripted action
        kActTypeReset,
        //! Special action for switching source of actions
        kActTypeSwitch,
        //! Action of shooting
        kActTypeShoot,
        //! The walk action
        kActTypeWalk,
        //! Wait to shoot action
        kActTypeWaitShoot,
        kActTypeWait,
        //! Action of following a ped to shoot at it
        kActTypeFollowToShoot,
        kActTypeFire,
        kActTypeReplaceCurrent,
        //! Action to pick weapon up
        kActTypePickUp,
        //! Drop weapon
        kActTypeDrop,
        kActTypeUndefined
    };

    //! Possible status of an action.
    enum ActionStatus {
        //! Initial status for action
        kActStatusNotStarted,
        //! When action is running
        kActStatusRunning,
        //! When action needs to wait for the end of animation to complete
        kActStatusWaitForAnim,
        //! When action needs to wait for some time to complete
        kActStatusWaitForTime,
        //! When action ends with success
        kActStatusSucceeded,
        //! When action ends with failure
        kActStatusFailed,
        //! When action is suspended during its execution
        kActStatusSuspended
    };

    //! Constructor for the class
    Action(ActionType type);
    //! Destructor of the class
    virtual ~Action() { }

    //! Entry point to execute the action
    virtual bool execute(int elapsed, Mission *pMission, PedInstance *pPed) = 0;

    //! Returns the source of the action
    ActionSource source() { return source_; }
    //! Sets the source of the action
    void setSource(ActionSource aSource) { source_ = aSource; }
    //! Returns the type of action
    ActionType type() { return type_; }
    //! Returns true if action is running
    bool isRunning() { return status_ == kActStatusRunning; }
    //! Returns true if action is waiting for animation to end
    bool isWaitingForAnimation() { return status_ == kActStatusWaitForAnim; }
    //! Returns true if action has succeeded or failed
    bool isFinished() { return status_ == kActStatusSucceeded || status_ == kActStatusFailed; }

    //! Sets the status to Running
    void setRunning() { status_ = kActStatusRunning; }
    //! Sets the status to WaitForTime
    void setWaitingForTime() { status_ = kActStatusWaitForTime; }
    //! Sets the status to Succeeded
    void setSucceeded() { status_ = kActStatusSucceeded; }
    //! Sets the status to Failed
    void setFailed() { status_ = kActStatusFailed; }

    //! Reset the action
    virtual void reset();

protected:
    /*! The type of action.*/
    ActionType type_;
    ActionSource source_;
    /*! This is the status of the action.*/
    ActionStatus status_;
};

/*!
 * MovementAction (not happy with the name) are every action a Ped can make except shooting.
 * A Ped can chain multiple MovementActions.
 * Normaly, a ped can shoot while doing movement actions except if current action is exclusive.
 * For example, ped cannot shoot while dropping his weapon on the ground.
 * Some actions are not possible while ped is in car (like walking) and other are (like driving).
 * The targetState field stores the state that the ped will have when action is realized.
 *
 * MovementAction exposes 2 methods for subclasses to implement :
 * - doStart() : method called one time when action is first started
 * - doExecute() : method called every time to do the action implementation
 */
class MovementAction : public Action {
public:
    //! Constructor for the class
    MovementAction(ActionType type, bool isExclusive = false, bool canExecVehicle = false);
    //! Destructor of the class
    virtual ~MovementAction() { pNext_ = NULL; }

    //! Entry point to execute the action
    bool execute(int elapsed, Mission *pMission, PedInstance *pPed);
    //! Return true if action has been suspended
    virtual bool suspend(PedInstance *pPed);
    //! Resume action
    virtual void resume(Mission *pMission, PedInstance *pPed);
    //! Returns true if action is currently suspended
    bool isSuspended() { return status_ == kActStatusSuspended; }

    //! Returns true if action is exclusive
    bool isExclusive() { return isExclusive_; }
    //! Returns true if the action can be executed while ped is in a vehicle
    bool canExecInVehicle() { return canExecInVehicle_; }

    //! Returns true if Behaviour must be warned when action is finished
    bool warnBehaviour() { return warnBehaviour_; }
    //! Set if Behaviour must be warned when action is finished
    void setWarnBehaviour(bool warn) { warnBehaviour_ = warn; }

    //! Return the next action in the list
    MovementAction *next() { return pNext_; }
    //! Return the previous action in the list
    MovementAction *previous() { return pPrevious_; }
    //! Set the previous action
    void setPrevious(MovementAction *pAction) { pPrevious_ = pAction; }
    //! Set given action as the next of this action
    void link(MovementAction *pNext) {
        pNext_ = pNext;
        if (pNext != NULL) {
            pNext->setPrevious(this);
        }
    }

    void unlinkNext();

    void insertNext(MovementAction *pAction);
    void insertPrevious(MovementAction *pAction);

    void removeAndJoinChain();
protected:
    //! Subclasses must implement this method to do somthing at the begining of the action
    virtual void doStart(Mission *pMission, PedInstance *pPed) {}
    //! Sublasses must implement this method to realize this action
    virtual bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed) = 0;
protected:
    /*! When this flag is set, the ped cannot shoot.*/
    bool isExclusive_;
    /*! This flag tells that the action can be executed when ped is in a vehicule.*/
    bool canExecInVehicle_;
    /*! Store the state the ped will have when executing the action.*/
    uint32 targetState_;
    /*! Previous action in the chain.*/
    MovementAction *pPrevious_;
    /*! Next action in the chain.*/
    MovementAction *pNext_;
    /*! This is the status of the action before it was suspended.*/
    ActionStatus savedStatus_;
    /*! This flag is used to alert the behaviour on the action lifecycle.*/
    bool warnBehaviour_;
};

/*!
 * This action is used to move a ped to a given point by walking.
 */
class WalkAction : public MovementAction {
public:
    //! Walt to given point
    WalkAction(const TilePoint &locT, int speed = -1);
    //! Walk to given object
    WalkAction(ShootableMapObject *smo, int speed = -1);

    //! Set the destination as the location of the given object
    void setDestination(ShootableMapObject *smo);
    //! Suspend action
    bool suspend(PedInstance *pPed);
protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
protected:
    /*! Where to walk to.*/
    TilePoint destLocT_;
    /*! Speed used to walk to destination.*/
    int newSpeed_;
};

/*!
 * This action is used to move a ped towards a direction.
 * Direction is either the one he's already pointing to or using a target location.
 * When direction is given by ped's direction, it is possible to specify a distance
 * so that movement stops when ped travels that distance.
 * When direction is given by a location, movement stops when
 * ped reaches that location.
 */
class WalkToDirectionAction : public MovementAction {
public:
    //! Walk to direction given by point
    WalkToDirectionAction(const WorldPoint &destLocW);
    //! Walk following ped's direction
    WalkToDirectionAction(int speed = -1);

    void setMaxDistanceToWalk(int distance) { maxDistanceToWalk_ = distance; }
    //! Suspend action
    bool suspend(PedInstance *pPed);
protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
protected:
    /*! Where to walk to.*/
    WorldPoint destLocW_;
    /*! Structure to hold information while walking.*/
    DirMoveType moveDirdesc_;
    /*! Count the distance the ped has walked since starting the action.*/
    int distWalked_;
    int maxDistanceToWalk_;
    /*! Speed used to walk to destination.*/
    int newSpeed_;
};

/*!
 * A trigger is an action that ends only when one of our agent
 * enter a zone defined by a center and a radius.
 */
class TriggerAction : public MovementAction {
public:
    TriggerAction(int32 range, const WorldPoint &loc);
protected:
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
protected:
    /*! Center of trigger zone.*/
    WorldPoint centerLoc_;
    /*! The range of trigger zone.*/
    int32 range_;
};

/*!
 * This action is used in scripted action when a ped
 * needs to escape the map and our agents must kill him.
 */
class EscapeAction : public MovementAction {
public:
    EscapeAction():
        MovementAction(kActTypeUndefined, false, true) {}
protected:
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
};

/*!
 * This action does nothing but its presence makes all scripted to start again
 * and potentially change source of actions.
 */
class ResetScriptedAction : public MovementAction {
public:
    ResetScriptedAction(ActionSource aSource):
        MovementAction(kActTypeReset, false, true) {
            sourceToReset_ = aSource;
        }

    ActionSource sourceToReset() { return sourceToReset_; }
protected:
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed) { return true; }
private:
    ActionSource sourceToReset_;
};

/*!
 * This action is special: it used when we want to terminate the current chain of action
 * in order to do something different but the current action is not suspendable.
 * So we insert this action just after the current action and when it will be executed,
 * the current chain will be dropped and the target action will take place.
 */
class ReplaceCurrentAction : public MovementAction {
public:
    ReplaceCurrentAction(MovementAction *newAction):
        MovementAction(kActTypeReplaceCurrent, false, true) {
            pTargetAction_ = newAction;
        }

    MovementAction *targetAction() { return pTargetAction_; }
protected:
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed) { return true; }
private:
    MovementAction *pTargetAction_;
};

/*!
 * This action is used to make a ped follow another ped.
 * Both must be walking.
 * This action is only available to player.
 */
class FollowAction : public MovementAction {
public:
    //! Walt to given point
    FollowAction(PedInstance *pTarget);
    void setTarget(PedInstance *pTarget) { pTarget_ = pTarget; }

protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);

    //! Update the target's last position
    void updateLastTargetPos();
protected:
    /*! This constant defines the closest distance a ped can
     * approach a target when following.*/
    static const int kFollowDistance;

    /*! The ped to follow.*/
    PedInstance *pTarget_;
    /*! To keep track of target position and see if it has moved.*/
    TilePoint targetLastPos_;
};

/*!
 * This action is used for a policeman to follow a target and when
 * the target is in shooting range, the action is finished.
 * The shooting range is 2/3 of the currently selected weapon range.
 */
class FollowToShootAction : public MovementAction {
public:
    //! Constructor
    FollowToShootAction(PedInstance *pTarget);
    void setTarget(PedInstance *pTarget) { pTarget_ = pTarget; }
protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);

protected:
    /*! The ped to follow.*/
    PedInstance *pTarget_;
    /*! To keep track of target position and see if it has moved.*/
    WorldPoint targetLastPosW_;
    /*! The distance.*/
    int followDistance_;
};

/*!
 * This action allows an agent to drop a weapon (identified by its
 * position in the agent's inventory) on the ground.
 * It is only available for our agents (so it's a player action).
 */
class PutdownWeaponAction : public MovementAction {
public:
    PutdownWeaponAction(uint8 weaponIdx);

protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
protected:
    /*! Index of the weapon to drop in the ped's inventory.*/
    uint8 weaponIdx_;
};

/*!
 * This action allows an agent to pick a weapon up from the ground
 * and to add it in his inventory.
 * It is only available for our agents (so it's a player action).
 */
class PickupWeaponAction : public MovementAction {
public:
    PickupWeaponAction(WeaponInstance *pWeapon);

    void setWeapon(WeaponInstance *pWeapon) { pWeapon_ = pWeapon; }
protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
protected:
    /*! The weapon to pick up.*/
    WeaponInstance *pWeapon_;
};

/*!
 * This action is used to get a ped in a vehicle.
 */
class EnterVehicleAction : public MovementAction {
public:
    EnterVehicleAction(Vehicle *pVehicle);

protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
protected:
    /*! The vehicle to enter.*/
    Vehicle *pVehicle_;
};

/*!
 * This action is used to drive a vehicle to a point.
 */
class DriveVehicleAction : public MovementAction {
public:
    DriveVehicleAction(GenericCar *pVehicle, const TilePoint &dest);

protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
protected:
    /*! Vehicle to drive.*/
    GenericCar *pVehicle_;
    /*! Destination point.*/
    TilePoint dest_;
};

/*!
 * This action is used to drive a train to a point.
 */
class DriveTrainAction : public MovementAction {
public:
    DriveTrainAction(TrainHead *pTrain, const TilePoint &dest);

protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);

    void dropAgents(const Mission &mission);
protected:
    /*! Train to drive.*/
    TrainHead *pTrain_;
    /*! Destination point.*/
    TilePoint dest_;
};

/*!
 * This action is used to make a ped wait.
 */
class WaitAction : public MovementAction {
public:

    enum WaitEnum {
        kWaitTime,
        kWaitWeapon
    };

    //! Wait for time
    WaitAction(WaitEnum waitFor, uint32 duration);
    //! Wait for weapon action
    WaitAction(WaitEnum waitFor);

protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);

protected:
    WaitEnum waitType_;
    /*! Duration of waiting.*/
    fs_utils::Timer waitTimer_;
};

/*!
 * This action is used by peds that shoot at other peds.
 * It is used to insert a delay between 2 shoots.
 * When the ped is a policeman and the target is the player agent, the policeman
 * yields a warning.
 */
class WaitBeforeShootingAction : public MovementAction {
public:
    WaitBeforeShootingAction(PedInstance *pPed);

    void setTarget(PedInstance *pPed) { pTarget_ = pPed; }
protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);

protected:
    /*! The ped watched by this ped.*/
    PedInstance *pTarget_;
    /*! Duration of waiting.*/
    fs_utils::Timer waitTimer_;
};

/*!
 * This action is used to make peds other than the player's agent shoot.
 */
class FireWeaponAction : public MovementAction {
public:
    FireWeaponAction(PedInstance *pPed);

    void setTarget(PedInstance *pPed) { pTarget_ = pPed; }
    //! Action cannot be suspended
    bool suspend(PedInstance *pPed) { return false; }
protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
protected:
    /*! The ped that is being shot at by the action owner.*/
    PedInstance *pTarget_;
    /*! tells if it is a single or automatic shot.*/
    uint8 shootType_;
};

/*!
 * A HitAction is used to implement the reaction of the ped to an impact.
 * When a Ped is hit by a shot, a HitAction is created and inserted directly
 * as the ped's current action but only if current action is not already
 * a HitAction.
 */
class HitAction : public MovementAction {
public:
    //! Constructor of the class
    HitAction(fs_dmg::DamageToInflict &d);

    //! HitAction cannot be suspended
    bool suspend(PedInstance *pPed) { return false; }
protected:
    //! Stores the damage received
    fs_dmg::DamageToInflict damage_;
};

/*!
 * This action is used to represent a hit when ped is suiciding by bullet.
 */
class FallDeadHitAction : public HitAction {
public:
    //!
    FallDeadHitAction(fs_dmg::DamageToInflict &d);
protected:
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
};

/*!
 * This action is used to represent a hit by a bullet
 * or explosion.
 */
class RecoilHitAction : public HitAction {
public:
    //!
    RecoilHitAction(fs_dmg::DamageToInflict &d);
protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
};

/*!
 * This action is used to represent a hit by a laser.
 */
class LaserHitAction : public HitAction {
public:
    //!
    LaserHitAction(fs_dmg::DamageToInflict &d);
protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
};

/*!
 * This class is used when a ped is burned. He walks while burning.
 * The ped walks towards a random direction.
 */
class WalkBurnHitAction : public HitAction {
public:
    //!
    WalkBurnHitAction(fs_dmg::DamageToInflict &d);
protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);

protected:
    /*!
     * A constant to define the time a burning ped
     * can walk before dying.
     */
    static const int kTimeToWalkBurning;

    /*! Structure to hold information while walking.*/
    DirMoveType moveDirdesc_;
    /*! Used to store the distance the ped has walked.*/
    int walkedDist_;
    /*! The direction he will walked towards.*/
    int moveDirection_;
    /*! Duration of burning.*/
    fs_utils::Timer burnTimer_;
};

/*!
 * This action is used to represent a hit by a laser.
 */
class PersuadedHitAction : public HitAction {
public:
    //!
    PersuadedHitAction(fs_dmg::DamageToInflict &d);
protected:
    void doStart(Mission *pMission, PedInstance *pPed);
    bool doExecute(int elapsed, Mission *pMission, PedInstance *pPed);
};

/*!
 * This action is for using a weapon. A ped can only have a single weapon action
 * at a given time.
 */
class UseWeaponAction : public Action {
public:
    UseWeaponAction(WeaponInstance *pWeapon) : Action(kActTypeUndefined) {
        pWeapon_ = pWeapon;
    };
    UseWeaponAction(ActionType aType, WeaponInstance *pWeapon) : Action(aType) {
        pWeapon_ = pWeapon;
    };

    //! Stop shooting (mainly used with AutomaticShootAction)
    virtual void stop() {};
protected:
    //! The weapon to use
    WeaponInstance *pWeapon_;
};

/*!
 * This action is used to shoot with a one shot gun.
 */
class ShootAction : public UseWeaponAction {
public:
    //! This constant is returned by Ped::addActionShootAt to indicate no action was added
    static const uint8 kShootActionNotAdded;
    //! This constant is returned by Ped::addActionShootAt to indicate that an AutomaticShootAction was added
    static const uint8 kShootActionAutomaticShoot;
    //! This constant is returned by Ped::addActionShootAt to indicate that a ShootAction was added
    static const uint8 kShootActionSingleShoot;

public:
    ShootAction(const WorldPoint &aimedAt, WeaponInstance *pWeapon);

    //! Entry point to execute the action
    bool execute(int elapsed, Mission *pMission, PedInstance *pPed);
    //! Update target position
    void setAimedAt(const WorldPoint &aimedAt);
protected:
    //! Fills the ShotAttributes with values
    void fillDamageDesc(Mission *pMission, PedInstance *pShooter, WeaponInstance *pWeapon, fs_dmg::DamageToInflict &dmg);
protected:
    //! Where the player aimed with the mouse
    WorldPoint aimedAt_;
    //! Time to wait between two shoot actions
    int timeToWait_;
};

/*!
 * This action is used to shoot with automatic gun.
 */
class AutomaticShootAction : public ShootAction {
public:
    AutomaticShootAction(const WorldPoint &aimedAt, WeaponInstance *pWeapon);

    bool execute(int elapsed, Mission *pMission, PedInstance *pPed);
    void stop();
protected:
    /*! Fire rate.*/
    fs_utils::Timer fireRateTimer_;
};

/*!
 * This action is for healing the owner with a medikit.
 */
class UseMedikitAction : public UseWeaponAction {
public:
    UseMedikitAction(WeaponInstance *pMedikit) : UseWeaponAction(pMedikit) {}

    //! Entry point to execute the action
    bool execute(int elapsed, Mission *pMission, PedInstance *pPed);
protected:
    //! Time to wait between two weapon actions
    int timeToWait_;
};

/*!
 * This action is used when player activates an energy shield.
 */
class UseEnergyShieldAction : public UseWeaponAction {
public:
    UseEnergyShieldAction(WeaponInstance *pEnergyShield) : UseWeaponAction(pEnergyShield) {}

    //! Entry point to execute the action
    bool execute(int elapsed, Mission *pMission, PedInstance *pPed);
    void stop();
protected:

};

#endif // IA_ACTIONS_H_

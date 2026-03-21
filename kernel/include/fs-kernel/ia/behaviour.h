/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2013  Bohdan Stelmakh <chamel@users.sourceforge.net>
 *   Copyright (C) 2013, 2024-2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef IA_BEHAVIOUR_H_
#define IA_BEHAVIOUR_H_

#include <list>

#include "fs-utils/misc/timer.h"
#include "fs-kernel/ia/actions.h"

namespace fs_knl {
class Mission;
class PedInstance;
class BehaviourComponent;
class WeaponInstance;

/*!
 * A Behaviour drives the ped's reactions.
 * It is composed of a set of components, each one
 * responsible for an aspect of the reaction.
 * Behaviour reacts to events and modify the ped's
 * actions.
 */
class Behaviour {
public:
    /*!
     * List of events that may affect behaviours.
     */
    enum BehaviourEventType {
        //! An agent has activated his Persuadotron
        kBehvEvtPersuadotronActivated,
        //! An agent has deactivated his Persuadotron
        kBehvEvtPersuadotronDeactivated,
        //! An ped has been hit
        kBehvEvtHit,
        //! A ped has shown his weapon
        kBehvEvtWeaponOut,
        //! A ped has cleared his weapon
        kBehvEvtWeaponCleared,
        //! An action has ended
        kBehvEvtActionEnded,
        //! An agent is getting into a vehicle so do his persuadeds
        kBehvEvtEnterVehicle,
        //! Car driver has been shot so he is ejected from the car
        kBehvEvtEjectedFromVehicle,
    };

    struct BehaviourEvent {
        PedInstance *pPed;
        BehaviourEventType evtType;
        void *pCtxt;
    };

    struct BehaviourParam {
        //! elapsed Time elapsed since last frame
        uint32_t elapsed;
        //! Mission data
        Mission *pMission;
        //! The owner of the behaviour
        PedInstance *pPed;
    };

    virtual ~Behaviour();

    void setOwner(PedInstance *pPed) { pThisPed_ = pPed; }
    //! Adds a component to the behaviour
    void addComponent(BehaviourComponent *pComp);
    //! Destroy existing components and set given one as new one
    void replaceAllcomponentsBy(BehaviourComponent *pComp);

    virtual void execute(uint32_t elapsed, Mission *pMission);

    virtual void handleBehaviourEvent(BehaviourEventType evtType, void *pCtxt = NULL);

    //! Alert this behaviour's hostile component (PlayerHostile or GuardArea) to a new enemy target
    void alertToEnemy(PedInstance *pOwner, PedInstance *pTarget);
protected:
    void destroyComponents();
protected:
    /*! The ped that use this behaviour.*/
    PedInstance *pThisPed_;
    /*! List of behaviour components.*/
    std::list <BehaviourComponent *> compLst_;
};

/*!
 * Abstract class that represent an aspect of a behaviour.
 * A component may be disabled according to certain types of events.
 */
class BehaviourComponent {
public:
    BehaviourComponent() { enabled_ = true; }
    virtual ~BehaviourComponent() {}

    bool isEnabled() { return enabled_; }
    void setEnabled(bool val) { enabled_ = val; }

    virtual void execute(const Behaviour::BehaviourParam &param) = 0;

    virtual void handleBehaviourEvent([[maybe_unused]] const Behaviour::BehaviourEvent &event){};

protected:
    bool enabled_;
};

/*!
 * This class defines commons behaviours for all agents (good or bad).
 * It is responsible for :
 * - regenerating life for agents that are hurt
 */
class CommonAgentBehaviourComponent : public BehaviourComponent {
public:
    //! Amount of health regenerated each period
    static const int kRegeratesHealthStep;

    CommonAgentBehaviourComponent(PedInstance *pPed);

    void execute(const Behaviour::BehaviourParam &param) override;

    void handleBehaviourEvent(const Behaviour::BehaviourEvent &event) override;
private:
    /*! Flag to indicate whether ped can regenerate his health.*/
    bool doRegenerates_;
    //! used for health regeneration
    fs_utl::Timer healthTimer_;
};

/*!
 * Component for user of Persuadotron. Only our agents use this component.
 */
class PersuaderBehaviourComponent : public BehaviourComponent {
public:
    PersuaderBehaviourComponent();

    void execute(const Behaviour::BehaviourParam &param) override;

    void handleBehaviourEvent(const Behaviour::BehaviourEvent &event) override;
private:
    /*! Flag to indicate an agent can use his persuadotron.*/
    bool doUsePersuadotron_;
    int persuadotronRange_;
};

/*!
 * Component for peds who are persuaded by an agent.
 * With this component, peds will follow their owner.
 * When owner has a gun, persuaded will look for a weapon if
 * he has not one already and will show it.
 */
class PersuadedBehaviourComponent : public BehaviourComponent {
public:
    PersuadedBehaviourComponent();

    void execute(const Behaviour::BehaviourParam &param) override;

    void handleBehaviourEvent(const Behaviour::BehaviourEvent &event) override;
private:
    WeaponInstance * findWeaponWithAmmo(Mission *pMission, PedInstance *pPed);
    void changeTargetWeaponInAltActions(WeaponInstance *pWeapon, PedInstance *pPed);

private:
    /*!
     * Status of persuaded.
     */
    enum PersuadedStatus {
        //! Behaviour will wait until current action is finished to initialize
        kPersuadStatusWaitForHitAction,
        //! Behaviour will run some initialization
        kPersuadStatusInitializing,
        //! Default status : just follow the leada
        kPersuadStatusFollow,
        //! Search for a nearby weapon
        kPersuadStatusLookForWeapon,
        //! Saw a weapon, go take it
        kPersuadStatusTakeWeapon
    };

    static const double kMaxRangeForSearchingWeapon;

    PersuadedStatus status_;

    //! used for delaying checking of nearby weapon search
    fs_utl::Timer checkWeaponTimer_;
};

/*!
 * Component for civilians who can panic when a gun is out.
 * Civilians in cars and which have scripted actions don't panic.
 * Civilians who have this component should only have one defaut action:
 * walk continuously.
 */
class PanicComponent : public BehaviourComponent {
public:
    //! Range for looking for armed ped
    static const int kScoutDistance;
    //! Base escape distance — scaled by perception in runAway()
    static const int kBaseDistanceToRun;

    PanicComponent();

    void execute(const Behaviour::BehaviourParam &param) override;

    void handleBehaviourEvent(const Behaviour::BehaviourEvent &event) override;
private:
    //! Checks whether there is an armed ped next to the ped : returns that ped
    PedInstance * findNearbyArmedPed(Mission *pMission, PedInstance *pPed);
    //! Makes the ped runs in the opposite direction from the armed ped
    void  runAway(PedInstance *pPed);
private:
    /*!
     * Status of panic behaviour.
     */
    enum PanicStatus {
        //! Default status : not panicking
        kPanicStatusCalm,
        //! Ped is in panic
        kPanicStatusInPanic
    };

    PanicStatus status_;
    /*! This timer is used to delay checking by the ped in order to
     * not consume too much CPU.*/
    fs_utl::Timer scoutTimer_;
    /*! Use to detect if ped is getting from panic to not panic.*/
    bool backFromPanic_;
    /*! The ped that frightened this civilian.*/
    PedInstance *pArmedPed_;
};

class PoliceBehaviourComponent : public BehaviourComponent {
public:
    PoliceBehaviourComponent();

    void execute(const Behaviour::BehaviourParam &param) override;

    void handleBehaviourEvent(const Behaviour::BehaviourEvent &event) override;
private:
    void handleEjectionFromVehicle(PedInstance *pPed, void *pCtxt);
    //! Find a nearby armed Ped and follow and shoot him
    bool findAndEngageNewTarget(Mission *pMission, PedInstance *pPed);
    //! Checks whether there is an armed ped next to the ped : returns that ped
    PedInstance * findArmedPedNotPolice(Mission *pMission, PedInstance *pPed);
    //! Initiate the process of following and shooting at a target
    void followAndShootTarget(PedInstance *pPed, PedInstance *pArmedGuy);
private:
    static const int kPoliceScoutDistance;
    static const int kPolicePendingTime;
    /*!
     * Status of police behaviour.
     */
    enum PoliceStatus {
        //! Police is walking or driving car
        kPoliceStatusDefault,
        //! Search for someone who pulled his gun
        kPoliceStatusAlert,
        //! Move closer from target to shoot at him
        kPoliceStatusFollowAndShootTarget,
        //! When target drops his weapon, wait some time
        kPoliceStatusPendingEndFollow,
        //! after waiting, check if police should go on patrol again
        kPoliceStatusCheckReengageOrDefault,
        //! Police ped is going away from vehicle
        kPoliceStatusOutOfVehicle
    };

    PoliceStatus status_;
    /*! This timer is used to delay checking by the ped in order to
     * not consume too much CPU.*/
    fs_utl::Timer scoutTimer_;
    /*! The ped that the police officer is watching and eventually shooting at.*/
    PedInstance *pTarget_;
};

/*!
 * Defensive behaviour for player agents.
 * Each scan period, checks for nearby armed hostile peds within a
 * Perception-scaled radius. If the agent already has a weapon drawn,
 * fires at the closest threat without moving.
 * Agents never auto-equip weapons and never move toward threats.
 */
class AgentDefenseBehaviourComponent : public BehaviourComponent {
public:
    AgentDefenseBehaviourComponent();

    void execute(const Behaviour::BehaviourParam &param) override;

private:
    //! Base detection range (same as other scout distances)
    static const int kBaseDefenseRange;

    //! Find the nearest armed hostile ped within the given range
    PedInstance * findNearestArmedHostile(Mission *pMission, PedInstance *pPed, int range);

    /*! Timer to throttle the hostile scan to avoid scanning every frame.*/
    fs_utl::Timer scoutTimer_;
    /*! The ped currently being engaged, checked every frame for death.*/
    PedInstance *pTarget_;
};

/*!
 * This behaviour is for peds that fight only player's agents.
 */
class PlayerHostileBehaviourComponent : public BehaviourComponent {
public:
    //! Range within which allies are alerted when this ped spots an enemy
    static const int kAllyAlertRange;
    //! Maximum detection range
    static const int kMaxIntelRange;

    PlayerHostileBehaviourComponent();

    void execute(const Behaviour::BehaviourParam &param) override;

    void handleBehaviourEvent(const Behaviour::BehaviourEvent &event) override;

    //! Force this component to engage pTarget immediately (called by ally alerting)
    void alertToEnemy(PedInstance *pOwner, PedInstance *pTarget);

private:
    //! Find a visible player agent within perception-scaled range with LOS check
    PedInstance * findVisiblePlayerAgent(Mission *pMission, PedInstance *pPed);
    void followAndShootTarget(PedInstance *pPed, PedInstance *pArmedGuy);
    //! Alert nearby allies of the same group to engage pTarget
    void alertNearbyAllies(Mission *pMission, PedInstance *pPed, PedInstance *pTarget);
    //! Compute effective detection range based on weapon and perception IPA
    int computeDetectionRange(PedInstance *pPed);

   /*!
     * Status for behavior.
     */
    enum PlayerHostileStatus {
        //! Default status : ped execute default actions
        kHostileStatusDefault,
        //! The owner has found a target : so he follows it and shoot when nearby
        kHostileStatusFollowAndShoot,
        //! When target is dead, wait some time
        kHostileStatusPendingEndFollow,
        //! after waiting, check if ped go back to default
        kHostileStatusCheckForDefault
    };

    PlayerHostileStatus status_;
    /*! This timer throttles the enemy scan to avoid checking every frame.*/
    fs_utl::Timer scoutTimer_;
    /*! The ped that the owner has targeted and potentially is shooting at.*/
    PedInstance *pTarget_;
};

/*!
 * Guard area behaviour: enemy stays in place and shoots at visible threats.
 */
class GuardAreaBehaviourComponent : public BehaviourComponent {
public:
    GuardAreaBehaviourComponent();

    void execute(const Behaviour::BehaviourParam &param) override;

    void handleBehaviourEvent(const Behaviour::BehaviourEvent &event) override;

    //! Force this component to engage pTarget immediately (called by ally alerting)
    void alertToEnemy(PedInstance *pOwner, PedInstance *pTarget);

private:
    //! Find a visible enemy within perception-scaled range
    PedInstance * findVisibleEnemy(Mission *pMission, PedInstance *pPed);
    //! Compute detection range based on weapon and perception
    int computeDetectionRange(PedInstance *pPed);

    enum GuardAreaStatus {
        //! Default: scanning for enemies
        kGuardStatusDefault,
        //! Actively shooting at a target (stationary)
        kGuardStatusShooting,
        //! Target dead or lost, brief pause before re-scanning
        kGuardStatusPendingRescan
    };

    GuardAreaStatus status_;
    fs_utl::Timer scoutTimer_;
    PedInstance *pTarget_;
};

}

#endif // IA_BEHAVIOUR_H_

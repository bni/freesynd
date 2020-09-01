/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2013  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2014  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef IA_BEHAVIOUR_H_
#define IA_BEHAVIOUR_H_

#include <list>

#include "utils/timer.h"
#include "ia/actions.h"

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
    enum BehaviourEvent {
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

    virtual ~Behaviour();

    void setOwner(PedInstance *pPed) { pThisPed_ = pPed; }
    //! Adds a component to the behaviour
    void addComponent(BehaviourComponent *pComp);
    //! Destroy existing components and set given one as new one
    void replaceAllcomponentsBy(BehaviourComponent *pComp);

    virtual void execute(int elapsed, Mission *pMission);

    virtual void handleBehaviourEvent(BehaviourEvent evtType, void *pCtxt = NULL);
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

    virtual void execute(int elapsed, Mission *pMission, PedInstance *pPed) = 0;

    virtual void handleBehaviourEvent(PedInstance *pPed, Behaviour::BehaviourEvent evtType, void *pCtxt){};

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

    void execute(int elapsed, Mission *pMission, PedInstance *pPed);

    void handleBehaviourEvent(PedInstance *pPed, Behaviour::BehaviourEvent evtType, void *pCtxt);
private:
    /*! Flag to indicate whether ped can regenerate his health.*/
    bool doRegenerates_;
    //! used for health regeneration
    fs_utils::Timer healthTimer_;
};

/*!
 * Component for user of Persuadotron. Only our agents use this component.
 */
class PersuaderBehaviourComponent : public BehaviourComponent {
public:
    PersuaderBehaviourComponent();

    void execute(int elapsed, Mission *pMission, PedInstance *pPed);

    void handleBehaviourEvent(PedInstance *pPed, Behaviour::BehaviourEvent evtType, void *pCtxt);
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

    void execute(int elapsed, Mission *pMission, PedInstance *pPed);

    void handleBehaviourEvent(PedInstance *pPed, Behaviour::BehaviourEvent evtType, void *pCtxt);
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
    fs_utils::Timer checkWeaponTimer_;
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
    //! The distance a panicking ped walks before calming down
    static const int kDistanceToRun;

    PanicComponent();

    void execute(int elapsed, Mission *pMission, PedInstance *pPed);

    void handleBehaviourEvent(PedInstance *pPed, Behaviour::BehaviourEvent evtType, void *pCtxt);
private:
    //! Checks whether there is an armed ped next to the ped : returns that ped
    PedInstance * findNearbyArmedPed(Mission *pMission, PedInstance *pPed);
    //! Makes the ped runs in the opposite direction from the armed ped
    void  runAway(PedInstance *pPed);
private:
    /*!
     * Status of police behaviour.
     */
    enum PanicStatus {
        //! Default status : not panicking
        kPanicStatusAlert,
        //! Ped is in panic
        kPanicStatusInPanic
    };

    PanicStatus status_;
    /*! This timer is used to delay checking by the ped in order to
     * not consume too much CPU.*/
    fs_utils::Timer scoutTimer_;
    /*! Use to detect if ped is getting from panic to not panic.*/
    bool backFromPanic_;
    /*! The ped that frightened this civilian.*/
    PedInstance *pArmedPed_;
};

class PoliceBehaviourComponent : public BehaviourComponent {
public:
    PoliceBehaviourComponent();

    void execute(int elapsed, Mission *pMission, PedInstance *pPed);

    void handleBehaviourEvent(PedInstance *pPed, Behaviour::BehaviourEvent evtType, void *pCtxt);
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
    fs_utils::Timer scoutTimer_;
    /*! The ped that the police officer is watching and eventually shooting at.*/
    PedInstance *pTarget_;
};

/*!
 * This behaviour is for peds that fight only player's agents.
 */
class PlayerHostileBehaviourComponent : public BehaviourComponent {
public:
    PlayerHostileBehaviourComponent();

    void execute(int elapsed, Mission *pMission, PedInstance *pPed);

    void handleBehaviourEvent(PedInstance *pPed, Behaviour::BehaviourEvent evtType, void *pCtxt);

private:
    //! looking for the nearest player agent
    PedInstance * findPlayerAgent(Mission *pMission, PedInstance *pPed);
    void followAndShootTarget(PedInstance *pPed, PedInstance *pArmedGuy);
private:
    static const int kEnemyScoutDistance;

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
    /*! The ped that the owner has targeted and potentially is shooting at.*/
    PedInstance *pTarget_;
};


#endif // IA_BEHAVIOUR_H_

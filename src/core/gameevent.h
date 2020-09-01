/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2011  Benoit Blancard <benblan@users.sourceforge.net>*
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

#ifndef GAMEVENT_H
#define GAMEVENT_H

#include <stddef.h>

/*!
 * An event is dispatched by the Game controller towards listener that
 * will handle it. There are several stream on which events are posted:
 * A game stream that concerns general events and a mission stream that
 * contains events inside a mission.
 */
class GameEvent {
public:
    /*!
     * The different streams.
     */
    enum EEventStream {
        //! General events
        kGame,
        //! In mission events
        kMission
    };

    enum EEventType {
        kNone,
        /*! An agent died during a mission.*/
        kAgentDied,
        /*! A research has ended.*/
        kResearch,
        /*! Sent when evacuation order arrives.*/
        kObjEvacuate,
        /*! Sent when an objective with a MapObject target arrives.*/
        kObjTargetSet,
        /*! Sent when an objective has failed.*/
        kObjFailed,
        /*! Send when an objective has been completed succesfuly.*/
        kObjCompleted,
        /*! Sent when a ped has shown his weapon.*/
        kEvtShootingWeaponSelected,
        /*! Sent when a ped cleared his selected shooting weapon.*/
        kEvtShootingWeaponDeselected,
        /*! Sent when a policeman warns a player agent.*/
        kEvtWarnAgent
    };
    //! The stream on which the event is posted
    EEventStream stream;
    //! The type of event
    EEventType type;
    //! An event may hold information for the listener
    void *pCtxt;

    //! Convenient method to send game event
    static void sendEvt(EEventStream stream, EEventType type, void *pCtx = NULL);
};

/*!
 * A listener will receive events on the stream he has subscrived.
 */
class GameEventListener {
public:
    virtual ~GameEventListener() {}
    /*!
     * This method is called when an event is posted.
     */
    virtual void handleGameEvent(GameEvent evt) = 0;
};

#endif //GAMEVENT_H

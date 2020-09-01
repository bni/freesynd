/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *   Copyright (C) 2016  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "train.h"
#include "mission.h"
#include "model/squad.h"

TrainBody::TrainBody(uint16 anId, uint8 aType, VehicleAnimation *pAnimation, int startHp, bool isMoveOnXAxis) :
    Vehicle(anId, aType, -1, pAnimation) {

    setHealth(startHp);
    setStartHealth(startHp);

    pNextBody_ = NULL;
    moveOnXaxis_ = isMoveOnXAxis;
}

TrainBody::~TrainBody() {
}

void TrainBody::dropAllPassengers(const Mission &mission, const TilePoint &dropPos) {
    while (!passengers_.empty()) {
        PedInstance *pPed = passengers_.front();
        pPed->leaveVehicle();
        pPed->setPosition(dropPos);
        // when leaving train, passengers walk towards the station
        if (pPed->isOurAgent()) {
            TilePoint movePos(dropPos);
            isMovementOnXAxis() ? movePos.ty += 2 : movePos.tx += 2;
            movePos.tz += 1; // train station are higher than trains
            mission.getSquad()->getPositionInSquadFormation(pPed->id(), &movePos);
            pPed->addActionWalk(movePos, false);
        } else {
            // here are only persuaded ped. So continue following owner
            pPed->setCurrentActionWithSource(Action::kActionDefault);
        }
        passengers_.pop_front();
    }
}

void TrainBody::changeTrainAndPassengersPosition(int distanceX, int distanceY) {
    addOffsetToPosition(distanceX, distanceY);

    if (!passengers_.empty()) {
        for (std::list<PedInstance *>::iterator it = passengers_.begin();
            it != passengers_.end(); it++
        ) {
            (*it)->setPosition(pos_);
        }
    }

    if (pNextBody_ != NULL) {
        pNextBody_->changeTrainAndPassengersPosition(distanceX, distanceY);
    }
}

TrainHead::TrainHead(uint16 anId, uint8 aType, VehicleAnimation *pAnimation, int startHp, bool isMoveOnXAxis) :
    TrainBody(anId, aType, pAnimation, startHp, isMoveOnXAxis) {}

TrainHead::~TrainHead() {

}

//! Set the destination to reach at given speed
bool TrainHead::initMovementToDestination(Mission *m, const TilePoint &destinationPt, int newSpeed) {
    clearDestination();

    dest_path_.push_front(destinationPt);
    speed_ = newSpeed;

    return true;
}

/*!
 * Moves a vehicle on the map.
 * \param elapsed Elapsed time sine last frame.
 */
bool TrainHead::doMove(int elapsed, Mission *m)
{
    bool updated = false;
    int remainingTime = elapsed;

    while ((!dest_path_.empty()) && remainingTime != 0) {

        // Get distance between vehicle and next NodePath
        WorldPoint destination(dest_path_.front());

        double distanceToNextNode = distanceToPosition(destination);
        // This is the time for all the remaining distance to the next node in the path
        double availableTimeToNextNode = (distanceToNextNode / (double)speed_) * 1000.0;
        // We cannot spend more time than the time remaining
        if (availableTimeToNextNode > remainingTime)
            availableTimeToNextNode = remainingTime;

        // computes distance travelled by vehicle in the available time
        WorldPoint currentPos(pos_);
        int  distanceX = 0, distanceY = 0;
        if (isMovementOnXAxis()) {
            int diffx = destination.x - currentPos.x;
            distanceX = (int)((diffx * (speed_ * availableTimeToNextNode) / distanceToNextNode) / 1000);
        } else {
            int diffy = destination.y - currentPos.y;
            distanceY = (int)((diffy * (speed_ * availableTimeToNextNode) / distanceToNextNode) / 1000);
        }

        // Updates the remaining time
        remainingTime -= availableTimeToNextNode;

        // Moves vehicle
        changeTrainAndPassengersPosition(distanceX, distanceY);

        stopIfDestinationReached(destination);

        updated = true;
    }

    return updated;
}

void TrainHead::stopIfDestinationReached(const WorldPoint &destinationPt) {
    bool stop = false;
    WorldPoint currentPos(pos_);

    stop = isMovementOnXAxis() ?
                abs(destinationPt.x - currentPos.x) < 4:
                abs(destinationPt.y - currentPos.y) < 4;

    if(stop) {
        dest_path_.pop_front();
        speed_ = 0;
    }
}

void TrainHead::appendTrainBody(TrainBody *pTrainBodyToAdd) {
    TrainBody *pBody = this;
    while (pBody->getNext() != NULL) {
        pBody = pBody->getNext();
    }
    pBody->setNext(pTrainBodyToAdd);
}


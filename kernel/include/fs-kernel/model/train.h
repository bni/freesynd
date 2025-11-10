/*
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".
 *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> 
 *   Copyright (C) 2025  Benoit Blancard <benblan@users.sourceforge.net>
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

#ifndef MODEL_TRAIN_H_
#define MODEL_TRAIN_H_

#include <string>
#include <list>

#include "vehicle.h"

namespace fs_knl {
/*!
 * A train is composed of TrainBody chained together.
 * The train head contains the driver and moves. The other parts
 * of the train move along with the TrainHead.
 */
class TrainBody : public Vehicle {
public:
    TrainBody(uint16_t id, uint8_t aType, Map *pMap, int startHp, bool isMoveOnXAxis, int maxSpeed = 0);
    ~TrainBody();

    TrainBody * getNext() { return pNextBody_; }

    void setNext(TrainBody *pNext) { pNextBody_ = pNext; }

    //! Set the destination to reach at given speed
    bool initMovementToDestination([[maybe_unused]] Mission *m,
                                    [[maybe_unused]] const TilePoint &destinationPt) override {
        return false;
    }

    bool doMove([[maybe_unused]] uint32_t elapsed, [[maybe_unused]] Mission *m) override {
        return false;
    }

    void dropAllPassengers(const Mission &mission, const TilePoint &dropPos);

protected:
    bool isMovementOnXAxis() {
        return moveOnXaxis_;
    }
    //! add given amount to train position and updates passengers position
    void changeTrainAndPassengersPosition(int distanceX, int distanceY);

protected:
    //! Next part of the train
    TrainBody *pNextBody_;
    //! True means this train is moving on the X axis, else on the Y axis
    bool moveOnXaxis_;
};

/*!
 * .
 */
class TrainHead : public TrainBody {
public:
    TrainHead(uint16_t id, uint8_t aType, Map *pMap, int startHp, bool isMoveOnXAxis, int maxSpeed);
    ~TrainHead();

    //! Set the destination to reach at given speed
    bool initMovementToDestination(Mission *m, const TilePoint &destinationPt) override;

    bool doMove(uint32_t elapsed, Mission *m) override;

    void appendTrainBody(TrainBody *pTrainBody);

private:
    // If the destination is reached the train stops
    void stopIfDestinationReached(const WorldPoint &destinationPt);
};

}
#endif // MODEL_TRAIN_H_

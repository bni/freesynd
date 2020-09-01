/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
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

#include "seqmodel.h"

SequenceModel::~SequenceModel() {
    listeners_.clear();
}

void SequenceModel::addModelListener(ModelListener *pListener) {
    listeners_.push_back(pListener);
}

void SequenceModel::removeModelListener(ModelListener *pListener) {
    for (std::list < ModelListener * >::iterator it = listeners_.begin();
         it != listeners_.end(); it++) {
             if (pListener == *it) {
                 listeners_.erase(it);
                 return;
             }
    }
}

void SequenceModel::fireModelChanged() {
    for (std::list < ModelListener * >::iterator it = listeners_.begin();
         it != listeners_.end(); it++) {
             (*it)->handleModelChanged();
    }
}

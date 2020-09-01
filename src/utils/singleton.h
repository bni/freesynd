/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#ifndef SINGLETON_H
#define SINGLETON_H

// From _An Automatic Singleton Utility_ by Scott Bilas
// http://www.drizzle.com/~scottb/publish/gpgems1_singleton.htm
template<typename T> class Singleton {
    static T *singleton_;

public:
    Singleton() {
        assert(!singleton_);
        singleton_ = static_cast<T *> (this);
    }

    virtual ~Singleton() {
        assert(singleton_);
        singleton_ = 0;
    }

    static T &singleton() {
        assert(singleton_);
        return *singleton_;
    }

    static T *singletonPtr() {
        return singleton_;
    }
};

template<typename T> T *Singleton<T>::singleton_ = 0;

#endif

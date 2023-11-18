/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#ifndef ENGINE_BASEAPP_H
#define ENGINE_BASEAPP_H

#include <string>
#include <memory>

#include "fs-engine/appcontext.h"
#include "fs-engine/gfx/screen.h"
#include "fs-engine/system.h"

/*!
 * Base class for a Freesynd application.
 */
class BaseApp {
public:
    //! Constructor
    BaseApp();
    //! Destructor
    virtual ~BaseApp();

    //! Initialize application
    virtual bool initialize(const std::string& iniPath, bool disable_sound) final;

    //! Destroy all components
    virtual void destroy() final;

protected:
    //! Child class implements this method for initalization
    virtual bool doInitialize(const std::string& iniPath, bool disable_sound);
    //! Child class overloads this method for destroying ressources
    virtual void doDestroy();

    /*! A structure to hold general application informations.*/
    std::unique_ptr<AppContext> context_;
    std::unique_ptr<Screen> screen_;
    std::unique_ptr<System> system_;
};

#endif // ENGINE_BASEAPP_H

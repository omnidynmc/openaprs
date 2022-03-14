/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, mySQAL APRS Injector                                       **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Daniel Robert Karrels                             **
 **                    Dynamic Networking Solutions                      **
 **                                                                      **
 ** This program is free software; you can redistribute it and/or modify **
 ** it under the terms of the GNU General Public License as published by **
 ** the Free Software Foundation; either version 1, or (at your option)  **
 ** any later version.                                                   **
 **                                                                      **
 ** This program is distributed in the hope that it will be useful,      **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of       **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with this program; if not, write to the Free Software          **
 ** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            **
 **************************************************************************
 $Id: Module.h,v 1.1.1.1 2003/08/17 01:16:09 omni Exp $
 **************************************************************************/

#ifndef __MODULE_H
#define __MODULE_H

#include <list>

#include <time.h>
#include <sys/time.h>
#include <dlfcn.h>

#include "openaprs.h"

namespace openaprs {

using std::string;
using std::map;
using std::list;
using std::pair;

/*
 **************************************************************************
 ** General Defines                                                      **
 **************************************************************************
 */

/*
 **************************************************************************
 ** Proto types                                                          **
 **************************************************************************
 */

typedef void (*moduleTypeFunc)();

/*
 **************************************************************************
 ** Structures                                                           **
 **************************************************************************
 */

class Module {
  public:

    Module(const string &);			// constructor
    virtual ~Module();				// destructor

    // return loaded
    bool is_loaded()
      { return loaded; }

    void setPersistent(const bool setMe)
      { myPersistent = setMe; }

    const char *getPath() { return path.c_str(); }
    const bool getPersistent() { return myPersistent; }

    time_t getLoaded();

    void runConstructor();
    void runDestructor();

  protected:
    bool loaded;
    bool myPersistent;

    moduleTypeFunc moduleConstructor;
    moduleTypeFunc moduleDestructor;

    string path;
    string description;

    time_t ts;

    void *dlhandler;
};

/*
 **************************************************************************
 ** Macro's                                                              **
 **************************************************************************
 */

}
#endif

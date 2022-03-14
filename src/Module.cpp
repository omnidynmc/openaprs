/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, mySQL APRS Injector                                        **
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
 $Id: Module.cpp,v 1.1.1.1 2003/08/17 01:16:18 omni Exp $
 **************************************************************************/

#include <list>
#include <new>
#include <string>

#include <dlfcn.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "Module.h"

#include "openaprs.h"
#include "openaprs_string.h"

namespace openaprs {

  using std::string;
  using std::list;

  Module::Module(const string &addMe) {
    path.append(addMe.c_str());

    // establish defaults
    ts = time(NULL);
    description.append("!none");
    myPersistent = false;

//    dlhandler = dlopen(addMe.c_str(), RTLD_LAZY);
    dlhandler = dlopen(addMe.c_str(), RTLD_NOW | RTLD_NODELETE);
    if (dlhandler == NULL) {
      loaded = false;
    }
    else {
      loaded = true;

      moduleConstructor = (moduleTypeFunc) dlsym(dlhandler, "module_constructor");
      moduleDestructor = (moduleTypeFunc) dlsym(dlhandler, "module_destructor");

      if (moduleConstructor == (moduleTypeFunc) NULL || moduleDestructor == (moduleTypeFunc) NULL) {
        loaded = false;
        dlclose(dlhandler);
      } // if
      else
        (*moduleConstructor)();
    } // else
  } // Module::Module

  Module::~Module() {
    if (loaded == true) {
      (*moduleDestructor)();
      dlclose(dlhandler);
    } // if
  } // Module::~Module
} // namespace openaprs

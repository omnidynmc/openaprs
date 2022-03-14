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
 $Id: ModuleList.h,v 1.1.1.1 2003/08/17 01:16:09 omni Exp $
 **************************************************************************/

#ifndef __MODULELIST_H
#define __MODULELIST_H

#include <list>

#include <time.h>
#include <sys/time.h>

#include "Module.h"

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
 ** Structures                                                           **
 **************************************************************************
 */

class ModuleList {
  public:
    ModuleList();				// constructor
    virtual ~ModuleList();			// destructor

    Module *find(const string &);		// find a module

    bool add(const string &, const bool);	// add a module

    // return error message
    const char *getError()
      { return errorMessage.c_str(); }

    // return number of items
    int num_items()
      { return itemCount; }

    void clear();				// remove all modules
    void clearAll();				// remove all modules
    const unsigned int remove(const string &);		// remove a module

  protected:
    list<Module*> *moduleList;			// linked list of items

    int itemCount;				// number of items in queue

    string errorMessage;			// container for error message
};

/*
 **************************************************************************
 ** Macro's                                                              **
 **************************************************************************
 */

/*
 **************************************************************************
 ** Proto types                                                          **
 **************************************************************************
 */

}
#endif

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
 $Id: ModuleList.cpp,v 1.2 2003/08/17 15:40:15 omni Exp $
 **************************************************************************/

#include <list>
#include <new>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <cassert>
#include <time.h>
#include <dlfcn.h>

#include "Module.h"
#include "ModuleList.h"

#include "openaprs.h"
#include "openaprs_string.h"

namespace openaprs {

using namespace std;

ModuleList::ModuleList() {
  // create a new moduleList
  try {
    moduleList = new list<Module*>;
  }
  catch(bad_alloc xa) {
    assert(false);
  }

  // setup variables
  itemCount = 0;
}

ModuleList::~ModuleList() {
  // cleanup
  clearAll();
  delete moduleList;
}

bool ModuleList::add(const string &addMe, const bool setPersistent) {
  Module *aModule;		// pointer to a new item

  if (find(addMe) != (Module *) NULL) {
    errorMessage = "Module already loaded.";
    return false;
  }

  // create a new Module record
  try {
    aModule = new Module(addMe);
  }
  catch(bad_alloc xa) {
    assert(false);
  }

  // initialize module defaults
  aModule->setPersistent(setPersistent);

  if (aModule->is_loaded()) {
    // add item to the list
    moduleList->push_back(aModule);

    // increase our item count number
    itemCount++;

    return true;
  }

  // cleanup
  delete aModule;

  errorMessage = dlerror();

  return false;
}

void ModuleList::clear() {
  Module *aModule;		// pointer to a module in the list
  list<Module*> tempList;	// temp list of modules

  while(!moduleList->empty()) {
    aModule = moduleList->front();

    // item record from list
    moduleList->pop_front();

    if (aModule->getPersistent() == false) {
      delete aModule;
      continue;
    }

    // save item to temp list
    tempList.push_back(aModule);
  }

  itemCount = 0;
  // save our new list
  while(!tempList.empty()) {
    moduleList->push_back(tempList.front());
    tempList.pop_front();
    itemCount++;
  }

  return;
}

void ModuleList::clearAll() {
  Module *aModule;		// pointer to a module in the list

  while(!moduleList->empty()) {
    aModule = moduleList->front();
    moduleList->pop_front();
    delete aModule;
  }

  itemCount = 0;

  return;
}

Module *ModuleList::find(const string &findMe) {
  list<Module*>::iterator ptr;
  Module *aModule;

  ptr = moduleList->begin();
  while(ptr != moduleList->end()) {
    aModule = *ptr;
    if (!cmp(aModule->getPath(), findMe.c_str()))
      return aModule;

    ptr++;
  }

  return NULL;
}

const unsigned int ModuleList::remove(const string &removeMe) {
  Module *aModule;		// pointer to a module in the list
  list<Module*> tempList;	// temp list of modules
  unsigned int numRemoved = 0;

  while(!moduleList->empty()) {
    aModule = moduleList->front();

    // item record from list
    moduleList->pop_front();

    if (!cmp(aModule->getPath(), removeMe.c_str())) {
      delete aModule;
      numRemoved++;
      continue;
    } // if

    // save item to temp list
    tempList.push_back(aModule);
  }

  itemCount = 0;
  // save our new list
  while(!tempList.empty()) {
    moduleList->push_back(tempList.front());
    tempList.pop_front();
    itemCount++;
  } // while

  return numRemoved;
}

}

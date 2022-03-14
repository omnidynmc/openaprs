#include <list>
#include <new>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <cassert>
#include <time.h>
#include <dlfcn.h>

#include <openframe/openframe.h>
#include <openframe/Module.h>
#include <openframe/ModuleList.h>

namespace openframe {
  ModuleList::ModuleList() {
    // create a new moduleList
    try {
      moduleList = new list<Module*>;
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch

    // setup variables
    itemCount = 0;
  } // ModuleList::ModuleList

  ModuleList::~ModuleList() {
    // cleanup
    clearAll();
    delete moduleList;
  } // ModuleList::~ModuleList

  bool ModuleList::add(const std::string &addMe, const bool setPersistent) {
    Module *aModule;		// pointer to a new item

    if (find(addMe) != (Module *) NULL) {
      errorMessage = "Module already loaded.";
      return false;
    } // if

    // create a new Module record
    try {
      aModule = new Module(addMe);
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch

    // initialize module defaults
    aModule->setPersistent(setPersistent);

    if (aModule->is_loaded()) {
      // add item to the list
      moduleList->push_back(aModule);

      // increase our item count number
      itemCount++;

      return true;
    } // if

    // cleanup
    delete aModule;

    errorMessage = dlerror();

    return false;
  } // ModuleList::add

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
      } // if

      // save item to temp list
      tempList.push_back(aModule);
    } // while

    itemCount = 0;
    // save our new list
    while(!tempList.empty()) {
      moduleList->push_back(tempList.front());
      tempList.pop_front();
      itemCount++;
    } // while

    return;
  } // ModuleList::clear

  void ModuleList::clearAll() {
    Module *aModule;		// pointer to a module in the list

    while(!moduleList->empty()) {
      aModule = moduleList->front();
      moduleList->pop_front();
      delete aModule;
    } // while

    itemCount = 0;

    return;
  } // ModuleList::clearAll

  Module *ModuleList::find(const std::string &findMe) {
    list<Module*>::iterator ptr;
    Module *aModule;

    ptr = moduleList->begin();
    while(ptr != moduleList->end()) {
      aModule = *ptr;
      if (!StringTool::compare(aModule->getPath(), findMe.c_str()))
        return aModule;

      ptr++;
    } // while

    return NULL;
  } // ModuleList::find

  const unsigned int ModuleList::remove(const std::string &removeMe) {
    Module *aModule;		// pointer to a module in the list
    list<Module*> tempList;	// temp list of modules
    unsigned int numRemoved = 0;

    while(!moduleList->empty()) {
      aModule = moduleList->front();

      // item record from list
      moduleList->pop_front();

      if (!StringTool::compare(aModule->getPath(), removeMe.c_str())) {
        delete aModule;
        numRemoved++;
        continue;
      } // if

      // save item to temp list
      tempList.push_back(aModule);
    } // while

    itemCount = 0;
    // save our new list
    while(!tempList.empty()) {
      moduleList->push_back(tempList.front());
      tempList.pop_front();
      itemCount++;
    } // while

    return numRemoved;
  } // ModuleList::remove

} // namespace openframe

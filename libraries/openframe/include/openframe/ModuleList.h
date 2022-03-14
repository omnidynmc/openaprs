#ifndef __OPENFRAME_MODULELIST_H
#define __OPENFRAME_MODULELIST_H

#include <list>

#include <time.h>
#include <sys/time.h>

#include "Module.h"

namespace openframe {
  using std::string;
  using std::map;
  using std::list;
  using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class ModuleList {
    public:
      ModuleList();				// constructor
      virtual ~ModuleList();			// destructor

      Module *find(const string &);		// find a module

      bool add(const string &, const bool);	// add a module
      const char *getError() { return errorMessage.c_str(); }
      int num_items() { return itemCount; }

      void clear();				// remove all modules
      void clearAll();				// remove all modules
      const unsigned int remove(const string &);		// remove a module

    protected:
      list<Module*> *moduleList;			// linked list of items

      int itemCount;				// number of items in queue

      string errorMessage;			// container for error message
  }; // class ModuleList

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openframe
#endif

#ifndef __OPENFRAME_MODULE_H
#define __OPENFRAME_MODULE_H

#include <list>
#include <map>
#include <string>

#include <time.h>
#include <sys/time.h>
#include <dlfcn.h>

namespace openframe {
  using std::string;
  using std::map;
  using std::list;
  using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

  typedef void (*moduleTypeFunc)();

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class Module {
    public:
      Module(const string &);			// constructor
      virtual ~Module();				// destructor

      // return loaded
      inline const bool is_loaded() const { return loaded; }
      void setPersistent(const bool setMe) { myPersistent = setMe; }
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
  }; // class Module

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/
} // namespace openframe
#endif

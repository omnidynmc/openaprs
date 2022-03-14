#include <list>
#include <new>
#include <string>
#include <iostream>

#include <dlfcn.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <openframe/Module.h>

namespace openframe {
  using std::string;
  using std::list;
  using std::cout;
  using std::endl;

  Module::Module(const string &addMe) {
    path.append(addMe.c_str());

    // establish defaults
    ts = time(NULL);
    description.append("!none");
    myPersistent = false;

//    dlhandler = dlopen(addMe.c_str(), RTLD_LAZY);
//    dlhandler = dlopen(addMe.c_str(), RTLD_NOW);
#ifdef OPENFRAME_MODULE_DEBUG
    dlhandler = dlopen(addMe.c_str(), RTLD_NOW | RTLD_NODELETE);
#else
    dlhandler = dlopen(addMe.c_str(), RTLD_NOW);
#endif

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
} // namespace openframe

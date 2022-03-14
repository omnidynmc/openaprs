#include <iostream>
#include <string>

#include <openframe/openframe.h>
#include <openframe/App_Abstract.h>
#include <openframe/ModuleList.h>
#include <openframe/stdinCommands.h>

namespace openframe {

/**************************************************************************
 ** stdinRELOADMODULE Class                                              **
 **************************************************************************/

  const int stdinRELOADMODULE::Execute(COMMAND_ARGUMENTS) {
    App_Abstract *app = static_cast<App_Abstract *>( ePacket->getVariable("app") );
    std::string name;

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    // initialize variables
    name = ePacket->getArguments()[0];

    unsigned int numAffected = app->modules()->num_items();
    app->modules()->remove(name);
    numAffected -= app->modules()->num_items();
    std::cout << "*** " << numAffected << " modules removed." << std::endl;

    if (app->modules()->add(name, true)) {
      std::cout << "*** Module: " << name << " loaded." << std::endl;
    } // if
    else
      std::cout << "*** Module: " << name << " failed to load; " << app->modules()->getError() << std::endl;

    return CMDERR_SUCCESS;
  } // stdinRELOADMODULE::Execute
} // namespace openframe

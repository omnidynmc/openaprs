#include <iostream>
#include <string>

#include <openframe/openframe.h>
#include <openframe/App_Abstract.h>
#include <openframe/Command.h>
#include <openframe/ModuleList.h>
#include <openframe/stdinCommands.h>

namespace openframe {

/**************************************************************************
 ** stdinREMOVEMODULE Class                                              **
 **************************************************************************/

  const int stdinREMOVEMODULE::Execute(COMMAND_ARGUMENTS) {
    App_Abstract *app = static_cast<App_Abstract *>( ePacket->getVariable("app") );
    std::string name;
    unsigned int numAffected;

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    // initialize variables
    name = ePacket->getArguments()[0];

    numAffected = app->modules()->num_items();
    app->modules()->remove(name);
    numAffected -= app->modules()->num_items();
    std::cout << "*** " << numAffected << " modules removed." << std::endl;

    return CMDERR_SUCCESS;
  } // stdinREMOVEMODULE::Execute
} // namespace openframe

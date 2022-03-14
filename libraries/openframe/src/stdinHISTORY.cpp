#include <iostream>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <openframe/App_Abstract.h>
#include <openframe/Command.h>
#include <openframe/stdinCommands.h>

namespace openframe {

/**************************************************************************
 ** stdinHISTORY Class                                                   **
 **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const int stdinHISTORY::Execute(COMMAND_ARGUMENTS) {
    App_Abstract *app = static_cast<App_Abstract *>( ePacket->getVariable("app") );
    app->history();
    return CMDERR_SUCCESS;
  } // stdinHISTORY::Execute
} // namespace openframe

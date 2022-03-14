#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <iostream>
#include <string>
#include <exception>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <openframe/App_Abstract.h>
#include <openframe/Command.h>
#include <openframe/stdinCommands.h>

namespace openframe {

/**************************************************************************
 ** stdinQUIT Class                                                      **
 **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/
  const int stdinQUIT::Execute(COMMAND_ARGUMENTS) {
    App_Abstract *app = static_cast<App_Abstract *>( ePacket->getVariable("app") );
    std::cout << "/QUIT Received, shutting down." << std::endl;
    app->set_done(true);

    return CMDERR_SUCCESS;
  } // stdinDTREE::Execute
} // namespace openframe

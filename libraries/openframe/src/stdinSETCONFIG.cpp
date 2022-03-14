#include <iostream>
#include <string>

#include <openframe/openframe.h>
#include <openframe/App_Abstract.h>
#include <openframe/Command.h>
#include <openframe/ModuleList.h>
#include <openframe/stdinCommands.h>

namespace openframe {

/**************************************************************************
 ** stdinSETCONFIG Class                                                 **
 **************************************************************************/

  const int stdinSETCONFIG::Execute(COMMAND_ARGUMENTS) {
    App_Abstract *app = static_cast<App_Abstract *>( ePacket->getVariable("app") );
    if (ePacket->args().size() < 2)
      return CMDERR_SYNTAX;

    // initialize variables
    std::string name = ePacket->getArguments()[0];
    std::string value = ePacket->args().trail(1);

    app->cfg->replace_string(name, value);
    std::cout << "OK" << std::endl;

    return CMDERR_SUCCESS;
  } // stdinSETCONFIG::Execute
} // namespace openframe

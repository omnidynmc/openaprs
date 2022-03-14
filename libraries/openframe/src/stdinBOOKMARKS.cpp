#include <openframe/config.h>

#include <openframe/App_Abstract.h>
#include <openframe/Command.h>
#include <openframe/stdinCommands.h>

namespace openframe {

/**************************************************************************
 ** stdinBOOKMARKS Class                                                 **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

  const int stdinBOOKMARKS::Execute(COMMAND_ARGUMENTS) {
    App_Abstract *app = static_cast<App_Abstract *>( ePacket->getVariable("app") );
    app->bookmarks();

    return CMDERR_SUCCESS;
  } // stdinBOOKMARKS::Execute
} // namespace openframe

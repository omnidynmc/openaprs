#include <openframe/config.h>

#include <iostream>
#include <string>
#include <list>

#include <openframe/openframe.h>
#include <openframe/App_Abstract.h>
#include <openframe/Command.h>
#include <openframe/stdinCommands.h>

namespace openframe {

/**************************************************************************
 ** stdinHELP Class                                                      **
 **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const int stdinHELP::Execute(COMMAND_ARGUMENTS) {
    App_Abstract *app = static_cast<App_Abstract *>( ePacket->getVariable("app") );
    std::list<string> treeList;			// returned tree list
    Stopwatch sw;

    app->showTree("STDIN", 1, "  ", treeList);

    sw.Start();

    Result res("HELP");
    while(!treeList.empty()) {
      Serialize s;
      s.add(treeList.front());
      Row r(s.compile());
      res.add(r);
      treeList.pop_front();
    } // while

    Result::print(res, 0, sw.Time());

    return CMDERR_SUCCESS;
  } // stdinHELP::Execute
} // namespace openframe

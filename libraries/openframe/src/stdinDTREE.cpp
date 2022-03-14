#include <list>
#include <string>

#include <openframe/openframe.h>
#include <openframe/App_Abstract.h>
#include <openframe/Command.h>
#include <openframe/stdinCommands.h>

namespace openframe {
/**************************************************************************
 ** stdinDTREE Class                                                     **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

  const int stdinDTREE::Execute(COMMAND_ARGUMENTS) {
    App_Abstract *app = static_cast<App_Abstract *>( ePacket->getVariable("app") );
    Stopwatch sw;
    std::list<string> treeList;			// returned tree list

    app->showTree(ePacket->getPath(), 1, "  ", treeList);

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
  } // stdinDTREE::Execute
} // namespace openframe

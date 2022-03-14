#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <exception>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <openframe/openframe.h>
#include <openframe/Command.h>
#include <openframe/stdinCommands.h>

namespace openframe {
/**************************************************************************
 ** stdinCAT Class                                                       **
 **************************************************************************/

  const int stdinCAT::Execute(COMMAND_ARGUMENTS) {
    Stopwatch sw;
    std::string filename;
    std::string contents;

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    filename = ePacket->getArguments()[0];

    sw.Start();

    try {
      contents = StringTool::getFileContents(filename);
    } // try
    catch(StringTool_Exception e) {
      std::cout << "ERROR: " << e.what() << std::endl;
      return CMDERR_SUCCESS;
    } // catch

    Result res("FILE");
    Serialize s;
    s.add(contents);
    Row r(s.compile());
    res.add(r);

    Result::print(res, 0, sw.Time());

    return CMDERR_SUCCESS;
  } // stdinCAT::Execute
} // namespace openframe

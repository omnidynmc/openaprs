#include <list>
#include <fstream>
#include <queue>
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

#include <openframe/openframe.h>
#include <openframe/Command.h>
#include <openframe/stdinCommands.h>

namespace openframe {

/**************************************************************************
 ** stdinBINARY Class                                                    **
 **************************************************************************/

  const int stdinBINARY::Execute(COMMAND_ARGUMENTS) {
    Stopwatch sw;
    std::string str;

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    str = ePacket->args().getTrailing(0);

    sw.Start();

    stringstream out;
    string pad = "[ Binary Conversion ]";
    out << "<" << StringTool::ppad(pad, "-", 80) << std::endl;
    out << StringTool::hexdump(str, "<   ");
    out << "<" << std::endl;
    stringstream labels;
    stringstream values;
    labels << "<   ";
    values << "<   ";
    size_t pos;
    for(pos=0; pos < str.length(); pos++) {
      if (pos && (pos % 8) == 0) {
        out << labels.str() << std::endl
             << values.str() << std::endl;
        labels.str("");
        values.str("");
        labels << "<   ";
        values << "<   ";
      } // if

      labels << str[pos] << "        ";
      values << StringTool::binary((int) str[pos]) << " ";
    } // for

    if (pos && (pos % 8) != 0) {
        out << labels.str() << std::endl
             << values.str() << std::endl;
    } // if

    std::cout << out.str();

    return CMDERR_SUCCESS;
  } // stdinBINARY::Execute
} // namespace openframe

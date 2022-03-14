/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Dynamic Networking Solutions                      **
 **                                                                      **
 ** This program is free software; you can redistribute it and/or modify **
 ** it under the terms of the GNU General Public License as published by **
 ** the Free Software Foundation; either version 1, or (at your option)  **
 ** any later version.                                                   **
 **                                                                      **
 ** This program is distributed in the hope that it will be useful,      **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of       **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with this program; if not, write to the Free Software          **
 ** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            **
 **************************************************************************
 $Id: stdinSTATS.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
 **************************************************************************/

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

#include "Command.h"
#include "Server.h"
#include "Table.h"

#include "openframe/Stopwatch.h"

#include "stdinCommands.h"

#include "match.h"
#include "openaprs_string.h"

namespace openaprs {
  using openframe::Stopwatch;
  using namespace std;

/**************************************************************************
 ** stdinSTATS Class                                                     **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

const int stdinSTATS::Execute(COMMAND_ARGUMENTS) {
  Stopwatch sw;
  list<string> optionList;		// Option List
  string name = "";

  app->writeLog(OPENAPRS_LOG_DEBUG, "stdinSTATS::Execute> /STATS received.");

  if (ePacket->getArguments().size() > 0) {
    // initialize variables
    name = string(".")+ePacket->getArguments()[0];
  } // if

  app->cfg->tree("openaprs.count"+name, optionList);

/*
  cout << "--------------------------------------------------" << endl;
  cout << "  STATS" << endl;
  cout << "--------------------------------------------------" << endl;

  while(!optionList.empty()) {
    cout << "   " << optionList.front().c_str() << endl;
    optionList.pop_front();
  } // while

  cout << "--------------------------------------------------" << endl;
*/

  sw.Start();

  Table res("STATS");

  while(!optionList.empty()) {
    Serialize s;
    s.add("  "+optionList.front());
    Row r(s.compile());
    res.add(r);
    optionList.pop_front();
  } // while

  Table::print(res, sw.Time());

  return 1;
} // stdinSTATS::Execute

}

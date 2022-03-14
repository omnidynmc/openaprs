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
 ** stdinDTREE Class                                                     **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

const int stdinDTREE::Execute(COMMAND_ARGUMENTS) {
  Stopwatch sw;
  list<string> treeList;			// returned tree list

  app->writeLog(OPENAPRS_LOG_DEBUG, "stdinDTREE::Execute> /DTREE received.");

  app->showTree(ePacket->getPath(), 1, "", treeList);

/*
  cout << "--------------------------------------------------" << endl;
  cout << "  HELP" << endl;
  cout << "--------------------------------------------------" << endl;
  while(!treeList.empty()) {
    cout << treeList.front() << endl;
    treeList.pop_front();
  } // while
  cout << "--------------------------------------------------" << endl;
*/

  sw.Start();

  Table res("HELP");

  while(!treeList.empty()) {
    Serialize s;
    s.add("  "+treeList.front());
    Row r(s.compile());
    res.add(r);
    treeList.pop_front();
  } // while

  Table::print(res, sw.Time());


  return 1;
} // stdinDTREE::Execute

}

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
 $Id: stdinECHO.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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

#include "openframe/ConfController.h"
#include "openframe/Result.h"
#include "openframe/Serialize.h"
#include "openframe/Stopwatch.h"
#include "openframe/StringTool.h"

#include "stdinCommands.h"

#include "match.h"

namespace openaprs {
  using openframe::ConfController;
  using openframe::Row;
  using openframe::Serialize;
  using openframe::Stopwatch;
  using openframe::StringTool;
  using namespace std;

/**************************************************************************
 ** stdinSHOWCONFIG Class                                                **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

const int stdinSHOWCONFIG::Execute(COMMAND_ARGUMENTS) {
  openframe::Stopwatch sw;
  VarController::matchListType matchList;
  string name = "";
  string sep = "";

  StringTool::pad(sep, "-", 80);

//  app->writeLog(OPENAPRS_LOG_DEBUG, "stdinSHOWCONFIG::Execute> /SHOW CONFIG received.");

  if (ePacket->getArguments().size() > 0) {
    // initialize variables
    name = ePacket->getArguments()[0];
  } // if

  app->cfg->tree(name, matchList);

  sw.Start();

  openframe::Result res("CONFIG");

  while(!matchList.empty()) {
    Serialize s;
    s.add("  "+matchList.front());
    Row r(s.compile());
    res.add(r);
    matchList.pop_front();
  } // while

  openframe::Result::print(res, 80);

  return CMDERR_SUCCESS;
} // stdinSHOWCONFIG::Execute

}

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
 $Id: stdinHASH.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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

#include "Create.h"
#include "Command.h"
#include "Server.h"

#include "stdinCommands.h"

#include "match.h"
#include "openaprs_string.h"

namespace openaprs {

using namespace std;

/**************************************************************************
 ** stdinHASH Class                                                      **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

const int stdinHASH::Execute(COMMAND_ARGUMENTS) {
  aprs::Create c;
  short hash;
  string call;

  if (ePacket->getArguments().size() < 1)
    return CMDERR_SYNTAX;

  call = ePacket->getArguments().getTrailing(0);

  app->writeLog(OPENAPRS_LOG_DEBUG, "stdinHASH::Execute> /HASH received.");

  if (c.Hash(call, hash))
    cout << "Passcode hash: " << hash << endl;
  else
    cout << "*** Invalid callsign: " << call << endl;

  return 1;
} // stdinHASH::Execute

}

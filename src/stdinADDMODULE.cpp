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
 $Id: stdinADDMODULE.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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

#include "stdinCommands.h"

#include "match.h"
#include "openaprs_string.h"

namespace openaprs {

using namespace std;

/**************************************************************************
 ** stdinADDMODULE Class                                                 **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

const int stdinADDMODULE::Execute(COMMAND_ARGUMENTS) {
  string name;

  app->writeLog(OPENAPRS_LOG_DEBUG, "stdinADDMODULE::Execute> /ADD MODULE received.");

  if (ePacket->getArguments().size() < 1)
    return CMDERR_SYNTAX;

  // initialize variables
  name = ePacket->getArguments()[0];

    if (app->module->add(name, true)) {
      cout << "*** Module: " << name << " loaded." << endl;
    } // if
    else
      cout << "*** Module: " << name << " failed to load; " << app->module->getError() << endl;

  return 1;
} // stdinADDMODULE::Execute

}

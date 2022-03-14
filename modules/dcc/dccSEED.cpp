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
 $Id: dccCREATEPOSITION.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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

#include "APRS.h"
#include "DCC.h"
#include "DCCUser.h"
#include "StringToken.h"
#include "Command.h"
#include "Server.h"
#include "Vars.h"

#include "dccCommands.h"

#include "match.h"
#include "openaprs_string.h"
#include "md5wrapper.h"

namespace dcc {

using namespace std;
using namespace openaprs;

  /**************************************************************************
   ** dccSEED Class                                                        **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  /**
   *
   *
   */
  const int dccSEED::Execute(COMMAND_ARGUMENTS) {
    DCCUser *dccuser;			// pointer to a user
    map<int, regexMatch> regexList;
    int r;
    string parseMe;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");

    dccuser->sendf("110 SD:%s\n", dccuser->seed().c_str());

    return CMDERR_SUCCESS;
  } // dccSIGNUP::Execute
} // namespace dcc

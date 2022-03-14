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
 $Id: dccKILLOBJECT.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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

namespace dcc {

using namespace std;
using namespace openaprs;

  /**************************************************************************
   ** dccKILLOBJECT Class                                                  **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const int dccKILLOBJECT::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DCCUser *dccuser;			// pointer to a user
    string objectName, parseMe;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    if (IsDCCUserLogin(dccuser) == false)
      return CMDERR_ACCESS;

    if (IsDCCUserGuest(dccuser)) {
      dccuser->reply("errAccessGuest");
      return CMDERR_SUCCESS;
    } // if

    if (!IsDCCUserVerified(dccuser)) {
      dccuser->reply("errLoginVerify");
      return CMDERR_SUCCESS;
    } // if

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    // initialize variables
    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    if (!v.isName("nm")) {
      dccuser->reply("errMissingObjectName");
      return CMDERR_SUCCESS;
    } // if

    objectName = v["nm"];

    if (!dbi->doKillObject(dccuser->callsign(), objectName)) {
      dccuser->reply("errObjectNotFound");
      return CMDERR_SUCCESS;
    } // if

    dccuser->reply("replyObjectKilled");

    return 1;
  } // dccCREATEOBJECT::Execute
} // namespace dcc

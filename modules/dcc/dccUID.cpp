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
   ** dccUID Class                                                         **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  /**
   *
   *
   * UID:		iPhone UID
   */
  const int dccUID::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DCCUser *dccuser;			// pointer to a user
    map<int, regexMatch> regexList;
    string parseMe;
    stringstream s;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    // We don't have to be logged in or verified to issue a signup command.
    //if (IsDCCUserLogin(dccuser) == false)
    //  return CMDERR_ACCESS;

    //if (!IsDCCUserVerified(dccuser)) {
    //  dccuser->reply("errLoginVerify");
    //  return CMDERR_SUCCESS;
    //} // if

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    // initialize variables
    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    if (!v.isName("ID")) {
      dccuser->reply("errMissingUid");
      return CMDERR_SUCCESS;
    } // if

    if (dbi->signup->isBanned(v["ID"])) {
      dccuser->reply("errBanned");
      dccuser->Kill("You are banned from using this service.");
      app->writeLog(MODULE_DCC_LOG_NORMAL, "Killed banned user with uid[%s] ip[%s]", v["ID"].c_str(), dccuser->ip().c_str());
      return CMDERR_SUCCESS;
    } // if

    dccuser->reply("replyUidOk");

    return 1;
  } // dccUID::Execute
} // namespace dcc

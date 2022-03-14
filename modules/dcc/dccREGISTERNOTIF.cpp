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
 $Id: dccRELOADMODULE.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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
#include "Send.h"
#include "Command.h"
#include "Server.h"
#include "Vars.h"

#include "openframe/Stopwatch.h"
#include "openframe/StringToken.h"

#include "dccCommands.h"

#include "dcc.h"
#include "match.h"
#include "openaprs.h"
#include "openaprs_string.h"

namespace dcc {
  using namespace std;
  using namespace openaprs;
  using namespace aprs;
  using openframe::Stopwatch;
  using openframe::StringToken;

  /**************************************************************************
   ** dccREGISTERNOTIF Class                                               **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const int dccREGISTERNOTIF::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DCCUser *dccuser;                                     // pointer to a user
    bool isOK;
    map<int, regexMatch> rl;
    string parseMe;
    string callsign;
    string uid;
    string device_token;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    if (IsDCCUserLogin(dccuser) == false)
      return CMDERR_ACCESS;

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    // initialize variables
    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    if (!v.isName("cl")) {
      dccuser->reply("errMissingCallsign");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("uid")) {
      dccuser->reply("errMissingUid");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("dt")) {
      dccuser->reply("errMissingDeviceToken");
      return CMDERR_SUCCESS;
    } // if

    // initialize variables
    callsign = v["cl"];
    uid = v["uid"];
    device_token = v["dt"];

    if (ereg("^([a-zA-Z0-9-]{2,9})$", callsign, rl) < 1) {
      dccuser->reply("errInvalidCallsign");
      return CMDERR_SUCCESS;
    } // if

    if (APRS::extractCallsign(strtoupper(callsign)) != strtoupper(dccuser->callsign())) {
      dccuser->reply("errInvalidSSID");
      return CMDERR_SUCCESS;
    } // if

    if (ereg("^([a-z0-9]+)$", device_token, rl) < 1) {
      dccuser->reply("errInvalidDeviceToken");
      return CMDERR_SUCCESS;
    } // if

    if (ereg("^([a-zA-Z0-9]+)$", uid, rl) < 1) {
      dccuser->reply("errInvalidUid");
      return CMDERR_SUCCESS;
    } // if

    isOK = dbi->signup->RegisterForNotifications(dccuser->id(), callsign, uid, device_token);

    app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (RN) %s registered %s for notifications with %s (%s)",
                      dccuser->callsign().c_str(),
                      callsign.c_str(),
                      device_token.c_str(),
                      (isOK) ? "yes" : "no");

    dccuser->setMessageSessionInMemcached(callsign);
    dccuser->reply("replyRegisterOk");

    return CMDERR_SUCCESS;
  } // dccREGISTERNOTIF::Execute
} // namespace dcc

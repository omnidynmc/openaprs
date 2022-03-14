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
 $Id: dccMESSAGING.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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
#include "StringToken.h"
#include "Command.h"
#include "Server.h"
#include "Vars.h"

#include "dccCommands.h"

#include "dcc.h"
#include "match.h"
#include "openaprs_string.h"

namespace dcc {

using namespace std;
using namespace openaprs;
using namespace aprs;

  /**************************************************************************
   ** dccMESSAGING Class                                                   **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const int dccMESSAGING::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DCCUser *dccuser;                                     // pointer to a user
    bool isOn;
    map<int, regexMatch> regexList;
    string parseMe;
    string source;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    if (IsDCCUserLogin(dccuser) == false)
      return CMDERR_ACCESS;

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    if (v.isName("on")) {
      if (strtoupper(v["on"]) == "YES")
        isOn = true;
      else if (strtoupper(v["on"]) == "NO")
        isOn = false;
      else {
        dccuser->reply("errInvalidOn");
        return CMDERR_SUCCESS;
      } // else
    } // if
    else {
      dccuser->reply("errMissingOn");
      return CMDERR_SUCCESS;
    } // else

    source = strtoupper(dccuser->callsign());
    if (v.isName("sr")) {
      if (APRS::extractCallsign(strtoupper(v["sr"])) != strtoupper(dccuser->callsign())) {
        dccuser->reply("errInvalidSSID");
        return CMDERR_SUCCESS;
      } // if

      source = strtoupper(v["sr"]);
    } // if

    if (isOn) {
      if (IsDCCUserMessaging(dccuser)) {
        dccuser->reply("errAlreadyOn");
        return CMDERR_SUCCESS;
      } // if

      SetDCCUserMessaging(dccuser);
      dbi->registerMessageSession(dccuser->id(), source);
      dccuser->setMessageSessionInMemcached(source);
      dccuser->reply("replyMessagingOn");
      dccuser->var()->replace_string("dcc.user.messaging.nick", source);
      app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (MS) Messaging set to `ON' for %s", source.c_str());
    } // if
    else {
      if (!IsDCCUserMessaging(dccuser)) {
        dccuser->reply("errAlreadyOff");
        return CMDERR_SUCCESS;
      } // if

      UnsetDCCUserMessaging(dccuser);
      source = dccuser->var()->get_string("dcc.user.messaging.nick");
      dbi->unregisterMessageSession(dccuser->id(), source);
      dccuser->reply("replyMessagingOff");
      app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (MS) Messaging set to `OFF' for %s", source.c_str());
      dccuser->var()->pop("dcc.user.messaging.nick");
    } // else

    return CMDERR_SUCCESS;
  } // dccMESSAGING::Execute
} // namespace dcc

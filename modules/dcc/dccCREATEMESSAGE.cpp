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
 $Id: dccCREATEMESSAGE.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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

#include "match.h"
#include "openaprs_string.h"

namespace dcc {

using namespace std;
using namespace openaprs;
using namespace aprs;

  /**************************************************************************
   ** dccCREATEMESSAGE Class                                               **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const int dccCREATEMESSAGE::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DCCUser *dccuser;                                     // pointer to a user
    bool local = false;
    map<int, regexMatch> regexList;               // Map of regex matches.
    string message, parseMe;
    string to, from;
    unsigned int numMessage = 0;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");
    from = dccuser->callsign();

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

    if (IsDCCUserLocal(dccuser))
      local = true;

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    if (!v.isName("to")) {
      dccuser->reply("errMissingTo");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("ms")) {
      dccuser->reply("errMissingMessage");
      return CMDERR_SUCCESS;
    } // if

    // initialize variables
    to = strtoupper(v["to"]);
    message = v["ms"];

    if (!APRS::isValidCallsign(to)) {
      dccuser->reply("errInvalidCallsign");
      return CMDERR_SUCCESS;
    } // if

    if (!APRS::isValidMessage(message)) {
      dccuser->reply("errInvalidMessage");
      return CMDERR_SUCCESS;
    } // if

    if (v.isName("sr")) {
      if (APRS::extractCallsign(strtoupper(v["sr"])) != strtoupper(dccuser->callsign())) {
        dccuser->reply("errInvalidSSID");
        return CMDERR_SUCCESS;
      } // if

      from = strtoupper(v["sr"]);
    } // if

    // Register this user as having messaging support.
    //SetDCCUserMessaging(dccuser);
    //dbi->registerMessageSession(dccuser->id(), dccuser->callsign());

    app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: %s created message for %s: %s", from.c_str(), to.c_str(), message.c_str());

    if (dbi->incLastMessageID(dccuser->id(), from))
      numMessage = dbi->getLastMessageID(dccuser->id(), from);

    if (dbi->createMessage(from, to, message, numMessage, local)) {
      dccuser->datapoint("num.create.message");
      dccuser->reply("replyMessageSent");
    } // if
    else
      dccuser->reply("errInvalidMessage");

    return CMDERR_SUCCESS;
  } // dccCREATEMESSAGE::Execute
} // namespace dcc

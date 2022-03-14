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
 $Id: dccCHECKMESSAGES.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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
   ** dccCHECKMESSAGES Class                                               **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const int dccCHECKMESSAGES::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DBI::setMapType messages;
    DBI::resultMapType r;
    DCCUser *dccuser;                                     // pointer to a user
    Vars v;
    bool isOk;
    bool toAll;
    bool isSent;
    string parseMe;
    string to;
    unsigned int numMessages;
    unsigned int i;
    time_t past_ts;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    if (IsDCCUserLogin(dccuser) == false)
      return CMDERR_ACCESS;

    // initialize variables
    toAll = false;
    to = dccuser->callsign();

    if (ePacket->getArguments().size() > 0) {
      parseMe = ePacket->getArguments().getTrailing(0);

      Vars v(parseMe);

      if (v.isName("ag"))
        past_ts = time_t(atoi(v["ag"].c_str()));

      if (v.isName("tg") && v["tg"] == "ALL")
        toAll = true;

      if (v.isName("sr")) {
        if (APRS::extractCallsign(strtoupper(v["sr"])) != dccuser->callsign()) {
          dccuser->reply("errInvalidSSID");
          return CMDERR_SUCCESS;
        } // if

        to = v["sr"];
      } // if
    } // if
    else
      past_ts = 0;

    dccuser->reply("replyOk");

    // Register user as having messaging support.
    //SetDCCUserMessaging(dccuser);
    if (strtoupper(to) != "GUEST") {
      dbi->registerMessageSession(dccuser->id(), to);
      dccuser->setMessageSessionInMemcached(to);
      app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: Registering %s (%s) for messages.", to.c_str(), dccuser->id().c_str());
    } // if

    numMessages = dbi->getMessagesByCallsign(to, (toAll) ? DBI::ALL : DBI::TO, past_ts, messages);

    for(i=0; i < messages.size(); i++) {
      r = messages[i].resultMap;
      if (r["source"] == to)
        isSent = true;
      else
        isSent = false;

      dccuser->sendf("300 CT:%s|SR:%s|AD:%s|MS:%s|SN:%s\n",
                  v.Escape(r["create_ts"]).c_str(),
                  v.Escape(r["source"]).c_str(),
                  v.Escape(r["addressee"]).c_str(),
                  v.Escape(r["text"]).c_str(),
                  (isSent) ? "yes" : "no");
    } // for

    dccuser->sendf("301 CL:%s|RS:%d|MS:%d messages returned.\n", to.c_str(), numMessages, numMessages);

    dccuser->datapoint("num.op.check.messages");
    return CMDERR_SUCCESS;
  } // dccCHECKMESSAGES::Execute
} // namespace dcc

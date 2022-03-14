/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** HAL9000, Internet Relay Chat Bot                                     **
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
 $Id: eventMESSAGETOVERIFY.cpp,v 1.2 2003/08/30 03:59:00 omni Exp $
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
#include "DBI.h"
#include "Command.h"
#include "Create.h"
#include "Send.h"
#include "Server.h"

#include "openframe/StringTool.h"

#include "eventCommands.h"

#include "openaprs.h"
#include "openaprs_string.h"

namespace events {
  using openframe::StringTool;
  using namespace handler;
  using namespace std;
  using namespace aprs;
  using aprs::Send;
  using aprs::Create;

/**************************************************************************
 ** eventMESSAGETOVERIFY Class                                           **
 **************************************************************************/

  /*********************
   ** OnEvent Members **
   *********************/

  const int eventMESSAGETOVERIFY::OnEvent(Event *eventInfo, const string &eventMatch, Packet *ePacket) {
    APRS *aprs = (APRS *) ePacket->getVariable("aprs");
    DBI *dbi = (DBI *) ePacket->getVariable("dbi");
    aprs::Create c;
    Send sn;
    stringstream s;			// Generic string stream.
    string aprsMessage;			// Message of aprs string
    string verifyKey;
    string verifyId;
    string verifySource;
    string reply;
    StringTool::regexMatchListType regexList;	// List of regex matches returned.
    int sendReply;

    /**
     * MySQL Injection
     *
     * If SQL is enabled, go ahead and proceed with this section
     * otherwise return here.
     *
     */

    if (!IsServerSQL || !IsServerSQLInject)
      return CMDERR_SUCCESS;

    // Is verify mode enabled?
    if (!app->cfg->get_bool("openaprs.verify.enable", false))
      return CMDERR_SUCCESS;

    // We don't want to ack unless the message was sent to me.
    if (aprs->getString("aprs.packet.message.target") !=
        app->cfg->get_string("openaprs.message.callsign", ""))
      return CMDERR_SUCCESS;

    // Ignore self messages.  Technically this shouldn't happen
    // but we'll be on the safe side.
    if (aprs->getString("aprs.packet.source") ==
        app->cfg->get_string("openaprs.message.callsign", ""))
      return CMDERR_SUCCESS;

    sn.logger(new App_Log(""));

    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Received message '%s' from %s", 
                                           aprs->getString("aprs.packet.message.text").c_str(),
                                           aprs->getString("aprs.packet.source").c_str());

    // Only process RF verifications, ignore anything from TCPIP only.
    if (StringTool::ereg("TCP(IP|XX)",
                              aprs->getString("aprs.packet.path"),
                              regexList) > 0)
      return CMDERR_SUCCESS;

    // Does this packet have a valid verification message?
    if (StringTool::ereg("^(K([0-9]+)([A-Z0-9]{8}))[ ]*$",
                              strtoupper(aprs->getString("aprs.packet.message.text")),
                              regexList) < 1)
      return CMDERR_SUCCESS;

    verifyId = regexList[2].matchString;
    verifyKey = regexList[3].matchString;
    verifySource = aprs->getString("aprs.packet.source.callsign");
    aprs->addString("aprs.packet.message.verify.id", verifyId);
    aprs->addString("aprs.packet.message.verify.key", verifyKey);
    aprs->addString("aprs.packet.message.verify.source", verifySource);

    sendReply = dbi->tryVerify(verifySource, verifyId, verifyKey, reply);

    if (sendReply)
      dbi->createMessage(OPENAPRS_CONFIG_CREATE, aprs->getString("aprs.packet.source"), reply, false);

    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Verify: %s@id(%s) using '%s' %s",
                                           verifySource.c_str(),
                                           verifyId.c_str(),
                                           verifyKey.c_str(),
                                           reply.c_str());

    app->writeLog(OPENAPRS_LOG_VERIFY, "*** Verify: %s@id(%s) using '%s' %s",
                                           verifySource.c_str(),
                                           verifyId.c_str(),
                                           verifyKey.c_str(),
                                           reply.c_str());

    return CMDERR_SUCCESS;
  } // eventMESSAGETOVERIFY::OnEvent
} // namespace commands

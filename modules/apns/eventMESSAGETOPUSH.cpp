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
 $Id: eventMESSAGETOPUSH.cpp,v 1.2 2003/08/30 03:59:00 omni Exp $
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

#include "APNS.h"
#include "APRS.h"
#include "ApnsMessage.h"
#include "Command.h"
#include "DBI.h"
#include "Server.h"
#include "StringToken.h"

#include "apnsCommands.h"

#include "apns.h"
#include "openaprs.h"
#include "openaprs_string.h"

namespace apns {
  using namespace aprs;
  using namespace handler;
  using namespace std;

/**************************************************************************
 ** eventMESSAGETOPUSH Class                                             **
 **************************************************************************/

  /*********************
   ** OnEvent Members **
   *********************/

  const int eventMESSAGETOPUSH::OnEvent(Event *eventInfo, const string &eventMatch, Packet *ePacket) {
    APRS *aprs = (APRS *) ePacket->getVariable("aprs");
    DBI *dbi = (DBI *) ePacket->getVariable("dbi");
    ApnsMessage *aMessage;
    Stopwatch sw;
    DBI::setMapType sm;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    bool isOK;
    double avg;
    string message;
    string target;
    string source;
    stringstream s;
    unsigned int i;

    // Don't send ACK only messages.
    if (aprs->isString("aprs.packet.message.ackonly"))
      return CMDERR_SUCCESS;

    // Basically we want to see if the target is
    // a user in DCC.  If they are then we want
    // to send the user the message.
    source = aprs->getString("aprs.packet.source");
    target = aprs->getString("aprs.packet.message.target");
    message = aprs->getString("aprs.packet.message.text");

    sw.Start();
    s.str("");

    s << "SELECT apns_register.id, apns_register.device_token, apns_register.environment FROM apns_register LEFT JOIN web_users ON web_users.id = apns_register.user_id WHERE "
      << "apns_register.callsign='" << Sql->Escape(target) << "' "
      << "AND apns_register.active='Y' "
      << "AND web_users.active='Y'";

    isOK = dbi->fetchRows(s.str(), sm);

    avg = aAPNS->profile.average("module.apns.push", sw.Time());
    app->cfg->replace_double("openaprs.count.time.average.module.apns.push", avg);

    if (isOK == false)
      return CMDERR_SUCCESS;

    if (sm.size() < 1)
      return CMDERR_SUCCESS;

    for(i=0; i < sm.size(); i++) {
      rm = sm[i].resultMap;

      s.str("");
      s << source << ": " << message;

      app->Query("INSERT INTO apns_push (apns_register_id, badge, alertmsg, sent, create_ts) VALUES ('%s', '1', '%s', 'Y', '%d')",
                 rm["id"].c_str(), Sql->Escape(s.str()).c_str(), time(NULL));

      try {
        aMessage = new ApnsMessage(rm["device_token"]);
      } // try
      catch (ApnsMessage_Exception e) {
        app->writeLog(MODULE_APNS_LOG_NORMAL, "WARNING: Failed to create new APNS message to (%s): <%s> %s", target.c_str(), source.c_str(), e.message());
        continue;
      } // catch

      aMessage->text(s.str());
      aMessage->actionKeyCaption("View");
      aMessage->badgeNumber(1);

      if (rm["environment"] == "prod")
        aMessage->environment(ApnsMessage::APNS_ENVIRONMENT_PROD);

      aAPNS->Push(aMessage);

      app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Queuing APNS to %s: <%s> %s", target.c_str(), source.c_str(), message.c_str());
    } // for

    app->writeLog(MODULE_APNS_LOG_NORMAL, "STATUS: Queued %d rows to APNS in (%.4f seconds, %.4f 5min)", sm.size(), sw.Time(), avg);

    return CMDERR_SUCCESS;
  } // eventMESSAGETOPUSH::OnEvent
} // namespace commands

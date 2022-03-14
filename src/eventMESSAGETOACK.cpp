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
 $Id: eventMESSAGETOACK.cpp,v 1.2 2003/08/30 03:59:00 omni Exp $
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
#include "Command.h"
#include "Create.h"
#include "DBI.h"
#include "Send.h"
#include "Server.h"
#include "StringTool.h"
#include "Worker.h"

#include "eventCommands.h"

#include "openaprs.h"
#include "openaprs_string.h"

namespace events {
  using namespace handler;
  using namespace std;
  using namespace aprs;
  using aprs::Send;
  using aprs::Create;
  using aprs::StringTool;

/**************************************************************************
 ** eventMESSAGETOACK Class                                              **
 **************************************************************************/

  /*********************
   ** OnEvent Members **
   *********************/

  const int eventMESSAGETOACK::OnEvent(Event *eventInfo, const string &eventMatch, Packet *ePacket) {
    APRS *aprs = (APRS *) ePacket->getVariable("aprs");
    Result *result = (Result *) ePacket->getVariable("result");
    DBI *dbi = (DBI *) ePacket->getVariable("dbi");
    aprs::Create c;
    Send sn;
    stringstream s;			// Generic string stream.
    string message;
    string decayId;
    string target;
    string source;
    string ack;

    sn.logger(new App_Log(""));

    /**
     * MySQL Injection
     *
     * If SQL is enabled, go ahead and proceed with this section
     * otherwise return here.
     *
     */

    if (!IsServerSQL || !IsServerSQLInject)
      return CMDERR_SUCCESS;

    // Is ack enabled?
    if (!app->cfg->get_bool("openaprs.message.ack", false))
      return CMDERR_SUCCESS;

    target = StringTool::toUpper(aprs->getString("aprs.packet.message.target"));
    source = StringTool::toUpper(aprs->getString("aprs.packet.source"));

    // send in ack if enabled and message was to me
    if (target == CFG_STRING("openaprs.message.callsign", "")) {
      if (aprs->isString("aprs.packet.message.id") || aprs->isString("aprs.packet.message.ackonly"))
        return CMDERR_SUCCESS;
      // send an ack if support is enabled and has a msg id
      app->writeLog(OPENAPRS_LOG_APRS, "*** Sending ack(%s) for '%s' from '%s'",
                          aprs->getString("aprs.packet.message.id").c_str(),
                          aprs->getString("aprs.packet.source").c_str(),
                          (OPENAPRS_CONFIG_CREATE).c_str()
                         );

      s.str("");
      s << "ack" << aprs->getString("aprs.packet.message.id");
      sn.Message(OPENAPRS_CONFIG_CREATE, source, s.str(), false, message);
      app->enqueueDispatch(new ThreadMessage("server.send", message));

      return CMDERR_SUCCESS;
    } // if

    if (!result->ack())
      return CMDERR_SUCCESS;

    // If the message has an id and we have a current user session
    // on the website send an ack on behalf of the user.
    if (aprs->isString("aprs.packet.message.id")) {
      // First find out if we have a user session for
      // the target user.  If we don't then don't bother to
      // go any further.
      s.str("");
      s << "ack" << aprs->getString("aprs.packet.message.id.reply");
      sn.Message(target, source, s.str(), false, message);
      app->enqueueDispatch(new ThreadMessage("server.send", message));
    } // if

    // If this is an ack from a web user session message then we
    // need to store it and stop retrying that users message.
    if (aprs->isString("aprs.packet.message.ack")) {
      ack = aprs->getString("aprs.packet.message.ack");

      // remove any decay for this message
      if (dbi->getMessageDecayID(target, source, ack, decayId))
        app->removeDecayById(decayId);

      if (dbi->setMessageAck(source, target, ack) )
          app->writeLog(OPENAPRS_LOG_APRS, "*** Received ack(%s) for '%s' from '%s'",
                                             ack.c_str(),
                                             target.c_str(),
                                             source.c_str());
    } // if

    return CMDERR_SUCCESS;
  } // eventMESSAGETOACK::OnEvent
} // namespace commands

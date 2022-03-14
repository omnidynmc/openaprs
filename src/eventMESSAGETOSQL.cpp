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
 $Id: eventMESSAGETOSQL.cpp,v 1.2 2003/08/30 03:59:00 omni Exp $
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
#include "Insert.h"
#include "Server.h"

#include "eventCommands.h"

#include "openaprs.h"
#include "openaprs_string.h"

namespace events {
  using namespace handler;
  using namespace std;
  using namespace aprs;

/**************************************************************************
 ** eventMESSAGETOSQL Class                                              **
 **************************************************************************/

  /*********************
   ** OnEvent Members **
   *********************/

  const int eventMESSAGETOSQL::OnEvent(Event *eventInfo, const string &eventMatch, Packet *ePacket) {
    APRS *aprs = (APRS *) ePacket->getVariable("aprs");
    DBI *dbi = (DBI *) ePacket->getVariable("dbi");
    Insert *sqlq = (Insert *) ePacket->getVariable("sqlq");
    map<int, regexMatch> regexList;		// List of regex matches returned.

    if (app->isMonitor(aprs->getString("aprs.packet.source")))
      app->writeLog(OPENAPRS_LOG_MONITOR, " +-- Message");

    /**
     * MySQL Injection
     *
     * If SQL is enabled, go ahead and proceed with this section
     * otherwise return here.
     *
     */

  if (!IsServerSQL || !IsServerSQLInject)
    return 1;

    // don't inject if this is an ack only
    if (aprs->isString("aprs.packet.message.ackonly"))
      return 1;

    sqlq->Add("messages", "('%s', '%s', '%s', %s, %s, %s, '%d')",
               aprs->getString("aprs.packet.id").c_str(),
               aprs->getString("aprs.packet.callsign.id").c_str(),
               aprs->getString("aprs.current.date").c_str(),
               NULL_OPTION(aprs, "aprs.packet.message.target"),
               NULL_STRING(dbi->handle()->Escape(aprs->getString("aprs.packet.message.text"))),
               NULL_STRING(dbi->handle()->Escape(aprs->getString("aprs.packet.message.id"))),
               time(NULL)
               );

    sqlq->Add("last_message", "('%s', '%s', %s, %s, %s, '%d')",
               aprs->getString("aprs.packet.id").c_str(),
               aprs->getString("aprs.packet.callsign.id").c_str(),
               NULL_OPTION(aprs, "aprs.packet.message.target"),
               NULL_STRING(dbi->handle()->Escape(aprs->getString("aprs.packet.message.text"))),
               NULL_STRING(dbi->handle()->Escape(aprs->getString("aprs.packet.message.id"))),
               time(NULL)
               );

    if (openaprs_string_regex("^((BLN[0-9A-Z]{1,6})|(NWS-[0-9A-Z]{1,5}))$",
                              aprs->getString("aprs.packet.message.target"), 
                              regexList)) {
      sqlq->Add("last_bulletin", "('%s', '%s', %s, %s, %s, '%d')",
                 aprs->getString("aprs.packet.id").c_str(),
                 aprs->getString("aprs.packet.callsign.id").c_str(),
                 NULL_OPTION(aprs, "aprs.packet.message.target"),
                 NULL_STRING(dbi->handle()->Escape(aprs->getString("aprs.packet.message.text"))),
                 NULL_STRING(dbi->handle()->Escape(aprs->getString("aprs.packet.message.id"))),
                 time(NULL)
                 );
    } // if

    // If there is no telemetry data in message
    // stop here otherwise try and inject it
    if (aprs->isString("aprs.packet.telemetry.message.type") == false)
      return CMDERR_SUCCESS;

    if (aprs->getString("aprs.packet.telemetry.message.type") == "EQNS") {
      sqlq->Add("telemetry_eqns", "('%s', '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, '%d')",
                 aprs->getString("aprs.packet.id").c_str(),
                 aprs->getString("aprs.packet.callsign.id").c_str(),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a0.a"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a0.b"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a0.c"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a1.a"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a1.b"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a1.c"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a2.a"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a2.b"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a2.c"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a3.a"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a3.b"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a3.c"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a4.a"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a4.b"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.a4.c"),
                 time(NULL)
                 );
    } // if
    else if (aprs->getString("aprs.packet.telemetry.message.type") == "UNIT") {
      sqlq->Add("telemetry_unit", "('%s', '%s',  %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, '%d')",
                 aprs->getString("aprs.packet.id").c_str(),
                 aprs->getString("aprs.packet.callsign.id").c_str(),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.analog0"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.analog1"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.analog2"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.analog3"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.analog4"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital0"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital1"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital2"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital3"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital4"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital5"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital6"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital7"),
                 time(NULL)
                 );
    } // else if
    else if (aprs->getString("aprs.packet.telemetry.message.type") == "PARM") {
      sqlq->Add("telemetry_parm", "('%s', '%s',  %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, '%d')",
                 aprs->getString("aprs.packet.id").c_str(),
                 aprs->getString("aprs.packet.callsign.id").c_str(),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.analog0"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.analog1"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.analog2"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.analog3"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.analog4"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital0"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital1"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital2"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital3"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital4"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital5"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital6"),
                 NULL_ESCAPE(aprs, "aprs.packet.telemetry.digital7"),
                 time(NULL)
                 );
    } // else if
    else if (aprs->getString("aprs.packet.telemetry.message.type") == "BITS") {
      sqlq->Add("telemetry_bits", "('%s', '%s',  '%s', '%s','%d')",
                 aprs->getString("aprs.packet.id").c_str(),
                 aprs->getString("aprs.packet.callsign.id").c_str(),
                 aprs->getString("aprs.packet.telemetry.bitsense").c_str(),
                 dbi->handle()->Escape(aprs->getString("aprs.packet.telemetry.project")).c_str(),
                 time(NULL)
                 );
    } // else if

    return CMDERR_SUCCESS;
  } // eventMESSAGETOSQL::OnEvent
} // namespace commands

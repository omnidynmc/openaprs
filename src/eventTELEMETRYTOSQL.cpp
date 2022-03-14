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
 $Id: eventTELEMETRYTOSQL.cpp,v 1.2 2003/08/30 03:59:00 omni Exp $
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
#include "Insert.h"
#include "Server.h"

#include "eventCommands.h"

#include "openaprs.h"
#include "openaprs_string.h"

namespace events {
  using namespace aprs;
  using namespace handler;
  using namespace std;

/**************************************************************************
 ** eventPOSITIONTOSQL Class                                             **
 **************************************************************************/

  /*********************
   ** OnEvent Members **
   *********************/

  const int eventTELEMETRYTOSQL::OnEvent(Event *eventInfo, const string &eventMatch, Packet *ePacket) {
    APRS *aprs = (APRS *) ePacket->getVariable("aprs");
    Insert *sqlq = (Insert *) ePacket->getVariable("sqlq");

    /**
     * MySQL Injection
     *
     * If SQL is enabled, go ahead and proceed with this section
     * otherwise return here.
     *
     */

    if (!IsServerSQL || !IsServerSQLInject)
      return 1;

    app->cfg->inc("openaprs.count.inserted.telemetry", 1);

    sqlq->Add("last_telemetry", "('%s', '%s', '%s', %s, %s, %s, %s, %s, %s, %s, %d)",
               aprs->getString("aprs.packet.id").c_str(),
               aprs->getString("aprs.packet.callsign.id").c_str(),
               aprs->getString("aprs.current.date").c_str(),
               NULL_OPTION(aprs, "aprs.packet.telemetry.sequence"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.analog0"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.analog1"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.analog2"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.analog3"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.analog4"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.digital"),
               time(NULL)
               );

    sqlq->Add("telemetry", "('%s', '%s', '%s', %s, %s, %s, %s, %s, %s, %s, %d)",
               aprs->getString("aprs.packet.id").c_str(),
               aprs->getString("aprs.packet.callsign.id").c_str(),
               aprs->getString("aprs.current.date").c_str(),
               NULL_OPTION(aprs, "aprs.packet.telemetry.sequence"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.analog0"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.analog1"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.analog2"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.analog3"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.analog4"),
               NULL_OPTION(aprs, "aprs.packet.telemetry.digital"),
               time(NULL)
               );

    return CMDERR_SUCCESS;
  } // eventTELEMETRYTOSQL::OnEvent
} // namespace commands

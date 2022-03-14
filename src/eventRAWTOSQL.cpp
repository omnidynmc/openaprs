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
 ** eventRAWTOSQL Class                                                  **
 **************************************************************************/

  /*********************
   ** OnEvent Members **
   *********************/

  const int eventRAWTOSQL::OnEvent(Event *eventInfo, const string &eventMatch, Packet *ePacket) {
    APRS *aprs = (APRS *) ePacket->getVariable("aprs");
    DBI *dbi = (DBI *) ePacket->getVariable("dbi");
    Insert *sqlq = (Insert *) ePacket->getVariable("sqlq");

    /**
     * MySQL Injection
     *
     * If SQL is enabled, go ahead and proceed with this section
     * otherwise return here.
     *
     */

    if (!IsServerSQL || !IsServerSQLInject)
      return CMDERR_SUCCESS;

    sqlq->Add("raw", "('%s', '%s', '%s', '%s', '%d')",
      aprs->getString("aprs.packet.id").c_str(),
      aprs->getString("aprs.packet.callsign.id").c_str(),
      aprs->getString("aprs.current.date").c_str(),
      dbi->handle()->Escape(aprs->getString("aprs.packet.raw")).c_str(),
      time(NULL));

    sqlq->Add("raw_meta", "('%s', '%s', '%d', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%d')",
      aprs->getString("aprs.packet.id").c_str(),
      aprs->getString("aprs.packet.callsign.id").c_str(),
      time(NULL),
      aprs->getString("aprs.current.date").c_str(),
      aprs->getString("aprs.packet.path0").c_str(),
      aprs->getString("aprs.packet.path1").c_str(),
      aprs->getString("aprs.packet.path2").c_str(),
      aprs->getString("aprs.packet.path3").c_str(),
      aprs->getString("aprs.packet.path4").c_str(),
      aprs->getString("aprs.packet.path5").c_str(),
      aprs->getString("aprs.packet.path6").c_str(),
      aprs->getString("aprs.packet.path7").c_str(),
      aprs->getString("aprs.packet.path8").c_str(),
      time(NULL));

    sqlq->Add("lastraw", "('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%d')",
      aprs->getString("aprs.packet.id").c_str(),
      aprs->getString("aprs.packet.callsign.id").c_str(),
      aprs->getString("aprs.packet.path0").c_str(),
      aprs->getString("aprs.packet.path1").c_str(),
      aprs->getString("aprs.packet.path2").c_str(),
      aprs->getString("aprs.packet.path3").c_str(),
      aprs->getString("aprs.packet.path4").c_str(),
      aprs->getString("aprs.packet.path5").c_str(),
      aprs->getString("aprs.packet.path6").c_str(),
      aprs->getString("aprs.packet.path7").c_str(),
      aprs->getString("aprs.packet.path8").c_str(),
      dbi->handle()->Escape(aprs->getString("aprs.packet.raw")).c_str(),
      time(NULL));

    if (aprs->isString("aprs.packet.error.message"))
      sqlq->Add("raw_errors", "('%s', '%s', CURRENT_TIMESTAMP, '%s', '%d')",
        aprs->getString("aprs.packet.id").c_str(),
        aprs->getString("aprs.packet.callsign.id").c_str(),
        dbi->handle()->Escape(aprs->getString("aprs.packet.error.message")).c_str(),
        time(NULL));

    return CMDERR_SUCCESS;
  } // eventRAWTOSQL::OnEvent
} // namespace commands

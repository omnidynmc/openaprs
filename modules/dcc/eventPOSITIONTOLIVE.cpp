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
 $Id: eventPOSITIONTOLIVE.cpp,v 1.2 2003/08/30 03:59:00 omni Exp $
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
#include "DCC.h"
#include "DCCUser.h"
#include "Send.h"
#include "Server.h"
#include "StringToken.h"
#include "Vars.h"
#include "Worker.h"

#include "dccCommands.h"

#include "openaprs.h"
#include "openaprs_string.h"

namespace dcc {

  using namespace handler;
  using namespace std;
  using namespace aprs;

/**************************************************************************
 ** eventPOSITIONTOLIVE Class                                            **
 **************************************************************************/

  /*********************
   ** OnEvent Members **
   *********************/

  const int eventPOSITIONTOLIVE::OnEvent(Event *eventInfo, const string &eventMatch, Packet *ePacket) {
    APRS *aprs = (APRS *) ePacket->getVariable("aprs");
    Packet::stringMapType::iterator ptr;
    Worker *worker = (Worker *) vars->get_void("worker");
    string field;
    stringstream s;

    // Process event strings to send to user.
    Vars v("");
    s.str("");
    for(ptr = aprs->stringBegin(); ptr != aprs->stringEnd(); ptr++) {
      if (!worker->fields()->findByEvent(ptr->first, field))
         continue;

      v.add(field, ptr->second);
    } // for

    // Set weather bit.
    if (aprs->isString("aprs.packet.weather"))
      v.add("WX", "Y");

    if (v.size() < 1)
      return CMDERR_SUCCESS;

    worker->sendToLive(v, "318");

    return CMDERR_SUCCESS;
  } // eventPOSITIONTOLIVE::OnEvent
} // namespace commands

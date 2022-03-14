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
 $Id: eventMESSAGETOSESSION.cpp,v 1.2 2003/08/30 03:59:00 omni Exp $
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
#include <pthread.h>

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

#include "dcc.h"
#include "openaprs.h"
#include "openaprs_string.h"

namespace dcc {

  using namespace handler;
  using namespace std;
  using namespace aprs;

/**************************************************************************
 ** eventMESSAGETOSESSION Class                                          **
 **************************************************************************/

  /*********************
   ** OnEvent Members **
   *********************/

  const int eventMESSAGETOSESSION::OnEvent(Event *eventInfo, const string &eventMatch, Packet *ePacket) {
    APRS *aprs = (APRS *) ePacket->getVariable("aprs");
    DCC::threadSetType threadSet;
    Worker *worker = (Worker *) vars->get_void("worker");
    Vars v;
    string message;
    string target;
    string source;

    // Don't send ACK only messages.
    if (aprs->isString("aprs.packet.message.ackonly"))
      return CMDERR_SUCCESS;

    // Basically we want to see if the target is
    // a user in DCC.  If they are then we want
    // to send the user the message.
    source = aprs->getString("aprs.packet.source");
    target = aprs->getString("aprs.packet.message.target");
    message = aprs->getString("aprs.packet.message.text");

    worker->sendTo(
                    target,
                    StringTool::stringf(1024, "300 CT:%d|SR:%s|MS:%s\n",time(NULL), source.c_str(), message.c_str() )
                   );

    return CMDERR_SUCCESS;
  } // eventMESSAGETOSESSION::OnEvent
} // namespace commands

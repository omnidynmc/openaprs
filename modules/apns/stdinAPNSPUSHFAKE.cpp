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

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "APNS.h"
#include "ApnsMessage.h"
#include "StringToken.h"
#include "Command.h"
#include "Server.h"
#include "Vars.h"

#include "apnsCommands.h"

#include "apns.h"
#include "match.h"
#include "openaprs_string.h"

namespace apns {

using namespace std;
using namespace openaprs;

/**************************************************************************
 ** stdinAPNSPUSHFAKE Class                                              **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

const int stdinAPNSPUSHFAKE::Execute(COMMAND_ARGUMENTS) {
  Vars v;
  ApnsMessage *aMessage;
  string deviceToken;
  string message;
  int r;

  app->writeLog(OPENAPRS_LOG_DEBUG, "stdinAPNSPUSH::Execute> /APNS PUSH received.");

  if (ePacket->getArguments().size() < 1)
    return CMDERR_SYNTAX;

  message = ePacket->getArguments().getTrailing(0);

  for(int i=0; i < 64/2; i++) {
    r = rand() % 255;
    deviceToken += strtolower(v.char2hex(r));
  } // for

  try {
    aMessage = new ApnsMessage(deviceToken);
  } // try
  catch (ApnsMessage_Exception e) {
    printf("*** Failed to create new APNS message to (%s) %s: %s\n", deviceToken.c_str(), message.c_str(), e.message());
    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Failed to create new APNS message to (%s) %s: %s", deviceToken.c_str(), message.c_str(), e.message());
    return CMDERR_SUCCESS;
  } // catch

  aMessage->text(message);
  aMessage->actionKeyCaption("View");
  aMessage->badgeNumber(1);

  aAPNS->Push(aMessage);

  printf("*** Queuing FAKE APNS to (%s) %s\n", deviceToken.c_str(), message.c_str());
  app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Queuing FAKE APNS to (%s) %s", deviceToken.c_str(), message.c_str());

  return CMDERR_SUCCESS;
} // stdinAPNSPUSHFAKE::Execute

}

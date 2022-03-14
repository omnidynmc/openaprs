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

#include "apnsCommands.h"

#include "apns.h"
#include "match.h"
#include "openaprs_string.h"

namespace apns {

using namespace std;
using namespace openaprs;

/**************************************************************************
 ** stdinAPNSPUSH Class                                                  **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

const int stdinAPNSPUSH::Execute(COMMAND_ARGUMENTS) {
  ApnsMessage *aMessage;
  bool isOK;
  string deviceToken;
  string message;

  app->writeLog(OPENAPRS_LOG_DEBUG, "stdinAPNSPUSH::Execute> /APNS PUSH received.");

  if (ePacket->getArguments().size() < 2) {
    deviceToken = "4aca19667e305c54d2dfcb65d2b5c0a60592b4e148cf28f392b15468d5f1e4d6";
    message = "This is a test";
  //  return CMDERR_SYNTAX;
  } // if
  else {
    deviceToken = ePacket->getArguments()[0];
    message = ePacket->getArguments().getTrailing(1);
  } // else

  try {
    aMessage = new ApnsMessage(deviceToken);
  } // try
  catch (ApnsMessage_Exception e) {
    printf("*** ApnsMessage Failed[%s] %s: %s\n", deviceToken.c_str(), message.c_str(), e.message());
    app->writeLog(MODULE_APNS_LOG_NORMAL, "WARNING: ApnsMessage Failed[%s] %s: %s", deviceToken.c_str(), message.c_str(), e.message());
    return CMDERR_SUCCESS;
  } // catch

  aMessage->text(message);
  aMessage->actionKeyCaption("View");
  aMessage->badgeNumber(1);

  aAPNS->Push(aMessage);

  printf("*** APNS Queuing[%s] %s\n", deviceToken.c_str(), message.c_str());
  app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: APNS Queuing APNS[%s] %s", deviceToken.c_str(), message.c_str());

  return CMDERR_SUCCESS;
} // stdinAPNSPUSH::Execute

}

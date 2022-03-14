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
 $Id: timerAPRSMAIL.cpp,v 1.2 2003/08/30 03:59:00 omni Exp $
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
#include "ApnsMessage.h"
#include "Command.h"
#include "PushController.h"
#include "Server.h"
#include "Timer.h"

#include "apnsCommands.h"

#include "apns.h"
#include "openaprs.h"
#include "openaprs_string.h"

namespace apns {

  using namespace std;
  using namespace openaprs;

/**************************************************************************
 ** timerAPRSMAIL Class                                                  **
 **************************************************************************/

  const int timerTESTTHROUGHPUT::OnTimer(Timer *aTimer) {
    ApnsMessage *aMessage;
    PushController p(DEVEL_APPLE_HOST, DEVEL_APPLE_PORT, DEVEL_RSA_CLIENT_CERT, DEVEL_RSA_CLIENT_KEY, DEVEL_CA_CERT_PATH, DEVEL_APPLE_TIMEOUT);
    string deviceToken;
    string message;
    stringstream s;
    int r;
    unsigned int testNum;
    unsigned int maxCreate = app->cfg->get_int("module.apns.test.max.create", 100);
    unsigned int numToCreate = rand() % maxCreate;

    printf("*** THROUGHPUT TEST: BEGIN with %d messages.\n", numToCreate);
    //app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: THROUGHPUT TEST: BEGIN with %d messages.", numToCreate);

    for(testNum=0; testNum < numToCreate; testNum++) {
      deviceToken = p.generateRandomDeviceToken();

      // create a random text message
      s.str("");
      for(int i=0; i < 64; i++) {
        r = (rand() % 94) + 32;
        s << (char) r;
      } // for

      message = s.str();

      try {
        aMessage = new ApnsMessage(deviceToken);
      } // try
      catch (ApnsMessage_Exception e) {
        printf("*** Test #%d - Failed to create new APNS message: %s\n", testNum, e.message());
        app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Failed to create new APNS message: %s", testNum, e.message());
        return CMDERR_SUCCESS;
      } // catch

      aMessage->text(message);
      aMessage->actionKeyCaption("View");
      aMessage->badgeNumber(1);

      aAPNS->Push(aMessage);

      printf("*** Test #%d - Queuing push: throughput test\n", testNum);
      //app->writeLog(MODULE_APNS_LOG_NORMAL, "INFO: Test #%d - Queuing push: throughput test", testNum);

    } // for

    printf("*** THROUGHPUT TEST: END %d messages queued.\n", numToCreate);
    app->writeLog(MODULE_APNS_LOG_NORMAL, "STATUS: THROUGHPUT TEST: END %d messages queued.", numToCreate);

    return CMDERR_SUCCESS;
  } // timerCREATEMESSAGES::OnTimer
} // namespace commands

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
 ** stdinAPNSTESTTHROUGHPUT Class                                        **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

const int stdinAPNSTESTTHROUGHPUT::Execute(COMMAND_ARGUMENTS) {
  Server::timerId tid;
  string maxCreateString;
  string timerIntervalString;
  unsigned int maxCreate;
  unsigned int timerInterval = 60;

  app->writeLog(OPENAPRS_LOG_DEBUG, "stdinAPNSPUSH::Execute> /APNS TEST PUSH received.");

  //if (ePacket->getArguments().size() < 1)
  //  return CMDERR_SYNTAX;

  if (app->cfg->is_int("module.apns.timer.test.id")) {
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Throughput Test: Stopped");
    tid = app->cfg->get_int("module.apns.timer.test.id");
    app->removeTimer(tid);
    app->cfg->pop("module.apns.timer.test.id");
  } // if
  else {
    if (ePacket->getArguments().size() > 0) {
      maxCreateString = ePacket->getArguments()[0];
      maxCreate = atoi(maxCreateString.c_str());
      app->cfg->replace_int("module.apns.test.max.create", maxCreate);
    } // if

    if (ePacket->getArguments().size() == 2) {
      timerIntervalString = ePacket->getArguments()[1];
      timerInterval = atoi(timerIntervalString.c_str());
    } // if

    tid = app->addTimer(timerInterval, TIMER_CONTINUOUS, false, true, new timerTESTTHROUGHPUT);

    app->cfg->push_int("module.apns.timer.test.id", tid);
    stringstream s;
    s << "*** Throughput Test: Initiated with max "
          << app->cfg->get_int("module.apns.test.max.create", 10)
          << " create and "
          << timerInterval
          << " second interval."
          << endl;
    app->writeLog(OPENAPRS_LOG_STDOUT, "%s", s.str().c_str());
  } // else

  return CMDERR_SUCCESS;
} // stdinAPNSTESTPUSH::Execute

}

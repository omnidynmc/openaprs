/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Daniel Robert Karrels                             **
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
 $Id: signal.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
 **************************************************************************/

#include <signal.h>
#include <unistd.h>

#include "Network.h"
#include "Server.h"

#include "config.h"
#include "openaprs.h"
#include "openaprs_signal.h"

namespace openaprs {

/*
 * This function is designed intentionally to handle all
 * signals that might interrupt the process.
 * It prevents the unblocking nature of signals from
 * corrupting buffers and possibly crashing the machine.
 */
void sighandle(int sig) {
  Network::connectionMapType connectionList;
  Network::connectionMapType::iterator ptr;
  static int lastsig;

  if (sig == NOSIGNAL) {
    switch(lastsig) {
      case SIGHUP:
        app->writeLog(OPENAPRS_LOG_STDOUT, "*** SIGHUP Received");
        app->writeLog(OPENAPRS_LOG_NORMAL, "*** SIGHUP Received");
        app->writeLog(OPENAPRS_LOG_DEBUG, "*** SIGHUP Received");

        UnsetServerConnected(app);
        app->removeConnection(app->getSocket());
        app->setSocket(-1);
        // app->restart(DEFAULT_RESTART);

        break;
      case SIGPIPE:
        app->writeLog(OPENAPRS_LOG_STDOUT, "*** SIGPIPE Received");
        app->writeLog(OPENAPRS_LOG_NORMAL, "*** SIGPIPE Received");
        app->writeLog(OPENAPRS_LOG_DEBUG, "*** SIGPIPE Received");

        UnsetServerConnected(app);
        app->removeConnection(app->getSocket());
        app->setSocket(-1);

        break;
      case SIGALRM:
        break;
      case SIGUSR1:

        app->toggleOption("openaprs.toggles.debug");

        app->writeLog(OPENAPRS_LOG_STDOUT, "*** SIGUSR1 Received, debug: %s", (IsServerDebug == true) ? "ON" : "OFF");
        app->writeLog(OPENAPRS_LOG_NORMAL, "*** SIGUSR1 Received, debug: %s", (IsServerDebug == true) ? "ON" : "OFF");
        app->writeLog(OPENAPRS_LOG_DEBUG, "*** SIGUSR1 Received, debug: %s", (IsServerDebug == true) ? "ON" : "OFF");
        break;
      case SIGUSR2:
        app->writeStats(true);

        app->listConnections(connectionList);
        for(ptr = connectionList.begin(); ptr != connectionList.end(); ptr++)
          app->writeLog(OPENAPRS_LOG_NORMAL, "*** Connections: %d) %s [%s]", ptr->first, ptr->second->getDescription().c_str(), ptr->second->getIP().c_str());

        app->writeLog(OPENAPRS_LOG_STDOUT, "*** SIGUSR2 Received, STATS");
        app->writeLog(OPENAPRS_LOG_NORMAL, "*** SIGUSR2 Received, STATS");
        app->writeLog(OPENAPRS_LOG_DEBUG, "*** SIGUSR2 Received, STATS");
        break;
      case SIGINT:
        app->writeLog(OPENAPRS_LOG_STDOUT, "*** SIGINT Received");
        app->writeLog(OPENAPRS_LOG_NORMAL, "*** SIGINT Received");
        app->writeLog(OPENAPRS_LOG_DEBUG, "*** SIGINT Received");

        app->Die("SIGINT Received");
        break;
      default:
        return;
        break;
    }

    lastsig = 0;
  }
  else
    lastsig = sig;
}

}

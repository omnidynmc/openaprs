/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** HAL9000, Internet Relay Chat Bot                                     **
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
 $Id: APNS.cpp,v 1.12 2003/09/05 22:23:41 omni Exp $
 **************************************************************************/

#include <fstream>
#include <string>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <list>
#include <map>
#include <new>
#include <iostream>
#include <fstream>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include "App_Log.h"
#include "Cfg.h"
#include "MGMT.h"
#include "MGMT_DBI.h"
#include "Server.h"
#include "StringTool.h"

#include "mgmtCommands.h"
#include "stdinCommands.h"

#include "mgmt.h"

namespace mgmt {

  using namespace openaprs;
  using namespace handler;
  using namespace putmysql;
  using namespace std;
  using aprs::StringTool;

/**************************************************************************
 ** APNS Class                                                           **
 **************************************************************************/

  MGMT::MGMT(App_Log *log) : _die(false) {
    logger(log);

    try {
      _dbi = new MGMT_DBI();
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    initializeSystem();

    return;
  } // MGMT::MGMT

  MGMT::~MGMT() {
    delete _dbi;

    deinitializeSystem();

    return;
  } // MGMT::~MGMT

  /***********************************************
   ** Initialization / Deinitailization Members **
   ***********************************************/
  const bool MGMT::initializeSystem() {
    initializeCommands();

    _consolef("*** Module MGMT: System Initialized");
    _logf("*** Module MGMT: System Initialized");

    return true;
  } // MGMT::initializeSystem

  const bool MGMT::initializeCommands() {
    app->addCommand("MGMT", "STDIN", "/MGMT", -1, Command::FLAG_CLEANUP, new stdinDTREE);
      app->addCommand("ACTIVATE", "STDIN/MGMT", "/MGMT ACTIVATE <id|callsig|email>", -1, Command::FLAG_CLEANUP, new stdinACTIVATE);
      app->addCommand("MEMCACHED", "STDIN/MGMT", "/MGMT MEMCACHED", -1, Command::FLAG_CLEANUP, new stdinDTREE);
        app->addCommand("GET", "STDIN/MGMT/MEMCACHED", "/MGMT MEMCACHED GET <ns:key>", -1, Command::FLAG_CLEANUP, new stdinMEMCACHEDGET);
      app->addCommand("SHOW", "STDIN/MGMT", "/MGMT SHOW", -1, Command::FLAG_CLEANUP, new stdinDTREE);
        app->addCommand("USER", "STDIN/MGMT/SHOW", "/MGMT SHOW USER <id|callsign|email>", -1, Command::FLAG_CLEANUP, new stdinSHOWUSER);
      app->addCommand("VERIFY", "STDIN/MGMT", "/MGMT VERIFY <id|callsig|email>", -1, Command::FLAG_CLEANUP, new stdinVERIFY);

    _logf("*** Module MGMT: Commands Initialized");
    _consolef("*** Module MGMT: Commands Initialized");

    return true;
  } // MGMT::initializeCommands

  const bool MGMT::deinitializeCommands() {
    app->removeCommand("STDIN/MGMT");

    _logf("*** Module MGMT: Commands Deinitialized");
    _consolef("*** Module MGMT: Commands Deinitialized");

    return true;
  } // MGMT::deinitializeCommands

  const bool MGMT::deinitializeSystem() {
    die(true);
    deinitializeCommands();

    app->cfg->pop_match("openaprs.count.module.mgmt.*");

    _logf("*** Module MGMT: System Deinitialized");
    _consolef("*** Module MGMT: System Deinitialized");

    return true;
  } // MGMT::deinistializeSystem
} // namespace mgmt


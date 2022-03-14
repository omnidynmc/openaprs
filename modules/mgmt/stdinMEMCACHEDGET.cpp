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
#include "MGMT.h"
#include "MGMT_DBI.h"
#include "MemcachedController.h"
#include "Server.h"

#include "openframe/Serialize.h"
#include "openframe/Stopwatch.h"
#include "openframe/StringToken.h"
#include "openframe/StringTool.h"
#include "openframe/Result.h"

#include "mgmtCommands.h"

#include "mgmt.h"

namespace mgmt {
  using namespace std;
  using namespace openaprs;
  using openframe::Serialize;
  using openframe::Stopwatch;
  using openframe::StringToken;
  using openframe::StringTool;

/**************************************************************************
 ** stdinMEMCACHEDGET Class                                              **
 **************************************************************************/

  const int stdinMEMCACHEDGET::Execute(COMMAND_ARGUMENTS) {
    StringToken st;
    Stopwatch sw;
    MemcachedController *memcached = NULL;
    MemcachedController::memcachedReturnEnum mcr;
    string find;
    string ns;
    string key;
    string buf;

    app->writeLog(OPENAPRS_LOG_DEBUG, "stdinMEMCACHEDGET::Execute> /MGMT MEMCACHED GET received.");

    if (ePacket->getArguments().size() != 1)
      return CMDERR_SYNTAX;

    st.setDelimiter(':');
    st = ePacket->getArguments()[0];

    if (st.size() != 2)
      return CMDERR_SYNTAX;

    ns = st[0];
    key = st[1];

    sw.Start();

    try {
      memcached = new MemcachedController("localhost");
      mcr = memcached->get(ns, key, buf);
    } // try
    catch(MemcachedController_Exception e) {
      cout << "ERROR: memcached get; " << e.message() << endl;
      return CMDERR_SYNTAX;
    } // catch


    Serialize s;
    openframe::Result res("namespace,key,value");
    if (mcr == MemcachedController::MEMCACHED_CONTROLLER_SUCCESS) {
      s.add(ns);
      s.add(key);
      s.add(buf);
      openframe::Row r(s.compile());
      res.add(r);
    } // if

    openframe::Result::print(res, sw.Time());

    delete memcached;

    return CMDERR_SUCCESS;
  } // stdinMEMCACHEDGET::Execute
} // namespace stdinSHOWUSER

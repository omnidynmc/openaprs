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
 **************************************************************************
 $Id: dccRELOADMODULE.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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
#include "DCC.h"
#include "DCCUser.h"
#include "Send.h"
#include "Command.h"
#include "Server.h"
#include "Vars.h"

#include "openframe/Stopwatch.h"
#include "openframe/StringToken.h"

#include "dccCommands.h"

#include "dcc.h"
#include "match.h"
#include "openaprs.h"
#include "openaprs_string.h"

namespace dcc {
  using namespace std;
  using namespace openaprs;
  using openframe::Stopwatch;
  using openframe::StringToken;

  /**************************************************************************
   ** dccFINDUSER Class                                                    **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const int dccFINDUSER::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DBI::setMapType sm;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    DCCUser *dccuser;                                     // pointer to a user
    Stopwatch sw;
    map<int, regexMatch> rl;
    char y[10];
    string parseMe;
    string callsign;
    string field, sf;
    stringstream s;
    unsigned int numReturned;
    unsigned int i;
    unsigned int limit = kDefaultResultLimit;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    if (IsDCCUserLogin(dccuser) == false)
      return CMDERR_ACCESS;

    if (IsDCCUserAdmin(dccuser) == false) {
      dccuser->reply("errAccessDenied");
      return CMDERR_SUCCESS;
    } // if

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    // initialize variables
    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    if (!v.isName("cl")) {
      dccuser->reply("errMissingCallsign");
      return CMDERR_SUCCESS;
    } // if

    // initialize variables
    callsign = v["cl"];

    if (ereg("^([*A-Za-z0-9._%+-]+[@][*A-Za-z0-9.-]+[.][*a-zA-Z]{2,4})$", callsign, rl) < 1
        && ereg("^([*a-zA-Z0-9]{2,7})$", callsign, rl) < 1) {
      dccuser->reply("errInvalidCallsign");
      return CMDERR_SUCCESS;
    } // if

    // DO STUFF HERE
    // PROFILING: START
    sw.Start();

    numReturned = dbi->signup->FindUser(callsign, sm);

    snprintf(y, sizeof(y), "%.4f", sw.Time());

    for(i=0; i < sm.size(); i++) {
      rm = sm[i].resultMap;

      // Build results
      s.str("");
      for(ptr = rm.begin(); ptr != rm.end(); ptr++) {
        if (!dccuser->findFieldByName(ptr->first, field, sf))
          continue;

        if (s.str().length() > 0)
          s << "|";

        s << field << ":" << v.Escape(ptr->second);
      } // if

      dccuser->sendf("319 %s\n", s.str().c_str());
    } // for

    dccuser->sendf("320 RS:%d|MH:%d|SW:%s|MS:%d of %d matches returned (%s seconds).\n", 
                sm.size(),
                numReturned,
                y,
                sm.size(),
                numReturned,
                y);

    app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (FU) %s received %d of %d matches (%s seconds)",
                      dccuser->callsign().c_str(),
                      sm.size(),
                      numReturned,
                      y);

    return CMDERR_SUCCESS;
  } // dccFINDUSER::Execute
} // namespace dcc

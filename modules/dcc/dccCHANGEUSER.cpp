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
#include "DBI.h"
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
   ** dccCHANGEUSER Class                                                  **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const int dccCHANGEUSER::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DBI::setMapType sm;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    DCCUser *dccuser;                                     // pointer to a user
    Stopwatch sw;
    bool isOK;
    map<int, regexMatch> rl;
    char y[10];
    char ca;
    string parseMe;
    string userId;
    string changeAction;
    string field, sf;
    stringstream s;

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

    if (!v.isName("idr")) {
      dccuser->reply("errMissingIdr");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("chg")) {
      dccuser->reply("errChangeAction");
      return CMDERR_SUCCESS;
    } // if

    // initialize variables
    userId = v["idr"];
    changeAction = v["chg"];

    if (ereg("^([0-9]+)$", userId, rl) < 1) {
      dccuser->reply("errInvalidIdr");
      return CMDERR_SUCCESS;
    } // if

    if (ereg("^(A|D|V|I)$", changeAction, rl) < 1) {
      dccuser->reply("errInvalidChangeAction");
      return CMDERR_SUCCESS;
    } // if

    if (!dbi->signup->isUserById(userId)) {
      dccuser->reply("errNoUser");
      return CMDERR_SUCCESS;
    } // if

    // initialize variables
    ca = changeAction[0];

    // DO STUFF HERE
    // PROFILING: START
    sw.Start();

    switch(ca) {
      case 'A':
        isOK = dbi->signup->setActivated(userId, true);
        break;
      case 'D':
        isOK = dbi->signup->deleteUser(userId);
        break;
      case 'I':
        isOK = dbi->signup->setIE(userId, true);
        break;
      case 'V':
        isOK = dbi->signup->setVerified(userId, true);
        break;
    } // switch

    snprintf(y, sizeof(y), "%.4f", sw.Time());
    dccuser->sendf("321 RS:%d|SW:%s|MS:User %s changed (%s seconds).\n", 
                (isOK ? 1 : 0),
                y,
                (isOK ? "has been" : "has not been"),
                y);

    app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (CU) %s %s user %s (%s seconds)",
                      dccuser->callsign().c_str(),
                      (isOK ? "changed" : "tried to change"),
                      userId.c_str(),
                      y);

    return CMDERR_SUCCESS;
  } // dccCHANGEUSER::Execute
} // namespace dcc

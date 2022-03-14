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
#include "DBI.h"
#include "MGMT.h"
#include "MGMT_DBI.h"
#include "Server.h"
#include "StringToken.h"
#include "StringTool.h"

#include "mgmtCommands.h"

#include "mgmt.h"

namespace mgmt {
  using namespace std;
  using namespace openaprs;

/**************************************************************************
 ** stdinAPNSTESTPUSH Class                                              **
 **************************************************************************/

  const int stdinSHOWUSER::Execute(COMMAND_ARGUMENTS) {
    aprs::StringTool::regexMatchListType rl;
    DBI::resultType res;
    bool isOK = false;
    string find;

    app->writeLog(OPENAPRS_LOG_DEBUG, "stdinSHOWUSER::Execute> /MGMT SHOW USER received.");

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    find = ePacket->getArguments().getTrailing(0);

    if (aprs::StringTool::ereg("[@]", find, rl))
      isOK = mgmt->dbi()->getUserByEmail(find, res);
    else if (aprs::StringTool::ereg("^[0-9]+$", find, rl))
      isOK = mgmt->dbi()->getUserById(find, res);
    else
      isOK = mgmt->dbi()->getUserByCallsign(find, res);

    if (isOK)
      mgmt->dbi()->print("User", res);
    else
      cout << "Not found." << endl;

    return CMDERR_SUCCESS;
  } // stdinAPNSTESTPUSH::Execute
} // namespace stdinSHOWUSER

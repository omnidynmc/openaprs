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
 ** stdinACTIVATE Class                                                  **
 **************************************************************************/

  const int stdinACTIVATE::Execute(COMMAND_ARGUMENTS) {
    aprs::StringTool::regexMatchListType rl;
    string find;
    DBI::resultSizeType numRows = 0;

    app->writeLog(OPENAPRS_LOG_DEBUG, "stdinSHOWUSER::Execute> /MGMT VERIFY received.");

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    find = ePacket->getArguments().getTrailing(0);

    if (aprs::StringTool::ereg("[@]", find, rl))
      numRows = mgmt->dbi()->activateUserByEmail(find);
    else if (aprs::StringTool::ereg("^[0-9]+$", find, rl))
      numRows = mgmt->dbi()->activateUserById(find);
    else
      numRows = mgmt->dbi()->activateUserByCallsign(find);

    //cout << numRows << " row" << (numRows != 1 ? "s" : "") << " affected." << endl << endl;
    app->writeLog(OPENAPRS_LOG_STDOUT, "%d row%s affected.", numRows, (numRows != 1 ? "s" : ""));

    return CMDERR_SUCCESS;
  } // stdinVERIFY::Execute
} // namespace stdinACTIVATE

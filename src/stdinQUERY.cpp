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
 $Id: stdinECHO.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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
#include "Command.h"
#include "DBI.h"
#include "Server.h"

#include "stdinCommands.h"

#include "match.h"

namespace openaprs {

using namespace std;
using namespace aprs;

/**************************************************************************
 ** stdinSHOWCONFIG Class                                                **
 **************************************************************************/

  const int stdinQUERY::Execute(COMMAND_ARGUMENTS) {
    string sqls;

    app->writeLog(OPENAPRS_LOG_DEBUG, "stdinQUERY::Execute> /QUERY received.");

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    // initialize variables
    sqls = ePacket->getArguments().getTrailing(0);

    DBI::resultType res;
    if (app->dbi->query(sqls, res))
      app->dbi->print(res);

    return CMDERR_SUCCESS;
  } // stdinQUERY::Execute
} // namespace openaprs

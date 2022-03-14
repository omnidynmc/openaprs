/**************************************************************************
 ** Copyright (C) 1999 Gregory A. Carter                                 **
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

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "Server.h"
#include "Command.h"

#include "openframe/Result.h"
#include "openframe/Serialize.h"
#include "openframe/Stopwatch.h"
#include "openframe/StringToken.h"

#include "stdinCommands.h"

namespace openaprs {
  using openframe::Serialize;
  using openframe::Stopwatch;
  using openframe::StringToken;
  using namespace std;

/**************************************************************************
 ** stdinBOOKMARKS Class                                                 **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

const int stdinBOOKMARKS::Execute(COMMAND_ARGUMENTS) {
  Stopwatch sw;

  app->bookmarks();

  return 1;
} // stdinBOOKMARKS::Execute

}

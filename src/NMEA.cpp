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
 $Id: NMEA.cpp,v 1.2 2005/12/13 21:07:03 omni Exp $
 **************************************************************************/

#include <list>
#include <new>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "NMEA.h"
#include "Log.h"
#include "LineBuffer.h"
#include "Server.h"

#include "openframe/Stopwatch.h"

#include "stdinCommands.h"

#include "config.h"
#include "openaprs.h"
#include "openaprs_string.h"

namespace openaprs {
  using openframe::Stopwatch;
  using std::string;
using std::list;
using std::ofstream;
using std::ostream;
using std::stringstream;
using std::ios;
using std::endl;
using std::cout;

  /**************************************************************************
   ** NMEA Class                                                           **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  NMEA::NMEA() {
    // set defaults
  }

  NMEA::~NMEA() {
    // cleanup
  }

  /***************  
   ** Variables **
   ***************/

  /**********************
   ** Checksum Members **
   **********************/

  const bool NMEA::isValid(const string &compareMe) {
    bool isFound = false;
    int i;

    for (i=compareMe.length(); i > 0 && !isFound; i--) {
      if (i == '*')
        isFound = true;
    } // for

    if (isFound == false)
      return false;

    i++;

    return (compareMe.substr(i, compareMe.length()-i) == Checksum(compareMe));

  } // NMEA::isValid

  const string NMEA::Checksum(const string &parseMe) {
    int cs = 0;
    unsigned int i;
    char c;
    char hexme[3];

    for(i=0; i < parseMe.length(); i++) {
      c = parseMe[i];

      // Skip the initial $
      if (c == '$')
        continue;

      // Stop processing sentence.
      if (c == '*')
        break;

      if (cs == 0)
       cs = (unsigned int) c;
      else
        cs = cs ^ (unsigned int) c;
    } // for

    snprintf(hexme, sizeof(hexme), "%X", cs);

    return string(hexme);
  } // NMEA::Checksum
} // namespace openaprs

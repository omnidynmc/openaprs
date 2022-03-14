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
 $Id: cs.cpp,v 1.9 2003/09/25 17:23:47 omni Exp $
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
#include <stdio.h>
#include <stdarg.h>

#include "App_Log.h"
#include "MGMT.h"
#include "Server.h"

#include "mgmt.h"

namespace mgmt {
  using namespace openaprs;
  using namespace std;

  extern "C" {
    /**********************
     ** Global Variables **
     **********************/

    MGMT *mgmt;

    /******************
     ** Log Commands **
     ******************/

    /********************
     ** Initialization **
     ********************/

    /*****************
     ** Constructor **
     *****************/

    void module_constructor() {

      try {
        mgmt = new MGMT( new App_Log(""));
      }
      catch(bad_alloc xa) {
        assert(false);
      } // catch

      return;
    } // module_constructor

    /****************
     ** Destructor **
     ****************/

    void module_destructor() {
      app->cfg->pop("module.mgmt");

      // close listening sockets
      delete mgmt;

      return;
    } // module_destructor
  } // extern "C"
} // namespace mgmt

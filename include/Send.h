/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
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
 $Id: Send.h,v 1.1 2005/11/21 18:16:02 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_SEND_H
#define __OPENAPRS_SEND_H

#include <string>
#include <map>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "LineBuffer.h"
#include "App_Log.h"
#include "Network.h"
#include "Options.h"
#include "Command.h"
#include "Timer.h"

#include "openaprs.h"
#include "openaprs_string.h"
#include "misc.h"

namespace aprs {

using std::string;
using std::map;
using std::list;
using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class Send : public OpenAPRS_Abstract {
  public:
    Send();				// constructor
    virtual ~Send();			// destructor

    /**********************
     ** Type Definitions **
     **********************/

    /***************
     ** Variables **
     ***************/

    /*************
     ** Members **
     *************/

    const bool Message(const string &, 
                       const string &, 
                       const string &, 
                       const bool, 
                       string &);
    const bool Object(const string &,
                      const string &, 
                      const time_t,
                      const double, 
                      const double,
                      const char, 
                      const char,
                      const double, 
                      const int,
                      const double, 
                      const bool,
                      const bool, 
                      const int,
                      const string &, 
                      const bool,
                      string &);
    const bool Position(const string &,
                        const time_t,
                        const double, 
                        const double,
                        const char, 
                        const char,
                        const double, 
                        const int,
                        const double, 
                        const bool,
                        const int,
                        const string &, 
                        const bool,
                        string &);

    /***************
     ** Variables **
     ***************/
  public:
  protected:
  private:
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

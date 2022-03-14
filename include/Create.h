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
 $Id: Create.h,v 1.1 2005/11/21 18:16:02 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_CREATE_H
#define __OPENAPRS_CREATE_H

#include <string>
#include <map>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

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
  class APRS_Message {
    public:
      bool local;
      int id;
      string source;
      string target;
      string message;
      string title;
      string decay_id;
  };

  class APRS_Position {
    public:
      bool local;
      string decay_id;
      string status;
      string source;
      string overlay;
      string title;
      char symbol_table, symbol_code;
      double latitude, longitude, altitude, speed;
      int course;
      unsigned int id;
     time_t broadcast_ts;
  };

  class APRS_Object {
    public:
      bool toKill;
      bool local;
      string decay_id;
      string name;
      string status;
      string source;
      string overlay;
      string title;
      char symbol_table, symbol_code;
      double latitude, longitude, altitude, speed;
      int course;
      unsigned int id;
      unsigned int beacon;
      time_t broadcast_ts;
      time_t expire_ts;
  };

class Create {
  public:
    Create();				// constructor
    virtual ~Create();			// destructor

    /**********************
     ** Type Definitions **
     **********************/

    /***************
     ** Variables **
     ***************/

    /*************
     ** Members **
     *************/

    const string Timestamp(const time_t, const bool);
    const bool Hash(const string &, short &);
    const bool Position(const double, const double, const double, const int,
                        const double, const char, const char, const bool,
                        const int, string &);
    const bool Object(const string &, const time_t, const double, const double,
                      const char, const char, const double, const int, const double,
                      const bool, const bool, const int, const string &,
                      string &);
    const bool Position(const time_t, const double, const double,
                        const char, const char, const double, const int, const double,
                        const bool, const int, const string &,
                      string &);
    const bool Message(const string &, const string &, const string &,
                       string &);
    const bool encodeMessageID(const unsigned int, string &);

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

#define kKey 0x73e2 // This is the seed for the key

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

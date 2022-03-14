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
 $Id: Send.cpp,v 1.2 2005/12/13 21:07:03 omni Exp $
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

#include "APRS.h"
#include "Create.h"
#include "Uplink.h"
#include "Log.h"
#include "Server.h"
#include "Send.h"

#include "config.h"
#include "openaprs.h"
#include "openaprs_string.h"

namespace aprs {

using std::string;
using std::list;
using std::ofstream;
using std::ostream;
using std::stringstream;
using std::ios;
using std::endl;
using std::cout;
using aprs::Create;
using openaprs::Server;

/**************************************************************************
 ** Send Class                                                           **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

Send::Send() {
  // set defaults
}

Send::~Send() {
  // cleanup
}

  /***************
   ** Variables **
   ***************/

  /*********************
   ** Message Members **
   *********************/

  const bool Send::Message(const string &sendSource,
                           const string &sendTarget,
                           const string &sendMessage,
                           const bool local,
                           string &ret) {
    aprs::Create c;
    string message;

    if (!c.Message(sendSource, sendTarget, sendMessage, message)) {

      _slogf(OPENAPRS_LOG_APRS, "failed to create message '%s' from %s to %s",
                                             sendMessage.c_str(), sendSource.c_str(),
                                             sendTarget.c_str());
      return false;
    } // if

    _slogf(OPENAPRS_LOG_APRS, "creating message %s>%s: %s%s",
                                           sendSource.c_str(),
                                           sendTarget.c_str(),
                                           sendMessage.c_str(),
                                           (local ? " (local)" : ""));

    message = sendSource
              + ">" + OPENAPRS_CONFIG_CREATE_DESTINATION + ",TCPIP*,qAC,"
              + OPENAPRS_CONFIG_CREATE
              + ":"
              + message;

//    if (!local)
//      app->Send("%s", message.c_str());

    app->addWork(new UplinkPacket(time(NULL), message));

    ret = message;

    return true;
  } // Send::Message

  /********************
   ** Object Members **
   ********************/

  const bool Send::Object(const string &source,
                          const string &name,
                          const time_t timestamp,
                          const double latitude,
                          const double longitude,
                          const char table,
                          const char code,
                          const double speed,
                          const int course,
                          const double altitude,
                          const bool alive,
                          const bool compress,
                          const int ambiguity,
                          const string &comment,
                          const bool local,
                          string &ret) {
    aprs::Create c;
    string object;

    if (!c.Object(name, timestamp, latitude, longitude, table,
                  code, speed, course, altitude, alive, false, ambiguity,
                  comment, object)) {

      _slogf(OPENAPRS_LOG_APRS, "failed to create object '%s' from %s",
                                             name.c_str(), source.c_str());
      return false;
    } // if

    if (!alive)
      _slogf(OPENAPRS_LOG_APRS, "deleting object '%s' by %s@[%f,%f]: %s%s",
                        (local ? "LOCAL " : ""),name.c_str(), source.c_str(), latitude, longitude,
                        (comment.length() ? comment.c_str() : "(empty)"),
                        (local ? " (local)" : "")
                      );
    else
      _slogf(OPENAPRS_LOG_APRS, "creating object '%s' from %s@[%f,%f]: %s%s",
                        name.c_str(), source.c_str(), latitude,
                        longitude,
                        (comment.length() ? comment.c_str() : "(empty)"),
                        (local ? " (local)" : "")
                       );


    object = source
                   + ">" + OPENAPRS_CONFIG_CREATE_DESTINATION + ",TCPIP*,qAC,"
                   + OPENAPRS_CONFIG_CREATE
                   + ":"
                   + object;

//    if (!local)
//      app->Send("%s", object.c_str());

    app->addWork(new UplinkPacket(time(NULL), object));

    ret = object;

    return true;
  } // Send::Object

  /**********************
   ** Position Members **
   **********************/

  const bool Send::Position(const string &source,
                            const time_t timestamp,
                            const double latitude, 
                            const double longitude,
                            const char table, 
                            const char code,
                            const double speed, 
                            const int course,
                            const double altitude, 
                            const bool compress, 
                            const int ambiguity,
                            const string &comment,
                            const bool local,
                            string &ret) {
    aprs::Create c;
    string position;

    if (!c.Position(timestamp, latitude, longitude, table,
                  code, speed, course, altitude, false, ambiguity,
                  comment, position)) {

      _slogf(OPENAPRS_LOG_APRS, "failed to create position '%s' from %s",
                                             source.c_str());

      return false;
    } // if

    _slogf(OPENAPRS_LOG_APRS, "creating position from %s@[%f,%f]: %s%s",
                      source.c_str(), latitude, longitude,
                      (comment.length() ? comment.c_str() : "(empty)"),
                      (local ? " (local)" : "")
                     );


    position = source
               + ">" + OPENAPRS_CONFIG_CREATE_DESTINATION + ",TCPIP*,qAC,"
               + OPENAPRS_CONFIG_CREATE
               + ":"
               + position;

//    if (!local)
//      app->Send("%s", position.c_str());

    app->addWork(new UplinkPacket(time(NULL), position));

    ret = position;

    return true;
  } // Send::Position

}

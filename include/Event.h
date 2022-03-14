/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** pBurst, Internet Relay Chat Protocol Burster                         **
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
 $Id: Event.h,v 1.1.1.1 2003/08/17 01:16:12 omni Exp $
 **************************************************************************/

#ifndef __LIBHANDLER_EVENT_H
#define __LIBHANDLER_EVENT_H

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "Command.h"
#include "Flag.h"

#include "openframe/VarController.h"

#include "openaprs_string.h"

namespace handler {

using namespace openaprs;
using openframe::VarController;
using std::string;
using std::stringstream;
using std::ofstream;
using std::ios;
using std::ios_base;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class EventHandler;

class Event : public Flag {
  public:
    // ### Type Definitions ###
    typedef unsigned int eventId;
    typedef unsigned int flagType;

    // ### Variables ###
    static const flagType FLAG_CLEANUP;

    // ### Members ###
    Event(const eventId, const string &, const unsigned int, EventHandler *);
    virtual ~Event();

    inline const eventId getId() const { return myId; }

    inline void setMatch(const string &setMe) { myMatch = setMe; }
    inline const string &getMatch() const { return myMatch; }

    virtual const bool openEvent(const string &, ios_base::openmode);
    virtual const bool closeEvent();
    virtual const unsigned int appendEvent(const string &);
    virtual const unsigned int appendEvent(const stringstream &);
    virtual void flushEvent();
    virtual const bool isEventOpen() { return myFile.is_open(); }

    inline void setEventHandler(EventHandler *setMe) { myEventHandler = setMe; }
    inline EventHandler *getEventHandler() { return myEventHandler; }

    Event &operator<<(const char &);
    Event &operator<<(const string &);
    Event &operator<<(const int &);
    Event &operator<<(const unsigned int &);
    Event &operator<<(const long int &);
    Event &operator<<(const unsigned long int &);
    Event &operator<<(const short int &);
    Event &operator<<(const unsigned short int &);
    Event &operator<<(const stringstream &);
    Event &operator<<(const double &);

  protected:
  private:
    // ### Variables ###
    EventHandler *myEventHandler;		// Pointer to our EventHandler.
    eventId myId;				// Id I have been assigned.
    ofstream myFile;			// Stream to file output.
    string myMatch;			// Requested match for eventging.
};

class EventHandler {
  public:
    EventHandler() {
      vars = new VarController;
    } // EventHandler
    virtual ~EventHandler() {
      delete vars;
    } // ~EventHandler

    /**********************
     ** Type Definitions **
     **********************/

    /*******************
     ** OnEvent Members **
     *******************/

    virtual const int OnEvent(Event *, const string &, Packet *) = 0;

    /**********************
     ** Operator Members **
     **********************/

    VarController *vars;

  protected:
  private:
    /***************
     ** Variables **
     ***************/
};

#define CREATE_EVENT(name) \
class name : public EventHandler { \
  public: \
    name() {} \
    virtual ~name() {} \
    const int OnEvent(Event *, const string &, Packet *); \
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace event
#endif

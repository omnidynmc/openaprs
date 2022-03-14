/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, mySQL APRS Injector                                        **
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
 $Id: DCC.h,v 1.8 2003/09/04 00:22:00 omni Exp $
 **************************************************************************/

#ifndef __MODULE_DCC_LISTENER_H
#define __MODULE_DCC_LISTENER_H

#include <set>

#include <netdb.h>
#include <unistd.h>

#include "DCC_Abstract.h"

#include "openframe/OFLock.h"

#include "md5wrapper.h"

namespace dcc {
  using openaprs::Network;
  using openframe::OFLock;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class Listener_Exception : public OpenAPRSAbstract_Exception {
  public:
    Listener_Exception(const string message) throw() : OpenAPRSAbstract_Exception(message) {
    } // Listener_Exception

  private:
}; // class Listener_Exception

class Network;
class Listener : public DCC_Abstract {
  public:
    Listener(const int, const unsigned int);
    virtual ~Listener();

    // ### Type Definitions ###

    // ### Members ###
    Connection *listen();

    // ### Variables ###

  protected:
  private:
    Network *_network;		// network class
    int _port;			// listener port
    int _socket;		// listener socket
    unsigned int _max;		// max connections
}; // Worker

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

extern "C" {
} // extern

}
#endif

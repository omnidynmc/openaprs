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
 $Id: APNS.cpp,v 1.12 2003/09/05 22:23:41 omni Exp $
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
#include <fstream>

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

#include "Listener.h"
#include "Network.h"

namespace dcc {
  using openaprs::Network;
  using openaprs::Connection;
  using namespace std;

/**************************************************************************
 ** Worker Class                                                         **
 **************************************************************************/

  Listener::Listener(const int port, const unsigned int max)
                     : _port(port), _max(max) {
    _network = new Network();

    _socket = _network->makeSocket(port, max, false);

    if (_socket == NETWORK_ERROR) {
      delete _network;
      throw Listener_Exception("could not create listener socket");
    } // if
  } // Listener::Listener

  Listener::~Listener() {
    close(_socket);
    delete _network;
  } // Worker::~Worker

  Connection *Listener::listen() {
    Connection *connection;
    int ret;
    struct in_addr in;
    stringstream s;

    ret = _network->acceptConnection(_socket, false);

    // process return value
    switch(ret) {
      // ignore this socket for now nothing to do
      case NETWORK_FALSE:
        return NULL;
        break;
      // if there is an error, remove the client
      case NETWORK_ERROR:
        return NULL;
        break;
    } // switch

    // Set IP for connection.
    connection = _network->findConnection(ret);

    assert(connection != NULL);		// bug

    _network->removeConnection(connection->getFd(), false);

    // snag the ip address so we can set it
    in.s_addr = ntohl(app->getConnectionPeerNetworkNumber(ret));

    connection->setIP(inet_ntoa(in));

    s.str("");
    s << _network->getConnectionPortNumber(ret);
    connection->setPort(s.str());

    return connection;
  } // Listener::listen

} // namespace dcc

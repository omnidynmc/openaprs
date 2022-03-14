/**************************************************************************
 ** OpenFrame Library                                                    **
 **************************************************************************
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

#ifndef __LIBOPENFRAME_LISTENCONTROLLER_H
#define __LIBOPENFRAME_LISTENCONTROLLER_H

#include <map>
#include <string>

#include <netdb.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "openframe/ConnectionManager.h"
#include "openframe/OpenFrame_Abstract.h"
#include "openframe/OFLock.h"

namespace openframe {
  using std::map;
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class ListenController : public ConnectionManager {
  public:
    typedef ConnectionManager super;

    ListenController(const int, const int, const string &host="");
    virtual ~ListenController();

    // ### Public Members ###
    const socketBaseReturn_t listen(const int, const int, const bool blocking = false);
    const socketBaseReturn_t accept(const int);

    const bool connected(const int);
    virtual void onDisconnect(const Connection *) = 0;
    virtual void onConnect(const Connection *) = 0;
    virtual void onRead(const Peer *) = 0;
    virtual const string::size_type onWrite(const Peer *, string &) = 0;
    virtual ListenController &start();
    virtual void stop();

    virtual const bool invite();

    static void *thread(void *);

  protected:
    // ### Variables ###
    std::string _host;
    int _port;
    int _max;

  private:
    // ### Private Members ###
    void _initialize();
    void _initializeThreads();
    void _deinitialize();
    void _deinitializeThreads();

    pthread_t _listener_tid;
    bool _initialized;
    Connection _con;
}; // class ListenController

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openframe
#endif

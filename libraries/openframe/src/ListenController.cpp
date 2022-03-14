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

#include <openframe/config.h>

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

#include <openframe/ListenController.h>
#include <openframe/StringTool.h>

namespace openframe {
  using namespace openframe::loglevel;

/**************************************************************************
 ** ListenController Class                                               **
 **************************************************************************/
  ListenController::ListenController(const int port, const int max, const std::string &host)
    : _max(max) {

    _host = host;
    _port = port;
    _initialized = false;

    return;
  } // ListenController::ListenController

  ListenController::~ListenController() {
    stop();

    return;
  } // ListenController::~ListenController

  ListenController &ListenController::start() {
    assert(_initialized != true);
    super::start();
    _initialize();
    return (ListenController &) *this;
  } // ListenController::start

  void ListenController::stop() {
    if (!_initialized) return;
    _deinitialize();
    super::stop();
  } // ListenController::stop

  const int ListenController::listen(const int port, const int max, const bool blocking) {
    SocketType sockaddr;
    int sock;
    int ret;				// returned value
    unsigned int opt;			// setsockopt options

    // setup the sock_addr data structure
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(port);

    // create the new socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
      _error = strerror(errno);
      _errno = errno;
      return SOCKETBASE_ERROR;
    } // if

    // allow the ip to be reused
    opt = 1;

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
      close(sock);
      _error = strerror(errno);
      _errno = errno;
      return SOCKETBASE_ERROR;
    } // if

    // bind the socket
    ret = bind(sock, (struct sockaddr *) &sockaddr, sizeof(SocketType));
    if (ret == -1) {
      _error = strerror(errno);
      _errno = errno;
      return SOCKETBASE_ERROR;
    } // if

    // set up the socket for listening
    ret = ::listen(sock, max);
    if (ret == -1) {
      _error = strerror(errno);
      _errno = errno;
      return SOCKETBASE_ERROR;
    } // if

    // set non-blocking
    if (!blocking) {
      if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
        close(sock);
        _error = strerror(errno);
        _errno = errno;
        return SOCKETBASE_ERROR;
      } // if
    } // if

    // return the new socket descriptor
    return sock;
  } // ListenController::listen

  const SocketBase::socketBaseReturn_t ListenController::accept(const int listener) {
    SocketType socketInfo;		// socket information
    int len;				// info length
    int sock;				// socket number

    // accept the connections if there are any
    len = sizeof(SocketType);
    sock = ::accept(listener, (struct sockaddr *) &socketInfo, (socklen_t *) &len);

    if (sock == -1) {
      switch(errno) {
        // The descriptor is invalid.
        case EBADF:
        // The per-process descriptor table is full.
        case EMFILE:
        // The descriptor references a file, not a socket.
        case ENOTSOCK:
        // listen(2) has not been called on the socket descriptor.
        case EINVAL:
        // The addr parameter is not in a writable part of the user address space.
        case EFAULT:
        // A connection arrived, but it was closed while waiting on the listen queue.
        case ECONNABORTED:
          // close and move on
          return SOCKETBASE_ERROR;
          break;
        // The accept() operation was interrupted.
        case EINTR:
        // The socket is marked non-blocking and no connections are present to be accepted.
        case EWOULDBLOCK:
          // no connections ignore and skip
          return SOCKETBASE_ERROR;
          break;
      } // switch
    } // if

    // initialize variables
    int ssize = 0;			// socket send size
    socklen_t optlen = sizeof(int);	// option length

    if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *) &ssize, &optlen) < 0) {
      _error = hstrerror(h_errno);
      _errno = h_errno;
      return SOCKETBASE_ERROR;
    } // if

    // we have a successul connection
    // return new socket
    return sock;
  } // ListenController::accept

  const bool ListenController::invite() {
    int sock = accept(_con.sock);
    if (sock < 1)
      return false;

    register_connection(sock);
    return true;
  } // ListenController::invite

  void ListenController::_initialize() {
    int sock = listen(_port, 1024, false);
    if (sock == SOCKETBASE_ERROR)
      throw SocketBase_Exception(_error);

    _con.sock = sock;

    _initializeThreads();
    _initialized = true;
  } // ListenController::_initialize

  void ListenController::_initializeThreads() {
    pthread_create(&_listener_tid, NULL, ListenController::thread, this);
//    _debugf("*** ListenController: Started Listener Thread %u", _listener_tid);
    LOG(LogInfo, << "ListenController: Started Listener Thread "
                 << _listener_tid
                 << std::endl);
  } // ListenController::_initializeThreads

  void ListenController::_deinitialize() {
    if (!_initialized)
      return;

    _initialized = false;
    set_done(true);
    _deinitializeThreads();

    close(_con.sock);
  } // ListenController::_deinitialize

  void ListenController::_deinitializeThreads() {
//    _debugf("*** ListenController: Waiting for Thread %u to Deinitialized", _listener_tid);
    LOG(LogInfo, << "ListenController: Waiting for Thread "
                 << _listener_tid
                 << " to Deinitialize"
                 << std::endl);
    pthread_join(_listener_tid, NULL);

//    _debugf("*** ListenController: Threads Deinitialized");
    LOG(LogInfo, << "ListenController: Threads Deinitialized"
                 << std::endl);
  } // ListenController::_deinitializeThreads

  void *ListenController::thread(void *listenController) {
    ListenController *lc = (ListenController *) listenController;
    const time_t done_check_intval = 10;
    time_t last_done_check = time(NULL);

    while( !lc->is_done() ) {
      if (last_done_check < time(NULL) - done_check_intval) {
        if ( lc->is_done() ) break;
        last_done_check = time(NULL);
      } // if

      bool didWork = lc->invite();
      if (!didWork) usleep(10000);
    } // while

    return NULL;
  } // ListenController::thread

} // namespace openframe

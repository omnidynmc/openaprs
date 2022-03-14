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

#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
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
#include <limits.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include <openframe/SocketController.h>
#include <openframe/scoped_lock.h>

namespace openframe {
  using namespace std;
  using namespace loglevel;

/**************************************************************************
 ** SocketController Class                                               **
 **************************************************************************/
  const time_t SocketController::DEFAULT_CONNECT_INTERVAL		= 60;

  SocketController::SocketController()
                   : _connect_retry_interval(DEFAULT_CONNECT_INTERVAL),
                     _con(NULL),
                     _initialized(false) {
  } // SocketController
  SocketController::SocketController(const string &host, const int port)
                   : _host(host),
                     _port(port),
                     _connect_retry_interval(DEFAULT_CONNECT_INTERVAL),
                     _con(NULL),
                     _initialized(false) {

    set_connected(false);
    return;
  } // SocketController::SocketController

  SocketController::~SocketController() {
    deinit_connection();
    return;
  } // SocketController::~SocketController

  bool SocketController::init() {
    set_connected(false);
    _initialized = true;
    return true;
  } // SocketController::init

  bool SocketController::connect() {
    assert(_initialized == true);
    if ( is_connected() ) return false;

    init_connection();

    bool ok = connect(_con, _host, _port);
    if (!ok) return false;

    set_connected(true);
    _con->peer_str = SocketController::peer_str(_con->sock);
    onConnect(_con);

    return true;
  } // SocketController::connect

  bool SocketController::connect(Connection *con, const string &host, const int port) {
    int err;
    int ofcmode;

    log(LogInfo) << "Connecting to " << host << ":" << port << std::endl;

    /* Set up a TCP socket */
    con->sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(con->sock == -1) {
      log(LogError) << "Could not get socket." << std::endl;
      return false;
    } // if

    memset(&con->server_addr, '\0', sizeof(struct sockaddr_in));
    con->server_addr.sin_family      = AF_INET;
    con->server_addr.sin_port        = htons(port);       /* Server Port number */
    con->host_info = gethostbyname(host.c_str());

    if(con->host_info) {
      /* Take the first IP */
      struct in_addr *address = (struct in_addr *)con->host_info->h_addr_list[0];
      con->server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*address)); /* Server IP */
    } // if
    else {
      log(LogError) << "Could not resolve hostname " << host << std::endl;
      return false;
    } // if

    /* Establish a TCP/IP connection to the SSL client */
    err = ::connect(con->sock, (struct sockaddr*) &con->server_addr, sizeof(con->server_addr));
    if(err == -1) {
      log(LogError) << "Could not connect to " << host << ":" << port << std::endl;
      return false;
    } // if

    log(LogInfo) << "Connected to " << host << ":" << port << std::endl;

    // First we make the socket nonblocking
    ofcmode=fcntl(con->sock,F_GETFL,0);
    ofcmode|=O_NDELAY;
    if(fcntl(con->sock,F_SETFL,ofcmode)) {
      log(LogError) << "Could not set socket to non-blocking" << std::endl;
      return false;
    } // if

    con->peer_str = SocketController::peer_str(con->sock);
    return true;
  } // SocketController::connect

  bool SocketController::is_connected() {
    scoped_lock slock(&_connected_l);
    return _connected;
  } // SocketController::is_connected

  void SocketController::set_connected(const bool am_i_connected) {
    scoped_lock slock(&_connected_l);
    _connected = am_i_connected;
  } // SocketController::connected

  std::string SocketController::peer_str() {
    scoped_lock slock(&_peer_str_l);
    return _peer_str;
  } // SocketController::peer_str

  std::string SocketController::peer_str(const int sock) {
    std::stringstream s;
    s << SocketController::derive_peer_ip(sock)
      << ":"
      << SocketController::derive_peer_port(sock);
    return s.str();
  } // SocketController::peer_str

  void SocketController::set_peer_str(const std::string &str) {
    scoped_lock slock(&_peer_str_l);
    _peer_str = str;
  } // SocketController::set_peer_str

  void SocketController::set_peer_str(const int sock) {
    scoped_lock slock(&_peer_str_l);
    _peer_str = SocketController::peer_str(sock);
  } // SocketController::set_peer_str

  time_t SocketController::set_next_connect_attempt() {
    scoped_lock slock(&_connected_l);
    _next_connect_attempt = time(NULL) + _connect_retry_interval;
    return _next_connect_attempt;
  } // SocketController::set_next_connect_attempt

  bool SocketController::is_connect_time() {
    scoped_lock slock(&_connected_l);
    return (_next_connect_attempt < time(NULL) && !_connected);
  } // SocketController::is_connect_time

  const int SocketController::write(const char *packet, const size_t len) {
    int ret = -1;

    if (!_connected)
      return ret;

    ret = ::write(_con->sock, packet, len);

    // no error we're good
    if (ret)
      return ret;

    char *error = strerror(errno);

    //  If we got an error from select let's figure out why.
    switch(errno) {
      /******************
       ** Fatal Errors **
       ******************/
      // The argument s is not a socket.
      case ENOTSOCK:
      // The destination address is a broadcast address, and
      // SO_BROADCAST has not been set on the socket.
      case EACCES:
      // The receive buffer pointer(s) point outside the pro-
      // cess's address space.
      case EFAULT:
      // The socket is associated with a connection-oriented
      // protocol and has not been connected (see connect(2)
      // and accept(2)).
      case ENOTCONN:
      // The argument s is an invalid descriptor.
      case EBADF:
        // this list of cases should not be added to the keep
        // list, we want to remove them
        log(LogDebug) << "write error; " << error << std::endl;
        disconnect();
        break;
      /**********************
       ** Non-Fatal Errors **
       **********************/
      // The output queue for a network interface was full.
      // This generally indicates that the interface has
      // stopped sending, but may be caused by transient con-
      // gestion.
      case ENOBUFS:
      // make sure when this happens that we only
      // attempt to write again to this socket
      // after one second passes, prevents cpu hogging
      // The socket is marked non-blocking, and the receive
      // operation would block, or a receive timeout had been
      // set, and the timeout expired before data were received.
      case EAGAIN:
      default:
        break;
    } // switch

    return -1;
  } // SocketController::write

  const SocketController::socketControllerReturn_t SocketController::write(const int sock, const void *packet, const size_t len) {
    int ret = ::write(sock, packet, len);

    // no error we're good
    if (ret >= 0)
      return ret;

    _error = strerror(errno);
    _errno = errno;

    //  If we got an error from select let's figure out why.
    switch(errno) {
      /******************
       ** Fatal Errors **
       ******************/
      // The argument s is not a socket.
      case ENOTSOCK:
      // The destination address is a broadcast address, and
      // SO_BROADCAST has not been set on the socket.
      case EACCES:
      // The receive buffer pointer(s) point outside the pro-
      // cess's address space.
      case EFAULT:
      // The socket is associated with a connection-oriented
      // protocol and has not been connected (see connect(2)
      // and accept(2)).
      case ENOTCONN:
      // The argument s is an invalid descriptor.
      case EBADF:
        // this list of cases should not be added to the keep
        // list, we want to remove them
        log(LogDebug) << "write error; " << _error << std::endl;
        return SOCKETCONTROLLER_ERROR;
        break;
      /**********************
       ** Non-Fatal Errors **
       **********************/
      // The output queue for a network interface was full.
      // This generally indicates that the interface has
      // stopped sending, but may be caused by transient con-
      // gestion.
      case ENOBUFS:
      // make sure when this happens that we only
      // attempt to write again to this socket
      // after one second passes, prevents cpu hogging
      // The socket is marked non-blocking, and the receive
      // operation would block, or a receive timeout had been
      // set, and the timeout expired before data were received.
      case EAGAIN:
      default:
        return SOCKETCONTROLLER_FALSE;
        break;
    } // switch

    return SOCKETCONTROLLER_ERROR;
  } // SocketController::write

  const int SocketController::read(void *packet, const size_t len) {
    fd_set readfds;
    int ret = -1;

    if (!_connected)
      return ret;

    timeval timeout = {0, 100000};
    FD_ZERO(&readfds);
    FD_SET(_con->sock, &readfds);

    ret = ::select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);

    if (ret == SOCKETCONTROLLER_ERROR)
      return ret;

    if (!FD_ISSET(_con->sock, &readfds))
      return 0;

    ret = read(_con->sock, packet, len);

    if (ret > 0)
      return ret;

    disconnect();

    return ret;
  } // SocketController::read

  const int SocketController::select(fd_set *r_set, fd_set *w_set, timeval timeout) {
    int ret = -1;

    ret = ::select(FD_SETSIZE, r_set, w_set, NULL, &timeout);

    if (ret == -1) {
      _error = strerror(errno);
      _errno = errno;

      //  If we got an error from select let's figure out why.
      switch(errno) {
        case EBADF:
        // The specified time limit is invalid.  One of its components is negative or too large.
        case EINVAL:
        // A signal was delivered before the time limit expired and before any of the selected events occurred.
        case EINTR:
        // One of the descriptor sets specified an invalid descriptor.
        default:
          log(LogDebug) << "select error; " << _error << std::endl;
          return SOCKETCONTROLLER_ERROR;
          break;
      } // switch

      assert(false);	// bug
    } // if

    return ret;
  } // SocketController::select

  const SocketController::socketControllerReturn_t SocketController::read(const int sock, void *packet, const size_t len) {
    int ret = ::read(sock, packet, len);

    // we read something
    if (ret >= 0)
      return ret;

    _error = strerror(errno);
    _errno = errno;
    //  If we got an error from select let's figure out why.
    switch(errno) {
      /******************
       ** Fatal Errors **
       ******************/
      // The receive buffer pointer(s) point outside the pro-
      // cess's address space.
      case EFAULT:
      // The socket is associated with a connection-oriented
      // protocol and has not been connected (see connect(2)
      // and accept(2)).
      case ENOTCONN:
      // The argument s is an invalid descriptor.
      case EBADF:
        // this list of cases should not be added to the keep
        // list, we want to remove them
        // don't remove the connection until the buffer is cleared.
        log(LogDebug) << "read error; " << _error << std::endl;
        return SOCKETCONTROLLER_ERROR;
        break;
      /**********************
       ** Non-Fatal Errors **
       **********************/
      // The receive was interrupted by delivery of a signal
      // before any data were available.
      case EINTR:
      // The socket is marked non-blocking, and the receive
      // operation would block, or a receive timeout had been
      // set, and the timeout expired before data were received.
      case EAGAIN:
      default:
        return SOCKETCONTROLLER_FALSE;
        break;
    } // switch

    return SOCKETCONTROLLER_ERROR;
  } // SocketController::read

  bool SocketController::disconnect() {
    assert(_initialized == true);
    int err;

    if (!_connected) {
      log(LogError) << "Disconnect from " << _host << ":" << _port
                    << " attempted but not connected." << std::endl;
      return false;
    } // if

    log(LogInfo) << "Disconnecting from " << _host << ":" << _port << std::endl;
    onDisconnect(_con);

    /* Terminate communication on a socket */
    err = close(_con->sock);
    if(err == -1)
      log(LogInfo) << "Could not close socket with " << _host << ":" << _port << std::endl;

    deinit_connection();

    return true;
  } // SocketController::disconnect()

  void SocketController::init_connection() {
    // this should never happen
    if (_con) return;

    set_connected(false);
    _next_connect_attempt = time(NULL) + _connect_retry_interval;

    try {
      _con = new Connection;
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch
  } // SocketController::init_connection

  void SocketController::deinit_connection() {
    if (!_con) return;
    delete _con;
    _con = NULL;
    set_connected(false);
  } // SocketController::deinit_connection

  const char *SocketController::derive_ip(const int sock) {
    struct in_addr in;
    in.s_addr = htonl( SocketController::derive_networknum(sock) );
    return inet_ntoa(in);
  } // SocketController::derive_ip

  const char *SocketController::derive_peer_ip(const int sock) {
    struct in_addr in;
    in.s_addr = htonl( SocketController::derive_peer_networknum(sock) );
    return inet_ntoa(in);
  } // SocketController::derive_peer_ip

  const int SocketController::derive_port(const int sock) {
    int len;				// length
    int ret;				// results
    struct sockaddr_in sockaddr;	// address

    len = sizeof(sockaddr_in);
    ret = getsockname(sock, (struct sockaddr *) &sockaddr, (socklen_t *) &len);

    if (ret == -1)
      return SOCKETCONTROLLER_ERROR;

    return ntohs(sockaddr.sin_port);
  } // SocketController::derive_port

  const int SocketController::derive_peer_port(const int sock) {
    int len;				// length
    int ret;				// results
    struct sockaddr_in sockaddr;	// address

    len = sizeof(sockaddr_in);
    ret = getpeername(sock, (struct sockaddr *) &sockaddr, (socklen_t *) &len);

    if (ret == -1)
      return SOCKETCONTROLLER_ERROR;

    return ntohs(sockaddr.sin_port);
  } // SocketController::derive_port

  const unsigned long SocketController::derive_networknum(const int sock) {
    int len;				// length
    int ret;				// return
    struct sockaddr_in sockaddr;	// address

    len = sizeof(sockaddr_in);
    ret = getsockname(sock, (struct sockaddr *) &sockaddr, (socklen_t *) &len);

    if (ret == -1)
      return SOCKETCONTROLLER_ERROR;

    return ntohl(sockaddr.sin_addr.s_addr);
  } // SocketController::derive_networknum

  const unsigned long SocketController::derive_peer_networknum(const int sock) {
    int len;				// length
    int ret;				// results
    struct sockaddr_in sockaddr;	// address

    len = sizeof(sockaddr_in);
    ret = getpeername(sock, (struct sockaddr *) &sockaddr, (socklen_t *) &len);

    if (ret == -1)
      return SOCKETCONTROLLER_FALSE;

    return ntohl(sockaddr.sin_addr.s_addr);
  } // SocketController::derive_peer_networknum

} // namespace smpp


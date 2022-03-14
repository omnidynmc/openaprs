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

#include <openframe/SocketBase.h>
#include <openframe/scoped_lock.h>

namespace openframe {
  using namespace loglevel;

/**************************************************************************
 ** SocketBase Class                                                     **
 **************************************************************************/
  SocketBase::SocketBase() {
  } // SocketBase

  SocketBase::~SocketBase() {
  } // SocketBase::~SocketBase

  bool SocketBase::connect(Connection *con, const std::string &host, const int port, const std::string &bind_ip) {
    assert(con != NULL);
    int err;
    int ofcmode;

    /* Set up a TCP socket */
    con->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(con->sock == -1) {
      _error = strerror(errno);
      _errno = errno;
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
      _error = hstrerror(h_errno);
      _errno = h_errno;
      return false;
    } // if

    if (bind_ip.length() > 0) {
      struct sockaddr_in la;
      struct hostent *vh;

      if ((vh = gethostbyname(bind_ip.c_str())) == NULL) {
        _error = hstrerror(h_errno);
        _errno = h_errno;
        return false;
      } // if

      bzero(&la, sizeof(la));
      bcopy(vh->h_addr, (char *) &la.sin_addr, vh->h_length);
      la.sin_family = vh->h_addrtype;
      la.sin_port = 0;

      if (bind(con->sock, (struct sockaddr *) & la, sizeof(la)) == -1) {
        close(con->sock);
        _error = strerror(errno);
        _errno = errno;
        return false;
      } // if
    }   // if

    /* Establish a TCP/IP connection to the SSL client */
    err = ::connect(con->sock, (struct sockaddr*) &con->server_addr, sizeof(con->server_addr));
    if(err == -1) {
      _error = strerror(errno);
      _errno = errno;
      return false;
    } // if

    // First we make the socket nonblocking
    ofcmode=fcntl(con->sock,F_GETFL,0);
    ofcmode|=O_NDELAY;
    if(fcntl(con->sock,F_SETFL,ofcmode)) {
      _error = strerror(errno);
      _errno = errno;
      return false;
    } // if

    std::stringstream s;
    s << host << ":" << port;
    con->host_str = s.str();
    con->peer_str = SocketBase::derive_peer_str(con->sock);
    return true;
  } // SocketBase::connect

  std::string SocketBase::derive_peer_str(const int sock) {
    std::stringstream s;
    s << SocketBase::derive_peer_ip(sock)
      << ":"
      << SocketBase::derive_peer_port(sock);
    return s.str();
  } // SocketBase::derive_peer_str

  SocketBase::socketBaseReturn_t SocketBase::write(const int sock, const void *packet, const size_t len) {
    int ret = ::write(sock, packet, len);

    // no error we're good
    if (ret >= 0) return ret;

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
        //log(LogDebug) << "SocketBase write error; " << _error << std::endl;
        return SOCKETBASE_ERROR;
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
        return SOCKETBASE_FALSE;
        break;
    } // switch

    return SOCKETBASE_ERROR;
  } // SocketBase::write

  SocketBase::socketBaseReturn_t SocketBase::select(fd_set *r_set, fd_set *w_set, timeval *timeout) {
    int ret = -1;

    ret = ::select(FD_SETSIZE, r_set, w_set, NULL, timeout);

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
          //log(LogDebug) << "SocketBase select error; " << _error << std::endl;
          return SOCKETBASE_ERROR;
          break;
      } // switch

      assert(false);	// bug
    } // if

    return ret;
  } // SocketBase::select

  SocketBase::socketBaseReturn_t SocketBase::read(const int sock, void *packet, const size_t len) {
    int ret = ::read(sock, packet, len);

    // we read something
    if (ret >= 0) return ret;

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
        //log(LogDebug) << "SocketBase read error; " << _error << std::endl;
        return SOCKETBASE_ERROR;
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
        return SOCKETBASE_FALSE;
        break;
    } // switch

    return SOCKETBASE_ERROR;
  } // SocketBase::read

  const char *SocketBase::derive_ip(const int sock) {
    struct in_addr in;
    in.s_addr = htonl( SocketBase::derive_networknum(sock) );
    return inet_ntoa(in);
  } // SocketBase::derive_ip

  const char *SocketBase::derive_peer_ip(const int sock) {
    struct in_addr in;
    in.s_addr = htonl( SocketBase::derive_peer_networknum(sock) );
    return inet_ntoa(in);
  } // SocketBase::derive_peer_ip

  const int SocketBase::derive_port(const int sock) {
    int len;				// length
    int ret;				// results
    struct sockaddr_in sockaddr;	// address

    len = sizeof(sockaddr_in);
    ret = getsockname(sock, (struct sockaddr *) &sockaddr, (socklen_t *) &len);

    if (ret == -1)
      return SOCKETBASE_ERROR;

    return ntohs(sockaddr.sin_port);
  } // SocketBase::derive_port

  const int SocketBase::derive_peer_port(const int sock) {
    int len;				// length
    int ret;				// results
    struct sockaddr_in sockaddr;	// address

    len = sizeof(sockaddr_in);
    ret = getpeername(sock, (struct sockaddr *) &sockaddr, (socklen_t *) &len);

    if (ret == -1)
      return SOCKETBASE_ERROR;

    return ntohs(sockaddr.sin_port);
  } // SocketBase::derive_port

  const unsigned long SocketBase::derive_networknum(const int sock) {
    int len;				// length
    int ret;				// return
    struct sockaddr_in sockaddr;	// address

    len = sizeof(sockaddr_in);
    ret = getsockname(sock, (struct sockaddr *) &sockaddr, (socklen_t *) &len);

    if (ret == -1)
      return SOCKETBASE_ERROR;

    return ntohl(sockaddr.sin_addr.s_addr);
  } // SocketBase::derive_networknum

  const unsigned long SocketBase::derive_peer_networknum(const int sock) {
    int len;				// length
    int ret;				// results
    struct sockaddr_in sockaddr;	// address

    len = sizeof(sockaddr_in);
    ret = getpeername(sock, (struct sockaddr *) &sockaddr, (socklen_t *) &len);

    if (ret == -1)
      return SOCKETBASE_FALSE;

    return ntohl(sockaddr.sin_addr.s_addr);
  } // SocketBase::derive_peer_networknum
} // namespace smpp


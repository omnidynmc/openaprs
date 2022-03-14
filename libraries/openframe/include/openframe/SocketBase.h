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

#ifndef __LIBOPENFRAME_SOCKETBASE_H
#define __LIBOPENFRAME_SOCKETBASE_H

#include <netdb.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "OpenFrame_Abstract.h"

namespace openframe {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define SOCKETBASE_TRUE			1
#define SOCKETBASE_FALSE		0
#define SOCKETBASE_ERROR		-1
//#define SOCKETBASE_MTU			1500
#define SOCKETBASE_MTU			100000

#ifndef MAX
# define MAX(a, b)				(a > b ? a : b)
#endif

#ifndef MIN
# define MIN(a, b)				(a < b ? a : b)
#endif

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  typedef struct {
      // ### Socket Communications ###
      struct sockaddr_in	server_addr;
      struct hostent		*host_info;
      int			sock;
      std::string		peer_str;
      std::string		host_str;
  } Connection;

  class SocketBase_Exception : public OpenFrame_Exception {
    public:
      SocketBase_Exception(const string message) throw() : OpenFrame_Exception(message) {
      } // OpenFrame_Exception

    private:
  }; // class SocketBase_Exception

  class SocketBase : public OpenFrame_Abstract {
    public:
      // ### Type Definitions ###
      typedef int socketBaseReturn_t;
      typedef sockaddr_in SocketType;

      // ### Constructors/Destructor ###
      SocketBase();
      virtual ~SocketBase();

      // ### Static Members ####
      static const socketBaseReturn_t derive_port(const int sock);
      static const socketBaseReturn_t derive_peer_port(const int sock);
      static const unsigned long derive_networknum(const int sock);
      static const unsigned long derive_peer_networknum(const int sock);
      static const char *derive_ip(const int sock);
      static const char *derive_peer_ip(const int sock);
      static std::string derive_peer_str(const int sock);

      // ### Object Members ###
      bool connect(Connection *, const std::string &host, const int port, const std::string &bind_ip="");
      socketBaseReturn_t read(const int, void *, size_t);
      socketBaseReturn_t write(const int, const void *, size_t);
      socketBaseReturn_t select(fd_set *r_set, fd_set *w_set) {
        timeval timeout = { 0, 10000 };
        return select(r_set, w_set, &timeout);
      };
      socketBaseReturn_t select(fd_set *, fd_set *, timeval *timeout);
      inline const char *error() const { return _error; }
      inline const int errnum() const { return _errno; }

    protected:
      const char *_error;
      int _errno;
    private:
  }; // class SocketBase

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openframe
#endif

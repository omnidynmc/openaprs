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

#ifndef __LIBOPENFRAME_SOCKETCONTROLLER_H
#define __LIBOPENFRAME_SOCKETCONTROLLER_H

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

#define SOCKETCONTROLLER_TRUE			1
#define SOCKETCONTROLLER_FALSE			0
#define SOCKETCONTROLLER_ERROR			-1
#define SOCKETCONTROLLER_MTU			1500

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
    /* Socket Communications */
    struct sockaddr_in   server_addr;
    struct hostent      *host_info;
    int                  sock;
    std::string		peer_str;
} Connection;

class SocketController_Exception : public OpenFrame_Exception {
  public:
    SocketController_Exception(const string message) throw() : OpenFrame_Exception(message) {
    } // OpenFrame_Exception

  private:
}; // class SocketController_Exception

class SocketController : public OpenFrame_Abstract {
  public:
    SocketController();
    SocketController(const string &, const int);
    virtual ~SocketController();
    bool init();

    static const time_t DEFAULT_CONNECT_INTERVAL;

    /**********************
     ** Type Definitions **
     **********************/
    typedef int socketControllerReturn_t;
    typedef sockaddr_in SocketType;

    /***************
     ** Variables **
     ***************/

    const inline string &host() const { return _host; }
    const inline int port() const { return _port; }

    bool is_connected();
    bool is_connect_time();
    time_t set_next_connect_attempt();

    std::string peer_str();
    static std::string peer_str(const int sock);
    void set_peer_str(const std::string &str);
    void set_peer_str(const int sock);

    bool connect();
    bool connect(Connection *, const std::string &host, const int port);
    bool disconnect();

    // socket tools
    static const socketControllerReturn_t derive_port(const int);
    static const socketControllerReturn_t derive_peer_port(const int);
    static const unsigned long derive_networknum(const int);
    static const unsigned long derive_peer_networknum(const int);
    static const char *derive_ip(const int);
    static const char *derive_peer_ip(const int);

    virtual void onDisconnect(const Connection *) = 0;
    virtual void onConnect(const Connection *) = 0;
    virtual const int write(const char *, size_t);
    virtual const int read(void *, size_t);

    const socketControllerReturn_t read(const int, void *, size_t);
    const socketControllerReturn_t write(const int, const void *, size_t);
    const socketControllerReturn_t select(fd_set *r_set, fd_set *w_set) {
      timeval timeout = { 0, 10000 };
      return select(r_set, w_set, timeout);
    };
    const socketControllerReturn_t select(fd_set *, fd_set *, timeval timeout);

    inline const char *error() const { return _error; }
    inline const int errnum() const { return _errno; }

  protected:
    void set_connected(const bool am_i_connected);
    void init_connection();
    void deinit_connection();

    const char *_error;
    int _errno;

    // constructor initialized variables
    bool _connected;
    OFLock _connected_l;
    string _host;
    int _port;
    time_t _connect_retry_interval;
    Connection *_con;

    time_t _next_connect_attempt;

  private:

    bool _initialized;

    std::string _peer_str;
    OFLock _peer_str_l;

}; // class SocketController

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

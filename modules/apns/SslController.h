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

#ifndef __MODULE_SSLCONTROLLER_H
#define __MODULE_SSLCONTROLLER_H

#include <netdb.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "ApnsAbstract.h"
#include "Log.h"
#include "Server.h"

namespace apns {

using namespace openaprs;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

typedef struct {
    /* SSL Vars */
    SSL_CTX         *ctx;
    SSL             *ssl;
    SSL_METHOD      *meth;
    X509            *server_cert;
    EVP_PKEY        *pkey;
    BIO             *sbio;

    /* Socket Communications */
    struct sockaddr_in   server_addr;
    struct hostent      *host_info;
    int                  sock;
} SSL_Connection;

class SslController : public ApnsAbstract {
  public:
    SslController(const string &, const int, const string &, const string &, const string &);
    virtual ~SslController();

    /**********************
     ** Type Definitions **
     **********************/

    /***************
     ** Variables **
     ***************/

    const inline string &host() const { return _host; }
    const inline int port() const { return _port; }
    const inline string &certfile() const { return _certfile; }
    const inline string &keyfile() const { return _keyfile; }
    const inline string &capath() const { return _capath; }

    const bool isConnected() { return _connected; }
    const bool connect() { return _connect(); }
    const bool disconnect() { return _disconnect(); }
    const int write(const char *, size_t);
    const int read(void *, size_t);

  protected:
  private:
    const bool _connect();
    const bool _disconnect();
    const bool _checkCert();
    void _initialize();
    void _deinitialize();

    // *** Variables ***
    bool _initialized;
    bool _connected;
    string _host;
    int _port;
    string _certfile;
    string _keyfile;
    string _capath;

    SSL_Connection *_sslcon;
}; // APNS

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

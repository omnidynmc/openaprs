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

#ifndef __OPENAPRS_MEMCACHEDCONTROLLER_H
#define __OPENAPRS_MEMCACHEDCONTROLLER_H

#include <set>

#include <netdb.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <libmemcached/memcached.h>

#include "OpenAPRS_Abstract.h"

#include "openframe/OFLock.h"

namespace openaprs {
  using openframe::OFLock;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

class MemcachedController_Exception : public OpenAPRSAbstract_Exception {
  public:
    MemcachedController_Exception(const string message) throw() : OpenAPRSAbstract_Exception(message) {
    } // OpenAbstract_Exception

  private:
}; // class MemcachedController_Exception

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class MemcachedController : public OpenAPRS_Abstract {
  public:
    MemcachedController(const string &);
    virtual ~MemcachedController();

    // ### Type Definitions ###
    enum memcachedReturnEnum {
      MEMCACHED_CONTROLLER_NOTFOUND,
      MEMCACHED_CONTROLLER_SUCCESS,
      MEMCACHED_CONTROLLER_ERROR
    };

    // ### Members ###
    const memcachedReturnEnum get(const string &, const string &, string &);
    void put(const string &, const string &, const string &);
    void put(const string &, const string &, const string &, const time_t);
    void replace(const string &, const string &, const string &);
    void replace(const string &, const string &, const string &, const time_t);
    void remove(const string &, const string &);
    void flush(const time_t);
    void expire(const time_t expire) { _expire = expire; }
    const time_t expire() const { return _expire; }

    // ### Variables ###

  protected:
  private:
    // ### Variables ###
    memcached_server_st *_servers;			// memcached server list
    memcached_st *_st;					// memcached instance
    string _memcachedServers;				// server list initialized
    time_t _expire;
}; // MemcachedController

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

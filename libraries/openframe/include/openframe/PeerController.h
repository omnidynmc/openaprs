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

#ifndef __LIBOPENFRAME_PEERCONTROLLER_H
#define __LIBOPENFRAME_PEERCONTROLLER_H

#include <vector>

#include "ConnectionManager.h"

namespace openframe {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class Host {
    public:
      Host(const std::string &hosts);
      virtual ~Host();

      std::string host() const { return _host; }
      int port() const { return _port; }

    private:
      std::string _host;
      int _port;
  }; // class Host


  class PeerController : public ConnectionManager {
    public:

      // ### Static Variables ###
      static const time_t kDefaultConnectRetryInterval;
      static const time_t kDefaultConnectReadTimeout;

      // ### Constructors/Destructor ###
      PeerController(const std::string &hosts, const std::string &bind_ip="");
      virtual ~PeerController();
      PeerController &init();
      PeerController &start();

      // ### Configuration Members ###
      bool is_connected();
      std::string connected_to();
      bool is_connecting();
      bool is_connect_time();
      bool try_connect();
      unsigned int num_connects();
      unsigned int num_disconnects();
      time_t set_next_connect_attempt();
      PeerController &set_connect_retry_interval(const time_t connect_retry_interval) {
        scoped_lock slock(&_next_connect_attempt_l);
        _connect_retry_interval = connect_retry_interval;
        return *this;
      } // set_connect_retry_interval
      time_t connect_retry_interval() {
        scoped_lock slock(&_next_connect_attempt_l);
        return _connect_retry_interval;
      } // connect_retry_interval
      PeerController &set_connect_read_timeout(const time_t connect_read_timeout) {
        scoped_lock slock(&_next_connect_attempt_l);
        _connect_read_timeout = connect_read_timeout;
        return *this;
      } // set_connect_read_timeout

      inline std::string::size_type in(std::string &buf) {
        scoped_lock slock(&_in_l);
        std::string::size_type ret = _in.size();
        buf.append(_in);
        _in = "";
        return ret;
      } // in
      inline std::string in() {
        scoped_lock slock(&_in_l);
        std::string buf = _in;
        _in = "";
        return buf;
      } // in
      inline void out(const std::string &buf) {
        scoped_lock slock(&_out_l);
        _out.append(buf);
        set_pending_write(true);
      } // out

      // ### ConnectionManager Pure Virtuals ###
      virtual void onConnect(const Connection *con);
      virtual void onConnectTimeout(const Peer *peer);
      virtual void onTryConnect(const std::string &host, const int port);
      virtual void onConnectError(const std::string &host, const int port, const char *error);
      virtual void onDisconnect(const Connection *con); // to detect disconnects and set_connected
      virtual void onRead(const Peer *);
      virtual const std::string::size_type onWrite(const Peer *peer, std::string &);
      virtual bool onTick();
      virtual bool onPeerWake(const Peer *peer);

    protected:
      typedef std::vector<Host *> hosts_t;
      typedef hosts_t::size_type hosts_st;

      Host *rotate_server();
      void set_connected(const bool am_i_connected);
      void set_connected_to(const std::string &host_str);
      void set_connecting(const bool am_i_connecting);
      unsigned int inc_num_connects();
      unsigned int inc_num_disconnects();

      // ### Constructor Initialized Variables ###
    private:
      bool _connected;
      OFLock _connected_l;
      std::string _hosts_str;
      std::string _bind_ip;
      time_t _connect_retry_interval;
      time_t _connect_read_timeout;
      unsigned int _num_connects;
      unsigned int _num_disconnects;
      OFLock _num_counters_l;

      time_t _next_connect_attempt;
      OFLock _next_connect_attempt_l;

      std::string _in;
      OFLock _in_l;
      std::string _out;
      OFLock _out_l;

      bool _connecting;
      OFLock _connecting_l;
      std::string _connected_to;

      hosts_t _hosts;
      size_t _host_index;
  }; // class PeerController

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openframe
#endif

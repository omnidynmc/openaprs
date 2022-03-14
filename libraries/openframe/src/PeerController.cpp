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

#include <openframe/PeerController.h>
#include <openframe/StringToken.h>
#include <openframe/scoped_lock.h>

namespace openframe {
  using namespace loglevel;

/**************************************************************************
 ** Host Class                                                           **
 **************************************************************************/

  Host::Host(const std::string &host) {
    StringToken st;
    st.setDelimiter(':');
    st = host;

    if (st.size() != 2) throw SocketBase_Exception("invalid host:port pair");
    _host = st[0];
    _port = atoi( st[1].c_str() );
  } // Host::Host

  Host::~Host() {
  } // Host::~Host


/**************************************************************************
 ** PeerController Class                                                 **
 **************************************************************************/
  const time_t PeerController::kDefaultConnectRetryInterval		= 15;
  const time_t PeerController::kDefaultConnectReadTimeout		= 120;

  PeerController::PeerController(const std::string &hosts_str, const std::string &bind_ip)
                 : _connected(false),
                   _hosts_str(hosts_str),
                   _bind_ip(bind_ip),
                   _connect_retry_interval(kDefaultConnectRetryInterval),
                   _connect_read_timeout(kDefaultConnectReadTimeout),
                   _num_connects(0),
                   _num_disconnects(0) {
  } // PeerController::PeerController

  PeerController::~PeerController() {
    ConnectionManager::stop();
    for(hosts_st i=0; i < _hosts.size(); i++)
      delete _hosts[i];
  } // PeerController::~PeerController

  PeerController &PeerController::init() {
    StringToken st;
    st.setDelimiter(',');
    st = _hosts_str;

    if (st.size() < 1) throw SocketBase_Exception("invalid hosts list");

    for(size_t i=0; i < st.size(); i++) {
      Host *host = new Host(st[i]);
      _hosts.push_back(host);
    } // for

    _host_index = 0;
    set_connected(false);
    set_connecting(false);
    set_connected_to("not connected");
    set_next_connect_attempt();
    return *this;
  } // PeerController::init

  PeerController &PeerController::start() {
    ConnectionManager::start();
    try_connect();
    return *this;
  } // PeerController::start

  Host *PeerController::rotate_server() {
    assert(_hosts.size() != 0);
    unsigned int index = _host_index++ % _hosts.size();
    return _hosts[index];
  } // PeerController::rotate_server

  bool PeerController::is_connected() {
    scoped_lock slock(&_connected_l);
    return _connected;
  } // PeerController::is_connected

  std::string PeerController::connected_to() {
    scoped_lock slock(&_connected_l);
    return _connected_to;
  } // PeerController::connected_to

  void PeerController::set_connected(const bool am_i_connected) {
    scoped_lock slock(&_connected_l);
    _connected = am_i_connected;
  } // PeerController::connected

  void PeerController::set_connected_to(const std::string &host_str) {
    scoped_lock slock(&_connected_l);
    _connected_to = host_str;
  } // PeerController::set_connected_to

  bool PeerController::is_connecting() {
    scoped_lock slock(&_connecting_l);
    return _connecting;
  } // PeerController::is_connecting

  void PeerController::set_connecting(const bool am_i_connecting) {
    scoped_lock slock(&_connecting_l);
    _connecting = am_i_connecting;
  } // PeerController::connecting

  unsigned int PeerController::num_connects() {
    scoped_lock slock(&_num_counters_l);
    return _num_connects;
  } // PeerController::num_connects

  unsigned int PeerController::inc_num_connects() {
    scoped_lock slock(&_num_counters_l);
    return ++_num_connects;
  } // PeerController::inc_num_connects

  unsigned int PeerController::num_disconnects() {
    scoped_lock slock(&_num_counters_l);
    return _num_disconnects;
  } // PeerController::num_disconnects

  unsigned int PeerController::inc_num_disconnects() {
    scoped_lock slock(&_num_counters_l);
    return ++_num_disconnects;
  } // PeerController::inc_num_disconnects

  time_t PeerController::set_next_connect_attempt() {
    scoped_lock slock(&_next_connect_attempt_l);
    _next_connect_attempt = time(NULL) + _connect_retry_interval;
    return _next_connect_attempt;
  } // PeerController::set_next_connect_attempt

  bool PeerController::is_connect_time() {
    scoped_lock slock(&_next_connect_attempt_l);
    return (_next_connect_attempt < time(NULL) && !is_connected() && !is_connecting() );
  } // PeerController::is_connect_time

  bool PeerController::try_connect() {
    set_connecting(true);
    inc_num_connects();
    set_next_connect_attempt();
    Host *host = rotate_server();

    openframe::Peer *peer;
    try {
      peer = new Peer;
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch

    onTryConnect(host->host(), host->port());
    bool ok = SocketBase::connect(peer, host->host(), host->port(), _bind_ip);
    if (!ok) {
      onConnectError(host->host(), host->port(), error());
      delete peer;
      set_connecting(false);
      return false;
    } // if

    register_connection(peer);
    set_connected(true);
    set_connecting(false);
    return true;
  } // PeerController::try_connect

  bool PeerController::onPeerWake(const Peer *peer) {
    scoped_lock slock(&_next_connect_attempt_l);
    if (!_connect_read_timeout) return false;
    bool want_disconnect = peer->last_read < time(NULL) - _connect_read_timeout;
    if (want_disconnect) {
      onConnectTimeout(peer);
      safe_disconnect(peer->sock);
    } // if
    return want_disconnect;
  } // PeerController::onWakePeer

  bool PeerController::onTick() {
    if ( is_connected() || is_connecting() ) return false;
    if ( is_connect_time() ) try_connect();
    return true;
  } // PeerController::onTick

  void PeerController::onConnectTimeout(const Peer *peer) {
    LOG(LogDebug, << "Connection to "
                  << peer->host_str
                  << " timed out after "
                  << time(NULL) - peer->last_read
                  << " seconds"<< std::endl);
  } // PeerController::onConnectTimeout

  void PeerController::onConnect(const Connection *con) {
    set_connected(true);
    set_connected_to(con->host_str);
    scoped_lock slock_in(&_in_l);
    scoped_lock slock_out(&_out_l);
    _in = "";
    _out = "";
    LOG(LogDebug, << "PeerController connected to " << con->host_str << std::endl);
  } // PeerController::onConnect

  void PeerController::onTryConnect(const std::string &host, const int port) {
  } // PeerController::onTryConnect

  void PeerController::onConnectError(const std::string &host, const int port, const char *error) {
  } // PeerController::onConnectError

  void PeerController::onDisconnect(const Connection *con) {
    set_connected(false);
    set_connected_to("not connected");
    inc_num_disconnects();
    LOG(LogDebug, << "PeerController disconnected from " << con->host_str << std::endl);
  } // PeerController::onDisconnect

  void PeerController::onRead(const openframe::Peer *peer) {
    scoped_lock slock(&_in_l);
    _in.append(peer->in, peer->in_len);
  } // PeerController::onRead

  const std::string::size_type PeerController::onWrite(const openframe::Peer *peer, std::string &ret) {
    scoped_lock slock(&_out_l);
    ret.append(_out);
    _out = "";
    return ret.size();
  } // PeerController::onWrite
} // namespace smpp


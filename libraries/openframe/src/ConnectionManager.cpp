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

#include <openframe/ConnectionManager.h>
#include <openframe/StringTool.h>
#include <openframe/scoped_lock.h>

namespace openframe {
  using namespace openframe::loglevel;

/**************************************************************************
 ** ConnectionManager Class                                              **
 **************************************************************************/
  const time_t ConnectionManager::kDisconnectExpire 	= 2;
  const time_t ConnectionManager::kPeerStatsInterval 	= 300;

  ConnectionManager::ConnectionManager()
                    : _initialized(false),
                      _use_cm_thread(true),
                      _use_cm_manager(true),
                      _done(false),
                      _pending_write(false),
                      _peer_stats_interval(kPeerStatsInterval) {
  } // ConnectionManager::ConnectionManager

  ConnectionManager::~ConnectionManager() {
    stop();
  } // ConnectionManager::~ConnectionManager

  ConnectionManager &ConnectionManager::start() {
    assert(_initialized != true);
    //super::start();
    _initialize();
    return (ConnectionManager &) *this;
  } // ConnectionManager::start

  void ConnectionManager::stop() {
    if (!_initialized) return;
    _deinitialize();
  } // ConnectionManager::stop

  bool ConnectionManager::transfers() {
    peers_itr itr;
    list<int> rm;

    scoped_lock slock(&_peers_l);

    fd_set fd_set_r = _fd_set_r;
    fd_set fd_set_w = _fd_set_w;

    socketBaseReturn_t ret, write_ret = 0;

    timeval timeout = { 0, 10000 };

    if ( is_pending_write() ) {
      ret = select(&fd_set_r, &fd_set_w);
      set_pending_write(false);
    } // if
    else {
      ret = SocketBase::select(&fd_set_r, NULL, &timeout);
      timeout.tv_sec = 0;
      timeout.tv_usec = 10000;
      write_ret = SocketBase::select(NULL, &fd_set_w, &timeout);
    } // else

    if (ret < 0 || write_ret < 0) return false;

    int did_work = 0;
    for(itr = _peers.begin(); itr != _peers.end(); itr++) {
      Peer *peer = itr->second;
      try_peer_stats(peer);

      if (!peer->disconnect && onPeerWake(peer)) did_work++;

      bool read_ok = FD_ISSET(peer->sock, &fd_set_r);

      if (read_ok) {
        char packet[SOCKETBASE_MTU];
        int bytes_read = SocketBase::read(peer->sock, packet, SOCKETBASE_MTU);

        if (bytes_read < 1) {
          // this one's gotta go
          rm.push_back(peer->sock);
          continue;
        } // if

        peer->in = packet;
        peer->in_len = bytes_read;
        peer->last_read = time(NULL);

        // log our stats
        peer->peer_stats.num_bytes_read += bytes_read;
        peer->peer_stats.num_packets_read++;
        peer->peer_stats.last_read_at = time(NULL);

        onRead(peer);

        peer->in_len = 0;
      } // if

      std::string ret;
      if ( onWrite(peer, ret) ) {
        peer->out.append(ret);
        set_pending_write(true);
      } // if

      bool want_write = peer->out.size();
      bool write_ok = want_write && FD_ISSET(peer->sock, &fd_set_w);
      if (write_ok) {
        int bytes_written = SocketBase::write(peer->sock, peer->out.data(), MIN(SOCKETBASE_MTU, peer->out.length()) );

        if (bytes_written < 1) {
          // this one's gotta go
          rm.push_back(peer->sock);
          continue;
        } // if

        if (bytes_written) {
          peer->out.erase(0, bytes_written);

          // log stats
          peer->peer_stats.num_bytes_written += bytes_written;
          peer->peer_stats.num_packets_written++;
          peer->peer_stats.last_write_at = time(NULL);
        } // if
      } // if

      // don't try and disconnect until the input/output buffer is flushed
      bool doDisconnect = peer->disconnect
                          && (peer->out.length() < 1 || peer->disconnect_expire < time(NULL)-kDisconnectExpire);
      if (doDisconnect) rm.push_back(peer->sock);
      if (read_ok || write_ok) did_work++;
    } // for

    while(!rm.empty()) {
      notify_socket_disconnected( rm.front() );
      rm.pop_front();
    } // while

    return (did_work > 0);
  } // ConnectionManager::transfers

  bool ConnectionManager::try_peer_stats(Peer *peer) {
    if (peer->peer_stats.last_stats_at + _peer_stats_interval > time(NULL)) return false;

    onPeerStats(peer);

    // reset stats for next period
    peer->peer_stats.num_bytes_read = 0;
    peer->peer_stats.num_bytes_written = 0;
    peer->peer_stats.num_packets_read = 0;
    peer->peer_stats.num_packets_written = 0;
    peer->peer_stats.last_stats_at = time(NULL);

    return true;
  } // ConnectionManager::try_peer_stats

  const bool ConnectionManager::write(const int sock, const std::string &buf) {
    peers_itr itr;

    scoped_lock slock(&_peers_l);

    itr = _peers.find(sock);
    if (itr == _peers.end()) return false;

    Peer *peer = itr->second;
    peer->out.append(buf);
    return true;
  } // ConnectionManager::write

  void ConnectionManager::register_connection(const int sock) {
    scoped_lock slock(&_peers_l);
    notify_socket_connected(sock);
  } // ConnectionManager::register_connection

  void ConnectionManager::register_connection(Peer *peer) {
    scoped_lock slock(&_peers_l);
    notify_socket_connected(peer);
  } // ConnectionManager::register_connection

  void ConnectionManager::unregister_connection(const int sock) {
    scoped_lock slock(&_peers_l);
    notify_socket_disconnected(sock);
  } // ConnectionManager::register_connection

  void ConnectionManager::notify_socket_connected(const int sock) {
    Peer *peer;
    try {
      peer = new Peer;
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch

    peer->sock = sock;
    init_peer(peer);

    // only track if we are employed to manage
    if (_use_cm_manager) {
      // add to socket sets
      FD_SET(sock, &_fd_set_r);
      FD_SET(sock, &_fd_set_w);

      _peers.insert(pair<int, Peer *>(sock, peer));
    } // if

    onConnect(peer);
    return;
  } // ConnectionManager::notify_socket_connected

  void ConnectionManager::notify_socket_connected(Peer *peer) {
    init_peer(peer);

    // only track if we are employed to manage
    if (_use_cm_manager) {
      // add to socket sets
      FD_SET(peer->sock, &_fd_set_r);
      FD_SET(peer->sock, &_fd_set_w);

      _peers.insert( std::make_pair(peer->sock, peer) );
    } // if

    onConnect(peer);
    return;
  } // ConnectionManager::notify_socket_connected

  const bool ConnectionManager::notify_socket_disconnected(const int sock) {
    peers_itr itr;

    itr = _peers.find(sock);
    if (itr == _peers.end()) return false;

    Peer *peer = itr->second;

    FD_CLR(peer->sock, &_fd_set_r);
    FD_CLR(peer->sock, &_fd_set_w);

    onDisconnect(peer);
    close(peer->sock);

    delete peer;
    _peers.erase(itr);
    return true;
  } // ConnectionManager::notify_socket_disconnect

  void ConnectionManager::init_peer(Peer *peer) {
    peer->last_read = time(NULL);
    peer->in_len = 0;
    peer->disconnect_expire = time(NULL);
    peer->peer_str = derive_peer_str(peer->sock);
    if (!peer->host_str.length()) peer->host_str = peer->peer_str;

    // log stats
    memset(&peer->peer_stats, '\0', sizeof(Peer::peer_stats_t) );
    peer->peer_stats.connected_at = time(NULL);
    peer->peer_stats.last_read_at = time(NULL);
    peer->peer_stats.last_write_at = time(NULL);
    peer->peer_stats.last_stats_at = time(NULL);
  } // ConnectionManager::init_peer

  // Don't call this from an inheritted class that
  // also locks it's list of peers, this will cause
  // a deadlock with transfers and onWrite
  bool ConnectionManager::disconnect(const int sock) {
    scoped_lock slock(&_peers_l);
    return safe_disconnect(sock);
  } // ConnectionManager::disconnect

  // Call this from any of the on* events
  // onConnect, onWakePeer, onWrite, onRead, onDisconnect
  bool ConnectionManager::safe_disconnect(const int sock) {
    peers_itr itr;

    itr = _peers.find(sock);
    if (itr == _peers.end()) return false;

    Peer *peer = itr->second;
    peer->disconnect = true;
    peer->disconnect_expire = time(NULL);
    return true;
  } // ConnectionManager::safe_disconnect

  void ConnectionManager::_initialize() {
    FD_ZERO(&_fd_set_r);
    FD_ZERO(&_fd_set_w);

    _initializeThreads();
    _initialized = true;
  } // ConnectionManager::_initialize

  void ConnectionManager::_initializeThreads() {
    if (!_use_cm_thread) return;

    pthread_create(&_thread_tid, NULL, ConnectionManager::thread, this);
//    _debugf("*** ConnectionManager: Started Peer Thread %u", _thread_tid);
    LOG(LogInfo, << "ConnectionManager: Started Peer Thread "
                 << _thread_tid
                 << std::endl);
  } // ConnectionManager::_initializeThreads

  void ConnectionManager::_deinitialize() {
    if (!_initialized)
      return;

    _initialized = false;
    set_done(true);
    _deinitializeThreads();

    peers_itr itr;
    list<int> rm;

    scoped_lock slock(&_peers_l);
    for(itr = _peers.begin(); itr != _peers.end(); itr++) {
      rm.push_back(itr->first);
      delete itr->second;
    } // for

    while( !rm.empty() ) {
      _peers.erase( rm.front() );
      rm.pop_front();
    } // while
  } // ConnectionManager::_deinitialize

  void ConnectionManager::_deinitializeThreads() {
    if (!_use_cm_thread) return;

    LOG(LogInfo, << "ConnectionManager: Waiting for Thread "
                 << _thread_tid
                 << " to Deinitialize"
                 << std::endl);
//    _debugf("*** ConnectionManager: Waiting for Thread %u to Deinitialize", _thread_tid);
    pthread_join(_thread_tid, NULL);

    //_debugf("*** ConnectionManager: Threads Deinitialized");
    LOG(LogInfo, << "ConnectionManager: Threads Deinitialized"
                 << std::endl);
  } // ConnectionManager::_deinitializeThreads

  void *ConnectionManager::thread(void *arg) {
    ConnectionManager *cm = static_cast<ConnectionManager *>(arg);
    const time_t done_check_intval = 2;
    time_t last_done_check = time(NULL);

    while(1) {
      if (last_done_check < time(NULL) - done_check_intval) {
        if ( cm->is_done() ) break;
        last_done_check = time(NULL);
      } // if

      int did_work = 0;
      if ( cm->transfers() ) did_work++;
      if ( cm->onTick() ) did_work++;

      if (!did_work) usleep(10000);
//      else usleep(1000);
    } // while

    return NULL;
  } // ConnectionManager::thread
} // namespace openframe

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

#ifndef __LIBOPENFRAME_CONNECTIONMANAGER_H
#define __LIBOPENFRAME_CONNECTIONMANAGER_H

#include <map>
#include <string>
#include <queue>

#include <netdb.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "SocketBase.h"
#include "OpenFrame_Abstract.h"
#include "OFLock.h"
#include "scoped_lock.h"

namespace openframe {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class Peer : public Connection {
  public:
    // ### Type Definitions ###
    struct peer_stats_t {
      size_t num_packets_read;
      size_t num_packets_written;
      size_t num_bytes_read;
      size_t num_bytes_written;
      time_t connected_at;
      time_t last_read_at;
      time_t last_write_at;
      time_t last_stats_at;
    };

    // ### Constructor/Destructor ###
    Peer() : disconnect(false) { }
    virtual ~Peer() { }

    // ### Public Variables ###
    const char *in;
    size_t in_len;
    std::string out;
    bool disconnect;
    time_t disconnect_expire;
    time_t last_read;

    peer_stats_t peer_stats;

    virtual std::string peer_stats_str() const {
      std::stringstream out;
      time_t diff = time(NULL) - peer_stats.last_stats_at;
      time_t diff_read = time(NULL) - peer_stats.last_read_at;
      time_t diff_write = time(NULL) - peer_stats.last_write_at;
      time_t diff_stats = time(NULL) - peer_stats.last_stats_at;

      double bps_read = double(peer_stats.num_bytes_read) / diff;
      double bps_write = double(peer_stats.num_bytes_written) / diff;

      double pps_read = double(peer_stats.num_packets_read) / diff;
      double pps_write = double(peer_stats.num_packets_written) / diff;

      out << "Stats " << host_str
          << ", bytes in " << peer_stats.num_bytes_read
          << ", bps in " << bps_read
          << ", pps in " << pps_read
          << ", last read " << diff_read << "s"
          << ", bytes out " << peer_stats.num_bytes_written
          << ", bps out " << bps_write
          << ", pps out " << pps_write
          << ", last write " << diff_write << "s"
          << ", next in " << diff_stats << "s";
      return out.str();
    } // peer_stats_str

  protected:
  private:
}; // class Listener

class ConnectionManager : public SocketBase {
  public:
    // ### Type Definitions ###
    typedef SocketBase super;

    typedef map<int, Peer *> peers_t;
    typedef peers_t::iterator peers_itr;
    typedef peers_t::const_iterator peers_citr;
    typedef peers_t::size_type peersSize_t;

    // ### Constructors/Deconstructor ###
    ConnectionManager();
    virtual ~ConnectionManager();

    // ### Static Members ###
    static const time_t kDisconnectExpire;
    static const time_t kPeerStatsInterval;
    static void *thread(void *);

    // ### Object Members ###
    const bool write(const int, const string &);
    void register_connection(const int sock);
    void register_connection(Peer *peer);
    void unregister_connection(const int sock);
    bool disconnect(const int sock);
    bool safe_disconnect(const int sock);

    // ### Pure Virtuals ###
    virtual void onConnect(const Connection *con) = 0;
    virtual void onDisconnect(const Connection *con) = 0;
    virtual void onRead(const Peer *peer) = 0;
    virtual bool onTick() { return false; }
    virtual bool onPeerWake(const Peer *peer) { return false; }
    virtual void onPeerStats(const Peer *peer) { }
    virtual const string::size_type onWrite(const Peer *peer, std::string &ret) = 0;

    // ### Configuration Members ###
    virtual ConnectionManager &start();
    virtual void stop();
    inline ConnectionManager &use_cm_thread(const bool ok) {
      _use_cm_thread = ok;
      return (ConnectionManager &) *this;
    } // use_cm_thread
    inline ConnectionManager &use_cm_manager(const bool ok) {
      _use_cm_manager = ok;
      return (ConnectionManager &) *this;
    } // use_cm_manager
    inline bool use_cm_thread() const { return _use_cm_thread; }
    inline bool use_cm_manager() const { return _use_cm_manager; }

    inline ConnectionManager &set_peer_stats_interval(const time_t peer_stats_interval) {
      _peer_stats_interval = peer_stats_interval;
      return *this;
    } // use_cm_manager

    inline ConnectionManager &set_pending_write(const bool pending_write) {
      scoped_lock slock(&_pending_write_l);
      _pending_write = pending_write;
      return *this;
    } // use_cm_manager

    inline bool is_done() {
      scoped_lock slock(&_done_l);
      return _done;
    } // done

    inline void set_done(const bool done) {
      scoped_lock slock(&_done_l);
      _done = done;
    } // set_done

    inline bool is_pending_write() {
      scoped_lock slock(&_pending_write_l);
      return _pending_write;
    } // is_pending_write

  protected:
    // ### Protected Members ###
    virtual void notify_socket_connected(const int);			// override me to listen and use connection elsewhere
    virtual void notify_socket_connected(Peer *peer);			// override me to listen and use connection elsewhere
    virtual void init_peer(Peer *peer);
    virtual const bool notify_socket_disconnected(const int);
    bool transfers();
    bool try_peer_stats(Peer *peer);

    // ### Variables ###

  private:
    // ### Private Members ###
    void _initialize();
    void _deinitialize();
    void _initializeThreads();
    void _deinitializeThreads();

    // ### Constructor Variables ###
    pthread_t _thread_tid;
    bool _initialized;
    bool _use_cm_thread;
    bool _use_cm_manager;
    bool _done;
    bool _pending_write;
    time_t _peer_stats_interval;

    // ### Variables ###
    OFLock _done_l;
    OFLock _pending_write_l;

    fd_set _fd_set_r;
    fd_set _fd_set_w;
    peers_t _peers;
    OFLock _peers_l;
}; // class ConnectionManager

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openframe
#endif

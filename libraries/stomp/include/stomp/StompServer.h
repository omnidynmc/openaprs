#ifndef __LIBSTOMP_STOMPSERVER_H
#define __LIBSTOMP_STOMPSERVER_H

#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <set>
#include <list>

#include <netdb.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "StompMessage.h"
#include "ExchangeManager.h"

#include <openframe/openframe.h>
#include <openstats/openstats.h>

namespace stomp {
  using openframe::OpenFrame_Exception;
  using openframe::ListenController;
  using openframe::StringTool;
  using openframe::OFLock;
  using openframe::Peer;
  using openframe::Connection;
  using openframe::ThreadMessenger;
  using std::map;
  using std::set;
  using std::stringstream;
  using std::string;
  using std::vector;
  using std::list;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/
#define DEVICE_BINARY_SIZE  32
#define MAXPAYLOAD_SIZE     256

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  // ### Forward Declarations ###
  class StompFrame;
  class StompMessage;
  class StompPeer;

  class StompServer_Exception : public OpenFrame_Exception {
    public:
      StompServer_Exception(const string message) throw() : OpenFrame_Exception(message) { };
  }; // class StompServer_Exception

  class StompServer : public ListenController,
                      public openstats::StatsClient_Interface,
                      public ThreadMessenger {
    public:
      StompServer(const int, const int);
      virtual ~StompServer();

      /**********************
       ** Type Definitions **
       **********************/
      typedef ListenController super;

      typedef map<int, StompPeer *> peers_t;
      typedef peers_t::iterator peers_itr;
      typedef peers_t::size_type peersSize_t;

      typedef set<StompMessage *> topics_t;
      typedef topics_t::iterator topics_itr;
      typedef topics_t::const_iterator topics_citr;
      typedef topics_t::size_type topicsSize_t;

      typedef set<StompMessage *> queues_t;
      typedef queues_t::iterator queues_itr;
      typedef queues_t::const_iterator queues_citr;
      typedef queues_t::size_type queuesSize_t;

      /***************
       ** Variables **
       ***************/
      static const time_t kDefaultLogstatsInterval;
      static const size_t kDefaultMaxWork;
      static const int HEADER_SIZE;

      // ### Core Members ###
      const bool run();
      virtual StompServer &start();

      // ### Options ###
      inline StompServer &max_work(const size_t max_work) {
        _max_work = max_work;
        return *this;
      } // max_work
      inline const size_t max_work() const { return _max_work; }
      inline StompServer &intval_logstats(const time_t intval_logstats) {
        _intval_logstats = intval_logstats;
        return *this;
      } // intval_logstats
      inline const size_t intval_logstats() const { return _intval_logstats; }
      inline const bool debug() const { return _debug; }
      inline StompServer &debug(const bool debug) {
        _debug = debug;
        return (StompServer &) *this;
      } // debug

      // ### SocketController pure virtuals ###
      void onConnect(const openframe::Connection *);
      void onDisconnect(const openframe::Connection *);
      void onRead(const Peer *);
      bool onPeerWake(const Peer *);
      const string::size_type onWrite(const Peer *, string &);

      // ### Stats Pure Virtuals ###
      virtual void onDescribeStats();
      virtual void onDestroyStats();


      // ### Queue Members ###
      void enqueue_queue(StompMessage *);
      void enqueue_queue(mesgList_t &);
      void exqueue_queues();
      void clear_queues();
      void dequeue_queue(StompMessage *);
      void enqueue_topic(StompMessage *);
      void exqueue_topics();
      void dequeue_topic(StompMessage *);

    protected:
      // ### Protected Members ###
      bool _process_peers(peers_t &peers);
      bool _process_peer(StompPeer *peer);
      void _process(StompPeer *, StompFrame *);
      void _process_connect(StompPeer *, StompFrame *);
      void _process_ack(StompPeer *, StompFrame *);
      void _process_send(StompPeer *, StompFrame *);
      void _process_subscribe(StompPeer *, StompFrame *);
      void _process_unsubscribe(StompPeer *, StompFrame *);
      void _process_receipt_id(StompPeer *, StompFrame *);
      void _send_queues(StompPeer *);

      // ### Protected Variables ###
      topics_t _topics;
      queues_t _queues;

    private:
      void init_stats(const bool startup=false);
      void _initializeThreads();
      void _deinitializeThreads();

      peers_t _peers;
      OFLock _peers_l;
      bool _debug;
      time_t _intval_logstats;
      size_t _max_work;
      ExchangeManager *_exch_manager;

      struct obj_stats_t {
        size_t num_peers;
        time_t last_report_peers;
      } _stats; // obj_stats_t
  }; // StompServer

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace stomp
#endif

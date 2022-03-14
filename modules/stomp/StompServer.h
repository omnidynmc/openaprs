#ifndef __MODULE_STOMP_STOMPSERVER_H
#define __MODULE_STOMP_STOMPSERVER_H

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

#include <openframe/openframe.h>

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

  typedef list<StompMessage *> mesgList_t;
  typedef mesgList_t::iterator mesgList_itr;
  typedef mesgList_t::const_iterator mesgList_citr;
  typedef mesgList_t::size_type mesgListSize_t;

  class StompServer_Exception : public OpenFrame_Exception {
    public:
      StompServer_Exception(const string message) throw() : OpenFrame_Exception(message) { };
  }; // class StompServer_Exception

  class StompServer : public ListenController, public ThreadMessenger {
    public:
      StompServer(const int, const int, const time_t, const time_t, const time_t, const time_t, const time_t);
      virtual ~StompServer();

      /**********************
       ** Type Definitions **
       **********************/
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
      static const time_t DEFAULT_STATS_INTERVAL;
      static const time_t CONNECT_RETRY_TIMEOUT;
      static const int HEADER_SIZE;

      // ### Core Members ###
      const bool run();

      // ### SocketController pure virtuals ###
      void onConnect(const openframe::Connection *);
      void onDisconnect(const openframe::Connection *);
      void onRead(const Peer *);
      const string::size_type onWrite(const Peer *, string &);

      const bool debug() const { return _debug; }
      StompServer &debug(const bool debug) {
        _debug = debug;
        return (StompServer &) *this;
      } // debug

      // ### Queue Members ###
      void enqueue_queue(StompMessage *);
      void enqueue_queue(mesgList_t &);
      void exqueue_queues();
      void dequeue_queue(StompMessage *);
      void enqueue_topic(StompMessage *);
      void exqueue_topics();
      void dequeue_topic(StompMessage *);

    protected:
      // ### Protected Members ###
      void _process(StompPeer *, StompFrame *);
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
      void _initializeThreads();
      void _deinitializeThreads();

      peers_t _peers;
      OFLock _peers_l;
      bool _debug;
      time_t _sessionInitInterval;
      time_t _enquireLinkInterval;
      time_t _inactivityInterval;
      time_t _logstatsInterval;
      time_t _responseInterval;
  }; // StompServer

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

  extern "C" {
  } // extern

} // namespace stomp
#endif

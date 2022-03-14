#ifndef __MODULE_STOMPPEER_H
#define __MODULE_STOMPPEER_H

#include <sstream>
#include <set>
#include <map>
#include <queue>

#include <openframe/openframe.h>

#include "StompServer.h"
#include "StompFrame.h"

namespace stomp {
  using openframe::OpenFrame_Abstract;
  using openframe::OpenFrame_Exception;
  using openframe::OFLock;
  using openframe::Refcount;
  using openframe::StringTool;
  using openframe::Stopwatch;
  using openframe::SocketController;
  using std::set;
  using std::map;
  using std::queue;
  using std::stringstream;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/
#define DEVICE_BINARY_SIZE  32
#define MAXPAYLOAD_SIZE     256

#define STOMP_FRAME_ARG		StompFrame *frame

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class StompPeer_Exception : public OpenFrame_Exception {
  public:
    StompPeer_Exception(const string message) throw() : OpenFrame_Exception(message) { };
}; // class StompPeer_Exception

class StompSub {
  public:
    enum ackModeEnum {
      ackModeAuto	= 0,
      ackModeClient	= 1,
      ackModeClientIndv	= 2
    };

    StompSub(const string &id, const string &name, const ackModeEnum ack_mode=ackModeAuto) : _id(id), _name(name), _ack_mode(ack_mode) { }
    virtual ~StompSub() { }


    typedef set<StompMessage *> mesgQueue_t;
    typedef mesgQueue_t::iterator mesgQueue_itr;
    typedef mesgQueue_t::const_iterator mesgQueue_citr;
    typedef mesgQueue_t::size_type mesgQueueSize_t;

    inline const string id() const { return _id; }
    inline const string name() const { return _name; }
    inline const ackModeEnum ack_mode() const { return _ack_mode; }
    inline const bool match(const string &name) const {
      return StringTool::match(_name.c_str(), name.c_str());
    } // match
    inline void enqueue(StompMessage *smesg) { _mesgQueue.insert(smesg); }
    inline void dequeue_all(mesgList_t &ret) {
      while( _mesgQueue.empty() ) {
        mesgQueue_itr itr = _mesgQueue.begin();
        ret.push_back(*itr);
        _mesgQueue.erase(itr);
      } // while
    } // dequeue_all

    inline const string toString() const {
      stringstream out;
      out << "id=" << _id
          << ",name=" << _name;
      return out.str();
    } // toString

  protected:
  private:
    friend class StompServer;
    friend class StompPeer;
    string _id;
    string _name;
    ackModeEnum _ack_mode;
    mesgQueue_t _mesgQueue;
}; // class StompSub

class StompMessage;
class StompPeer : public OpenFrame_Abstract, public Refcount {
  public:
    StompPeer(const int, const time_t, const time_t, const time_t, const time_t, const time_t);
    virtual ~StompPeer();

    friend class StompServer;

    static const int HEADER_SIZE;

    /**********************
     ** Type Definitions **
     **********************/
    typedef queue<StompFrame *> frameQueue_t;
    typedef frameQueue_t::size_type frameQueueSize_t;

    typedef map<string, StompSub *> topicSubs_t;
    typedef topicSubs_t::iterator topicSubs_itr;
    typedef topicSubs_t::const_iterator topicSubs_citr;
    typedef topicSubs_t::size_type topicSubsSize_t;

    typedef map<string, StompSub *> queueSubs_t;
    typedef queueSubs_t::iterator queueSubs_itr;
    typedef queueSubs_t::const_iterator queueSubs_citr;
    typedef queueSubs_t::size_type queueSubsSize_t;

    enum stompStageEnum {
      stompStageCommand 	= 0,
      stompStageHeaders 	= 1,
      stompStageBodyLen		= 2,
      stompStageBodyNul		= 3,
      stompStageWaitForNul	= 4
    };

    /*************
     ** Members **
     *************/
    const uint32_t next_sequence_number() { return _sequence_number++; }

    inline const int sock() const { return _sock; }

    // ### Public Members - Topic/Queues ###
    const bool subscribe_topic(const string &, const string &, StompSub::ackModeEnum ack=StompSub::ackModeAuto);
    const bool unsubscribe_topic(const string &);
    const bool is_topic_subscriber(const string &);
    const bool is_topic_match(const string &);
    void clear_topics();

    const bool subscribe_queue(const string &, const string &, StompSub::ackModeEnum ack=StompSub::ackModeAuto);
    const bool unsubscribe_queue(const string &, mesgList_t &);
    void unsubscribe_queue_all(mesgList_t &);
    const bool is_queue_subscriber(const string &);
    const bool is_queue_match(const string &, StompSub *&);
    void clear_queues();

    const size_t receive(const string &);
    const string::size_type transmit(string &);
    const size_t process();

    const bool dequeue(StompFrame *&ret) {
      if (!_frameQ.size()) return false;
      ret = _frameQ.front();
      _frameQ.pop();
      return true;
    } // dequeue
    //const bool exqueue(const time_t);

    // ### Protocol Commands ###
    const size_t send_frame(StompFrame *);
    const size_t send_error(const string &, const string &body="");
    const bool next_frame(StompFrame *&frame) {
      process();
      if ( !_frameQ.size() ) return false;
      frame = _frameQ.front();
      _frameQ.pop();
      return true;
    } // next_frame
    void connect(const string &, const string &, stompHeader_t);
    void connected(const string &, stompHeader_t);
    void send(const string &, const string &, stompHeader_t);
    void subscribe(const string &, stompHeader_t);
    void unsubscribe(const string &, stompHeader_t);
    void begin(const string &, stompHeader_t);
    void commit(const string &, stompHeader_t);
    void ack(const string &, stompHeader_t);
    void abort(const string &, stompHeader_t);
    void message(const string &, const string &, const string &, stompHeader_t);
    void receipt(const string &, stompHeader_t);

    // ### Stats Goodies ###
    const double pps() const {
      double pps = double(_num_packets) / double(time(NULL) - _time_connected);
      return pps;
    } // pps

    // ### SocketController pure virtuals ###
    void onConnect(const Peer *);
    void onDisconnect(const Peer *);
    void onRead(const Peer *);

    const bool run();
    const bool disconnect() const { return _disconnect; }
    void wantDisconnect();

    inline const string toString() const {
      stringstream out;
      out << SocketController::derive_peer_ip(_sock)
          << ":"
          << SocketController::derive_peer_port(_sock);
      return out.str();
    } // toString

  protected:
    const size_t _process_command();
    const size_t _process_headers();
    const size_t _process_body_len();
    const size_t _process_body_nul();
    const size_t _process_waitfor_nul();
    void _process_reset();

  private:
    typedef struct {
      string command;
      string body;
      stompHeader_t headers;
      size_t content_length;
    } stompStage_t;

    // ### Private Members ###
    Stopwatch *_logstats;
    const bool _checkInactivity();
    const bool _checkEnquireLink();
    const bool _checkLogstats();
    void _expireIdleConnection();
    const string::size_type _write(const string &);

    // ### Private Variables ###
    frameQueue_t _sentQ;
    topicSubs_t _topicSubs;
    queueSubs_t _queueSubs;

    OFLock _in_l;
    OFLock _out_l;
    ByteData _in;
    string _out;

    bool _disconnect;
    bool _bound;
    int _sock;
    frameQueue_t _frameQ;
    stompStageEnum _stage;
    stompStage_t _stagedFrame;
    time_t _sessionInitInterval;
    time_t _sessionInitTs;
    time_t _enquireLinkInterval;
    time_t _enquireLinkTs;
    time_t _inactivityInterval;
    time_t _inactivityTs;
    time_t _responseInterval;
    time_t _responseTs;
    time_t _time_connected;
    uint32_t _sequence_number;
    unsigned int _lastId;			// last message id in use
    unsigned long _num_packets;
    unsigned long _num_bytes;
}; // StompPeer

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

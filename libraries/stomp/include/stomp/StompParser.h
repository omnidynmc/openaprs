#ifndef __MODULE_STOMPPARSER_H
#define __MODULE_STOMPPARSER_H

#include <sstream>
#include <set>
#include <map>
#include <queue>
#include <string>

#include <openframe/openframe.h>
#include <openstats/openstats.h>

#include "StompServer.h"
#include "StompFrame.h"
#include "StompMessage.h"
#include "Subscription.h"

namespace stomp {
  using openframe::OpenFrame_Abstract;
  using openframe::OpenFrame_Exception;
  using openframe::OFLock;
  using openframe::Refcount;
  using openframe::StringTool;
  using openframe::Stopwatch;
  using std::map;
  using std::queue;
  using std::string;
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
  class StompMessage;

  class StompParser_Exception : public OpenFrame_Exception {
    public:
      StompParser_Exception(const std::string message) throw() : OpenFrame_Exception(message) { };
  }; // class StompParser_Exception

  class StompParser : public openstats::StatsClient_Interface,
                      public Refcount {
    public:
      static const time_t kDefaultStatsInterval;

      StompParser();
      virtual ~StompParser();

      /**********************
       ** Type Definitions **
       **********************/
      typedef queue<StompFrame *> frameQueue_t;
      typedef frameQueue_t::size_type frameQueueSize_t;

      typedef std::set<Subscription *> subscriptions_t;
      typedef subscriptions_t::iterator subscriptions_itr;
      typedef subscriptions_t::const_iterator subscriptions_citr;
      typedef subscriptions_t::size_type subscriptions_st;

      enum stompStageEnum {
        stompStageCommand 	= 0,
        stompStageHeaders 	= 1,
        stompStageBodyLen	= 2,
        stompStageBodyNul	= 3,
        stompStageWaitForNul	= 4
      };

      // ### Public Members - Bindings ###
      const bool bind(Subscription *);
      const bool unbind(Subscription *);
      void unbind_all();
      const bool is_subscriber(const std::string &);
      const bool is_match(const std::string &, Subscription *&);
      const bool ack(const std::string &, const std::string &);

      size_t receive(const char *buf, const size_t len);
      const string::size_type transmit(string &);
      void reset();
      const size_t process();

      virtual void onError(StompFrame *frame) = 0;
      virtual void onBind(Subscription *sub) = 0;
      virtual void onUnbind(Subscription *sub) = 0;

      const bool dequeue(StompFrame *&ret) {
        if (!_frameQ.size()) return false;
        ret = _frameQ.front();
        _frameQ.pop();
        return true;
      } // dequeue

      // ### Protocol Commands ###
      const size_t send_frame(StompFrame *);
      const size_t send_error(const std::string &, const std::string &body="");
      const bool next_frame(StompFrame *&frame) {
        if ( _frameQ.empty() ) return false;
        frame = _frameQ.front();
        _frameQ.pop();
        return true;
      } // next_frame
      void connect(const std::string &, const std::string &);
      void connect(const std::string &, const std::string &, stompHeader_t);
      void connected(const std::string &);
      void connected(const std::string &, stompHeader_t);
      void send(const std::string &, const std::string &);
      void send(const std::string &, const std::string &, stompHeader_t);
      void subscribe(const std::string &);
      void subscribe(const std::string &, stompHeader_t);
      void unsubscribe(const std::string &);
      void unsubscribe(const std::string &, stompHeader_t);
      void begin(const std::string &);
      void begin(const std::string &, stompHeader_t);
      void commit(const std::string &, stompHeader_t);
      void commit(const std::string &);
      void ack(const std::string &);
      void ack(const std::string &, stompHeader_t);
      void abort(const std::string &);
      void abort(const std::string &, stompHeader_t);
      void message(const std::string &, const std::string &, const std::string &);
      void message(const std::string &, const std::string &, const std::string &, stompHeader_t);
      void receipt(const std::string &);
      void receipt(const std::string &, stompHeader_t);

      // ### Stats Pure Virtuals ###
      virtual void onDescribeStats();
      virtual void onDestroyStats();

    protected:
      size_t advance_until(const char ch);
      const size_t _process_command();
      const size_t _process_headers();
      const size_t _process_body_len();
      const size_t _process_body_nul();
      const size_t _process_waitfor_nul();
      void _process_reset();
      std::string _uniq_id;

      typedef struct {
        std::string command;
        std::string body;
        stompHeader_t headers;
        size_t content_length;
      } stompStage_t;

      // ### Protected Members ###
      void _init();
      void init_stats(const bool startup=false);
      bool try_stats();
      const string::size_type _write(const std::string &);

      // ### Protected Variables ###
      frameQueue_t _sentQ;
      subscriptions_t _subscriptions;

      OFLock _in_l;
      OFLock _out_l;
      std::string _in;
      //ByteData _in;
      size_t _in_pos;
      std::string _out;

      frameQueue_t _frameQ;
      stompStageEnum _stage;
      stompStage_t _stagedFrame;

      struct obj_stats_t {
        size_t num_frames_in;
        size_t num_frames_out;
        size_t num_bytes_in;
        size_t num_bytes_out;
        time_t last_report_at;
        time_t created_at;
      } _stats;

    private:
  }; // StompParser

  std::ostream &operator<<(std::ostream &ss, const StompParser::stompStageEnum stage);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace stomp
#endif

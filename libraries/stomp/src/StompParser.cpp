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

#include <openframe/openframe.h>
#include <openstats/openstats.h>

#include "StompMessage.h"
#include "StompParser.h"
#include "Subscription.h"

#undef STOMP_PARSER_DEBUG

namespace stomp {
  using namespace openframe;
  using namespace openframe::loglevel;
  using namespace std;

/**************************************************************************
 ** StompPeer Class                                                       **
 **************************************************************************/

  const time_t StompParser::kDefaultStatsInterval		= 300;

  StompParser::StompParser() {
    _in_pos = 0;
    _init();
    return;
  } // StompParser::StompParser

  StompParser::~StompParser() {
    _process_reset();
    unbind_all();
    onDestroyStats();
    return;
  } // StompParser::~StompParser

  void StompParser::_init() {
    // timers
    init_stats(true);
    _in.reserve(1000000);
    reset();
    _process_reset();
  } // StompPeer::_init

  void StompParser::init_stats(const bool startup) {
    _stats.num_frames_in = 0;
    _stats.num_frames_out = 0;
    _stats.num_bytes_in = 0;
    _stats.num_bytes_out = 0;
    _stats.last_report_at = time(NULL);
    if (startup) _stats.created_at = time(NULL);
  } // StompParser::init_stats

  bool StompParser::try_stats() {
    if (_stats.last_report_at > time(NULL) - kDefaultStatsInterval) {
      return false;
    } // if
    init_stats();
    return true;
  } // StompParser::try_stats

  void StompParser::onDescribeStats() {
    set_stat_id_prefix("libstomp.stomppeer."+_uniq_id);
    describe_stat("num.frames.in", "num frames in", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.frames.out", "num frames out", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.bytes.in", "num bytes in", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.bytes.out", "num bytes in", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
  } // StompParser::onDescribeStats

  void StompParser::onDestroyStats() {
    stats()->destroy_stat("libstomp.stomppeer."+_uniq_id+"*");
  } // StompParser::onDestroyStats

  const size_t StompParser::send_error(const std::string &message, const std::string &body) {
    StompFrame *frame = new StompFrame("ERROR", body);
    frame->add_header("message", message);
    bool ok = send_frame(frame);
    onError(frame);
    frame->release();
    return ok;
  } // StompParser::send_error

  const size_t StompParser::send_frame(StompFrame *frame) {
    assert(frame != NULL); // bug
    std::string ret = frame->compile();
    size_t len = ret.length();

    _stats.num_frames_out++;
    _stats.num_bytes_out += len;
    datapoint("num.frames.out", 1);
    datapoint("num.bytes.out", len );
    return _write(ret);
  } // StompParser::send_frame

  size_t StompParser::receive(const char *buf, const size_t len) {
    openframe::scoped_lock slock(&_in_l);
    _in.append(buf, len);
    _stats.num_bytes_in += len;

    datapoint("num.bytes.in", len);
    return len;
  } // StompParser::receive

  const size_t StompParser::process() {
    size_t ret = 0;

    for(subscriptions_itr itr = _subscriptions.begin(); itr != _subscriptions.end(); itr++) {
      Subscription *sub = *itr;
      StompMessage *smesg;

      size_t limit = 1000;
      for(size_t i = 0; i < limit && sub->dequeue_for_send(smesg); i++) {
        smesg->replace_header("subscription", sub->id() );
        send_frame( dynamic_cast<StompFrame *>(smesg) );
        if ( !smesg->requires_resp() ) smesg->release(); // if we dont need response release
      } // for
    } // for

    openframe::scoped_lock slock(&_in_l);
    if (_in.length() < 1) return ret;

    switch(_stage) {
      case stompStageCommand:
        ret = _process_command();
        break;
      case stompStageHeaders:
        ret = _process_headers();
        break;
      case stompStageBodyLen:
        ret = _process_body_len();
        break;
      case stompStageBodyNul:
        ret = _process_body_nul();
        break;
      case stompStageWaitForNul:
        ret = _process_waitfor_nul();
        break;
      default:
        assert(false);		// bug
        break;
    } // switch

#ifdef STOMP_PARSER_DEBUG
std::cout << _stagedFrame.command << " stage(" << _stage << ") ret(" << ret << ")" << std::endl;
#endif

    return ret;
  } // StompParer::process

  const size_t StompParser::_process_command() {
    string command;
    command.reserve(30);

    size_t len = advance_until('\n');
    if (!len) return 0;

    command.assign(_in.data()+_in_pos, len-1);

#ifdef STOMP_PARSER_DEBUG
std::cout << "COMMAND(" << command << ")" << std::endl;
#endif

    _in_pos += len;	// skip newline

    StringTool::replace("\r", "", command);

    if (command.length() == 0) {
      _process_reset();
      return 1;		// in case this is the end of a frame
    } // if

    _stagedFrame.command = command;
    _stage = stompStageHeaders;
    return command.length();
  } // StompParser::process_command

  const size_t StompParser::_process_headers() {
    size_t ret = 0;
    StompHeader *header;
    string header_str;

    while(1) {
//std::cout << "MEH(" << _in.substr(_in_pos) << ")" << std::endl;
      size_t len = advance_until('\n');
      if (!len) return 0;

      header_str.assign(_in.data()+_in_pos, len-1);

#ifdef STOMP_PARSER_DEBUG
std::cout << "LEN(" << len << ") HEADER(" << header_str << ")" << std::endl;
#endif
      _in_pos += len; // skip newline

      ret += header_str.length();
      StringTool::replace("\r", "", header_str);
      if (header_str.length() == 0) {
        stompHeader_itr itr = _stagedFrame.headers.find("Content-Length");
        if (itr != _stagedFrame.headers.end()) {
          _stagedFrame.content_length = atoi( itr->second->value().c_str() );
          _stage = stompStageBodyLen;
        } // if
        else
          _stage = stompStageBodyNul;
        break;
      } // if

      try {
        header = new StompHeader(header_str);
      } // try
      catch(Stomp_Exception ex) {
        // unable to parse header send error
        send_error("unable to parse header");
        _process_reset();
        break;
      } //catch

      if (_stagedFrame.headers.find(header->name()) != _stagedFrame.headers.end()) {
        // stomp 1.1 spec says only the first header of the same name is accepted
        delete header;
        continue;
      } // if

      _stagedFrame.headers.insert( make_pair(header->name(), header) );

    } // while

    return ret;
  } // StompParser::_process_headers

  const size_t StompParser::_process_body_len() {
    size_t ret = 0;

    if (_stagedFrame.content_length > _in.length() -_in_pos)
      // not enough in the buffer not ready yet
      return 0;

    _stagedFrame.body.assign(_in.data()+_in_pos, _stagedFrame.content_length);
    ret = _stagedFrame.body.length();
    _stage = stompStageWaitForNul;
    _in_pos += ret;

    return ret;
  } // StompParser::_process_body_len

  const size_t StompParser::_process_waitfor_nul() {
    size_t len = advance_until('\0');
    if (!len) return 0;

    _in_pos += len; // skip nul

    StompFrame *frame = new StompFrame(_stagedFrame.command, _stagedFrame.body);
    frame->insert(_stagedFrame.headers.begin(), _stagedFrame.headers.end());
    _stagedFrame.headers.clear();

    _frameQ.push(frame);
    _stats.num_frames_in++;
    datapoint("num.frames.in", 1);
    _process_reset();

    return len;
  } // StompParser::_process_waitfor_nul

  size_t StompParser::advance_until(const char ch) {
    size_t pos = _in_pos;

    while(pos < _in.length() ) {
      if (_in[pos] == ch) return (pos - _in_pos) + 1;
      ++pos;
    } // while

    return false;
  } // StompParser::advance_until

  const size_t StompParser::_process_body_nul() {
    size_t len = advance_until('\0');
    if (!len) return 0;
//std::cout << "LEN(" << len << ") POS(" << _in_pos << ") _IN(" << _in.length() << ")" << std::endl;
    _stagedFrame.body.assign(_in.data()+_in_pos, len-1);
    _in_pos += len;

#ifdef STOMP_PARSER_DEBUG
std::cout << "BODY(" << _stagedFrame.body << ")" << std::endl;
#endif
    size_t ret = _stagedFrame.body.length();
    StompFrame *frame = new StompFrame(_stagedFrame.command, _stagedFrame.body);
    frame->insert(_stagedFrame.headers.begin(), _stagedFrame.headers.end());
    _stagedFrame.headers.clear();

    _frameQ.push(frame);

    _stats.num_frames_in++;
    datapoint("num.frames.in", 1);
    _process_reset();

    return ret;
  } // StompParser::_process_body_nul

  void StompParser::_process_reset() {
    _stagedFrame.command = "";
    _stagedFrame.body = "";
    _stagedFrame.content_length = 0;

    if (_in_pos > 500000) {
      _in.erase(0, _in_pos);
      _in_pos = 0;
    } // if

    for(stompHeader_itr itr = _stagedFrame.headers.begin(); itr != _stagedFrame.headers.end(); itr++)
      delete itr->second;

    _stagedFrame.headers.clear();
    _stage = stompStageCommand;
  } // StompParser::_process_reset

  const string::size_type StompParser::_write(const string &buf) {
    openframe::scoped_lock slock(&_out_l);
    _out.append(buf);
    return buf.size();
  } // StompParser::_write

  const string::size_type StompParser::transmit(string &ret) {
    openframe::scoped_lock slock(&_out_l);
    ret = _out;
    _out = "";
    return ret.size();
  } // StompParser::transmit

  void StompParser::reset() {
    _stage = stompStageCommand;

    init_stats(true);

    openframe::scoped_lock sl_out(&_out_l);
    openframe::scoped_lock sl_in(&_in_l);
    _in = _out = "";
    _in_pos = 0;
    _process_reset();
  } // StompParser::reset

  /**************************************************************************
   ** Commands                                                             **
   **************************************************************************/
  void StompParser::connect(const std::string &username, const std::string &passcode) {
    stompHeader_t headers;
    connect(username, passcode, headers);
  } // StompParser::connect

  void StompParser::connect(const std::string &username, const std::string &passcode, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("CONNECT");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("login", username);
    frame->add_header("passcode", passcode);
    send_frame(frame);
    frame->release();
  } // StompParser::connect

  void StompParser::connected(const std::string &session_id) {
    stompHeader_t headers;
    connected(session_id, headers);
  } // StompParser::connected

  void StompParser::connected(const string &session_id, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("CONNECTED");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("session", session_id);
    send_frame(frame);
    frame->release();
  } // StompParser::connected

  void StompParser::send(const std::string &destination, const std::string &body) {
    stompHeader_t headers;
    send(destination, body, headers);
  } // StompParser::send

  void StompParser::send(const std::string &destination, const std::string &body, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("SEND", body);
    frame->insert(headers.begin(), headers.end());
    frame->replace_header("destination", destination);
    send_frame(frame);
    frame->release();
  } // StompParser::send

  void StompParser::subscribe(const std::string &destination) {
    stompHeader_t headers;
    subscribe(destination, headers);
  } // StompParser::subscribe

  void StompParser::subscribe(const std::string &destination, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("SUBSCRIBE");
    frame->insert(headers.begin(), headers.end());
    frame->replace_header("destination", destination);
    send_frame(frame);
    frame->release();
  } // StompParser::subscribe

  void StompParser::unsubscribe(const std::string &destination) {
    stompHeader_t headers;
    unsubscribe(destination, headers);
  } // StompParser::unsubscribe

  void StompParser::unsubscribe(const string &destination, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("SUBSCRIBE");
    frame->insert(headers.begin(), headers.end());
    frame->replace_header("destination", destination);
    send_frame(frame);
    frame->release();
  } // StompParser::unsubscribe

  void StompParser::begin(const std::string &transaction) {
    stompHeader_t headers;
    begin(transaction, headers);
  } // StompParser::begin

  void StompParser::begin(const std::string &transaction, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("BEGIN");
    frame->insert(headers.begin(), headers.end());
    frame->replace_header("transaction", transaction);
    send_frame(frame);
    frame->release();
  } // StompParser::begin

  void StompParser::commit(const std::string &transaction) {
    stompHeader_t headers;
    commit(transaction, headers);
  } // StompParser::commit

  void StompParser::commit(const std::string &transaction, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("COMMIT");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("transaction", transaction);
    send_frame(frame);
    frame->release();
  } // StompParser::commit

  void StompParser::ack(const std::string &message_id) {
    stompHeader_t headers;
    ack(message_id, headers);
  } // StompParser::ack

  void StompParser::ack(const std::string &message_id, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("ACK");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("message-id", message_id);
    send_frame(frame);
    frame->release();
  } // StompParser::ack

  void StompParser::abort(const std::string &transaction) {
    stompHeader_t headers;
    abort(transaction, headers);
  } // StompParser::abort

  void StompParser::abort(const string &transaction, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("ABORT");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("transaction", transaction);
    send_frame(frame);
    frame->release();
  } // StompParer::abort

  void StompParser::message(const std::string &destination, const std::string &message_id,
                            const std::string &body) {
    stompHeader_t headers;
    message(destination, message_id, body, headers);
  } // StompParser::message

  void StompParser::message(const std::string &destination, const std::string &message_id,
                            const std::string &body, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("MESSAGE", body);
    frame->insert(headers.begin(), headers.end());
    frame->replace_header("destination", destination)
          .replace_header("message-id", message_id);
    send_frame(frame);
    frame->release();
  } // StompParser::message

  void StompParser::receipt(const std::string &receipt_id) {
    stompHeader_t headers;
    receipt(receipt_id, headers);
  } // StompParser::receipt

  void StompParser::receipt(const std::string &receipt_id, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("RECEIPT");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("receipt-id", receipt_id);
    send_frame(frame);
    frame->release();
  } // StompParser::receipt

  // ### Subscriptions ###
  const bool StompParser::bind(Subscription *sub) {
    assert(sub != NULL);
    subscriptions_itr itr = _subscriptions.find(sub);
    if (itr != _subscriptions.end()) return false;
    sub->retain();
    _subscriptions.insert(sub);
    //log(LogInfo) << "StompParser bound to " << sub << endl;
    onBind(sub);
    return true;
  } // StompParser::bind

  const bool StompParser::unbind(Subscription *sub) {
    assert(sub != NULL);
    subscriptions_itr itr = _subscriptions.find(sub);
    if (itr == _subscriptions.end()) return false;
    //log(LogInfo) << "StompParser unbound from " << sub << endl;
    onUnbind(sub);
    _subscriptions.erase(sub);
    sub->release();
    return true;
  } // StompParser::unbind

  void StompParser::unbind_all() {
    for(subscriptions_itr itr = _subscriptions.begin(); itr != _subscriptions.end(); itr++) {
      Subscription *sub = *itr;
      onUnbind(sub);
      //log(LogInfo) << "StompParser unbound from " << sub << endl;
      sub->release();
    } // for
    _subscriptions.clear();
  } // StompParser::unbind_all

  const bool StompParser::is_subscriber(const string &id) {
//    subscriptions_itr itr = _subscriptions.find(id);
//    if (itr == _subscriptions.end()) return false;
    return true;
  } // is_subscriber

  const bool StompParser::is_match(const string &name, Subscription *&ret) {
    subscriptions_itr itr;
    for(itr = _subscriptions.begin(); itr != _subscriptions.end(); itr++) {
      Subscription *sub = *itr;
      if ( sub->match(name) && sub->prefetch_ok() ) {
        ret = sub;
        return true;
      } // if
    } // for

    return false;
  } // StompParser::is_match

  const bool StompParser::ack(const string &id, const string &message_id) {
    for(subscriptions_itr itr = _subscriptions.begin(); itr != _subscriptions.end(); itr++) {
      Subscription *sub = *itr;
      if ( sub->is_id(id) ) return sub->dequeue(message_id);
    } // for
    return false;
  } // StompParser::ack

  std::ostream &operator<<(std::ostream &ss, const StompParser::stompStageEnum stage) {
    switch(stage) {
      case StompParser::stompStageCommand:
        ss << "stageCommand";
        break;
      case StompParser::stompStageHeaders:
        ss << "stageHeaders";
        break;
      case StompParser::stompStageBodyLen:
        ss << "stageBodyLen";
        break;
      case StompParser::stompStageBodyNul:
        ss << "stageBodyNul";
        break;
      case StompParser::stompStageWaitForNul:
        ss << "stageWaitForNul";
        break;
      default:
        ss << "unknown";
        break;
    } // switch
    return ss;
  } // operator<<
} // namespace stomp


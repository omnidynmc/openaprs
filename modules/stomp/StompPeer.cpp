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

#include "StompMessage.h"
#include "StompPeer.h"
#include "StompServer.h"

#include "stomp.h"

namespace stomp {
  using namespace openframe;
  using namespace openframe::loglevel;
  using namespace std;

/**************************************************************************
 ** StompPeer Class                                                       **
 **************************************************************************/

  const int StompPeer::HEADER_SIZE			= 1;

  StompPeer::StompPeer(const int sock, const time_t sessionInitInterval, const time_t enquireLinkInterval, const time_t inactivityInterval, const time_t responseInterval, const time_t logstatsInterval) :
    Refcount(), _sock(sock), _sessionInitInterval(sessionInitInterval), _enquireLinkInterval(enquireLinkInterval), _inactivityInterval(inactivityInterval), _responseInterval(responseInterval) {

    // timers
    _sessionInitTs = time(NULL) + _sessionInitInterval;
    _enquireLinkTs = time(NULL) + _enquireLinkInterval;
    _inactivityTs = time(NULL) + _inactivityInterval * 2;
    _stage = stompStageCommand;

    _disconnect = false;
    _bound = false;
    _lastId = 0;
    _sequence_number = 0;
    _num_packets = 0;
    _num_bytes = 0;
    _time_connected = time(NULL);

    try {
      _logstats = new Stopwatch(logstatsInterval);
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    _process_reset();
    return;
  } // StompPeer::StompPeer

  StompPeer::~StompPeer() {
    delete _logstats;
    return;
  } // StompPeer::~StompPeer

  const bool StompPeer::run() {
    bool didWork = false;
    _checkEnquireLink();
    _checkLogstats();
    didWork = process();

    return didWork;
  } // StompPeer::run

  const size_t StompPeer::send_error(const string &message, const string &body) {
    StompFrame *frame = new StompFrame("ERROR", body);
    StompHeader *header = new StompHeader("message", message);
    frame->insert( make_pair(header->name(), header) );
    wantDisconnect();
    return send_frame(frame);
  } // StompPeer::send_error

  const bool StompPeer::_checkLogstats() {
    if (!_logstats->Next()) return false;

    double pps = double(_num_packets) / double(time(NULL) - _time_connected);
    double kbps = (double(_num_bytes) / double(time(NULL) - _time_connected)) / 1024.0;
    double kb = double(_num_bytes) / double(1024);
    log(LogNotice) << "### Module Stomp: Stats peer "
                   << SocketController::derive_peer_ip(_sock) << ":" << SocketController::derive_peer_port(_sock)
                   << ", sentQ " << _sentQ.size()
                   << ", packets " << _num_packets
                   << ", pps " << pps << "/s, bytes "
                   << kb << "k, "
                   << kbps << " Kbps"
                   << endl;

    for(queueSubs_citr citr=_queueSubs.begin(); citr != _queueSubs.end(); citr++) {
      StompSub *ssub = citr->second;
      log(LogInfo) << "### Sub Queue " << ssub->name() << ", enqueue " << ssub->_mesgQueue.size() << endl;
    } // for

    return true;
  } // StompPeer::_checkLogstats

  const bool StompPeer::_checkInactivity() {
    if (time(NULL) > _inactivityTs)
      return true;

    return false;
  } // StompPeer::_checkInactivity

  const bool StompPeer::_checkEnquireLink() {
    if (time(NULL) < _enquireLinkTs)
      return false;

    _enquireLinkTs = time(NULL) + _enquireLinkInterval;
//    enquire_link();
    return true;
  } // StompPeer::_checkEnquireLink

  const size_t StompPeer::send_frame(StompFrame *frame) {
    assert(frame != NULL); // bug

    string ret = frame->compile();

    stringstream out;
    out << "[ STOMP Packet to " << SocketController::derive_peer_ip(_sock) << ":" << SocketController::derive_peer_port(_sock) << " " << std::setprecision(3) << std::fixed << pps() << " pps ]";
    log(LogDebug) << StringTool::ipad(out.str(), "-", 79) << ">" << endl;
    log(LogDebug) << StringTool::hexdump(ret, ">>  ") << ">>" << endl;

    log(LogDebug) << ">>  " << frame->toString() << endl;

    frame->release();

    return _write(ret);
  } // StompPeer::send_frame

  void StompPeer::_expireIdleConnection() {
/*
    if (!_timeout || !isConnected())
      // Don't expire if timeout is 0
      return;

    if (time(NULL) < (_lastActivityTs+_timeout))
      // not time to expire yet
      return;

    _logf("INFO: Connection expired after %d seconds.", _timeout);

    // connection is expired, disconnect for now.
    disconnect();
*/
  } // StompPeer::_expireIdleConnection

  const size_t StompPeer::receive(const string &buf) {
    _in_l.Lock();
    _in.append(buf);
    _in_l.Unlock();
    return buf.size();
  } // StompPeer::receive

  const size_t StompPeer::process() {
    size_t ret = 0;
    _in_l.Lock();

    for(queueSubs_itr itr = _queueSubs.begin(); itr != _queueSubs.end(); itr++) {
      StompSub *ssub = itr->second;
      for(StompSub::mesgQueue_itr m_itr = ssub->_mesgQueue.begin(); m_itr != ssub->_mesgQueue.end(); m_itr++) {
        StompMessage *smesg = (*m_itr);
        if ( smesg->is_sent() ) continue;
        send_frame( (StompFrame *) smesg);
        smesg->mark_sent();
      } // for
    } // for

    if (_in.length() < 1) {
      _in_l.Unlock();
      return ret;
    } // if

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

    _in_l.Unlock();

    return ret;
  } // StompPeer::process

  const size_t StompPeer::_process_command() {
    size_t ret = 0;
    string command;

    bool ok = _in.removeLine(command);
    if (!ok) return ret;

    ret = command.length();
    StringTool::replace("\r", "", command);

    if (command.length() == 0) {
      _process_reset();
      return 0;
    } // if

log(LogDebug) << "COMMAND(" << StringTool::safe(command) << ")" << endl;

    _stagedFrame.command = command;
    _stage = stompStageHeaders;
    return ret;
  } // StompPeer::process_command

  const size_t StompPeer::_process_headers() {
    size_t ret = 0;
    StompHeader *header;
    string header_str;

    while( _in.removeLine(header_str) ) {
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

log(LogInfo) << header->toString() << endl;
      if (_stagedFrame.headers.find(header->name()) != _stagedFrame.headers.end()) {
        // stomp 1.1 spec says only the first header of the same name is accepted
        delete header;
        continue;
      } // if

      _stagedFrame.headers.insert( make_pair(header->name(), header) );

    } // while

    return ret;
  } // StompPeer::_process_headers

  const size_t StompPeer::_process_body_len() {
    size_t ret = 0;

    if (_stagedFrame.content_length > _in.length())
      // not enough in the buffer not ready yet
      return 0;

    _stagedFrame.body = _in.removeBytes(_stagedFrame.content_length);
    ret = _stagedFrame.body.length();
    _stage = stompStageWaitForNul;

    return ret;
  } // StompPeer::_process_body_len

  const size_t StompPeer::_process_waitfor_nul() {
    string nulbyte;
    bool ok = _in.removeCString(nulbyte);
    if (!ok) return 0;

    StompFrame *frame = new StompFrame(_stagedFrame.command, _stagedFrame.body);
    frame->insert(_stagedFrame.headers.begin(), _stagedFrame.headers.end());
    _stagedFrame.headers.clear();

    _frameQ.push(frame);
    _process_reset();

    return nulbyte.length();
  } // _process_waitfor_nul

  const size_t StompPeer::_process_body_nul() {
    bool ok = _in.removeCString(_stagedFrame.body);
    if (!ok) return 0; // not ready yet

    size_t ret = _stagedFrame.body.length();
    StompFrame *frame = new StompFrame(_stagedFrame.command, _stagedFrame.body);
    frame->insert(_stagedFrame.headers.begin(), _stagedFrame.headers.end());
    _stagedFrame.headers.clear();

    _frameQ.push(frame);

    _process_reset();

    return ret;
  } // StompPeer::_process_body_nul

  void StompPeer::_process_reset() {
    _stagedFrame.command = "";
    _stagedFrame.body = "";
    _stagedFrame.content_length = 0;
    for(stompHeader_itr itr = _stagedFrame.headers.begin(); itr != _stagedFrame.headers.end(); itr++)
      delete itr->second;

    _stagedFrame.headers.clear();
    _stage = stompStageCommand;
  } // StompPeer::_process_reset

  const string::size_type StompPeer::_write(const string &buf) {
    _out_l.Lock();
    _out.append(buf);
    _out_l.Unlock();
//    cout << " > " << StringTool::safe(buf) << endl;
    return buf.size();
  } // write

  const string::size_type StompPeer::transmit(string &ret) {
    _out_l.Lock();
    ret = _out;
    _out = "";
    _out_l.Unlock();
    return ret.size();
  } // StompPeer::transmit

  void StompPeer::wantDisconnect() {
    _disconnect = true;
  } // StompPeer::disconnect

  /**************************************************************************
   ** Commands                                                             **
   **************************************************************************/
  void StompPeer::connect(const string &username, const string &passcode, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("CONNECT");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("login", username);
    frame->add_header("passcode", passcode);
    send_frame(frame);
  } // Stomp::connect

  void StompPeer::connected(const string &session_id, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("CONNECTED");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("session", session_id);
    send_frame(frame);
  } // Stomp::connected

  void StompPeer::send(const string &destination, const string &body, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("SEND", body);
    frame->insert(headers.begin(), headers.end());
    frame->add_header("destination", destination);
    send_frame(frame);
  } // Stomp::send

  void StompPeer::subscribe(const string &destination, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("SUBSCRIBE");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("destination", destination);
    send_frame(frame);
  } // Stomp::subscribe

  void StompPeer::unsubscribe(const string &destination, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("SUBSCRIBE");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("destination", destination);
    send_frame(frame);
  } // Stomp::unsubscribe

  void StompPeer::begin(const string &transaction, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("BEGIN");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("transaction", transaction);
    send_frame(frame);
  } // Stomp::begin

  void StompPeer::commit(const string &transaction, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("COMMIT");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("transaction", transaction);
    send_frame(frame);
  } // Stomp::commit

  void StompPeer::ack(const string &message_id, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("ACK");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("message-id", message_id);
    send_frame(frame);
  } // Stomp::ack

  void StompPeer::abort(const string &transaction, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("ABORT");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("transaction", transaction);
    send_frame(frame);
  } // Stomp::abort

  void StompPeer::message(const string &destination, const string &message_id, const string &body, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("MESSAGE", body);
    frame->insert(headers.begin(), headers.end());
    frame->add_header("destination", destination)
         .add_header("message-id", message_id);
    send_frame(frame);
  } // Stomp::message

  void StompPeer::receipt(const string &receipt_id, stompHeader_t headers) {
    StompFrame *frame = new StompFrame("RECEIPT");
    frame->insert(headers.begin(), headers.end());
    frame->add_header("receipt-id", receipt_id);
    send_frame(frame);
  } // Stomp::receipt

  // ### Subscriptions ###
  const bool StompPeer::subscribe_topic(const string &id, const string &name, const StompSub::ackModeEnum ack) {
    topicSubs_itr itr = _topicSubs.find(id);
    if (itr != _topicSubs.end()) return false;
    StompSub *ssub = new StompSub(id, name, ack);
    _topicSubs.insert( make_pair(id, ssub) );
    log(LogInfo) << "++  topicQ " << ssub->toString() << " [ " << toString() << " ]" << endl;
    return true;
  } // StompPeer::subscribe_topic

  const bool StompPeer::unsubscribe_topic(const string &id) {
    topicSubs_itr itr = _topicSubs.find(id);
    if (itr == _topicSubs.end()) return false;
    delete itr->second;
    _topicSubs.erase(id);
    return true;
  } // StompPeer::unsubscribe_topic

  const bool StompPeer::is_topic_subscriber(const string &id) {
    topicSubs_itr itr = _topicSubs.find(id);
    if (itr == _topicSubs.end()) return false;
    return true;
  } // is_topic_subscriber

  const bool StompPeer::is_topic_match(const string &name) {
    topicSubs_itr itr;
    for(itr = _topicSubs.begin(); itr != _topicSubs.end(); itr++) {
      StompSub *ssub = itr->second;
      if ( ssub->match(name) ) return true;
    } // for
    return false;
  } // is_topic_match
  void StompPeer::clear_topics() {
    for(topicSubs_itr itr = _topicSubs.begin(); itr != _topicSubs.end(); itr++)
      delete itr->second;
    _topicSubs.clear();
  } // StompPeer::clear_topics

  const bool StompPeer::subscribe_queue(const string &id, const string &name, const StompSub::ackModeEnum ack) {
    queueSubs_itr itr = _queueSubs.find(id);
    if (itr != _queueSubs.end()) return false;
    StompSub *ssub = new StompSub(id, name, ack);
    _queueSubs.insert( make_pair(id, ssub) );
    log(LogInfo) << "++  queueQ " << ssub->toString()  << " [ " << toString() << " ]" << endl;
    return true;
  } // StompPeer::subscribe_queue

  const bool StompPeer::unsubscribe_queue(const string &id, mesgList_t &ret) {
    queueSubs_itr itr = _queueSubs.find(id);
    if (itr == _queueSubs.end()) return false;
    itr->second->dequeue_all(ret);
    delete itr->second;
    _queueSubs.erase(id);
    return true;
  } // StompPeer::unsubscribe_queue

  void StompPeer::unsubscribe_queue_all(mesgList_t &ret) {
    for(queueSubs_itr itr = _queueSubs.begin(); itr != _queueSubs.end(); itr++) {
      StompSub *ssub = itr->second;
      ssub->dequeue_all(ret);
      delete ssub;
    } // for
    _queueSubs.clear();
  } // StompPeer::unsubscribe_queue_all

  const bool StompPeer::is_queue_subscriber(const string &id) {
    queueSubs_itr itr = _queueSubs.find(id);
    if (itr == _queueSubs.end()) return false;
    return true;
  } // is_queue_subscriber

  const bool StompPeer::is_queue_match(const string &name, StompSub *&ret) {
    queueSubs_itr itr;
    for(itr = _queueSubs.begin(); itr != _queueSubs.end(); itr++) {
      StompSub *ssub = itr->second;
      if ( ssub->match(name) ) {
        ret = ssub;
        return true;
      } // if
    } // for
    return false;
  } // is_queue_match
  void StompPeer::clear_queues() {
    for(queueSubs_itr itr = _queueSubs.begin(); itr != _queueSubs.end(); itr++)
      delete itr->second;
    _queueSubs.clear();
  } // StompPeer::clear_queues
} // namespace stomp


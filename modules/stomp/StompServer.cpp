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

#include "Server.h"

#include "StompMessage.h"
#include "StompServer.h"
#include "StompPeer.h"

#include "stomp.h"

namespace stomp {
  using namespace std;
  using namespace openframe::loglevel;
  using openframe::StringTool;
  using openframe::VarController;

/**************************************************************************
 ** StompServer Class                                                    **
 **************************************************************************/
  const time_t StompServer::CONNECT_RETRY_TIMEOUT 	= 60;
  const time_t StompServer::DEFAULT_STATS_INTERVAL 	= 3600;

  StompServer::StompServer(const int port, const int max, const time_t sessionInitInterval, const time_t enquireLinkInterval, const time_t inactivityInterval, const time_t responseInterval, const time_t logstatsInterval) :
    ListenController(port, max), _debug(false), _sessionInitInterval(sessionInitInterval), _enquireLinkInterval(enquireLinkInterval), _inactivityInterval(inactivityInterval), _logstatsInterval(logstatsInterval),
    _responseInterval(responseInterval) {

    return;
  } // StompServer::StompServer

  StompServer::~StompServer() {
    return;
  } // StompServer::~StompServer

  const bool StompServer::run() {
    bool didWork = false;
    peers_itr ptr;

    exqueue_queues();
    exqueue_topics();

    list<int> discList;

    _peers_l.Lock();
    for(ptr = _peers.begin(); ptr != _peers.end(); ptr++) {
      StompPeer *peer = ptr->second;
      for(size_t i=0; i < 100; i++) {
        StompFrame *frame;
        bool ok = peer->next_frame(frame);
        if (!ok) break;
        didWork |= ok;
        _process(peer, frame);
        if ( peer->disconnect() ) discList.push_back( peer->sock() );
      } // for
      _send_queues(peer);
    } // for

    _peers_l.Unlock();

    while( !discList.empty() ) {
      disconnect( discList.front() );
      discList.pop_front();
    } // while

    return didWork;
  } // StompServer::run

  void StompServer::_send_queues(StompPeer *peer) {
    list<queues_itr> rm;
    for(queues_itr itr = _queues.begin(); itr != _queues.end(); itr++) {
      StompMessage *smesg = (*itr);
      StompSub *ssub;
      bool ok = peer->is_queue_match( smesg->destination(), ssub );
      if (!ok) continue;
      ssub->enqueue(smesg);
      rm.push_back(itr);
    } // for

    while( !rm.empty() ) {
      StompMessage *smesg = *rm.front();
      _queues.erase(smesg);
      rm.pop_front();
    } // while
  } // StompServer::_send_queues

  void StompServer::_process(StompPeer *peer, StompFrame *frame) {
    stompHeader_t headers;
    switch( frame->type() ) {
      case StompFrame::commandAck:
        _process_ack(peer, frame);
        break;
      case StompFrame::commandConnect:
        peer->connected("1", headers);
        break;
      case StompFrame::commandSend:
        _process_send(peer, frame);
        break;
      case StompFrame::commandSubscribe:
        _process_subscribe(peer, frame);
        break;
      case StompFrame::commandUnsubscribe:
        _process_unsubscribe(peer, frame);
        break;
      default:
        peer->send_error("unknown command");
        break;
    } // switch

    _process_receipt_id(peer, frame);
  } // StompServer::process

  void StompServer::_process_ack(StompPeer *peer, StompFrame *frame) {
    if (!frame->is_header("message-id")) {
      peer->send_error("ack missing message-id");
      return;
    } // if

    if (!frame->is_header("subscription")) {
      peer->send_error("ack missing subscription");
      return;
    } // if

  } // StompServer::_process_ack

  void StompServer::_process_send(StompPeer *peer, StompFrame *frame) {
    if (!frame->is_header("destination")) {
      peer->send_error("send missing destination");
      return;
    } // if

    StringToken st;
    st.setDelimiter('/');
    string destination = frame->get_header("destination");
    st = destination;

    if (st.size() < 2) {
      peer->send_error("destination invalid");
      return;
    } // if

    if (st[0] == "topic") {
      StompMessage *smesg = new StompMessage( st.trail(1), frame->body());
      enqueue_topic(smesg);
    } // if
    else if (st[0] == "queue") {
      StompMessage *smesg = new StompMessage( st.trail(1), frame->body());
      enqueue_queue(smesg);
    } // else if
    else {
      peer->send_error("destination must be a topic or queue");
      return;
    } // else
  } // StompServer::_process_send

  void StompServer::_process_subscribe(StompPeer *peer, StompFrame *frame) {
    stompHeader_t headers;
    if (!frame->is_header("destination")) {
      peer->send_error("subscribe missing destination");
      return;
    } // if

    if (!frame->is_header("id")) {
      peer->send_error("required id field missing");
      return;
    } // if

    StompSub::ackModeEnum ack = StompSub::ackModeAuto;
    if (frame->is_header("ack")) {
      string ack = frame->get_header("ack");
      if (ack == "client")
        ack = StompSub::ackModeClient;
      else if (ack == "client-individual")
        ack = StompSub::ackModeClientIndv;
      else if (ack == "auto")
        ack = StompSub::ackModeAuto;
      else {
        peer->send_error("ack must be auto or client");
        return;
      } // else
    } // if

    StringToken st;
    st.setDelimiter('/');
    string destination = frame->get_header("destination");
    st = destination;

    if (st.size() < 2) {
      peer->send_error("destination invalid");
      return;
    } // if

    string id = frame->get_header("id");
    if (st[0] == "topic")
      peer->subscribe_topic(id, st.trail(1), ack);
    else if (st[0] == "queue")
      peer->subscribe_queue(id, st.trail(1), ack);
    else {
      peer->send_error("destination must be a topic or queue");
      return;
    } // else
  } // StompServer::_process_subscribe

  void StompServer::_process_unsubscribe(StompPeer *peer, StompFrame *frame) {
    stompHeader_t headers;

    if (!frame->is_header("destination")) {
      peer->send_error("subscribe missing destination");
      return;
    } // if

    StringToken st;
    st.setDelimiter('/');
    string destination = frame->get_header("destination");
    st = destination;

    if (st.size() < 2) {
      peer->send_error("destination invalid");
      return;
    } // if

    if (st[0] == "topic")
      peer->unsubscribe_topic( st.trail(1) );
    else if (st[0] == "queue") {
      mesgList_t unsent;
      peer->unsubscribe_queue( st.trail(1), unsent );
      enqueue_queue(unsent);
    } // else if
    else {
      peer->send_error("destination must be a topic or queue");
      return;
    } // else
  } // StompServer::_process_unsubscribe

  void StompServer::_process_receipt_id(StompPeer *peer, StompFrame *frame) {
    if (!frame->is_header("receipt-id"))
      return;

    string receipt_id = frame->get_header("receipt-id");
    stompHeader_t headers;
    peer->receipt(receipt_id, headers);
  } // StompServer::_process_receipt_id

  void StompServer::onConnect(const Connection *con) {
    StompPeer *peer;

    openframe::scoped_lock slock(&_peers_l);
    log(LogNotice) << "### STOMP Connected to " << derive_ip(con->sock) << ":" << derive_port(con->sock) << endl;

    try {
      peer = new StompPeer(con->sock, _sessionInitInterval, _enquireLinkInterval, _inactivityInterval, _responseInterval, _logstatsInterval);
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    peer->elogger(&elog, "stomp");
    _peers.insert( make_pair(con->sock, peer) );

    return;
  } // Worker::onConnect

  // ### Queue Members ###
  void StompServer::enqueue_queue(StompMessage *message) {
    message->retain();
    _queues.insert(message);
    log(LogInfo) << "+   queueQ " << message->toString() << endl;
  } // StompServer::enqueue_queue
  void StompServer::enqueue_queue(mesgList_t &unsent) {
    while( !unsent.empty() ) {
      StompMessage *smesg = unsent.front();
      enqueue_queue(smesg);
      unsent.pop_front();
    } // while
  } // StompServer::enqueue_queue
  void StompServer::exqueue_queues() {
    list<queues_itr> rm;
    for(queues_itr itr = _queues.begin(); itr != _queues.end(); itr++) {
      if ((*itr)->is_inactive())
        rm.push_back(itr);
    } // for

    while(!rm.empty()) {
      StompMessage *smesg = *rm.front();
      log(LogInfo) << "E   queueQ " << smesg->toString() << endl;
      smesg->release();
      _queues.erase(rm.front());
      rm.pop_front();
    } // while
  } // StompServer::exqueue_queues
  void StompServer::dequeue_queue(StompMessage *message) {
    log(LogInfo) << "-   queueQ " << message->toString() << endl;
  } // StompServer::dequeue_queue

  void StompServer::enqueue_topic(StompMessage *message) {
    message->retain();
    _topics.insert(message);
    log(LogInfo) << "+   topicQ " << message->toString() << endl;
  } // StompServer::enqueue_topic
  void StompServer::exqueue_topics() {
    list<topics_itr> rm;
    for(topics_itr itr = _topics.begin(); itr != _topics.end(); itr++) {
      if ((*itr)->is_inactive())
        rm.push_back(itr);
    } // for

    while(!rm.empty()) {
      StompMessage *smesg = *rm.front();
      log(LogInfo) << "E   topicQ " << smesg->toString() << endl;
      smesg->release();
      _topics.erase(rm.front());
      rm.pop_front();
    } // while
  } // StompServer::exqueue_topics
  void StompServer::dequeue_topic(StompMessage *message) {
    log(LogInfo) << "-   topicQ " << message->toString() << endl;
  } // StompServer::dequeue_topic

  void StompServer::onDisconnect(const Connection *con) {
    peers_itr ptr;

    log(LogNotice) << "### STOMP Disconnected from " << derive_ip(con->sock) << ":" << derive_port(con->sock) << endl;

    _peers_l.Lock();
    ptr = _peers.find(con->sock);
    if (ptr != _peers.end()) {
      StompPeer *peer = ptr->second;
      mesgList_t unsent;
      peer->unsubscribe_queue_all(unsent);
      while( !unsent.empty() ) {
        // pull any messages queued to that client
        // and reassign; no need to change retain count
        // as we're maintaining a 1-1 relationship between queues
        StompMessage *smesg = unsent.front();
        _queues.insert(smesg);
        smesg->mark_unsent();
        unsent.pop_front();
      } // while

      peer->release();
      _peers.erase(ptr);
    } // if
    _peers_l.Unlock();

    return;
  } // StompServer::onDisconnect

  void StompServer::onRead(const Peer *lis) {
    peers_itr ptr;

    _peers_l.Lock();
    ptr = _peers.find(lis->sock);
    if (ptr != _peers.end()) {
      if (_debug) {
        stringstream out;
        out << "[ STOMP Packet from " << derive_peer_ip(lis->sock) << ":" << derive_peer_port(lis->sock) << " " << std::setprecision(3) << std::fixed << ptr->second->pps() << " pps ]";
        log(LogDebug) << "<" << StringTool::ppad(out.str(), "-", 79) << endl;
        log(LogDebug) << StringTool::hexdump(lis->in, "<   ") << "<" << endl;
      } // if

      ptr->second->receive(lis->in);
    } // if
    _peers_l.Unlock();

    return;
  } // StompServer::onRead

  const string::size_type StompServer::onWrite(const Peer *lis, string &ret) {
    peers_itr ptr;

    _peers_l.Lock();
    ptr = _peers.find(lis->sock);
    if (ptr != _peers.end())
      ptr->second->transmit(ret);
    _peers_l.Unlock();

    return ret.size();
  } // StompServer::onWrite
} // namespace stomp

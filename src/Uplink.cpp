/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Dynamic Networking Solutions                      **
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
 **************************************************************************
 $Id: Server.cpp,v 1.9 2006/03/21 20:55:12 omni Exp $
 **************************************************************************/

#include <list>
#include <new>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include <openframe/openframe.h>

#include "App_Log.h"
#include "Create.h"
#include "DBI.h"
#include "Decay.h"
#include "Uplink.h"
#include "Feed.h"
#include "MemcachedController.h"
#include "Network.h"
#include "StringTool.h"
#include "Send.h"
#include "Store.h"

#include "openaprs.h"

namespace openaprs {
  using namespace openframe::loglevel;
  using aprs::APRS_Message;
  using aprs::APRS_Position;
  using aprs::APRS_Object;
  using aprs::Create;
  using aprs::Send;
  using openframe::StringToken;
  using std::list;
  using std::ofstream;
  using std::ostream;
  using std::ios;

/**************************************************************************
 ** Server Class                                                         **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

  const time_t Uplink::kLogstatsInterval		= 3600;
  const time_t Uplink::kDecayRetry			= 15;
  const time_t Uplink::kDecayTimeout			= 900;
  const size_t Uplink::kAckLimit			= 1024;

  Uplink::Uplink(const std::string &id,
                 const std::string &hosts,
                 const std::string &vhost,
                 const std::string &stomp_hosts,
                 const std::string &stomp_login,
                 const std::string &stomp_passcode,
                 const bool useMemcached)
         : _id(id),
           _hosts(hosts),
           _vhost(vhost),
           _stomp_hosts(stomp_hosts),
           _stomp_login(stomp_login),
           _stomp_passcode(stomp_passcode),
           _last_logstats( time(NULL) ),
           _logstats_interval(kLogstatsInterval),
           _decay_retry(kDecayRetry),
           _decay_timeout(kDecayTimeout),
           _useMemcached(useMemcached) {

//    logger(alogger);

    // initialize variables
    _timeConnected = 0;
    _totalBytesRead = 0;
    _totalPackets = 0;
    _timeLastRead = 0;
    _queueSize = 0;
    _numBytesPerSecond = 0.0;
    _numPacketsPerSecond = 0.0;
    _burst = "";
    _create = false;

    try {
      _dbi = new DBI();
      _dbi->logger(new App_Log(_logger->ident()));
      _decay = new Decay();
      _decay->logger(new App_Log(_logger->ident()));
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    _profile = new Stopwatch();
    _profile->add("bps", 300);
    _profile->add("pps", 300);
    _profile->add("run.loop", 300);
  } // Uplink::Uplink

  Uplink::~Uplink() {
    if (_feed) {
      _feed->stop();
      delete _feed;
    } // if

    delete _decay;
    delete _dbi;
    delete _profile;
    delete _store;

    while( !_queueList.empty() ) {
      delete _queueList.front();
      _queueList.pop_front();
    } // while

    //_disconnect();
  } // Uplink

  Uplink &Uplink::init() {
    _store = new Store("localhost", 300);
    _store->logger(new App_Log(_logger->ident()));
    _store->init();

    _feed = new Feed(_stomp_hosts, _stomp_login, _stomp_passcode, "/queue/feeds.*");
    _feed->set_elogger( elogger(), elog_name() );
    _feed->set_connect_read_timeout(0);
    _feed->start();

    _last_ack = time(NULL);
    _num_messages = 0;
    _num_frames_out = 0;
    _num_frames_in = 0;
    return *this;
  } // Uplink::init

  void Uplink::stop() {
    try_ack(true);
  } // Uplink::stop

  bool Uplink::try_ack(const bool force) {
    if (!_num_messages) return false;
    bool ok = _num_messages >= 1024 || _last_ack < time(NULL) - 2 || force;
    if (!ok) return false;

    stomp::StompFrame *frame = new stomp::StompFrame("ACK");
    frame->add_header("subscription", _feed->sub_id() );
    frame->add_header("message-id", _last_message_id );
    size_t len = _feed->send_frame(frame);
    frame->release();

    time_t diff = time(NULL) - _last_ack;

    LOG(LogInfo, << "Sending ack for "
                    << _num_messages
                    << " message" << (_num_messages == 1 ? "" : "s")
                    << " to " << _feed->dest_in()
                    << " after " << diff << " second" << (diff == 1 ? "" : "s")
                    << " " << (force ? "(force)" : "")
                    << std::endl);

    _num_frames_out++;
    datapoint("num.frames.out", 1);
    datapoint("num.bytes.out", len);
    _num_messages = 0;
    _last_ack = time(NULL);

    return true;
  } // Uplink::try_ack

  void Uplink::onDescribeStats() {
    describe_stat("num.frames.out", "uplink"+_id+"/frames out", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.frames.in", "uplink"+_id+"/frames in", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.bytes.out", "uplink"+_id+"/bytes out", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.bytes.in", "uplink"+_id+"/bytes in", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.queue", "uplink"+_id+"/queue", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeMean);
    describe_stat("time.run", "uplink"+_id+"/run loop time", openstats::graphTypeGauge, openstats::dataTypeFloat, openstats::useTypeMean);
    //describe_stat("num.packets", "num packets", openstats::graphTypeGauge, openstats::dataTypeInt );
    //describe_stat("num.connects", "num connects", openstats::graphTypeCounter, openstats::dataTypeInt$
    //describe_stat("num.disconnects", "num disconnects", openstats::graphTypeCounter, openstats::dataT$
  } // Uplink::onDescribeStats

  void Uplink::onDestroyStats() {
    destroy_stat("*");
  } // Uplink::onDestroyStats

/***************
 ** Variables **
 ***************/

  // ### Public Members ###
  const Uplink::queueListType::size_type Uplink::dequeue(queueListType &queueList, const int limit) {
    int i = 0;

    while(!_queueList.empty()) {
      if (i > limit)
        break;

      queueList.push_back(_queueList.front());
      _queueList.pop_front();
      --_queueSize;
      i++;
    } // while

    return _queueSize;
  } // Uplink::dequeue

  // ### Protected Members ###
  const bool Uplink::run() {
    Stopwatch sw;

    sw.Start();

    bool didWork = false;
    try_logstats();

    if (!_feed->is_ready()) return false;

    for(size_t i=0; i < 1024 && _feed->process(); i++);
    size_t num_frames_in = 0;
    size_t num_bytes_in = 0;
    for(size_t i=0; i < 1024 && _queueSize < kAckLimit; i++) {
      stomp::StompFrame *frame;
      bool ok = _feed->next_frame(frame);
      if (!ok) break;
      didWork |= ok;
      if (frame->is_command(stomp::StompFrame::commandMessage) ) {
        _last_message_id = frame->get_header("message-id");
        std::string aprs_created_str = frame->get_header("APRS-Created");
        time_t aprs_created = atoi( aprs_created_str.c_str() );
        _queueList.push_back( new UplinkPacket(aprs_created, frame->body()) );
        ++_queueSize;
        _num_messages++;
      } // if

      num_bytes_in += frame->body().length();
      num_frames_in++;
      frame->release();
    } // for
    _num_frames_in += num_frames_in;
    datapoint("num.frames.in", num_frames_in);
    datapoint("num.bytes.in", num_bytes_in);
    datapoint("num.queue", _queueSize );

    try_ack();

    static openframe::Intval create_at(1);
    if ( create_at.is_next() ) {
      _decayNext();
      didWork |= _createMessages();
      didWork |= _createPositions();
      didWork |= _createObjects();
    } // if

    _profile->average("run.loop", sw.Time());
    datapoint_float("time.run", sw.Time() );
    return didWork;
  } // Uplink::run

  unsigned int Uplink::send(const char *sendFormat, ...) {
    char sendBuffer[MAXBUF + 1] = {0};
    va_list va;

    va_start(va, sendFormat);
    vsnprintf(sendBuffer, sizeof(sendBuffer), sendFormat, va);
    va_end(va);

    stomp::StompFrame *frame = new stomp::StompFrame("SEND", sendBuffer);
    std::string buf = frame->toString();
    openframe::StringTool::stripcrlf(buf);
    LOG(LogInfo, << "--> " << buf << std::endl);
    frame->add_header("destination", "/queue/push.aprs.is" );
    size_t len = _feed->send_frame(frame);
    frame->release();

    _num_frames_out++;
    datapoint("num.frames.out", 1);
    datapoint("num.bytes.out", len);

    return strlen(sendBuffer);
  } // Uplink::send

  void Uplink::message(ThreadMessage *tm) {
    if (tm->action() == "disconnect")
      ;//disconnect();
    else if (tm->action() == "remove.decay")
      _processMessage(tm);
    else if (tm->action() == "server.send" && create()) {
      send("%s\n", tm->message().c_str());
      _queueList.push_back( new UplinkPacket(time(NULL), tm->message()) );
      ++_queueSize;
    } // else
  } // Uplink::message

  void Uplink::_processMessage(ThreadMessage *tm) {
    if (create()) _decay->remove(tm->message());
  } // Uplink::_processMessage

  void Uplink::_decayNext() {
    Decay::decayStringsType decayStrings;

    _decay->next(decayStrings);
    while(!decayStrings.empty()) {
      send("%s\n", decayStrings.front().c_str());
      _queueList.push_back( new UplinkPacket(time(NULL), decayStrings.front()) );
      ++_queueSize;
      decayStrings.pop_front();
    } // while
  } // Uplink::_decayNext

  void Uplink::try_logstats() {
    if (_last_logstats > time(NULL) - _logstats_interval) return;

    int diff = time(NULL) - _last_logstats;
    double fps_in = double(_num_frames_in) / diff;
    double fps_out = double(_num_frames_out) / diff;

    LOG(LogNotice, << "Stats queue " << _queueList.size()
                   << ", decay " << _decay->size()
                   << ", frames in " << _num_frames_in
                   << ", fps in " << fps_in << "/s"
                   << ", frames out " << _num_frames_out
                   << ", fps out " << fps_out << "/s"
                   << ", next in " << _logstats_interval
                   << ", connect attempts " << _feed->num_connects()
                   << "; " << _feed->connected_to()
                   << std::endl);

    _num_frames_in = 0;
    _num_frames_out = 0;
    _last_logstats = time(NULL);
  } // try_stats

  const unsigned int Uplink::_createPositions() {
    DBI::resultMapType rs;
    DBI::setMapType sm;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    Send sn;
    APRS_Position p;
    list<APRS_Position> positions;
    std::string position;
    std::string decayId;
    stringstream s;
    unsigned int numCreated = 0;
    unsigned int i = 0;		// Generic counter

    if (!create()) return numCreated;

    sn.logger(new App_Log(_logger->ident()));

    if (_dbi->create->getPendingPositions(sm)) {
      for(i=0; i < sm.size(); i++) {
        rm = sm[i].resultMap;
        p.id = atoi(rm["id"].c_str());
        p.broadcast_ts = atoi(rm["broadcast_ts"].c_str());

        // initialize variables
        p.speed = p.altitude = 0.0;
        p.course = 0;

        p.source = rm["source"];
        p.latitude = atof(rm["latitude"].c_str());
        p.longitude = atof(rm["longitude"].c_str());
        p.symbol_table = rm["symbol_table"][0];
        p.symbol_code = rm["symbol_code"][0];
        p.local = (rm["local"] == "Y") ? true : false;

        if (rm.find("speed") != rm.end())
          p.speed = atof(rm["speed"].c_str());

        if (rm.find("course") != rm.end())
          p.course = int(atof(rm["course"].c_str()));

        if (rm.find("altitude") != rm.end())
          p.altitude = atof(rm["altitude"].c_str());

        if (rm.find("status") != rm.end())
          p.status = rm["status"];
        else
          p.status = "";

        s.str("");
        s << "Create"
          << " position \'"
          << p.source
          << "\'";

        p.title = s.str();

        positions.push_back(p);
      } // while

      while(!positions.empty()) {
        p = positions.front();

        if (sn.Position(p.source, time(NULL), p.latitude, p.longitude,
                        p.symbol_table, p.symbol_code, p.speed,
                        p.course, p.altitude, false, 0, _dbi->handle()->unescape(p.status), p.local,
                        position)) {

          if (!p.local)
            send("%s\n", position.c_str());
          //if (IsServerConsole)
          //  cout << "C";

          _dbi->create->setPositionSent(p.id, time(NULL));
          numCreated++;
        } // if
        else {
          // Don't keep trying to create the same object over and over
          // if an error occurred in creation.
          _dbi->create->setPositionError(p.id);
        } // else

        positions.pop_front();
      } // while

    } // if

    return numCreated;
  } // Uplink::_createPositions

  const unsigned int Uplink::_createMessages() {
    DBI::resultMapType rs;
    DBI::setMapType sm;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    APRS_Message m;
    aprs::Send sn;
    list<APRS_Message> msgs;
    unsigned int i;
    std::string msgid;
    std::string message;
    stringstream s;
    unsigned int numCreated = 0;

    // initialize variables
    i = 0;

    if (!create())
      return numCreated;

    sn.logger(new App_Log(_logger->ident()));

    if (_dbi->create->getPendingMessages(sm)) {
      for(i=0; i < sm.size(); i++) {
        rm = sm[i].resultMap;
        // initialize variables
        m.source = rm["source"];
        m.target = rm["target"];
        m.message = rm["message"];
        m.message = _dbi->handle()->unescape(m.message);
        m.id = atoi(rm["id"].c_str());

        if (rm["local"] == "Y")
          m.local = true;
        else
          m.local = false;

        s.str("");
        s << "Create message \'" << m.message << "\' to " << m.target;
        m.title = s.str();

        // we've got to save it like this so that
        // the Sql query doesn't get squashed by
        // send reparsing messages.
        msgs.push_back(m);
      } // while

      while(!msgs.empty()) {
        m = msgs.front();

        msgid="";

        // only tack on msgid if the client might support reply-acks
        s.str("");
        if (_dbi->lastMessageID(_dbi->handle()->Escape(m.target), msgid)
            && msgid.length() == 2)
          s << m.message << msgid;
        else
          s << m.message;

        if (sn.Message(m.source, m.target, s.str(), m.local, message)) {
          //if (IsServerConsole)
          //  cout << "M";

          if (!m.local) {
            _decay->add(m.source,
                        m.title,
                        message,
                        _decay_retry,
                        _decay_timeout,
                        m.decay_id);

            if (!m.local)
              send("%s\n", message.c_str());
          } // if

          _dbi->create->setMessageSent(m.id, m.decay_id, time(NULL));
          _setMessageSessionInMemcached(m.source);

          numCreated++;
        } // if
        else {
          // if there was an error generated while trying to create this message
          // flag the message as errored and give up, don't keep trying.
          _dbi->create->setMessageError(m.id);
        } // else

        msgs.pop_front();
      } // while
    } // if

    return numCreated;
  } // Uplink::_createMessages

  const bool Uplink::_setMessageSessionInMemcached(const std::string &source) {
    std::string key = aprs::StringTool::toUpper(source);
    bool isOK = true;

    return _store->setAckInMemcached(key, "1", 300);
  } // _setMessageSessionInMemcached

  const unsigned int Uplink::_createObjects() {
    DBI::resultMapType rs;
    DBI::setMapType sm;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    Send sn;
    APRS_Object o;
    list<APRS_Object> objects;
    std::string object;
    std::string decayId;
    stringstream s;
    unsigned int numCreated=0;
    unsigned int i=0;		// Generic counter

    if (!create())
      return numCreated;

    sn.logger(new App_Log(_logger->ident()));

    if (_dbi->create->getPendingObjects(time(NULL), sm)) {
      for(i=0; i < sm.size(); i++) {
        rm = sm[i].resultMap;
        o.id = atoi(rm["id"].c_str());
        o.broadcast_ts = atoi(rm["broadcast_ts"].c_str());
        o.expire_ts = atoi(rm["expire_ts"].c_str());
        o.beacon = atoi(rm["beacon"].c_str());

        if (o.broadcast_ts > 0
            && o.broadcast_ts > (time(NULL) - o.beacon))
          continue;

        // initialize variables
        o.speed = o.altitude = 0.0;
        o.course = 0;

        o.name = rm["name"];
        o.source = rm["source"];
        o.latitude = atof(rm["latitude"].c_str());
        o.longitude = atof(rm["longitude"].c_str());
        o.symbol_table = rm["symbol_table"][0];
        o.symbol_code = rm["symbol_code"][0];
        o.decay_id = rm["decay_id"];
        o.local = (rm["local"] == "Y") ? true : false;

        if (rm.find("speed") != rm.end())
          o.speed = atof(rm["speed"].c_str());

        if (rm.find("course") != rm.end())
          o.course = int(atof(rm["course"].c_str()));

        if (rm.find("altitude") != rm.end())
          o.altitude = atof(rm["altitude"].c_str());

        if (rm.find("status") != rm.end())
          o.status = rm["status"];
        else
          o.status = "";

        o.toKill = ((rm["kill"][0] == 'N') ? true : false);

        // remove any decays for this object
        if (_dbi->getObjectDecayID(o.name, decayId))
          _decay->remove(decayId);

        s.str("");
        s << (o.toKill  == true ? "Create" : "Delete")
          << " object \'"
          << o.name
          << "\'";

        o.title = s.str();

        objects.push_back(o);
      } // while

      while(!objects.empty()) {
        o = objects.front();

        if (sn.Object(o.source, o.name, time(NULL), o.latitude, o.longitude,
                      o.symbol_table, o.symbol_code, o.speed,
                      o.course, o.altitude, o.toKill, false, 0, _dbi->handle()->unescape(o.status),
                      o.local, object)) {

          if (!o.local) {
            if (o.broadcast_ts == 0)
              _decay->add(o.source, o.title, object, 30, 300, o.decay_id);

            send("%s\n", object.c_str());
          } // if

          //if (IsServerConsole)
          //  cout << "C";

          _dbi->create->setObjectSent(o.id, o.decay_id, time(NULL));
          numCreated++;
        } // if
        else {
          // Don't keep trying to create the same object over and over
          // if an error occurred in creation.
          //app->Query("UPDATE create_object SET error='Y' WHERE id = '%d'",
          //           o.id);
          _dbi->create->setObjectError(o.id);
        } // else

        objects.pop_front();
      } // while

    } // if

    return numCreated;
  } // Uplink::_createObjects
} // namespace openaprs

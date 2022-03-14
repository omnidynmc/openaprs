/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** HAL9000, Internet Relay Chat Bot                                     **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Daniel Robert Karrels                             **
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
 $Id: APNS.cpp,v 1.12 2003/09/05 22:23:41 omni Exp $
 **************************************************************************/

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

#include "APRS.h"
#include "Command.h"
#include "DBI.h"
#include "DCCUser.h"
#include "EarthTools.h"
#include "Insert.h"
#include "Fields.h"
#include "NMEA.h"
#include "Network.h"
#include "MemcachedController.h"
#include "PutMySQL.h"
#include "Reply.h"
#include "Server.h"
#include "Symbols.h"
#include "Worker.h"
#include "Vars.h"

#include "openframe/Stopwatch.h"
#include "openframe/StringTool.h"

#include "dccCommands.h"

#include "openaprs.h"
#include "md5wrapper.h"

namespace dcc {
  using namespace std;
  using namespace aprs;
  using namespace putmysql;
  using openaprs::DBI;
  using openaprs::EarthTools;
  using openaprs::Server;
  using handler::Event;
  using handler::EventHandler;
  using openaprs::MemcachedController;
  using openaprs::Command;
  using openaprs::CommandTree;
  using openaprs::Network;
  using openaprs::NMEA;
  using openframe::StringTool;
  using openframe::Stopwatch;

/**************************************************************************
 ** Worker Class                                                         **
 **************************************************************************/

  Worker::Worker(DCC_Log *dcc_log, const time_t statsInterval, const bool useMemcached, const time_t timeMemcachedExpire, const time_t timeout)
                 : _useMemcached(useMemcached), _statsInterval(statsInterval),
                   _timeMemcachedExpire(timeMemcachedExpire), _timeout(timeout) {
    logger(dcc_log);
    _lastStatsTs = time(NULL) + _statsInterval;
    _timeNextMemcachedTry = 0;
    _totalUsersServed = 0;

    _dbi = NULL;
    try {
      _dbi = new DBI();
      _dbi->logger(new App_Log(_logger->ident()));
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    _memcached = NULL;
    if (_useMemcached) {
      _memcached = new MemcachedController("localhost");
      //_memcached->flush(0);
      _memcached->expire(_timeMemcachedExpire);
    } // if

    try {
      _network = new Network();
      _command = new DCC_Command();
      _reply = new Reply();
      _fields = new Fields();
      _symbols = new Symbols();
      _profile = new Stopwatch();
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    _profile->add("run.loop", 300);
    _profile->add("user.command", 300);

    _initializeSystem();
  } // Worker::Worker

  Worker::~Worker() {
    _deinitializeSystem();

    clearWork();
    clearUsers();

    _profile->clear();

    if (_memcached) {
      _memcached->flush(0);
      delete _memcached;
    } // if

    if (_dbi)
      delete _dbi;

    _command->removeCommand("DCC");
    delete _command;
    delete _network;
    delete _reply;
    delete _symbols;
    delete _fields;
    delete _profile;
  } // Worker::~Worker

  void Worker::onDescribeStats() {
  } // Worker::onDescribeStats

  void Worker::onDestroyStats() {
    destroy_stat("*");
  } // Worker::onDestroyStats

  void Worker::_initializeSystem() {
    _initializeEvents();
    _dcclogf("System Initialized");
  } // Worker::_initializeSystem

  void Worker::_deinitializeSystem() {
    _deinitializeEvents();
    _dcclogf("System Deinitialized");
  } // Worker::_deinitializeSystem

  void Worker::_initializeEvents() {
    EventHandler *event;

    event = new eventMESSAGETOSESSION();
    event->vars->push_void("worker", this);
    _eventMap["module.dcc.event.message.session"] = app->addEvent("aprs.message", Event::FLAG_CLEANUP, event);

    event = new eventPOSITIONTOLIVE();
    event->vars->push_void("worker", this);
    _eventMap["module.dcc.event.live.position"] = app->addEvent("aprs.position", Event::FLAG_CLEANUP, event);

    _dcclogf("Events Initialized");
  } // Worker::initializeEvents

  void Worker::_deinitializeEvents() {
    app->removeEvent(_eventMap["module.dcc.event.message.session"]);
    app->removeEvent(_eventMap["module.dcc.event.live.position"]);
    _dcclogf("Events Deinitialized");
  } // Worker::_deinitializeEvents

  void Worker::run() {
    DCCUser *dccuser;
    Work *work;
    Stopwatch sw;
    Stopwatch sw2;
    dccuserSetType::iterator ptr;
    dccuserSetType removeList;
    stringstream s;

    sw.Start();
    _writeStats();

    _network->readConnections();
    _network->writeConnections();

    while(!_workSet.empty()) {
      work = *_workSet.begin();
      _processWork(work);
      _workSet.erase(work);
      delete work;
    } // for

    _dccuserSet_l.Lock();
    for(ptr = _dccuserSet.begin(); ptr != _dccuserSet.end(); ptr++) {
      dccuser = *ptr;
      sw2.Start();
      _process(dccuser);
      _profile->average("user.command", sw2.Time());
      if (IsDCCUserRemove(dccuser))
        removeList.insert(dccuser);
    } // for
    _dccuserSet_l.Unlock();

    // flush connection buffers
    _network->writeConnections();

    _dccuserSet_l.Lock();
    while(!removeList.empty()) {
      dccuser = *removeList.begin();
      _network->removeConnection(dccuser->socket());
      _dccuserSet.erase(dccuser);
      removeList.erase(dccuser);
      //_resultSet.insert(dccuser);
      _dcclogf("%s@%s has disconnected.", dccuser->callsign().c_str(), dccuser->ip().c_str());
      delete dccuser;
    } // while
    _dccuserSet_l.Unlock();

    _profile->average("run.loop", sw.Time());

  } // Worker::run

  void Worker::_processWork(Work *work) {
    DCCUser *dccuser;
    dccuserSetType::iterator ptr;

    assert(work != NULL);

    switch(work->type()) {
      case Work::WORK_CONNECTION:
        //work->connection()->blocking(false);
        //work->connection()->retain(true);
        _network->addConnection(work->connection(), "DCC Client");
        dccuser = new DCCUser(this, work->connection()->getFd(), work->connection()->getIP(), _dbi, _memcached,
                              _command, _reply, _fields, _symbols, _timeout, time(NULL));
        dccuser->logger(new DCC_Log(_logger->ident()));
        dccuser->replace_stats(stats(), stat_id_prefix("dccuser"));

        _dccuserSet_l.Lock();
        _dccuserSet.insert(dccuser);

        if (_maint)
          dccuser->maint_mode();
        else {
          dccuser->Connect();
          dccuser->Welcome();
        } // else
        _dccuserSet_l.Unlock();

        _dcclogf("Connected to %s", dccuser->ip().c_str());
        _totalUsersServed++;
        break;
      case Work::WORK_MESSAGE:
        message(work->to(), work->message());
        break;
      case Work::WORK_SEND:
        send(work->message());
        break;
      case Work::WORK_WALLOPS:
        break;
      case Work::WORK_SEND_ALL:
        send(work->message());
        break;
      case Work::WORK_WALLOPS_ALL:
        wallops(work->message());
        break;
      case Work::WORK_WALLOPS_REPLY_ALL:
        wallopsReply(work->message());
        break;
      default:
        break;
    } // switch
  } // Worker::_processWork

  void Worker::_process(DCCUser *dccuser) {
    int ret;
    string dataReturned;

    assert(dccuser != NULL);		// bug

    dccuser->chkFlood();
    dccuser->chkTimeout();

    // don't want to play with this one?
    if (!IsDCCUserNormal(dccuser))
      return;

    while(true) {
      ret = _network->readConnectionPacket(dccuser->socket(), dataReturned);

      if (ret == NETWORK_TRUE && !_maint) {
        dccuser->Parse(dataReturned);
      } // if
      else if (ret == NETWORK_FALSE) {
        return;
      } // else if
      else if (ret == NETWORK_ERROR) {
        _dcclogf("DCC %s Peer has closed the connection.", dccuser->ip().c_str());
        dccuser->addFlag(DCCUser::FLAG_REMOVE);
        return;
      } // else if
    } // while

  } // Worker::_process

  void Worker::send(const int socket, const string &buf) {
    _network->sendConnectionPacket(socket, buf, buf.length());
  } // Worker::send

  void Worker::send(const string &message) {
    _dccuserSet_l.Lock();
    _send(message);
    _dccuserSet_l.Unlock();
  } // Worker::send

  void Worker::_send(const string &message) {
    DCCUser *dccuser;
    dccuserSetType::iterator ptr;

    for(ptr = _dccuserSet.begin(); ptr != _dccuserSet.end(); ptr++) {
      dccuser = *ptr;

      // Don't send message to user if they aren't logged
      // in or don't support messaging.
      if (!IsDCCUserLogin(dccuser))
        continue;

      dccuser->send(message);
    } // for

  } // Worker::_send

  void Worker::sendTo(const string &to, const string &message) {
    _dccuserSet_l.Unlock();
    _sendTo(to, message);
    _dccuserSet_l.Unlock();
  } // Worker::sendTo

  void Worker::_sendTo(const string &to, const string &message) {
    DCCUser *dccuser;
    dccuserSetType::iterator ptr;

    for(ptr = _dccuserSet.begin(); ptr != _dccuserSet.end(); ptr++) {
      dccuser = *ptr;

      // Don't send message to user if they aren't logged
      // in or don't support messaging.
      if (!IsDCCUserLogin(dccuser))
        continue;

      if (StringTool::toUpper(dccuser->callsign()) != StringTool::toUpper(to))
        continue;

      dccuser->send(message);
    } // for

  } // Worker::_sendTo

  /**
   * DCC::SendToLive
   *
   * Sends a message to all clients.
   *
   * Returns: Number of bytes sent.
   */
  void Worker::sendToLive(Vars &v, const string &reply) {
    _dccuserSet_l.Lock();
    _sendToLive(v, reply);
    _dccuserSet_l.Unlock();
  } // Worker::sendToLive

  void Worker::_sendToLive(Vars &v, const string &reply) {
    DCCUser *dccuser;				// Pointer to a looped to DCCUser.
    NMEA n;
    EarthTools et;
    dccuserSetType::iterator ptr;		// Iterator to a DCCUser map.
    bool isFiltered;
    string filter;
    string fields;
    string compiled;
    stringstream s;
    Vars v2;
    unsigned int i;
    double lat, lon;

    for(ptr = _dccuserSet.begin(); ptr != _dccuserSet.end(); ptr++) {
      // initialize variables
      dccuser = *ptr;

      if (IsDCCUserNormal(dccuser) == false || !IsDCCUserLive(dccuser))
        continue;

      filter = dccuser->var()->get_string("dcc.user.live.filter", "");
      fields = dccuser->var()->get_string("dcc.user.live.fields", "");

      v2 = filter;
      isFiltered = false;
//cout << "SIZE(" << v2.size() << ")" << endl;
      for(i=0; i < v2.size() && !isFiltered; i++) {
//cout << i << ") " << filter << " !!! " << v2[i] << " " << v2.getFieldAtIndex(i) << endl;
        if (v.isFiltered(v2[i], v2.getFieldAtIndex(i)) != Vars::NO)
          isFiltered = true;
      } // for

      if (isFiltered == true)
        continue;

      // Send as NMEA waypoint if requested.
      if (IsDCCUserNMEA(dccuser)) {
        if (!v.isName("la") || !v.isName("ln") || !v.isName("sr"))
          continue;

        lat = atof(v["la"].c_str());
        lon = atof(v["ln"].c_str());

        s.str("");
        s << "$GPWPL,"
          << et.DecimalToNMEA(lat, true) << ","
          << ((lat > 0) ? "N" : "S") << ","
          << et.DecimalToNMEA(lon, false) << ","
          << ((lon < 0) ? "W" : "E") << ","
          << (v.isName("nm") ? v["nm"] : v["sr"]);

        dccuser->sendf("%s*%s\n", s.str().c_str(), n.Checksum(s.str()).c_str());
        continue;
      } // if

      if (v.compile(compiled, fields))
        dccuser->sendf("%s %s\n", reply.c_str(), compiled.c_str());

    } // for

  } // DCC::_sendToLive

  void Worker::wallops(const string &message) {
    _dccuserSet_l.Lock();
    _wallops(message);
    _dccuserSet_l.Unlock();
  } // Worker::wallops

  void Worker::_wallops(const string &message) {
    DCCUser *dccuser;
    dccuserSetType::iterator ptr;

    for(ptr = _dccuserSet.begin(); ptr != _dccuserSet.end(); ptr++) {
      dccuser = *ptr;

      // Don't send message to user if they aren't logged
      // in or don't support messaging.
      if (!IsDCCUserLogin(dccuser))
        continue;

      dccuser->send(message);
    } // for

  } // Worker::_wallops

  const bool Worker::wallopsReply(const string &name) {
    bool ret;
    _dccuserSet_l.Lock();
    ret = _wallopsReply(name);
    _dccuserSet_l.Unlock();
    return ret;
  } // Worker::wallopsReply

  const bool Worker::_wallopsReply(const string &name) {
    string message;

    // Does the error exist?
    if (!_reply->find(name, message)) {
      _wallops("999 MS:Unknown error please contact support.\n");
      return false;
    } // if

    _wallops(message+"\n");
    return true;
  } // Worker::_wallopsReply

  void Worker::message(const string &to, const string &message) {
    _dccuserSet_l.Lock();
    _message(to, message);
    _dccuserSet_l.Unlock();
  } // Worker::message

  void Worker::_message(const string &to, const string &message) {
    DCCUser *dccuser;
    dccuserSetType::iterator ptr;
    string target = StringTool::toUpper(to);

    for(ptr = _dccuserSet.begin(); ptr != _dccuserSet.end(); ptr++) {
      dccuser = *ptr;

      // Don't send message to user if they aren't logged
      // in or don't support messaging.
      if (!IsDCCUserLogin(dccuser) || !IsDCCUserMessaging(dccuser) ||
          !dccuser->var()->exists("dcc.user.messaging.nick"))
        continue;

      if (target != StringTool::toUpper(dccuser->var()->get_string("dcc.user.messaging.nick")))
        continue;

      dccuser->send(message);
    } // for

  } // Worker::_message

  const Worker::resultSetType::size_type Worker::results(resultSetType &resultSet, const int limit) {
    resultSetType::size_type num = 0;
    resultSetType::iterator ptr;
    assert(limit > 0);                  // catch bugs

    while(!_resultSet.empty()) {
      ptr = _resultSet.begin();
      if (num > (unsigned int) limit)
        break;

      resultSet.insert((*ptr));
      _resultSet.erase(ptr);
      num++;
    } // while

    return num;
  } // Worker::results

  void Worker::_writeStats() {
    if (_lastStatsTs > time(NULL))
      return;

    _dccuserSet_l.Lock();
    _dcclogf("Stats work %d, users %d, total users %d, parse %0.5fs",
             _workSet.size(),
             _dccuserSet.size(),
             _totalUsersServed,
             _profile->average("user.command")
            );
    _dccuserSet_l.Unlock();

    _lastStatsTs = time(NULL) + _statsInterval;
  } // Worker::writeStats

  void Worker::add(Work *work) {
    assert(work != NULL);		// catch bugs

    _workSet.insert(work);
  } // Worker::add

  void Worker::clearWork() {
    _clearWork();
  } // Worker::clearWork

  void Worker::_clearWork() {
    Work *work;

    while(!_workSet.empty()) {
      work = *_workSet.begin();
      _workSet.erase(work);
      delete work;
    } // while
  } // Worker::_clearWork

  void Worker::clearUsers() {
    _dccuserSet_l.Lock();
    _clearUsers();
    _dccuserSet_l.Unlock();
  } // Worker::clearUsers

  void Worker::_clearUsers() {
    DCCUser *dccuser;
    dccuserSetType::iterator ptr;

    while(!_dccuserSet.empty()) {
      dccuser = *_dccuserSet.begin();

      if (dccuser->socket() > 0)
        _network->removeConnection(dccuser->socket());

      _dccuserSet.erase(dccuser);
      delete dccuser;
    } // while
  } // Worker::_clearUsers
} // namespace dcc

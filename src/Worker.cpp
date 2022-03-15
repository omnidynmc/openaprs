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

#include <string>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <list>
#include <map>
#include <new>
#include <iostream>

#include <errno.h>
#include <time.h>
#include <math.h>

#include <openframe/Stopwatch.h>

#include "APRS.h"
#include "App_Log.h"
#include "DBI.h"
#include "Feed.h"
#include "Insert.h"
#include "MemcachedController.h"
#include "PutMySQL.h"
#include "Server.h"
#include "Store.h"
#include "StringTool.h"
#include "Worker.h"
#include "Vars.h"

#include "openaprs.h"
#include "md5wrapper.h"

namespace openaprs {
/**************************************************************************
 ** Worker Class                                                         **
 **************************************************************************/

  const char *Worker::kStompDestErrors		= "/topic/feeds.aprs.is.errors";
  const char *Worker::kStompDestRejects		= "/topic/feeds.aprs.is.rejects";
  const char *Worker::kStompDestDuplicates	= "/topic/feeds.aprs.is.duplicates";

  Worker::Worker(const std::string &id,
                 const std::string &stomp_hosts,
                 const std::string &stomp_login,
                 const std::string &stomp_passcode,
                 const time_t statsInterval,
                 const time_t insertInterval,
                 const bool useMemcached,
                 const time_t timeMemcachedExpire)
         : _id(id),
           _stomp_hosts(stomp_hosts),
           _stomp_login(stomp_login),
           _stomp_passcode(stomp_passcode),
           _useMemcached(useMemcached),
           _insertInterval(insertInterval),
           _statsInterval(statsInterval),
           _timeMemcachedExpire(timeMemcachedExpire) {
    _lastStatsTs = time(NULL) + _statsInterval;
    _lastInsertTs = time(NULL) + _insertInterval;
    _numPackets = 0;
    _numSqlInserted = 0;
    _numSqlFailed = 0;
    _timeMessageSessionExpire = 300;

    init_stats(_stompstats, true);
    _stompstats.report_interval = 5;

    _dbi = NULL;
    _sqlq = NULL;
    try {
      _dbi = new DBI();
      _dbi->logger(new App_Log(_logger->ident()));
      _sqlq = new Insert();
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    _store = NULL;

    _memcached = NULL;
    if (_useMemcached) {
      _memcached = new MemcachedController("localhost");
      //_memcached->flush(0);
      _memcached->expire(_timeMemcachedExpire);
    } // if

    _profile = new openframe::Stopwatch();
    _profile->add("packet300", 300);
    _profile->add("packet900", 900, 60);
    _profile->add("packet3600", 3600, 300);
    _profile->add("memcached", 300);
    _profile->add("memcached.callsign", 300);
    _profile->add("memcached.ack", 300);
    _profile->add("memcached.path", 300);
    _profile->add("memcached.status", 300);
    _profile->add("insertsql", 300);
    _profile->add("sql.insert.packet", 300);
    _profile->add("run.loop", 300);
  } // Worker::Worker

  Worker::~Worker() {
    Work *work;
    Result *result;
    resultSetType::iterator rptr;
    workSetType::iterator wptr;

    _resultSet_l.Lock();
    while(!_resultSet.empty()) {
      rptr = _resultSet.begin();
      result = *rptr;
      _resultSet.erase(rptr);
      delete result;
    } // while
    _resultSet_l.Unlock();

    _workSet_l.Lock();
    while(!_workSet.empty()) {
      wptr = _workSet.begin();
      work = *wptr;
      _workSet.erase(wptr);
      delete work;
    } // while
    _workSet_l.Unlock();

    _profile->clear();

    if (_feed) delete _feed;
    if (_store) delete _store;
    if (_sqlq) delete _sqlq;
    if (_dbi) delete _dbi;
    if (_memcached) delete _memcached;

    delete _profile;
  } // Worker::~Worker

  Worker &Worker::init() {
    _store = new Store("localhost", _timeMemcachedExpire, _statsInterval);
    _store->replace_stats( stats(), stat_id_prefix("store") );
    _store->logger( new App_Log( _logger->ident() ) );
    _store->init();

    _feed = new Feed(_stomp_hosts,
                     _stomp_login,
                     _stomp_passcode,
                     "",
                     "/topic/feed.errors.aprs.is");
// FIXME
//    _feed->elogger( elogger(), elog_name() );
    _feed->set_elogger( &elog, "uplink" );
    _feed->set_connect_read_timeout(0);
    _feed->start();

    return *this;
  } // Worker::init

  void Worker::onDescribeStats() {
    describe_stat("num.work.in", "worker"+_id+"/num work in", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.work.out", "worker"+_id+"/work out", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.aprs.rejects", "worker"+_id+"/aprs rejects", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.aprs.duplicates", "worker"+_id+"/aprs duplicates", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.aprs.errors", "worker"+_id+"/aprs errors", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.sql.inserted", "worker"+_id+"/sql inserted", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.sql.failed", "worker"+_id+"/sql failed", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("time.run", "worker"+_id+"/run loop time", openstats::graphTypeGauge, openstats::dataTypeFloat, openstats::useTypeMean);
    describe_stat("time.sql.insert", "worker"+_id+"/sql insert time", openstats::graphTypeGauge, openstats::dataTypeFloat, openstats::useTypeMean);
    describe_stat("time.parse.aprs", "worker"+_id+"/parse aprs time", openstats::graphTypeGauge, openstats::dataTypeFloat, openstats::useTypeMean);
    describe_stat("time.write.event", "worker"+_id+"/write event time", openstats::graphTypeGauge, openstats::dataTypeFloat, openstats::useTypeMean);

    // APRS Packet Stats
    describe_stat("aprs_stats.rate.packet", "aprs stats/rate/packet", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.position", "aprs stats/rate/positions", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.message", "aprs stats/rate/message", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.telemetry", "aprs stats/rate/telemetry", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.status", "aprs stats/rate/status", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.capabilities", "aprs stats/rate/capabilities", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.peet_logging", "aprs stats/rate/peet_logging", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.weather", "aprs stats/rate/weather", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.dx", "aprs stats/rate/dx", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.experimental", "aprs stats/rate/experimental", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.beacon", "aprs stats/rate/beacon", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.unknown", "aprs stats/rate/unknown", openstats::graphTypeCounter);

    describe_stat("aprs_stats.rate.reject.invparse", "aprs stats/rate/reject/invparse", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.reject.duplicate", "aprs stats/rate/reject/duplicate", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.reject.tofast", "aprs stats/rate/reject/tofast", openstats::graphTypeCounter);
    describe_stat("aprs_stats.rate.reject.tosoon", "aprs stats/rate/reject/tosoon", openstats::graphTypeCounter);

    describe_stat("aprs_stats.ratio.position", "aprs stats/ratio/positions", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.message", "aprs stats/ratio/message", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.telemetry", "aprs stats/ratio/telemetry", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.status", "aprs stats/ratio/status", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.capabilities", "aprs stats/ratio/capabilities", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.peet_logging", "aprs stats/ratio/peet_logging", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.weather", "aprs stats/ratio/weather", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.dx", "aprs stats/ratio/dx", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.experimental", "aprs stats/ratio/experimental", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.beacon", "aprs stats/ratio/beacon", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.unknown", "aprs stats/ratio/unknown", openstats::graphTypeGauge, openstats::dataTypeFloat);

    describe_stat("aprs_stats.ratio.reject.invparse", "aprs stats/ratio/reject/invparse", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.reject.duplicate", "aprs stats/ratio/reject/duplicate", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.reject.tosoon", "aprs stats/ratio/reject/too soon", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("aprs_stats.ratio.reject.tofast", "aprs stats/ratio/reject/too fast", openstats::graphTypeGauge, openstats::dataTypeFloat);
  } // Worker::onDescribeStats

  void Worker::onDestroyStats() {
    destroy_stat("*");
  } // Worker::onDestroyStats

  void Worker::init_stats(obj_stompstats_t &stats, const bool startup) {
    memset(&stats.aprs_stats, '\0', sizeof(aprs_stats_t) );

    stats.last_report_at = time(NULL);
    if (startup) stats.created_at = time(NULL);
  } // Worker::init_stats

  void Worker::try_stats() {
    try_stompstats();
//    if (_stats.last_report_at > time(NULL) - _stats.report_interval) return;

  } // Worker::try_stats

  void Worker::try_stompstats() {
    if (_stompstats.last_report_at > time(NULL) - _stompstats.report_interval) return;

    datapoint("aprs_stats.rate.packet", _stompstats.aprs_stats.packet);
    datapoint("aprs_stats.rate.position", _stompstats.aprs_stats.position);
    datapoint("aprs_stats.rate.message", _stompstats.aprs_stats.message);
    datapoint("aprs_stats.rate.telemetry", _stompstats.aprs_stats.telemetry);
    datapoint("aprs_stats.rate.status", _stompstats.aprs_stats.status);
    datapoint("aprs_stats.rate.capabilities", _stompstats.aprs_stats.capabilities);
    datapoint("aprs_stats.rate.peet_logging", _stompstats.aprs_stats.peet_logging);
    datapoint("aprs_stats.rate.weather", _stompstats.aprs_stats.weather);
    datapoint("aprs_stats.rate.dx", _stompstats.aprs_stats.dx);
    datapoint("aprs_stats.rate.experimental", _stompstats.aprs_stats.experimental);
    datapoint("aprs_stats.rate.beacon", _stompstats.aprs_stats.beacon);
    datapoint("aprs_stats.rate.unknown", _stompstats.aprs_stats.unknown);

    datapoint("aprs_stats.rate.reject.invparse", _stompstats.aprs_stats.reject_invparse);
    datapoint("aprs_stats.rate.reject.duplicate", _stompstats.aprs_stats.reject_duplicate);
    datapoint("aprs_stats.rate.reject.tofast", _stompstats.aprs_stats.reject_tofast);
    datapoint("aprs_stats.rate.reject.tosoon", _stompstats.aprs_stats.reject_tosoon);

    datapoint_float("aprs_stats.ratio.position", OPENSTATS_PERCENT(_stompstats.aprs_stats.position, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.message", OPENSTATS_PERCENT(_stompstats.aprs_stats.message, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.telemetry", OPENSTATS_PERCENT(_stompstats.aprs_stats.telemetry, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.status", OPENSTATS_PERCENT(_stompstats.aprs_stats.status, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.capabilities", OPENSTATS_PERCENT(_stompstats.aprs_stats.capabilities, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.peet_logging", OPENSTATS_PERCENT(_stompstats.aprs_stats.peet_logging, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.weather", OPENSTATS_PERCENT(_stompstats.aprs_stats.weather, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.dx", OPENSTATS_PERCENT(_stompstats.aprs_stats.dx, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.experimental", OPENSTATS_PERCENT(_stompstats.aprs_stats.experimental, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.beacon", OPENSTATS_PERCENT(_stompstats.aprs_stats.beacon, _stompstats.aprs_stats.packet) );

    datapoint_float("aprs_stats.ratio.reject.invparse", OPENSTATS_PERCENT(_stompstats.aprs_stats.reject_invparse, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.reject.duplicate", OPENSTATS_PERCENT(_stompstats.aprs_stats.reject_duplicate, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.reject.tofast", OPENSTATS_PERCENT(_stompstats.aprs_stats.reject_tofast, _stompstats.aprs_stats.packet) );
    datapoint_float("aprs_stats.ratio.reject.tosoon", OPENSTATS_PERCENT(_stompstats.aprs_stats.reject_tosoon, _stompstats.aprs_stats.packet) );

    init_stats(_stompstats);
  } // Worker::try_stompstats

  const bool Worker::run() {
    Work *work;
    Result *result;
    openframe::Stopwatch sw;
    workSetType::iterator ptr;
    stringstream s;

    sw.Start();
    try_stats();
    _store->try_stats();
    writeStats();

    _resultSet_l.Lock();
    _workSet_l.Lock();
    bool didWork = !_workSet.empty();
    size_t num_work_in = 0;
    while(!_workSet.empty() && num_work_in < 100) {
      ptr = _workSet.begin();
      work = *ptr;
      result = parse(work);
      _workSet.erase(ptr);

      delete work;		// delete work we're done with it

      assert(result != NULL);	// bug

      _resultSet.insert(result);
      num_work_in++;
    } // while
    _resultSet_l.Unlock();
    _workSet_l.Unlock();
    datapoint("num.work.in", num_work_in);

    insertSqlFromQueue();
    _profile->average("run.loop", sw.Time());
    datapoint_float("time.run", sw.Time() );

    return didWork;
  } // Worker::run

  void Worker::writeStats() {
    if (_lastStatsTs > time(NULL))
      return;

    _workSet_l.Lock();
    _resultSet_l.Lock();
    _logf("Stats packets %d, queue %d, results %d, %0.4f/%0.4f/%0.4f 5min/15min/1hr",
          _numPackets,
          _workSet.size(),
          _resultSet.size(),
          _profile->average("packet300"),
          _profile->average("packet900"),
          _profile->average("packet3600")
         );
    _workSet_l.Unlock();
    _resultSet_l.Unlock();

    _logf("Sql inserted %d, failed %d, time %0.4f",
          _numSqlInserted,
          _numSqlFailed,
          _profile->average("insertsql")
         );

    _lastStatsTs = time(NULL) + _statsInterval;
  } // Worker::writeStats

  void Worker::insertSqlFromQueue() {
    Insert::insertListType queue;
    openframe::Stopwatch sw;

    if (_lastInsertTs > time(NULL))
      return;

    _sqlq->Dequeue(queue);

    sw.Start();
    size_t num_sql_failed = 0;
    size_t num_sql_inserted = 0;
    while(!queue.empty()) {
      if (_dbi->handle()->query(queue.front().c_str(), queue.front().size()) == false) {
        _logf("*** SQL Error: #%d %s", _dbi->handle()->getErrorNumber(), _dbi->handle()->getErrorMessage().c_str());
        _debugf("Worker::Insert::Flush> #%d %s", _dbi->handle()->getErrorNumber(), _dbi->handle()->getErrorMessage().c_str());
        _slogf(OPENAPRS_LOG_SQL, "%s", queue.front().c_str());
        ++num_sql_failed;
      } // if
      else
        ++num_sql_inserted;

      queue.pop_front();
    } // while
    _numSqlInserted += num_sql_inserted;
    _numSqlFailed += num_sql_failed;
    datapoint("num.sql.inserted", num_sql_inserted);
    datapoint("num.sql.failed", num_sql_failed);

    double timeInsertSql = sw.Time();
    _profile->average("insertsql", timeInsertSql);
    datapoint_float("time.sql.insert", sw.Time() );

    _lastInsertTs = time(NULL) + _insertInterval;
  } // Worker::insertSqlFromQueue

  void Worker::add(Work *work) {
    assert(work != NULL);		// catch bugs

    openframe::scoped_lock slock(&_workSet_l);
    _workSet.insert(work);
  } // Worker::add

  const Worker::resultSetType::size_type Worker::results(resultSetType &resultSet, const int limit) {
    resultSetType::size_type num = 0;
    resultSetType::iterator ptr;
    assert(limit > 0);			// catch bugs

    _resultSet_l.Lock();
    while(!_resultSet.empty()) {
      ptr = _resultSet.begin();
      if (num > (unsigned int) limit)
        break;

      resultSet.insert((*ptr));
      _resultSet.erase(ptr);
      num++;
    } // while
    _resultSet_l.Unlock();

    datapoint("num.work.out", num);
    return num;
  } // Worker::results

  Result *Worker::parse(Work *work) {
    Result *result = new Result(work->packet(), time(NULL));
    openframe::Stopwatch sw;
    openframe::Stopwatch sw_memcached;
    string parseMe = work->packet();
    string callsignId;
    string packetId;
    string pathId;
    string statusId;
    stringstream s;
    time_t timestamp = work->timestamp();

    _numPackets++;

    // ## PROFILING: measure time to parse
    sw.Start();

    try {
      result->_aprs = new aprs::APRS(parseMe, timestamp);
    } // try
    catch(aprs::APRS_Exception e) {
      result->_aprs = NULL;
      result->_error = e.message();
      _stompstats.aprs_stats.reject_invparse++;
      post_error(kStompDestErrors, work->packet(), result );
      return result;
    } // catch

    // take care of callsign id
    if (_store->getCallsignId(result->_aprs->source(), callsignId))
      result->_aprs->replaceString("aprs.packet.callsign.id", callsignId);
    else {
      result->_reject = true;
      result->_error = "could not get callsign id";
    } // if

    // take care of callsign id
    if (_store->getPathId(result->_aprs->path(), pathId))
      result->_aprs->replaceString("aprs.packet.path.id", pathId);
    else {
      result->_reject = true;
      result->_error = "could not get path id";
    } // else

    // take care of packet id
    if (_store->getPacketId(callsignId, packetId))
      result->_aprs->replaceString("aprs.packet.id", packetId);
    else {
      result->_reject = true;
      result->_error = "could not get packet id";
    } // ele

    // if we're a position or status report we'll have some additional text as a comment
    if (result->_aprs->packetType() == aprs::APRS::APRS_PACKET_POSITION) {
      if (_store->getStatusId(result->_aprs->status(), statusId))
        result->_aprs->replaceString("aprs.packet.status.id", statusId);
      else {
        result->_reject = true;
        result->_error = "could not get status id";
      } // else
    } // if

    sw_memcached.Start();
    checkForDuplicates(result);
    checkForPositionErrors(result);
    double timeMemcached = sw_memcached.Time();

    processResult(result);

    result->_parseTime = sw.Time();
    _profile->average("packet300", result->_parseTime);
    _profile->average("packet900", result->_parseTime);
    _profile->average("packet3600", result->_parseTime);
    _profile->average("memcached", timeMemcached);
    datapoint_float("time.parse.aprs", sw.Time() );

    if (result->_reject) {
      if (!result->_error.length() && result->_aprs->isString("aprs.packet.error.message"))
        result->_error = result->_aprs->getString("aprs.packet.error.message");
      post_error(kStompDestRejects, work->packet(), result);
    } // if

    return result;
  } // Worker::parse

  void Worker::post_error(const char *dest, const std::string &packet, const Result *result) {
    std::string status;
    if (result->_reject) {
      status = "rejected";
      datapoint("num.aprs.rejects", 1);
    } // if
    else if (result->_duplicate) {
     status = "duplicate";
     datapoint("num.aprs.duplicates", 1);
    } // else if
    else {
     status = "error";
     datapoint("num.aprs.errors", 1);
    } // else

    stringstream out;
    out << "{ \"packet\"  : \"" << openframe::StringTool::escape(packet) << "\"," << std::endl
        << "  \"error\"   : \"" << openframe::StringTool::escape(result->_error) << "\"," << std::endl
        << "  \"status\"  : \"" << status << "\"," << std::endl
        << "  \"created\" : \"" << time(NULL) << "\" }"
        << std::endl;

    _feed->post( dest, out.str() );
  } // Worker::post_error

  void Worker::checkForDuplicates(Result *result) {
    Vars *v;
    md5wrapper md5;
    string buf;
    string key;
    string body;

    body = aprs::StringTool::toLower(result->_aprs->source() + ":" + result->_aprs->body());
    key = md5.getHashFromString(body);

    bool found = _store->getDuplicateFromMemcached(key, buf);

    if (found) {
      _debugf("memcached{dup} found key %s", key.c_str());
      _debugf("memcached{dup} body: %s", body.c_str());
      _debugf("memcached{dup} data: %s", buf.c_str());

      v = new Vars(buf);
      bool exists = v->exists("ct");
      if (exists) {
        time_t diff = time(NULL) - atoi( (*v)["ct"].c_str() );
        if (diff < _timeMemcachedExpire) {
          result->_duplicate = true;
          _stompstats.aprs_stats.reject_duplicate++;
          post_error(kStompDestDuplicates, result->_aprs->body(), result);
        } // if
      } // if
      else assert(false);	// bug
      delete v;
    } // if
    else {
      // not found must add it
      v = new Vars();

      v->add("sr", result->_aprs->source());
      v->add("ct", result->_aprs->getString("aprs.packet.timestamp"));
      if (result->_aprs->packetType() == aprs::APRS::APRS_PACKET_POSITION) {
        v->add("la", result->_aprs->getString("aprs.packet.position.latitude.decimal"));
        v->add("ln", result->_aprs->getString("aprs.packet.position.longitude.decimal"));
      } // if

      v->compile(buf, "");
      delete v;

      _store->setDuplicateInMemcached(key, buf);
    } // else
  } // Worker::checkForDuplicates

  void Worker::checkForPositionErrors(Result *result) {
    aprs::APRS *aprs = result->_aprs;
    Vars *v;
    bool isLame = false;
    md5wrapper md5;
    string buf = "";
    string key;
    string body;
    string comment;
    double speed;
    double distance;
    double tlat, tlng;
    time_t diff, ct;

    if (result->duplicate())
      // don't process dups
      return;

    if (aprs->packetType() != aprs::APRS::APRS_PACKET_POSITION
        || aprs->isString("aprs.packet.object.name"))
      // is not a position or is an object which will be automated and
      // can present multiple posotions by same source quickly
      return;

    key = aprs::StringTool::toLower(result->_aprs->source());

    bool found = _store->getPositionFromMemcached(key, buf);

    comment = md5.getHashFromString(aprs->getString("aprs.packet.comment"));
    if (found) {
      // do position err checks
      v = new Vars(buf);

      // is it a position we can use?
      if (v->exists("la,ln,ct,cm")) {
        tlat = atof((*v)["la"].c_str());
        tlng = atof((*v)["ln"].c_str());
        ct = atol((*v)["ct"].c_str());

        distance = aprs::APRS::calcDistance(tlat, tlng, aprs->lat(), aprs->lng(), 'M');

        diff = aprs->timestamp() - ct;
        speed = aprs::APRS::calcSpeed(distance, diff, 8, 1);

        if (diff < 5
            && comment == (*v)["cm"]) {
          // probably don't want to do this, catches digis that advertise
          // two packets with different comment content
          _debugf("memcached{pos}, found key %s", key.c_str());
          _debugf("memcached{pos}, data: %s", buf.c_str());
          _debugf("memcached{pos}, pos: %f, %f", aprs->lat(), aprs->lng());
          _debugf("memcached{pos}, lame: %d seconds", diff);
          _stompstats.aprs_stats.reject_tosoon++;
          aprs->addString("aprs.packet.error.message", "position: tx < 5 seconds");
          isLame = true;
        } // if
        else if (speed > 500 && comment == (*v)["cm"]) {
          _debugf("memcached{pos}, found key %s", key.c_str());
          _debugf("memcached{pos}, data: %s", buf.c_str());
          _debugf("memcached{pos}, pos: %f, %f", aprs->lat(), aprs->lng());
          _debugf("memcached{pos}, lame: speed %d", speed);
          _stompstats.aprs_stats.reject_tofast++;
          aprs->addString("aprs.packet.error.message", "position: gps glitch speed > 500");
          isLame = true;
        } // if

      } // if

      delete v;

      result->_reject = isLame;
    } // if

    if (!result->reject()) {
      v = new Vars();

      v->add("sr", aprs->source());
      v->add("la", aprs->getString("aprs.packet.position.latitude.decimal"));
      v->add("ln", aprs->getString("aprs.packet.position.longitude.decimal"));
      v->add("ct", aprs->getString("aprs.packet.timestamp"));
      v->add("cm", comment);

      v->compile(buf, "");
      delete v;

      //_logf("key: %s, not found", result->_aprs->source().c_str());

      _store->setPositionInMemcached(key, buf);
    } // if
  } // Worker::checkForPositionErrors

  void Worker::checkForMessageAcks(Result *result) {
    aprs::APRS *aprs = result->_aprs;
    openframe::Stopwatch sw;
    bool isSession;
    string buf;
    string key;
    string target;

    // is this a message that we can ack?
    if (!aprs->isString("aprs.packet.message.id") && !aprs->isString("aprs.packet.message.ack"))
      return;

    target = aprs->getString("aprs.packet.message.target");
    key = aprs::StringTool::toUpper(target);

    sw.Start();

    bool found = _store->getAckFromMemcached(key, buf);

    if (found) {
      _debugf("memcached{ack} found key %s", key.c_str());
      _debugf("memcached{ack} data: %s", buf.c_str());
      result->_ack = true;
    } // if
    else {
      buf="1";

      // Find out if the target user of this message has
      // an open session with OpenAPRS.
      isSession = _dbi->isUserSession(target, (time(NULL)-_timeMessageSessionExpire));

      if (isSession) {
        result->_ack = true;
        _store->setAckInMemcached(key, buf, _timeMessageSessionExpire);
      } // if
    } // else

  } // Worker::checkForMessageAcks

  void Worker::processResult(Result *result) {
    aprs::APRS *aprs;
    Packet *nPacket;
    string id;

    assert(result != NULL);

    aprs = result->aprs();

    if (aprs == NULL)
      // nothing to do
      return;

    if (IsServerDropDuplicates
        && (result->duplicate() || result->reject()))
      // also nothing to do
      return;

    openframe::Stopwatch sw;
    sw.Start();

    nPacket = new Packet();
    nPacket->addVariable("aprs", aprs);
    nPacket->addVariable("sqlq", _sqlq);
    nPacket->addVariable("dbi", _dbi);
    nPacket->addVariable("memcached", _memcached);
    nPacket->addVariable("log", _logger);
    nPacket->addVariable("result", result);

    _stompstats.aprs_stats.packet++;
    app->writeEvent("aprs.raw", nPacket);
    switch(aprs->packetType()) {
      case aprs::APRS::APRS_PACKET_POSITION:
        _stompstats.aprs_stats.position++;
        app->writeEvent("aprs.position", nPacket);
        break;
      case aprs::APRS::APRS_PACKET_MESSAGE:
        _stompstats.aprs_stats.message++;
        checkForMessageAcks(result);
        app->writeEvent("aprs.message", nPacket);
        break;
      case aprs::APRS::APRS_PACKET_TELEMETRY:
        _stompstats.aprs_stats.telemetry++;
        app->writeEvent("aprs.telemetry", nPacket);
        break;
      case aprs::APRS::APRS_PACKET_STATUS:
        _stompstats.aprs_stats.status++;
        app->writeEvent("aprs.status", nPacket);
        break;
      case aprs::APRS::APRS_PACKET_CAPABILITIES:
        _stompstats.aprs_stats.capabilities++;
        app->writeEvent("aprs.capabilities", nPacket);
        break;
      case aprs::APRS::APRS_PACKET_PEETLOGGING:
        _stompstats.aprs_stats.peet_logging++;
        app->writeEvent("aprs.peetlogging", nPacket);
        break;
      case aprs::APRS::APRS_PACKET_WEATHER:
        _stompstats.aprs_stats.weather++;
        app->writeEvent("aprs.weather", nPacket);
        break;
      case aprs::APRS::APRS_PACKET_DX:
        _stompstats.aprs_stats.dx++;
        app->writeEvent("aprs.dx", nPacket);
        break;
      case aprs::APRS::APRS_PACKET_EXPERIMENTAL:
        _stompstats.aprs_stats.experimental++;
        app->writeEvent("aprs.experimental", nPacket);
        break;
      case aprs::APRS::APRS_PACKET_BEACON:
        _stompstats.aprs_stats.beacon++;
        app->writeEvent("aprs.beacon", nPacket);
        break;
      case aprs::APRS::APRS_PACKET_UNKNOWN:
        _stompstats.aprs_stats.unknown++;
        app->writeEvent("aprs.unknown", nPacket);
        break;
      default:
        break;
    } // switch

    delete nPacket;

    datapoint_float("time.write.event", sw.Time() );
  } // Worker::processResult
} // namespace openaprs

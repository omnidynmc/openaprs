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
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <new>
#include <iostream>

#include <errno.h>
#include <time.h>
#include <math.h>

#include "App_Log.h"
#include "DBI.h"
#include "MemcachedController.h"
#include "Store.h"
#include "Vars.h"

#include <openframe/Stopwatch.h>
#include <openstats/StatsClient_Interface.h>

#include "openaprs.h"
#include "md5wrapper.h"

namespace openaprs {

/**************************************************************************
 ** Store Class                                                         **
 **************************************************************************/
  const time_t Store::kDefaultReportInterval			= 3600;

  Store::Store(const std::string memcached_host, const time_t expire_interval, const time_t report_interval)
        : _memcached_host(memcached_host),
          _expire_interval(expire_interval) {


    init_stats(_stats, true);
    init_stats(_stompstats, true);

    _stats.report_interval = report_interval;
    _stompstats.report_interval = 5;

    _last_cache_fail_at = 0;

    _dbi = NULL;
    _memcached = NULL;
    _profile = NULL;
  } // Store::Store

  Store::~Store() {
    if (_memcached) delete _memcached;
    if (_dbi) delete _dbi;
    if (_profile) delete _profile;
  } // Store::~Store

  Store &Store::init() {
    try {
      _dbi = new DBI();
      _dbi->logger(new App_Log(_logger->ident()));
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    _memcached = new MemcachedController(_memcached_host);
    //_memcached->flush(0);
    _memcached->expire(_expire_interval);

    _profile = new openframe::Stopwatch();
    _profile->add("memcached.callsign", 300);
    _profile->add("memcached.ack", 300);
    _profile->add("memcached.path", 300);
    _profile->add("memcached.status", 300);
    _profile->add("memcached.duplicates", 300);
    _profile->add("memcached.positions", 300);
    _profile->add("sql.insert.path", 300);

    return *this;
  } // Store::init

  void Store::init_stats(obj_stats_t &stats, const bool startup) {
    memset(&stats.cache_store, '\0', sizeof(memcache_stats_t) );
    memset(&stats.cache_ack, '\0', sizeof(memcache_stats_t) );
    memset(&stats.cache_callsign, '\0', sizeof(memcache_stats_t) );
    memset(&stats.cache_packet, '\0', sizeof(memcache_stats_t) );
    memset(&stats.cache_path, '\0', sizeof(memcache_stats_t) );
    memset(&stats.cache_status, '\0', sizeof(memcache_stats_t) );
    memset(&stats.cache_duplicates, '\0', sizeof(memcache_stats_t) );
    memset(&stats.cache_positions, '\0', sizeof(memcache_stats_t) );

    memset(&stats.sql_store, '\0', sizeof(sql_stats_t) );
    memset(&stats.sql_ack, '\0', sizeof(sql_stats_t) );
    memset(&stats.sql_callsign, '\0', sizeof(sql_stats_t) );
    memset(&stats.sql_packet, '\0', sizeof(sql_stats_t) );
    memset(&stats.sql_path, '\0', sizeof(sql_stats_t) );
    memset(&stats.sql_status, '\0', sizeof(sql_stats_t) );

    stats.last_report_at = time(NULL);
    if (startup) stats.created_at = time(NULL);
  } // init_stats

  void Store::onDescribeStats() {
    describe_stat("num.cache.store.hits", "store/cache/store/num hits - store", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.store.misses", "store/cache/store/num misses - store", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.store.tries", "store/cache/store/num tries - store", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.store.stored", "store/cache/store/num stored - store", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.store.hitrate", "store/cache/store/num hitrate - store", openstats::graphTypeGauge, openstats::dataTypeFloat);

    describe_stat("num.cache.ack.hits", "store/cache/ack/num hits - ack", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.ack.misses", "store/cache/ack/num misses - ack", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.ack.tries", "store/cache/ack/num tries - ack", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.ack.stored", "store/cache/ack/num stored - ack", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.ack.hitrate", "store/cache/ack/num hitrate - ack", openstats::graphTypeGauge, openstats::dataTypeFloat);

    describe_stat("num.cache.callsign.hits", "store/cache/callsign/num hits - callsign", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.callsign.misses", "store/cache/callsign/num misses - callsign", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.callsign.tries", "store/cache/callsign/num tries - callsign", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.callsign.stored", "store/cache/callsign/num stored - callsign", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.callsign.hitrate", "store/cache/callsign/num hitrate - callsign", openstats::graphTypeGauge, openstats::dataTypeFloat);

    describe_stat("num.cache.packet.hits", "store/cache/packet/num hits - packet", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.packet.misses", "store/cache/packet/num misses - packet", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.packet.tries", "store/cache/packet/num tries - packet", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.packet.stored", "store/cache/packet/num stored - packet", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.packet.hitrate", "store/cache/packet/num hitrate - packet", openstats::graphTypeGauge, openstats::dataTypeFloat);

    describe_stat("num.cache.path.hits", "store/cache/path/num hits - path", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.path.misses", "store/cache/path/num misses - path", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.path.tries", "store/cache/path/num tries - path", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.path.stored", "store/cache/path/num stored - path", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.path.hitrate", "store/cache/path/num hitrate - path", openstats::graphTypeGauge, openstats::dataTypeFloat);

    describe_stat("num.cache.status.hits", "store/cache/status/num hits - status", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.status.misses", "store/cache/status/num misses - status", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.status.tries", "store/cache/status/num tries - status", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.status.stored", "store/cache/status/num stored - status", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.status.hitrate", "store/cache/status/num hitrate - status", openstats::graphTypeGauge, openstats::dataTypeFloat);

    describe_stat("num.cache.duplicates.hits", "store/cache/duplicates/num hits - duplicates", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.duplicates.misses", "store/cache/duplicates/num misses - duplicates", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.duplicates.tries", "store/cache/duplicates/num tries - duplicates", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.duplicates.stored", "store/cache/duplicates/num stored - duplicates", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.duplicates.hitrate", "store/cache/duplicates/num hitrate - duplicates", openstats::graphTypeGauge, openstats::dataTypeFloat);

    describe_stat("num.cache.positions.hits", "store/cache/positions/num hits - positions", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.positions.misses", "store/cache/positions/num misses - positions", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.positions.tries", "store/cache/positions/num tries - positions", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.positions.stored", "store/cache/positions/num stored - positions", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.cache.positions.hitrate", "store/cache/positions/num hitrate - positions", openstats::graphTypeGauge, openstats::dataTypeFloat);

    describe_stat("num.sql.store.hits", "store/sql/store/num hits - store", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.store.misses", "store/sql/store/num misses - store", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.store.tries", "store/sql/store/num tries - store", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.store.inserted", "store/sql/store/num inserted - store", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.store.failed", "store/sql/store/num failed - store", openstats::graphTypeCounter, openstats::dataTypeInt);

    describe_stat("num.sql.callsign.hits", "store/sql/callsign/num hits - callsign", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.callsign.misses", "store/sql/callsign/num misses - callsign", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.callsign.tries", "store/sql/callsign/num tries - callsign", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.callsign.hitrate", "store/sql/callsign/num hitrate - callsign", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("num.sql.callsign.inserted", "store/sql/callsign/num inserted - callsign", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.callsign.failed", "store/sql/callsign/num failed - callsign", openstats::graphTypeCounter, openstats::dataTypeInt);

    describe_stat("num.sql.packet.hits", "store/sql/packet/num hits - packet", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.packet.misses", "store/sql/packet/num missess - packet", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.packet.tries", "store/sql/packet/num tries - packet", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.packet.inserted", "store/sql/packet/num inserted - packet", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.packet.failed", "store/sql/packet/num failed - packet", openstats::graphTypeCounter, openstats::dataTypeInt);

    describe_stat("num.sql.path.hits", "store/sql/path/num hits - path", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.path.misses", "store/sql/path/num misses - path", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.path.tries", "store/sql/path/num tries - path", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.path.hitrate", "store/sql/path/num hitrate - path", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("num.sql.path.inserted", "store/sql/path/num inserted - path", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.path.failed", "store/sql/path/num failed - path", openstats::graphTypeCounter, openstats::dataTypeInt);

    describe_stat("num.sql.status.hits", "store/sql/status/num hits - status", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.status.misses", "store/sql/status/num misses - status", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.status.tries", "store/sql/status/num tries - status", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.status.hitrate", "store/sql/status/num hitrate - status", openstats::graphTypeGauge, openstats::dataTypeFloat);
    describe_stat("num.sql.status.inserted", "store/sql/status/num inserted - status", openstats::graphTypeCounter, openstats::dataTypeInt);
    describe_stat("num.sql.status.failed", "store/sql/status/num failed - status", openstats::graphTypeCounter, openstats::dataTypeInt);
  } // Store::onDescribeStats

  void Store::onDestroyStats() {
    destroy_stat("*");
  } // Store::onDestroyStats

  void Store::try_stats() {
    try_stompstats();

    if (_stats.last_report_at > time(NULL) - _stats.report_interval) return;

    _logf("Memcached{callsign} hits %d, misses %d, tries %d, rate %%%0.2f average %0.4fs",
          _stats.cache_callsign.hits,
          _stats.cache_callsign.misses,
          _stats.cache_callsign.tries,
          OPENSTATS_PERCENT(_stats.cache_callsign.hits, _stats.cache_callsign.tries),
            _profile->average("memcached.callsign")
         );

    _logf("Memcached{path} hits %d, misses %d, tries %d, rate %%%0.2f average %0.4fs",
          _stats.cache_path.hits,
          _stats.cache_path.misses,
          _stats.cache_path.tries,
          OPENSTATS_PERCENT(_stats.cache_path.hits, _stats.cache_path.tries),
          _profile->average("memcached.path")
       );

    _logf("Memcached{status} hits %d, misses %d, tries %d, rate %%%0.2f average %0.4fs",
          _stats.cache_status.hits,
          _stats.cache_status.misses,
          _stats.cache_status.tries,
          OPENSTATS_PERCENT(_stats.cache_status.hits, _stats.cache_status.tries),
          _profile->average("memcached.status")
       );

    _logf("Memcached{pos} hits %d, misses %d, tries %d, rate %%%0.2f average %0.4fs",
          _stats.cache_positions.hits,
          _stats.cache_positions.misses,
          _stats.cache_positions.tries,
          OPENSTATS_PERCENT(_stats.cache_positions.hits, _stats.cache_positions.tries),
          _profile->average("memcached.positions")
       );

    _logf("Memcached{dups} hits %d, misses %d, tries %d, rate %%%0.2f average %0.4fs",
          _stats.cache_duplicates.hits,
          _stats.cache_duplicates.misses,
          _stats.cache_duplicates.tries,
          OPENSTATS_PERCENT(_stats.cache_duplicates.hits, _stats.cache_duplicates.tries),
          _profile->average("memcached.duplicates")
       );

    _logf("Sql{callsign} hits %d, misses %d, tries %d, rate %%%0.2f",
          _stats.sql_callsign.hits,
          _stats.sql_callsign.misses,
          _stats.sql_callsign.tries,
          OPENSTATS_PERCENT(_stats.sql_callsign.hits, _stats.sql_callsign.tries)
       );

    _logf("Sql{path} hits %d, misses %d, tries %d, rate %%%0.2f",
          _stats.sql_path.hits,
          _stats.sql_path.misses,
          _stats.sql_path.tries,
          OPENSTATS_PERCENT(_stats.sql_path.hits, _stats.sql_path.tries)
       );

    _logf("Sql{status} hits %d, misses %d, tries %d, rate %%%0.2f",
          _stats.sql_status.hits,
          _stats.sql_status.misses,
          _stats.sql_status.tries,
          OPENSTATS_PERCENT(_stats.sql_status.hits, _stats.sql_status.tries)
       );

    init_stats(_stats);
  } // Store::try_stats

  void Store::try_stompstats() {
    if (_stompstats.last_report_at > time(NULL) - _stompstats.report_interval) return;

    // this prevents stompstats from having to lookup strings in
    // its hash tables over and over again in realtime at ~35 pps

    datapoint("num.sql.callsign.tries", _stompstats.sql_callsign.tries);
    datapoint("num.sql.callsign.hits", _stompstats.sql_callsign.hits);
    datapoint_float("num.sql.callsign.hitrate", OPENSTATS_PERCENT(_stompstats.sql_callsign.hits, _stompstats.sql_callsign.tries) );
    datapoint("num.sql.callsign.misses", _stompstats.sql_callsign.misses);
    datapoint("num.sql.callsign.inserted", _stompstats.sql_callsign.inserted);
    datapoint("num.sql.callsign.failed", _stompstats.sql_callsign.failed);

    datapoint("num.sql.packet.inserted", _stompstats.sql_packet.inserted);
    datapoint("num.sql.packet.failed", _stompstats.sql_packet.failed);

    datapoint("num.cache.callsign.tries", _stompstats.cache_callsign.tries);
    datapoint("num.cache.callsign.misses", _stompstats.cache_callsign.misses);
    datapoint("num.cache.callsign.hits", _stompstats.cache_callsign.hits);
    datapoint_float("num.cache.callsign.hitrate", OPENSTATS_PERCENT(_stompstats.cache_callsign.hits, _stompstats.cache_callsign.tries) );
    datapoint("num.cache.callsign.stored", _stompstats.cache_callsign.stored);

    datapoint("num.sql.path.tries", _stompstats.sql_path.tries);
    datapoint("num.sql.path.hits", _stompstats.sql_path.hits);
    datapoint_float("num.sql.path.hitrate", OPENSTATS_PERCENT(_stompstats.sql_path.hits, _stompstats.sql_path.tries) );
    datapoint("num.sql.path.misses", _stompstats.sql_path.misses);
    datapoint("num.sql.path.inserted", _stompstats.sql_path.inserted);
    datapoint("num.sql.path.failed", _stompstats.sql_path.failed);

    datapoint("num.cache.path.tries", _stompstats.cache_path.tries);
    datapoint("num.cache.path.misses", _stompstats.cache_path.misses);
    datapoint("num.cache.path.hits", _stompstats.cache_path.hits);
    datapoint_float("num.cache.path.hitrate", OPENSTATS_PERCENT(_stompstats.cache_path.hits, _stompstats.cache_path.tries) );
    datapoint("num.cache.path.stored", _stompstats.cache_path.stored);

    datapoint("num.sql.status.tries", _stompstats.sql_status.tries);
    datapoint("num.sql.status.hits", _stompstats.sql_status.hits);
    datapoint_float("num.sql.status.hitrate", OPENSTATS_PERCENT(_stompstats.sql_status.hits, _stompstats.sql_status.tries) );
    datapoint("num.sql.status.misses", _stompstats.sql_status.misses);
    datapoint("num.sql.status.inserted", _stompstats.sql_status.inserted);
    datapoint("num.sql.status.failed", _stompstats.sql_status.failed);

    datapoint("num.cache.status.tries", _stompstats.cache_status.tries);
    datapoint("num.cache.status.hits", _stompstats.cache_status.hits);
    datapoint("num.cache.status.misses", _stompstats.cache_status.misses);
    datapoint_float("num.cache.status.hitrate", OPENSTATS_PERCENT(_stompstats.cache_status.hits, _stompstats.cache_status.tries) );
    datapoint("num.cache.status.stored", _stompstats.cache_status.stored);

    datapoint("num.cache.duplicates.tries", _stompstats.cache_duplicates.tries);
    datapoint("num.cache.duplicates.misses", _stompstats.cache_duplicates.misses);
    datapoint("num.cache.duplicates.hits", _stompstats.cache_duplicates.hits);
    datapoint_float("num.cache.duplicates.hitrate", OPENSTATS_PERCENT(_stompstats.cache_duplicates.hits, _stompstats.cache_duplicates.tries) );
    datapoint("num.cache.duplicates.stored", _stompstats.cache_duplicates.stored);

    datapoint("num.cache.positions.tries", _stompstats.cache_positions.tries);
    datapoint("num.cache.positions.misses", _stompstats.cache_positions.misses);
    datapoint("num.cache.positions.hits", _stompstats.cache_positions.hits);
    datapoint_float("num.cache.positions.hitrate", OPENSTATS_PERCENT(_stompstats.cache_positions.hits, _stompstats.cache_positions.tries) );
    datapoint("num.cache.positions.stored", _stompstats.cache_positions.stored);

    datapoint("num.cache.ack.tries", _stompstats.cache_ack.tries);
    datapoint("num.cache.ack.misses", _stompstats.cache_ack.misses);
    datapoint("num.cache.ack.hits", _stompstats.cache_ack.hits);
    datapoint_float("num.cache.ack.hitrate", OPENSTATS_PERCENT(_stompstats.cache_ack.hits, _stompstats.cache_ack.tries) );
    datapoint("num.cache.ack.stored", _stompstats.cache_ack.stored);

    init_stats(_stompstats);
  } // Store::try_stompstats()

  bool Store::getCallsignId(const std::string &source, std::string &ret_id) {
    int i;

    // try and find in memcached
    if (getCallsignIdFromMemcached(source, ret_id)) return true;

    // not in memcached find in sql
    _stats.sql_callsign.tries++;
    _stompstats.sql_callsign.tries++;
    if (_dbi->inject->getCallsignId(source, ret_id)) {
      setCallsignIdInMemcached(source, ret_id);
      _stats.sql_callsign.hits++;
      _stompstats.sql_callsign.hits++;
      return true;
    } // if
    _stats.sql_callsign.misses++;
    _stompstats.sql_callsign.misses++;

    // try again just in case another thread beat us
    for(i=0; i < 3; i++) {
      // not in sql try and create it
      if (_dbi->inject->insertCallsign(source, ret_id)) {
        setCallsignIdInMemcached(source, ret_id);
        _stats.sql_callsign.inserted++;
        _stompstats.sql_callsign.inserted++;
        return true;
      } // if

      if (_dbi->inject->getCallsignId(source, ret_id)) {
        setCallsignIdInMemcached(source, ret_id);
        return true;
      } // if
      usleep(100000);
    } // for

    _stats.sql_callsign.failed++;
    _stompstats.sql_callsign.failed++;

    return false;
  } // Store::getCallsignId

  bool Store::getPacketId(const std::string &callsignId, std::string &ret_id) {
    openframe::Stopwatch sw;
    bool isOK = false;

    sw.Start();

    // try again just in case another thread beat us
    for(int i=0; i < 3; i++) {
      isOK = _dbi->inject->insertPacket(callsignId, ret_id);
      if (isOK) {
        _stats.sql_packet.inserted++;
        _stompstats.sql_packet.inserted++;
        break;
      } // if

      usleep(100000);
    } // for

    if (!isOK) {
      _stats.sql_packet.failed++;
      _stompstats.sql_packet.failed++;
    } // if

    _profile->average("sql.insert.packet", sw.Time());

    return isOK;
  } // Store::getPacketId

  bool Store::getCallsignIdFromMemcached(const std::string &source, std::string &ret_id) {
    MemcachedController::memcachedReturnEnum mcr;
    openframe::Stopwatch sw;
    std::string buf;
    std::string key;

    if (!isMemcachedOk()) return false;

    _stats.cache_callsign.tries++;
    _stompstats.cache_callsign.tries++;

    sw.Start();

    try {
      mcr = _memcached->get("callsign", aprs::StringTool::toUpper(source), buf);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
    } // catch

    _profile->average("memcached.callsign", sw.Time());

    if (mcr != MemcachedController::MEMCACHED_CONTROLLER_SUCCESS) {
      _stats.cache_callsign.misses++;
      _stompstats.cache_callsign.misses++;
      return false;
    } // if

    _stats.cache_callsign.hits++;
    _stompstats.cache_callsign.hits++;

    ret_id = buf;

    return true;
  } // getCallsignIdFromMemcached

  bool Store::setCallsignIdInMemcached(const std::string &source, const std::string &id) {
    std::string key = openframe::StringTool::toUpper(source);
    bool isOK = true;

    if (!isMemcachedOk()) return false;

    try {
      _memcached->put("callsign", key, id);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
      return false;
    } // catch

    _stats.cache_callsign.stored++;
    _stompstats.cache_callsign.stored++;

    return isOK;
  } // setCallsignIdInMemcached

  bool Store::getPathId(const std::string &path, std::string &ret_id) {
    md5wrapper md5;
    std::string hash = md5.getHashFromString(path);

    if (getPathIdFromMemcached(hash, ret_id)) return true;

    _stats.sql_path.tries++;
    _stompstats.sql_path.tries++;
    if (_dbi->inject->getPathId(hash, ret_id)) {
      _stats.sql_path.hits++;
      _stompstats.sql_path.hits++;
      setPathIdInMemcached(hash, ret_id);
      return true;
    } // if
    _stats.sql_path.misses++;
    _stompstats.sql_path.misses++;

    // try again just in case another thread beat us
    for(int i=0; i < 3; i++) {
      if (_dbi->inject->insertPath(hash, path, ret_id)) {
        _stats.sql_path.inserted++;
        _stompstats.sql_path.inserted++;
        setPathIdInMemcached(hash, ret_id);
        return true;
      } // if

      if (_dbi->inject->getPathId(hash, ret_id)) {
        setPathIdInMemcached(hash, ret_id);
        return true;
      } // if
      usleep(100000);
    } // for

    _stats.sql_path.failed++;
    _stompstats.sql_path.failed++;

    return false;
  } // Store::getPathId

  bool Store::getPathIdFromMemcached(const std::string &hash, std::string &ret_id) {
    MemcachedController::memcachedReturnEnum mcr;
    openframe::Stopwatch sw;
    std::string buf;

    if (!isMemcachedOk())
      return false;

    _stats.cache_path.tries++;
    _stompstats.cache_path.tries++;

    sw.Start();

    try {
      mcr = _memcached->get("path", hash, buf);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
      return false;
    } // catch

    _profile->average("memcached.path", sw.Time());

    if (mcr != MemcachedController::MEMCACHED_CONTROLLER_SUCCESS) {
      _stats.cache_path.misses++;
      _stompstats.cache_path.misses++;
      return false;
    } // if

    _stats.cache_path.hits++;
    _stompstats.cache_path.hits++;

    ret_id = buf;

    return true;
  } // Store::getPathIdFromMemcached

  bool Store::setPathIdInMemcached(const std::string &hash, const std::string &id) {
    bool isOK = true;

    if (!isMemcachedOk()) return false;

    try {
      _memcached->put("path", hash, id);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
      isOK = false;
    } // catch

    if (isOK) {
      _stats.cache_path.stored++;
      _stompstats.cache_path.stored++;
    } // if

    return isOK;
  } // Store::setPathInMemcached

  bool Store::getStatusId(const std::string &path, std::string &ret_id) {
    md5wrapper md5;
    std::string hash = md5.getHashFromString(path);

    if (getStatusIdFromMemcached(hash, ret_id)) return true;

    _stats.sql_status.tries++;
    _stompstats.sql_status.tries++;
    if (_dbi->inject->getStatusId(hash, ret_id)) {
      _stats.sql_status.hits++;
      _stompstats.sql_status.hits++;
      setStatusIdInMemcached(hash, ret_id);
      return true;
    } // if
    _stats.sql_status.misses++;
    _stompstats.sql_status.misses++;

    // try again just in case another thread beat us
    for(int i=0; i < 3; i++) {
      if (_dbi->inject->insertStatus(hash, path, ret_id)) {
        _stats.sql_status.inserted++;
        _stompstats.sql_status.inserted++;

        setStatusIdInMemcached(hash, ret_id);
        return true;
      } // if

      if (_dbi->inject->getStatusId(hash, ret_id)) {
        setStatusIdInMemcached(hash, ret_id);
        return true;
      } // if

      usleep(100000);
    } // for

    _stats.sql_status.failed++;
    _stompstats.sql_status.failed++;

    return false;
  } // Store::getStatusId

  bool Store::getStatusIdFromMemcached(const std::string &hash, std::string &ret_id) {
    MemcachedController::memcachedReturnEnum mcr;
    openframe::Stopwatch sw;
    std::string buf;

    if (!isMemcachedOk()) return false;

    _stats.cache_status.tries++;
    _stompstats.cache_status.tries++;

    sw.Start();

    try {
      mcr = _memcached->get("status", hash, buf);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
      return false;
    } // catch

    _profile->average("memcached.status", sw.Time());

    if (mcr != MemcachedController::MEMCACHED_CONTROLLER_SUCCESS) {
      _stats.cache_status.misses++;
      _stompstats.cache_status.misses++;
      return false;
    } // if

    _stats.cache_status.hits++;
    _stompstats.cache_status.hits++;

    ret_id = buf;

    return true;
  } // Store::getStatusIdFromMemcached

  bool Store::setStatusIdInMemcached(const std::string &hash, const std::string &ret_id) {
    bool isOK = true;

    if (!isMemcachedOk()) return false;

    try {
      _memcached->put("status", hash, ret_id);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
      isOK = false;
    } // catch

    if (isOK) {
      _stats.cache_status.stored++;
      _stompstats.cache_status.stored++;
    } // if

    return isOK;
  } // Store::setStatusInMemcached

  //
  // Memcache Duplicates
  //
  bool Store::getDuplicateFromMemcached(const std::string &key, std::string &ret) {
    MemcachedController::memcachedReturnEnum mcr;
    openframe::Stopwatch sw;
    std::string buf;

    if (!isMemcachedOk()) return false;

    _stats.cache_duplicates.tries++;
    _stompstats.cache_duplicates.tries++;

    sw.Start();

    try {
      mcr = _memcached->get("duplicates", key, buf);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
    } // catch

    _profile->average("memcached.duplicates", sw.Time());

    if (mcr != MemcachedController::MEMCACHED_CONTROLLER_SUCCESS) {
      _stats.cache_duplicates.misses++;
      _stompstats.cache_duplicates.misses++;
      return false;
    } // if

    _stats.cache_duplicates.hits++;
    _stompstats.cache_duplicates.hits++;

    ret = buf;

    return true;
  } // getDuplicateFromMemcached

  bool Store::setDuplicateInMemcached(const std::string &key, const std::string &buf) {
    bool isOK = true;

    if (!isMemcachedOk()) return false;

    try {
      _memcached->put("duplicates", key, buf, 30);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
      return false;
    } // catch

    _stats.cache_duplicates.stored++;
    _stompstats.cache_duplicates.stored++;

    return isOK;
  } // setDuplicateInMemcached

  //
  // Memcache Positions
  //
  bool Store::getPositionFromMemcached(const std::string &key, std::string &ret) {
    MemcachedController::memcachedReturnEnum mcr;
    openframe::Stopwatch sw;
    std::string buf;

    if (!isMemcachedOk()) return false;

    _stats.cache_positions.tries++;
    _stompstats.cache_positions.tries++;

    sw.Start();

    try {
      mcr = _memcached->get("positions", key, buf);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
    } // catch

    _profile->average("memcached.positions", sw.Time());

    if (mcr != MemcachedController::MEMCACHED_CONTROLLER_SUCCESS) {
      _stats.cache_positions.misses++;
      _stompstats.cache_positions.misses++;
      return false;
    } // if

    _stats.cache_positions.hits++;
    _stompstats.cache_positions.hits++;

    ret = buf;

    return true;
  } // getPositionFromMemcached

  bool Store::setPositionInMemcached(const std::string &key, const std::string &buf) {
    bool isOK = true;

    if (!isMemcachedOk()) return false;

    try {
      _memcached->put("positions", key, buf, 30);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
      return false;
    } // catch

    _stats.cache_positions.stored++;
    _stompstats.cache_positions.stored++;

    return isOK;
  } // setPositionInMemcached

  //
  // Memcache Acks
  //
  bool Store::getAckFromMemcached(const std::string &key, std::string &ret) {
    MemcachedController::memcachedReturnEnum mcr;
    openframe::Stopwatch sw;
    std::string buf;

    if (!isMemcachedOk()) return false;

    _stats.cache_ack.tries++;
    _stompstats.cache_ack.tries++;

    sw.Start();

    try {
      mcr = _memcached->get("ack", key, buf);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
    } // catch

    _profile->average("memcached.ack", sw.Time());

    if (mcr != MemcachedController::MEMCACHED_CONTROLLER_SUCCESS) {
      _stats.cache_ack.misses++;
      _stompstats.cache_ack.misses++;
      return false;
    } // if

    _stats.cache_ack.hits++;
    _stompstats.cache_ack.hits++;

    ret = buf;

    return true;
  } // getAckFromMemcached

  bool Store::setAckInMemcached(const std::string &key, const std::string &buf, const time_t expire) {
    bool isOK = true;

    if (!isMemcachedOk()) return false;

    try {
      _memcached->put("ack", key, buf, expire);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      _last_cache_fail_at = time(NULL);
      return false;
    } // catch

    _stats.cache_ack.stored++;
    _stompstats.cache_ack.stored++;

    return isOK;
  } // setAckInMemcached

} // namespace openaprs

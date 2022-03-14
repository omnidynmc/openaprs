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
#include "DBI_Rollup.h"
#include "MemcachedController.h"
#include "Server.h"
#include "Worker.h"

#include "openframe/Stopwatch.h"
#include "openframe/StringToken.h"

#include "openaprs.h"

namespace rollup {
  using namespace std;
  using namespace aprs;
  using namespace putmysql;
  using openaprs::DBI;
  using openaprs::Server;
  using openaprs::MemcachedController;
  using openframe::StringToken;

/**************************************************************************
 ** Worker Class                                                         **
 **************************************************************************/

  Worker::Worker(const time_t statsInterval, const time_t rollupInterval, const time_t partitionInterval, const time_t optimizeInterval,
                 const bool deleteWeather,
                 const string &interval, const int limit)
                 : _statsInterval(statsInterval), _rollupInterval(rollupInterval), _partitionInterval(partitionInterval),
                   _optimizeInterval(optimizeInterval), _deleteWeather(deleteWeather),
                   _interval(interval), _limit(limit) {
    _lastStatsTs = time(NULL) + _statsInterval;
    _timeNextMemcachedTry = 0;
    _numRollup = 0;
    _numLastSelection = 0;

    _dbi = NULL;
    try {
      _dbi = new DBI_Rollup();
      _dbi->logger(new App_Log(_logger->ident()));
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    try {
      _profile = new Stopwatch();
      _timerRollupInterval = new Stopwatch(_rollupInterval);
      _timerPartitionInterval = new Stopwatch(_partitionInterval);
      _timerOptimizeInterval = new Stopwatch(_optimizeInterval);
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    _profile->add("run.loop", 300);
    _profile->add("rollup.weather", 300);
    _profile->add("rollup.delete", 300);

  } // Worker::Worker

  Worker::~Worker() {
    _profile->clear();

    if (_dbi)
      delete _dbi;

    delete _profile;
    delete _timerRollupInterval;
    delete _timerPartitionInterval;
    delete _timerOptimizeInterval;
  } // Worker::~Worker

  void Worker::run() {
    Stopwatch sw;
    stringstream s;

    sw.Start();
    _writeStats();
    _rollupWeather();
    _partition();
    _optimize();

    _profile->average("run.loop", sw.Time());

  } // Worker::run

  void Worker::_partition() {
    DBI::resultSizeType numRows;
    Stopwatch sw;
    string add = app->cfg->get_string("module.rollup.partition.add", "");
    string drop = app->cfg->get_string("module.rollup.partition.drop", "");

    if (!_timerPartitionInterval->Next())
      return;

    partitionMapType pm;
    // try add partitions
    if (_parsePartitions(add, pm)) {
      partitionMapType::iterator ptr;
      for(ptr = pm.begin(); ptr != pm.end(); ptr++) {
        _dbi->addPartitions(ptr->first, ptr->second);
        _rolluplogf("adding partitions for %s:%s", ptr->first.c_str(), ptr->second.c_str());
      } // for
    } // if

    if (_parsePartitions(drop, pm)) {
      partitionMapType::iterator ptr;
      for(ptr = pm.begin(); ptr != pm.end(); ptr++) {
        _dbi->dropPartitions(ptr->first, ptr->second);
        _rolluplogf("dropping partitions for %s:%s", ptr->first.c_str(), ptr->second.c_str());
      } // for
    } // if
  } // _partition

  const Worker::partitionMapSizeType Worker::_parsePartitions(const string &parts, partitionMapType &ret) {
    StringToken st;
    StringToken part;

    ret.clear();

    st.setDelimiter(',');
    part.setDelimiter(':');
    st = parts;

    if (!st.size())
      return 0;

    for(size_t i=0; i < st.size(); i++) {
      part = st[i];
      if (part.size() != 2) {
        _rolluplogf("parse error in parititon config at %i for %s", i, parts.c_str());
        continue;
      } // if

      ret.insert( make_pair(part[0], part[1]) );
    } // for

    return ret.size();
  } // Worker::_parsePartitions

  void Worker::_optimize() {
    DBI::resultSizeType numRows;
    Stopwatch sw;
    string tables = app->cfg->get_string("module.rollup.optimize.tables", "");

    if (!_timerOptimizeInterval->Next())
      return;

    optimizeType opt;
    // try add partitions
    if (_parseOptimize(tables, opt)) {
      for(size_t i=0; i < opt.size(); i++) {
        _dbi->optimizeTable(opt[i]);
        _rolluplogf("optimize table %s", opt[i].c_str());
      } // for
    } // if
  } // _optimize

  const Worker::optimizeSizeType Worker::_parseOptimize(const string &tables, optimizeType &ret) {
    StringToken st;

    ret.clear();

    st.setDelimiter(',');
    st = tables;

    if (!st.size())
      return 0;

    for(size_t i=0; i < st.size(); i++)
      ret.push_back( st[i] );

    return ret.size();
  } // Worker::_parseOptimize

  void Worker::_rollupWeather() {
    DBI::resultType res;
    DBI::resultSizeType numRows;
    Stopwatch sw;

    if (!_timerRollupInterval->Next())
      return;

    sw.Start();
    numRows = _dbi->getWeatherRollupCallsignId(_interval, _limit, res);

    if ((_numLastSelection && !numRows)
        || numRows)
      _rolluplogf("selected %d id%s for rollup in %0.5fs", numRows, (numRows == 1 ? "" : "s"), sw.Time());

    _numLastSelection = numRows;

    for(DBI::resultSizeType i=0; i < numRows; i++) {
      string id = res[i][0].c_str();
      string callsign = res[i][1].c_str();
      string packet_date = res[i][2].c_str();
      _rollupWeatherForCallsignId(id, callsign, packet_date);
    } // for
  } // Worker::_rollupWeather

  void Worker::_rollupWeatherForCallsignId(const string &id, const string &callsign, const string &packet_date) {
    DBI::resultType res;
    DBI::resultSizeType numRows;
    Stopwatch sw;

    _rolluplogf("rolling up weather for %s on %s", callsign.c_str(), packet_date.c_str());

    numRows = _dbi->setLastRollupForCallsignId(id);
    if (!numRows) {
      _rolluplogf("failed to set last weather rollup for id %s", id.c_str());
      return;
    } // if

    sw.Start();
    numRows = _dbi->rollupWeatherForCallsignId(id, packet_date);
    if (!numRows) {
      _rolluplogf("nothing to rollup for id %s", id.c_str());
      return;
    } // if

    _profile->average("rollup.weather", sw.Time());
    _rolluplogf("rolled up to %d rows for %s, %0.5fs", numRows, callsign.c_str(), sw.Time());
    _numRollup++;

    if (_deleteWeather) {
      sw.Start();
      numRows = _dbi->removeWeatherAfterRollup(id, packet_date);
      if (!numRows) {
        _rolluplogf("failed to remove weather after rollup for id %s", id.c_str());
        return;
      } // if
      _profile->average("rollup.delete", sw.Time());
      _rolluplogf("removed %d weather rows for %s, %0.5fs", numRows, callsign.c_str(), sw.Time());
    } // if

  } // Worker::_rollupWeatherForCallsignId

  void Worker::_writeStats() {
    if (_lastStatsTs > time(NULL))
      return;

    _rolluplogf("Rollup Stats weather %d %0.5fs, delete %0.5fs",
             _numRollup,
             _profile->average("rollup.weather"),
             _profile->average("rollup.delete")
            );

    _lastStatsTs = time(NULL) + _statsInterval;
  } // Worker::writeStats
} // namespace rollup

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
 $Id: DB.cpp,v 1.2 2005/12/13 21:07:03 omni Exp $
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
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "APRS.h"
#include "DBI_Rollup.h"
#include "Server.h"

#include "SSQLS.h"

#include "config.h"

namespace rollup {
  using std::string;
  using std::list;
  using std::ofstream;
  using std::ostream;
  using std::stringstream;
  using std::ios;
  using std::endl;
  using std::cout;
  using aprs::StringTool;

/**************************************************************************
 ** DB Class                                                             **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

  DBI_Rollup::DBI_Rollup() : DBI() {
    _query["rollupWeatherForCallsignId"] = "\
INSERT INTO \
  weather_rollup \
  (wind_direction, \
   wind_speed, \
   wind_gust, \
   wind_sustained, \
   temperature, \
   rain_hour, \
   rain_calendar_day, \
   rain_24hour_day, \
   humidity, \
   barometer, \
   luminosity, \
   packet_date, \
   create_ts, \
   callsign_id, \
   num_rolled) \
SELECT \
  SUM(wind_direction)/COUNT(*) AS wind_direction, \
  SUM(wind_speed)/COUNT(*) AS wind_speed, \
  SUM(wind_gust)/COUNT(*) AS wind_gust, \
  SUM(wind_sustained)/COUNT(*) AS wind_sustained, \
  SUM(temperature)/COUNT(*) AS temperature, \
  SUM(rain_hour)/COUNT(*) AS rain_hour, \
  SUM(rain_calendar_day)/COUNT(*) AS rain_calendar_day, \
  SUM(rain_24hour_day)/COUNT(*) AS rain_24hour_day, \
  SUM(humidity)/COUNT(*) AS humidity, \
  SUM(barometer)/COUNT(*) AS barometer, \
  SUM(luminosity)/COUNT(*) AS luminosity, \
  packet_date, \
  create_ts, \
  callsign_id, \
  COUNT(*) AS num_rolled \
FROM \
  weather \
WHERE \
  callsign_id=%0q:callsign_id \
  AND packet_date >= DATE(%1q:date) AND packet_date < DATE(%1q:date) + INTERVAL 1 DAY \
GROUP BY DATE(packet_date), FLOOR(HOUR(packet_date)/4)*4";

    _query["removeWeatherAfterRollup"] = "\
DELETE FROM \
  weather \
WHERE \
  callsign_id=%0q:callsign_id \
  AND packet_date >= DATE(%1q:date) AND packet_date < DATE(%1q:date) + INTERVAL 1 DAY";

    _query["getWeatherRollupCallsignId"] = "\
SELECT DISTINCT \
  c.id, \
  c.source, \
  DATE(w.packet_date) \
FROM \
  weather w \
  LEFT JOIN callsign c ON c.id=w.callsign_id \
WHERE \
  w.packet_date < DATE(NOW())-INTERVAL %0:interval \
  AND c.id IS NOT NULL \
LIMIT %1";

    _query["setLastRollupForCallsignId"] = "\
INSERT INTO \
  weather_meta (callsign_id, last_rollup, create_ts) \
VALUES \
  (%0q, CURRENT_TIMESTAMP(), UNIX_TIMESTAMP()) \
ON DUPLICATE KEY UPDATE \
  last_rollup=CURRENT_TIMESTAMP()";

  _query["addPartitions"] = "CALL partition_add(CURRENT_DATE()+INTERVAL %1 DAY, %0q, 'openaprs');";
  _query["dropPartitions"] = "CALL partition_drop(CURRENT_DATE()-INTERVAL %1 DAY, %0q, 'openaprs');";
  _query["optimizeTable"] = "OPTIMIZE TABLE %0";

  } // DBI_Rollup::DBI_Rollup

  DBI_Rollup::~DBI_Rollup() {
  } // DBI_Rollup::~DBI_Rollup

  const DBI::resultSizeType DBI_Rollup::getWeatherRollupCallsignId(const string &interval, const int limit, DBI::resultType &res) {
    DBI::queryType query = _sqlpp->query(_query["getWeatherRollupCallsignId"]);
    query.parse();

    try {
      res = query.store(interval, limit);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** DBI_Rollup::getWeartherRollupCallsignId: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.num_rows();
  } // MGMT_DBI::getWeatherRollupByCallsignId

  const DBI::resultSizeType DBI_Rollup::rollupWeatherForCallsignId(const string &id, const string &packet_date) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["rollupWeatherForCallsignId"]);
    query.parse();

    try {
      res = query.execute(id, packet_date);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** DBI_Rollup::rollupWeatherForCallsignId: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // DBI_Rollup::rollupWeatherForCallsignId

  const DBI::resultSizeType DBI_Rollup::removeWeatherAfterRollup(const string &id, const string &packet_date) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["removeWeatherAfterRollup"]);
    query.parse();

    try {
      res = query.execute(id, packet_date);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** DBI_Rollup::removeWeatherAfterRollup: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // DBI_Rollup::removeWeatherAfterRollup

  const DBI::resultSizeType DBI_Rollup::setLastRollupForCallsignId(const string &id) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["setLastRollupForCallsignId"]);
    query.parse();

    try {
      res = query.execute(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** DBI_Rollup::setLastrollupForCallsignId: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // DBI_Rollup::setLastRollupForCallsignId

  const DBI::resultSizeType DBI_Rollup::optimizeTable(const string &table) {
    mysqlpp::StoreQueryResult res;
    DBI::queryType query = _sqlpp->query(_query["optimizeTable"]);
    query.parse();

    try {
      res = query.store(table);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** DBI_Rollup::optimizeTable: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Rollup::optimizeTable

  const DBI::resultSizeType DBI_Rollup::addPartitions(const string &table, const string &interval) {
    mysqlpp::StoreQueryResult res;
    DBI::queryType query = _sqlpp->query(_query["addPartitions"]);
    query.parse();

    try {
      res = query.store(table, interval);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** DBI_Rollup::addPartitions: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Rollup::addPartitions

  const DBI::resultSizeType DBI_Rollup::dropPartitions(const string &table, const string &interval) {
    mysqlpp::StoreQueryResult res;
    DBI::queryType query = _sqlpp->query(_query["dropPartitions"]);
    query.parse();

    try {
      res = query.store(table, interval);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** DBI_Rollup::dropPartitions: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Rollup::dropPartitions
} // namespace rollup

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
#include "DBI_Sat.h"
#include "Server.h"

#include "SSQLS.h"

#include "config.h"

namespace sat {
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

  DBI_Sat::DBI_Sat() : DBI() {
    _query["selectSats"] = "\
SELECT \
  t.*, \
  sf.file_name \
FROM \
  satellite_names t \
  LEFT JOIN satellite_folders sf ON sf.id = t.folder_id \
WHERE \
  t.folder_id = 1 \
  AND NOT t.number IN (-1, -2) \
ORDER BY \
  t.name \
";
//  NOT t.number IN (-1, -2)

    _query["updateSats"] = "\
INSERT INTO satellite_lastposition \
  (number, packet_date, name, elevation, azimuth, latitude, longitude, altitude, create_ts) \
VALUES ( \
  %0q:number, \
  FROM_UNIXTIME(%1:packet_date), \
  %2q:name, \
  %3:elevation, \
  %4:azimuth, \
  %5:latitude, \
  %6:longitude, \
  %7:altitude, \
  %8:create_ts \
) \
ON DUPLICATE KEY UPDATE \
  packet_date = VALUES(packet_date), \
  elevation = VALUES(elevation), \
  azimuth = VALUES(azimuth), \
  latitude = VALUES(latitude), \
  longitude = VALUES(longitude), \
  altitude = VALUES(altitude), \
  create_ts = VALUES(create_ts) \
";

    _query["updateNextSats"] = "\
INSERT IGNORE INTO satellite_next120positions \
  (number, packet_date, name, elevation, azimuth, latitude, longitude, altitude, create_ts) \
VALUES ( \
  %0q:number, \
  FROM_UNIXTIME(%1:packet_date), \
  %2q:name, \
  %3:elevation, \
  %4:azimuth, \
  %5:latitude, \
  %6:longitude, \
  %7:altitude, \
  %8:create_ts \
) \
";

    _query["removeOldNext"] = "\
DELETE FROM \
  satellite_next120positions \
WHERE \
  packet_date < NOW() - INTERVAL %0:interval \
";

  } // DBI_Sat::DBI_Sat

  DBI_Sat::~DBI_Sat() {
  } // DBI_Sat::~DBI_Sat

  const DBI::resultSizeType DBI_Sat::getSats(DBI::resultType &res) {
    DBI::queryType query = _sqlpp->query(_query["selectSats"]);
    query.parse();

    try {
      res = query.store();
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("%s", query.str().c_str() );
      _logf("*** DBI_Sat::getSats: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    while(query.more_results()) query.store_next();

    return res.num_rows();
  } // DBI_Sat::getWeatherSatByCallsignId

  const DBI::resultSizeType DBI_Sat::updateSats(const string &number, time_t packet_date, const string &name, const double elevation, const double azimuth, const double latitude, const double longitude, const double altitude, const time_t create_ts) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["updateSats"]);
    query.parse();

    try {
      res = query.execute(number, packet_date, name, elevation, azimuth, latitude, longitude, altitude, create_ts);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("%s", query.str(number, packet_date, name, elevation, azimuth, latitude, longitude, altitude, create_ts).c_str() );
      _logf("*** DBI_Sat::updateSats: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // DBI_Sat::updateSats

  const DBI::resultSizeType DBI_Sat::updateNextSats(const string &number, time_t packet_date, const string &name, const double elevation, const double azimuth, const double latitude, const double longitude, const double altitude, const time_t create_ts) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["updateNextSats"]);
    query.parse();

    try {
      res = query.execute(number, packet_date, name, elevation, azimuth, latitude, longitude, altitude, create_ts);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("%s", query.str(number, packet_date, name, elevation, azimuth, latitude, longitude, altitude, create_ts).c_str() );
      _logf("*** DBI_Sat::updateNextSats: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // DBI_Sat::updateNextSats

  const DBI::resultSizeType DBI_Sat::removeOldNext(const string &interval) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["removeOldNext"]);
    query.parse();

    try {
      res = query.execute(interval);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("%s", query.str(interval).c_str());
      _logf("*** DBI_Sat::removeOldNext: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // DBI_Sat::removeOldNext

  const DBI::resultSizeType DBI_Sat::setLastSatForCallsignId(const string &id) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["setLastSatForCallsignId"]);
    query.parse();

    try {
      res = query.execute(id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** DBI_Sat::setLastrollupForCallsignId: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // DBI_Sat::setLastSatForCallsignId

  const DBI::resultSizeType DBI_Sat::optimizeTable(const string &table) {
    mysqlpp::StoreQueryResult res;
    DBI::queryType query = _sqlpp->query(_query["optimizeTable"]);
    query.parse();

    try {
      res = query.store(table);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** DBI_Sat::optimizeTable: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Sat::optimizeTable

  const DBI::resultSizeType DBI_Sat::addPartitions(const string &table, const string &interval) {
    mysqlpp::StoreQueryResult res;
    DBI::queryType query = _sqlpp->query(_query["addPartitions"]);
    query.parse();

    try {
      res = query.store(table, interval);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** DBI_Sat::addPartitions: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Sat::addPartitions

  const DBI::resultSizeType DBI_Sat::dropPartitions(const string &table, const string &interval) {
    mysqlpp::StoreQueryResult res;
    DBI::queryType query = _sqlpp->query(_query["dropPartitions"]);
    query.parse();

    try {
      res = query.store(table, interval);
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** DBI_Sat::dropPartitions: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    while(query.more_results())
      query.store_next();

    return res.num_rows();
  } // DBI_Sat::dropPartitions
} // namespace rollup

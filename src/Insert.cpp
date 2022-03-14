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
 $Id: Insert.cpp,v 1.3 2006/03/21 20:55:12 omni Exp $
 **************************************************************************/

#include <cstdlib>
#include <cstring>
#include <list>
#include <string>
#include <sstream>
#include <iostream>

#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include "Insert.h"
#include "PutMySQL.h"
#include "Server.h"

#include "match.h"

namespace openaprs {

    using namespace std;

/**************************************************************************
 ** Insert Class                                                         **
 **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  Insert::Insert() {
    queryList["lastraw"] = "INSERT INTO lastraw (packet_id, callsign_id, destination, digipeater_0, digipeater_1, digipeater_2, digipeater_3, digipeater_4, digipeater_5, digipeater_6, digipeater_7, information, create_ts) VALUES";
    updateList["lastraw"] = "packet_id=VALUES(packet_id), callsign_id=VALUES(callsign_id), destination=VALUES(destination), digipeater_0=VALUES(digipeater_0), digipeater_1=VALUES(digipeater_1), digipeater_2=VALUES(digipeater_2), digipeater_3=VALUES(digipeater_3), digipeater_4=VALUES(digipeater_4), digipeater_5=VALUES(digipeater_5), digipeater_6=VALUES(digipeater_6), digipeater_7=VALUES(digipeater_7), information=VALUES(information), create_ts=VALUES(create_ts)";

    queryList["lastposition"] = "INSERT INTO lastposition (packet_id, callsign_id, name, destination, path_id, latitude, longitude, course, speed, altitude, status_id, symbol_table, symbol_code, overlay, phg_power, phg_haat, phg_gain, phg_range, phg_direction, phg_beacon, dfs_power, dfs_haat, dfs_gain, dfs_range, dfs_direction, `range`, type, weather, telemetry, postype, mbits, create_ts) VALUES";
    updateList["lastposition"] = "packet_id=VALUES(packet_id), name=VALUES(name), callsign_id=VALUES(callsign_id), destination=VALUES(destination), path_id=VALUES(path_id), latitude=VALUES(latitude), longitude=VALUES(longitude), course=VALUES(course), speed=VALUES(speed), altitude=VALUES(altitude), status_id=VALUES(status_id), symbol_table=VALUES(symbol_table), symbol_code=VALUES(symbol_code), overlay=VALUES(overlay), phg_power=VALUES(phg_power), phg_haat=VALUES(phg_haat), phg_gain=VALUES(phg_gain), phg_range=VALUES(phg_range), phg_direction=VALUES(phg_direction), phg_beacon=VALUES(phg_beacon), dfs_power=VALUES(dfs_power), dfs_haat=VALUES(dfs_haat), dfs_gain=VALUES(dfs_gain), dfs_range=VALUES(dfs_range), dfs_direction=VALUES(dfs_direction), `range`=VALUES(`range`), type=VALUES(type), weather=VALUES(weather), telemetry=VALUES(telemetry), postype=VALUES(postype), mbits=VALUES(mbits), create_ts=VALUES(create_ts)";

    queryList["mem_lastposition"] = "INSERT INTO mem_lastposition (packet_id, callsign_id, path_id, name, latitude, longitude, course, symbol_table, symbol_code, phg_range, phg_direction, type) VALUES";
    updateList["mem_lastposition"] = "packet_id=VALUES(packet_id), callsign_id=VALUES(callsign_id), path_id=VALUES(path_id), name=VALUES(name), latitude=VALUES(latitude), longitude=VALUES(longitude), course=VALUES(course), symbol_table=VALUES(symbol_table), symbol_code=VALUES(symbol_code), phg_range=VALUES(phg_range), phg_direction=VALUES(phg_direction), type=VALUES(type)";

    queryList["last_frequency"] = "INSERT INTO last_frequency (packet_id, callsign_id, name, frequency, `range`, range_east, tone, afrs_type, receive, alternate, type, create_ts) VALUES";
    updateList["last_frequency"] = "packet_id=VALUES(packet_id), callsign_id=VALUES(callsign_id), name=VALUES(name), frequency=VALUES(frequency), `range`=VALUES(`range`), range_east=VALUES(range_east), tone=VALUES(tone), afrs_type=VALUES(afrs_type), receive=VALUES(receive), alternate=VALUES(alternate), type=VALUES(type), create_ts=VALUES(create_ts)";

    queryList["lastweather"] = "INSERT INTO lastweather (packet_id, callsign_id, packet_date, latitude, longitude, wind_direction, wind_speed, wind_gust, temperature, rain_hour, rain_calendar_day, rain_24hour_day, humidity, barometer, luminosity, create_ts) VALUES";
    updateList["lastweather"] = "packet_id=VALUES(packet_id), callsign_id=VALUES(callsign_id), packet_date=VALUES(packet_date), latitude=VALUES(latitude), longitude=VALUES(longitude), wind_direction=VALUES(wind_direction), wind_speed=VALUES(wind_speed), wind_gust=VALUES(wind_gust), temperature=VALUES(temperature), rain_hour=VALUES(rain_hour), rain_calendar_day=VALUES(rain_calendar_day), rain_24hour_day=VALUES(rain_24hour_day), humidity=VALUES(humidity), barometer=VALUES(barometer), luminosity=VALUES(luminosity), create_ts=VALUES(create_ts)";

    queryList["last_bulletin"] = "INSERT INTO last_bulletin (packet_id, callsign_id, addressee, text, id, create_ts) VALUES";
    updateList["last_bulletin"] = "packet_id=VALUES(packet_id), callsign_id=VALUES(callsign_id), addressee=VALUES(addressee), text=VALUES(text), id=VALUES(id), create_ts=VALUES(create_ts)";

    queryList["last_message"] = "INSERT INTO last_message (packet_id, callsign_id, addressee, text, id, create_ts) VALUES";
    updateList["last_message"] = "packet_id=VALUES(packet_id), callsign_id=VALUES(callsign_id), addressee=VALUES(addressee), text=VALUES(text), id=VALUES(id), create_ts=VALUES(create_ts)";

    queryList["telemetry_unit"] = "INSERT INTO telemetry_unit (packet_id, callsign_id, a_0, a_1, a_2, a_3, a_4, d_0, d_1, d_2, d_3, d_4, d_5, d_6, d_7, create_ts) VALUES";
    updateList["telemetry_unit"] = "packet_id=VALUES(packet_id), callsign_id=VALUES(callsign_id), a_0=VALUES(a_0), a_1=VALUES(a_1), a_2=VALUES(a_2), a_3=VALUES(a_3), a_4=VALUES(a_4), d_0=VALUES(d_0), d_1=VALUES(d_1), d_2=VALUES(d_2), d_3=VALUES(d_3), d_4=VALUES(d_4), d_5=VALUES(d_5), d_6=VALUES(d_6), d_7=VALUES(d_7), create_ts=VALUES(create_ts)";

    queryList["telemetry_parm"] = "INSERT INTO telemetry_parm (packet_id, callsign_id, a_0, a_1, a_2, a_3, a_4, d_0, d_1, d_2, d_3, d_4, d_5, d_6, d_7, create_ts) VALUES";
    updateList["telemetry_parm"] = "packet_id=VALUES(packet_id), callsign_id=VALUES(callsign_id), a_0=VALUES(a_0), a_1=VALUES(a_1), a_2=VALUES(a_2), a_3=VALUES(a_3), a_4=VALUES(a_4), d_0=VALUES(d_0), d_1=VALUES(d_1), d_2=VALUES(d_2), d_3=VALUES(d_3), d_4=VALUES(d_4), d_5=VALUES(d_5), d_6=VALUES(d_6), d_7=VALUES(d_7), create_ts=VALUES(create_ts)";

    queryList["telemetry_eqns"] = "INSERT INTO telemetry_eqns (packet_id, callsign_id, a_0, b_0, c_0, a_1, b_1, c_1, a_2, b_2, c_2, a_3, b_3, c_3, a_4, b_4, c_4, create_ts) VALUES";
    updateList["telemetry_eqns"] = "packet_id=VALUES(packet_id), callsign_id=VALUES(callsign_id), a_0=VALUES(a_0), b_0=VALUES(b_0), c_0=VALUES(c_0), a_1=VALUES(a_1), b_1=VALUES(b_1), c_1=VALUES(c_1), a_2=VALUES(a_2), b_2=VALUES(b_2), c_2=VALUES(c_2), a_3=VALUES(a_3), b_3=VALUES(b_3), c_3=VALUES(c_3), a_4=VALUES(a_4), b_4=VALUES(b_4), c_4=VALUES(c_4), create_ts=VALUES(create_ts)";

    queryList["telemetry_bits"] = "INSERT INTO telemetry_bits (packet_id, callsign_id, bitsense, project_title, create_ts) VALUES";
    updateList["telemetry_bits"] = "packet_id=VALUES(packet_id), callsign_id=VALUES(callsign_id), bitsense=VALUES(bitsense), project_title=VALUES(project_title), create_ts=VALUES(create_ts)";

    queryList["last_telemetry"] = "INSERT INTO last_telemetry (packet_id, callsign_id, packet_date, sequence, analog_0, analog_1, analog_2, analog_3, analog_4, digital, create_ts) VALUES";
    updateList["last_telemetry"] = "packet_id=VALUES(packet_id), callsign_id=VALUES(callsign_id), packet_date=VALUES(packet_date), sequence=VALUES(sequence), analog_0=VALUES(analog_0), analog_1=VALUES(analog_1), analog_2=VALUES(analog_2), analog_3=VALUES(analog_3), analog_4=VALUES(analog_4), digital=VALUES(digital), create_ts=VALUES(create_ts)";

    queryList["raw"] = "INSERT INTO raw (packet_id, callsign_id, packet_date, information, create_ts) VALUES";
    queryList["raw_meta"] = "INSERT INTO raw_meta (packet_id, callsign_id, packet_time, packet_date, destination, digipeater_0, digipeater_1, digipeater_2, digipeater_3, digipeater_4, digipeater_5, digipeater_6, digipeater_7, create_ts) VALUES";
    queryList["raw_errors"] = "INSERT INTO raw_errors (packet_id, callsign_id, packet_date, message, create_ts) VALUES";
    queryList["position"] = "INSERT INTO position (packet_id, callsign_id, status_id, packet_date, path_id, latitude, longitude, course, speed, altitude, symbol_table, symbol_code, time_of_fix, create_ts) VALUES";
    queryList["mem_position"] = "INSERT INTO mem_position (callsign_id, packet_date, latitude, longitude) VALUES";
    queryList["igated"] = "INSERT INTO igated (packet_id, callsign_id, igate, latitude, longitude, create_ts) VALUES";
    queryList["cache_24hr"] = "INSERT INTO cache_24hr (packet_id, callsign_id, status_id, packet_date, path_id, latitude, longitude, course, speed, altitude, symbol_table, symbol_code, time_of_fix, create_ts) VALUES";
    queryList["weather"] = "INSERT INTO weather (packet_id, callsign_id, packet_date, wind_direction, wind_speed, wind_gust, temperature, rain_hour, rain_calendar_day, rain_24hour_day, humidity, barometer, luminosity, create_ts) VALUES";
    queryList["messages"] = "INSERT INTO messages (packet_id, callsign_id, packet_date, addressee, text, id, create_ts) VALUES";
    queryList["telemetry"] = "INSERT INTO telemetry (packet_id, callsign_id, packet_date, sequence, analog_0, analog_1, analog_2, analog_3, analog_4, digital, create_ts) VALUES";
  } // Insert::Insert

  Insert::~Insert() {
    queryList.clear();
    updateList.clear();
  } // Insert::~Insert

  /********************
   ** Insert Members **
   ********************/

  /**
   * Insert::Add
   *
   * Adds a record to the insert list.
   *
   * Returns: True if successful, false if failed.
   */
  const bool Insert::Add(const string &addTable, const char *fmt, ...) {
    insertListType newList;
    insertMapType::iterator ptr;              // pointer to a map
    char query[PUTMYSQL_MAXBUF+1];
    int query_size;
    va_list va;

    if (!IsServerSQLInject)
      return false;

    Lock();

    va_start(va, fmt);
    query_size = vsnprintf(query, sizeof(query), fmt, va);
    va_end(va);

    // is there already an insert by that name?
    if ((ptr = insertList.find(addTable)) != insertList.end()) {
      ptr->second.push_back(query);
      Unlock();
      return true;
    } // if

    newList.push_back(query);

    insertList.insert(pair<string, insertListType>(addTable, newList));

    Unlock();

    return true;
  } // Insert::Add

  /**
   * Insert::Flush()
   *
   * Flushes insert que.
   *
   * Returns: True if successful, false if failed.
   */
  const bool Insert::Flush() {
    Stopwatch sw;
    insertMapType::iterator ptr;		// pointer to a map
    string query;				// query to insert
    string update;
    int i;

    sw.Start();

    for(ptr = insertList.begin(); ptr != insertList.end(); ptr++) {
      query = "";

      for(i = 1; !ptr->second.empty(); i++) {
        if ((query.length() + ptr->second.front().length()) >= (PUTMYSQL_MAXBUF/2)) {
          if (updateList.find(ptr->first) != updateList.end())
            update = " ON DUPLICATE KEY UPDATE " + updateList[ptr->first];
          else
            update = "";

          if (app->Query("%s %s%s", queryList[ptr->first].c_str(), query.c_str(), update.c_str()) == false) {
            app->writeLog(OPENAPRS_LOG_NORMAL, "*** SQL Error: #%d %s", Sql->getErrorNumber(), Sql->getErrorMessage().c_str());
            app->writeLog(OPENAPRS_LOG_DEBUG, "Insert::Flush> #%d %s", Sql->getErrorNumber(), Sql->getErrorMessage().c_str());
          } // if

          if (IsServerConsole)
            cout << "i" << i;
//            app->writeLog(OPENAPRS_LOG_STDOUT, "i%d", i);

          //if (Sql->num_affected_rows() > 10)
          //  cout << queryList[ptr->first].c_str() << " " << query.c_str() << endl;


          query = "";
          i = 0;
        } // if

        if (query.length() > 0)
          query += string(", ");

        query += ptr->second.front();
        ptr->second.pop_front();
      } // while

      // cout << queryList[ptr->first] << " " << query << endl;
      if (query.length() > 0) {
          if (updateList.find(ptr->first) != updateList.end())
            update = " ON DUPLICATE KEY UPDATE " + updateList[ptr->first];
          else
            update = "";

        if (app->Query("%s %s%s", queryList[ptr->first].c_str(), query.c_str(), update.c_str()) == false) {
          app->writeLog(OPENAPRS_LOG_NORMAL, "*** SQL Error: #%d %s", Sql->getErrorNumber(), Sql->getErrorMessage().c_str());
          app->writeLog(OPENAPRS_LOG_DEBUG, "Insert::Flush> #%d %s", Sql->getErrorNumber(), Sql->getErrorMessage().c_str());
        } // if

        if (IsServerConsole)
          cout << "I" << i;
//          app->writeLog(OPENAPRS_LOG_STDOUT, "I%d", i);
      } // if
    } // for

    insertList.clear();

    //cout << sw.Time() << endl;

    return true;
  } // Insert::Flush

  /**
   * Insert::Dequeue
   *
   * Flushes insert que.
   *
   * Returns: True if successful, false if failed.
   */
  const bool Insert::Dequeue(insertListType &queue) {
    Stopwatch sw;
    insertMapType::iterator ptr;		// pointer to a map
    string query;				// query to insert
    string update;
    int i;

    Lock();

    sw.Start();

    for(ptr = insertList.begin(); ptr != insertList.end(); ptr++) {
      query = "";

      for(i = 1; !ptr->second.empty(); i++) {
        if ((query.length() + ptr->second.front().length()) >= (PUTMYSQL_MAXBUF/2)) {
          if (updateList.find(ptr->first) != updateList.end())
            update = " ON DUPLICATE KEY UPDATE " + updateList[ptr->first];
          else
            update = "";

          queue.push_back(queryList[ptr->first] + " " + query + update);
          //if (app->Query("%s %s%s", queryList[ptr->first].c_str(), query.c_str(), update.c_str()) == false) {
          //  app->writeLog(OPENAPRS_LOG_NORMAL, "*** SQL Error: #%d %s", Sql->getErrorNumber(), Sql->getErrorMessage().c_str());
          //  app->writeLog(OPENAPRS_LOG_DEBUG, "Insert::Flush> #%d %s", Sql->getErrorNumber(), Sql->getErrorMessage().c_str());
          //} // if

          if (IsServerConsole)
            cout << "i" << i;
//            app->writeLog(OPENAPRS_LOG_STDOUT, "i%d", i);

          //if (Sql->num_affected_rows() > 10)
          //  cout << queryList[ptr->first].c_str() << " " << query.c_str() << endl;

          query = "";
          i = 0;
        } // if

        if (query.length() > 0)
          query += string(", ");

        query += ptr->second.front();
        ptr->second.pop_front();
      } // while

      // cout << queryList[ptr->first] << " " << query << endl;
      if (query.length() > 0) {
          if (updateList.find(ptr->first) != updateList.end())
            update = " ON DUPLICATE KEY UPDATE " + updateList[ptr->first];
          else
            update = "";

        queue.push_back(queryList[ptr->first] + " " + query + update);
        //if (app->Query("%s %s%s", queryList[ptr->first].c_str(), query.c_str(), update.c_str()) == false) {
        //  app->writeLog(OPENAPRS_LOG_NORMAL, "*** SQL Error: #%d %s", Sql->getErrorNumber(), Sql->getErrorMessage().c_str());
        //  app->writeLog(OPENAPRS_LOG_DEBUG, "Insert::Flush> #%d %s", Sql->getErrorNumber(), Sql->getErrorMessage().c_str());
        //} // if

        if (IsServerConsole)
          cout << "I" << i;
//          app->writeLog(OPENAPRS_LOG_STDOUT, "I%d", i);
      } // if
    } // for

    insertList.clear();

    Unlock();
    //cout << sw.Time() << endl;

    return true;
  } // Insert::Dequeue

} // namespace handler

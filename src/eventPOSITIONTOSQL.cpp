/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** HAL9000, Internet Relay Chat Bot                                     **
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
 $Id: eventPOSITIONTOSQL.cpp,v 1.2 2003/08/30 03:59:00 omni Exp $
 **************************************************************************/

#include <list>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <iostream>
#include <string>
#include <exception>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "App_Log.h"
#include "APRS.h"
#include "Command.h"
#include "DBI.h"
#include "Insert.h"
#include "MemcachedController.h"
#include "Server.h"
#include "Serialize.h"

#include "eventCommands.h"

#include "openaprs.h"
#include "openaprs_string.h"

namespace events {

  using namespace aprs;
  using namespace handler;
  using namespace std;

/**************************************************************************
 ** eventPOSITIONTOSQL Class                                             **
 **************************************************************************/

  /*********************
   ** OnEvent Members **
   *********************/

  const int eventPOSITIONTOSQL::OnEvent(Event *eventInfo, const string &eventMatch, Packet *ePacket) {
    App_Log *log = (App_Log *) ePacket->getVariable("log");
    APRS *aprs = (APRS *) ePacket->getVariable("aprs");
    DBI *dbi = (DBI *) ePacket->getVariable("dbi");
    Insert *sqlq = (Insert *) ePacket->getVariable("sqlq");
    MemcachedController *memcached = (MemcachedController *) ePacket->getVariable("memcached");

    if (app->isMonitor(aprs->getString("aprs.packet.source")))
      app->writeLog(OPENAPRS_LOG_MONITOR, " +-- Position");

    /**
     * MySQL Injection
     *
     * If SQL is enabled, go ahead and proceed with this section
     * otherwise return here.
     *
     */

    if (!IsServerSQL || !IsServerSQLInject)
      return 1;

    /**
     * MySQL Inject: lastposition, position
     *
     * Inject our lastposition and position object.
     */

    app->cfg->inc("openaprs.count.inserted.position", 1);

    // if we're just deleteing an object stop here
    if (aprs->getString("aprs.packet.object.action") == "_") {
      dbi->position->deleteObject( aprs->getString("aprs.packet.object.name") );
      dbi->create->disableObjectBeacon( aprs->getString("aprs.packet.object.name") );
      app->cfg->inc("openaprs.count.deleted.object", dbi->handle()->num_affected_rows());
      return CMDERR_SUCCESS;
    } // if

    sqlq->Add("lastposition", "('%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, %s, %s, %s, '%s', '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, '%s', '%c', '%c', '%s', %s, '%d')",
                   aprs->getString("aprs.packet.id").c_str(),
                   aprs->getString("aprs.packet.callsign.id").c_str(),
                   dbi->handle()->Escape(aprs->getString("aprs.packet.object.name")).c_str(),
                   aprs->getString("aprs.packet.path0").c_str(),
                   aprs->getString("aprs.packet.path.id").c_str(),
                   aprs->getString("aprs.packet.position.latitude.decimal").c_str(),
                   aprs->getString("aprs.packet.position.longitude.decimal").c_str(),
                   NULL_OPTION(aprs, "aprs.packet.dirspd.direction"),
                   NULL_OPTION(aprs, "aprs.packet.dirspd.speed"),
                   NULL_OPTION(aprs, "aprs.packet.altitude"),
                   aprs->getString("aprs.packet.status.id").c_str(),
                   dbi->handle()->Escape(aprs->getString("aprs.packet.symbol.table")).c_str(),
                   dbi->handle()->Escape(aprs->getString("aprs.packet.symbol.code")).c_str(),
                   NULL_OPTION(aprs, "aprs.packet.symbol.overlay"),
                   NULL_OPTION(aprs, "aprs.packet.phg.power"),
                   NULL_OPTION(aprs, "aprs.packet.phg.haat"),
                   NULL_OPTION(aprs, "aprs.packet.phg.gain"),
                   NULL_OPTION(aprs, "aprs.packet.phg.range"),
                   NULL_OPTION(aprs, "aprs.packet.phg.directivity"),
                   NULL_OPTION(aprs, "aprs.packet.phg.beacon"),
                   NULL_OPTION(aprs, "aprs.packet.dfs.power"),
                   NULL_OPTION(aprs, "aprs.packet.dfs.haat"),
                   NULL_OPTION(aprs, "aprs.packet.dfs.gain"),
                   NULL_OPTION(aprs, "aprs.packet.dfs.range"),
                   NULL_OPTION(aprs, "aprs.packet.dfs.directivity"),
                   NULL_OPTION(aprs, "aprs.packet.rng"),
                   aprs->getString("aprs.packet.object.type").c_str(),
                   (aprs->isString("aprs.packet.weather") ? 'Y' : 'N'),
                   (aprs->isString("aprs.packet.telemetry") ? 'Y' : 'N'),
                   aprs->getString("aprs.packet.position.type").c_str(),
                   NULL_OPTION(aprs, "aprs.packet.mic_e.raw.mbits"),
                   time(NULL)
                  );

    sqlq->Add("mem_lastposition", "('%s', '%s', '%s', '%s', '%s', '%s', %s, '%s', '%s', %s, %s, '%s')",
                   aprs->getString("aprs.packet.id").c_str(),
                   aprs->getString("aprs.packet.callsign.id").c_str(),
                   aprs->getString("aprs.packet.path.id").c_str(),
                   dbi->handle()->Escape(aprs->getString("aprs.packet.object.name")).c_str(),
                   aprs->getString("aprs.packet.position.latitude.decimal").c_str(),
                   aprs->getString("aprs.packet.position.longitude.decimal").c_str(),
                   NULL_OPTION(aprs, "aprs.packet.dirspd.direction"),
                   dbi->handle()->Escape(aprs->getString("aprs.packet.symbol.table")).c_str(),
                   dbi->handle()->Escape(aprs->getString("aprs.packet.symbol.code")).c_str(),
                   NULL_OPTION(aprs, "aprs.packet.phg.range"),
                   NULL_OPTION(aprs, "aprs.packet.phg.directivity"),
                   aprs->getString("aprs.packet.object.type").c_str()
                  );

    // add digicoords in memcached
    string key = aprs::StringTool::toUpper( aprs->getString("aprs.packet.source") );
    Serialize s;
    s.add(aprs->getString("aprs.packet.position.latitude.decimal"));
    s.add(aprs->getString("aprs.packet.position.longitude.decimal"));
    string buf = s.compile();

    try {
      memcached->put("digicoords", key, buf, 86400);
    } // try
    catch(MemcachedController_Exception e) {
      log->log(e.message());
    } // catch


    // If this has a valid igate store it.
    if (aprs->isString("aprs.packet.igate") && 0) {
      sqlq->Add("igated", "('%s', '%s', '%s', '%s', '%s', '%d')",
                   aprs->getString("aprs.packet.id").c_str(),
                   aprs->getString("aprs.packet.callsign.id").c_str(),
                   aprs->getString("aprs.packet.igate").c_str(),
                   aprs->getString("aprs.packet.position.latitude.decimal").c_str(),
                   aprs->getString("aprs.packet.position.longitude.decimal").c_str(),
                   time(NULL));
    } // if

    if (aprs->isString("aprs.packet.afrs.frequency")) {
      sqlq->Add("last_frequency", "('%s', '%s', '%s', '%s', %s, %s, %s, %s, %s, %s, '%s', '%d')",
                     aprs->getString("aprs.packet.id").c_str(),
                     aprs->getString("aprs.packet.callsign.id").c_str(),
                     dbi->handle()->Escape(aprs->getString("aprs.packet.object.name")).c_str(),
                     aprs->getString("aprs.packet.afrs.frequency").c_str(),
                     NULL_OPTION(aprs, "aprs.packet.afrs.range"),
                     NULL_OPTION(aprs, "aprs.packet.afrs.range.east"),
                     NULL_OPTION(aprs, "aprs.packet.afrs.tone"),
                     NULL_OPTION(aprs, "aprs.packet.afrs.type"),
                     NULL_OPTION(aprs, "aprs.packet.afrs.frequency.receive"),
                     NULL_OPTION(aprs, "aprs.packet.afrs.frequency.alternate"),
                     aprs->getString("aprs.packet.object.type").c_str(),
                     time(NULL)
                    );
    } // if

    if (!aprs->isString("aprs.packet.object.name")) {
      sqlq->Add("position", "('%s', '%s', '%s', '%s', '%s', %s, %s, %s, %s, %s, '%s', '%s', %s, '%d')",
                 aprs->getString("aprs.packet.id").c_str(),
                 aprs->getString("aprs.packet.callsign.id").c_str(),
                 aprs->getString("aprs.packet.status.id").c_str(),
                 aprs->getString("aprs.current.date").c_str(),
                 aprs->getString("aprs.packet.path.id").c_str(),
                 NULL_OPTION(aprs, "aprs.packet.position.latitude.decimal"),
                 NULL_OPTION(aprs, "aprs.packet.position.longitude.decimal"),
                 NULL_OPTION(aprs, "aprs.packet.dirspd.direction"),
                 NULL_OPTION(aprs, "aprs.packet.dirspd.speed"),
                 NULL_OPTION(aprs, "aprs.packet.altitude"),
                 dbi->handle()->Escape(aprs->getString("aprs.packet.symbol.table")).c_str(),
                 dbi->handle()->Escape(aprs->getString("aprs.packet.symbol.code")).c_str(),
                 NULL_OPTION(aprs, "aprs.packet.timestamp"),
                 time(NULL)
                );

      sqlq->Add("mem_position", "('%s', '%s', %s, %s)",
                 aprs->getString("aprs.packet.callsign.id").c_str(),
                 aprs->getString("aprs.current.date").c_str(),
                 NULL_OPTION(aprs, "aprs.packet.position.latitude.decimal"),
                 NULL_OPTION(aprs, "aprs.packet.position.longitude.decimal")
                );
/*
      sqlq->Add("cache_24hr", "('%s', '%s', '%s', '%s', '%s', %s, %s, %s, %s, %s, '%s', '%s', %s, '%d')",
                 aprs->getString("aprs.packet.id").c_str(),
                 aprs->getString("aprs.packet.callsign.id").c_str(),
                 aprs->getString("aprs.packet.status.id").c_str(),
                 aprs->getString("aprs.current.date").c_str(),
                 aprs->getString("aprs.packet.path.id").c_str(),
                 NULL_OPTION(aprs, "aprs.packet.position.latitude.decimal"),
                 NULL_OPTION(aprs, "aprs.packet.position.longitude.decimal"),
                 NULL_OPTION(aprs, "aprs.packet.dirspd.direction"),
                 NULL_OPTION(aprs, "aprs.packet.dirspd.speed"),
                 NULL_OPTION(aprs, "aprs.packet.altitude"),
                 dbi->handle()->Escape(aprs->getString("aprs.packet.symbol.table")).c_str(),
                 dbi->handle()->Escape(aprs->getString("aprs.packet.symbol.code")).c_str(),
                 NULL_OPTION(aprs, "aprs.packet.timestamp"),
                 time(NULL)
                );
*/
    } // if
    else
      app->cfg->inc("openaprs.count.inserted.object", 1);

    /**
     * WEATHER
     *
     * Limited to 30 minutes per report.
     *
     */
    // Is the packet broadcasting weather information?
    if (aprs->getString("aprs.packet.weather").length() > 0) {
      app->cfg->inc("openaprs.count.inserted.weather", 1);

      sqlq->Add("lastweather", "('%s', '%s', '%s', %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, '%-2f', %s, %d)",
                 aprs->getString("aprs.packet.id").c_str(),
                 aprs->getString("aprs.packet.callsign.id").c_str(),
                 aprs->getString("aprs.current.date").c_str(),
                 NULL_OPTION(aprs, "aprs.packet.position.latitude.decimal"),
                 NULL_OPTION(aprs, "aprs.packet.position.longitude.decimal"),
                 NULL_OPTION(aprs, "aprs.packet.weather.wind.direction"),
                 NULL_OPTION(aprs, "aprs.packet.weather.wind.speed"),
                 NULL_OPTION(aprs, "aprs.packet.weather.wind.gust"),
                 NULL_OPTION(aprs, "aprs.packet.weather.temperature.celcius"),
                 NULL_OPTION(aprs, "aprs.packet.weather.rain.hour"),
                 NULL_OPTION(aprs, "aprs.packet.weather.rain.midnight"),
                 NULL_OPTION(aprs, "aprs.packet.weather.rain.24hour"),
                 NULL_OPTION(aprs, "aprs.packet.weather.humidity"),
                 (float) (atof(aprs->getString("aprs.packet.weather.pressure").c_str())), // FIXME: no need to divide
                 NULL_OPTION(aprs, "aprs.packet.weather.luminosity.wsm"),
                 time(NULL)
                );

      sqlq->Add("weather", "('%s', '%s', '%s', %s, %s, %s, %s, %s, %s, %s, %s, '%-2f', %s, %d)",
                 aprs->getString("aprs.packet.id").c_str(),
                 aprs->getString("aprs.packet.callsign.id").c_str(),
                 aprs->getString("aprs.current.date").c_str(),
                 NULL_OPTION(aprs, "aprs.packet.weather.wind.direction"),
                 NULL_OPTION(aprs, "aprs.packet.weather.wind.speed"),
                 NULL_OPTION(aprs, "aprs.packet.weather.wind.gust"),
                 NULL_OPTION(aprs, "aprs.packet.weather.temperature.celcius"),
                 NULL_OPTION(aprs, "aprs.packet.weather.rain.hour"),
                 NULL_OPTION(aprs, "aprs.packet.weather.rain.midnight"),
                 NULL_OPTION(aprs, "aprs.packet.weather.rain.24hour"),
                 NULL_OPTION(aprs, "aprs.packet.weather.humidity"),
                 (float) (atoi(aprs->getString("aprs.packet.weather.pressure").c_str())), // FIXME: no need to divide
                 NULL_OPTION(aprs, "aprs.packet.weather.luminosity.wsm"),
                 time(NULL)
                );

    } // if

    return CMDERR_SUCCESS;
  } // eventPOSITIONTOSQL::OnEvent
} // namespace commands

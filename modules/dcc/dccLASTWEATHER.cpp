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
 $Id: dccLASTWEATHER.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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

#include "APRS.h"
#include "DCC.h"
#include "DCCUser.h"
#include "Send.h"
#include "Command.h"
#include "Server.h"
#include "Vars.h"

#include "openframe/Stopwatch.h"
#include "openframe/StringToken.h"

#include "dccCommands.h"

#include "dcc.h"
#include "match.h"
#include "openaprs_string.h"

namespace dcc {
  using namespace std;
  using namespace openaprs;
  using namespace aprs;
  using openframe::Stopwatch;
  using openframe::StringToken;

  /**************************************************************************
   ** dccLASTWEATHER Class                                                 **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  /**
   *
   * CL:		Callsign
   * AG:		Age in seconds
   *
   * SYNTAX: .LP CL:<callsign(wildcards[*,?])>[|AG:age|ST:start]
   *
   */
  const int dccLASTWEATHER::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DBI::setMapType sm;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    DCCUser *dccuser;                                     // pointer to a user
    Stopwatch sw;
    map<int, regexMatch> rl;
    char y[10];
    string parseMe;
    string callsign, sf;
    string field;
    stringstream s;
    time_t start_ts = 3600;
    time_t end_ts = time(NULL);
    unsigned int numReturned;
    unsigned int i;
    unsigned int limit = kDefaultResultLimit;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    if (IsDCCUserLogin(dccuser) == false)
      return CMDERR_ACCESS;

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    if (!v.isName("cl")) {
      dccuser->reply("errMissingCallsign");
      return CMDERR_SUCCESS;
    } // if

    if (v.isName("st"))
      end_ts = time(NULL) - time_t(atoi(v["st"].c_str()));

    if (v.isName("ag"))
      start_ts = end_ts - time_t(atoi(v["ag"].c_str()));

    if (v.isName("lm")) {
      if (!ereg("^[0-9]+$", v["lm"], rl)) {
        dccuser->reply("errInvalidLimit");
        return CMDERR_SUCCESS;
      } // if

      limit = atoi(v["lm"].c_str());

      if (limit > 1000) {
        dccuser->reply("errInvalidLimit");
        return CMDERR_SUCCESS;
      } // if
    } // if

    sf = "";
    if (v.isName("sf"))
      sf = v["sf"];

    // initialize variables
    callsign = v["cl"];

    dccuser->reply("replyOk");

    // PROFILING: START
    sw.Start();

    numReturned = dbi->weather->LastByCallsign(callsign, start_ts, end_ts, limit, sm);

    snprintf(y, sizeof(y), "%.4f", sw.Time());

    for(i=0; i < sm.size(); i++) {
      rm = sm[i].resultMap;

      // Build results
      s.str("");
      for(ptr = rm.begin(); ptr != rm.end(); ptr++) {
        if (!dccuser->findFieldByName(ptr->first, field, sf))
          continue;

        if (s.str().length() > 0)
          s << "|";

        s << field << ":" << v.Escape(ptr->second);
      } // if

      dccuser->sendf("308 %s\n", s.str().c_str());
    } // for

    dccuser->sendf("309 RS:%d|MH:%d|SW:%s|MS:%d of %d matches returned (%s seconds).\n", 
                sm.size(),
                numReturned,
                y, 
                sm.size(),
                numReturned,
                y);

    app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (WX) %s received %d of %d matches (%s seconds)",
                      dccuser->callsign().c_str(),
                      sm.size(),
                      numReturned,
                      y);

    dccuser->datapoint("num.op.last.weather");
    return CMDERR_SUCCESS;
  } // dccLASTWEATHER::Execute

  /**************************************************************************
   ** dccLASTWEATHERCENTER Class                                           **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  /**
   *
   * CN:		<lat>,<lon>
   * RG:		<range in meters>
   * AG:		Age in seconds
   *
   * SYNTAX: .WC CN:<lat>,<lon>|RG:<range in meters>[|AG:age|ST:start]
   *
   */
  const int dccLASTWEATHERCENTER::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DBI::setMapType sm;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    DCCUser *dccuser;                                     // pointer to a user
    Stopwatch sw;
    StringToken st;
    char y[10];
    double clat, clon, lat1, lon1, lat2, lon2, rg;
    map<int, regexMatch> rl;
    string parseMe;
    string callsign, sf;
    string field;
    stringstream s;
    time_t end_ts = time(NULL);
    time_t start_ts = end_ts - 3600;
    unsigned int numReturned;
    unsigned int i;
    unsigned int limit = kDefaultResultLimit;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    if (IsDCCUserLogin(dccuser) == false)
      return CMDERR_ACCESS;

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    if (!v.isName("cn")) {
      dccuser->reply("errMissingCenter");
      return CMDERR_SUCCESS;
    } // if

    st.setDelimiter(',');
    st = v["cn"];

    if (st.size() != 2) {
      dccuser->reply("errInvalidCenter");
      return CMDERR_SUCCESS;
    } // if

    clat = atof(st[0].c_str());
    clon = atof(st[1].c_str());

    if (!APRS::isValidLatLong(clat,clon)) {
      dccuser->reply("errInvalidCenter");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("rg")) {
      dccuser->reply("errMissingRange");
      return CMDERR_SUCCESS;
    } // if

    rg = atof(v["rg"].c_str());

    if (rg < 0.0 || rg > 200.00) {
      dccuser->reply("errInvalidRange");
      return CMDERR_SUCCESS;
    } // if

    // Try and calculate bounding box.
    APRS::calcDestPoint(315, (rg*2), clat, clon, lat1, lon1);
    APRS::calcDestPoint(135, (rg*2), clat, clon, lat2, lon2);

//cout << clat << " " << clon << endl;
//cout << lat1 << " " << lon1 << endl;
//cout << lat2 << " " << lon2 << endl;

    if (!APRS::isValidLatLong(lat1,lon1) || !APRS::isValidLatLong(lat2,lon2)) {
      dccuser->reply("errInvalidBBOX");
      return CMDERR_SUCCESS;
    } // if

    if (v.isName("st"))
      end_ts = time(NULL) - time_t(atoi(v["st"].c_str()));

    if (v.isName("ag"))
      start_ts = end_ts - time_t(atoi(v["ag"].c_str()));

    if (v.isName("lm")) {
      if (!ereg("^[0-9]+$", v["lm"], rl)) {
        dccuser->reply("errInvalidLimit");
        return CMDERR_SUCCESS;
      } // if

      limit = atoi(v["lm"].c_str());

      if (limit > 1000) {
        dccuser->reply("errInvalidLimit");
        return CMDERR_SUCCESS;
      } // if
    } // if

    sf = "";
    if (v.isName("sf"))
      sf = v["sf"];

    // initialize variables
    callsign = v["cl"];

    dccuser->reply("replyOk");

    // PROFILING: START
    sw.Start();

    numReturned = dbi->weather->LastByLatLong(lat1,
                                                      lon1,
                                                      lat2,
                                                      lon2,
                                                      start_ts, 
                                                      end_ts, 
                                                      limit, 
                                                      sm);

    snprintf(y, sizeof(y), "%.4f", sw.Time());

    for(i=0; i < sm.size(); i++) {
      rm = sm[i].resultMap;

      // Build results
      s.str("");
      for(ptr = rm.begin(); ptr != rm.end(); ptr++) {
        if (!dccuser->findFieldByName(ptr->first, field, sf))
          continue;

        if (s.str().length() > 0)
          s << "|";

        s << field << ":" << v.Escape(ptr->second);
      } // if

      dccuser->sendf("308 %s\n", s.str().c_str());
    } // for

    dccuser->sendf("309 RS:%d|MH:%d|SW:%s|BB:%f,%f,%f,%f|MS:%d of %d matches returned (%s seconds).\n", 
                sm.size(),
                numReturned,
                y, 
                lat1,
                lon1,
                lat2,
                lon2,
                sm.size(),
                numReturned,
                y);

    app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (WC) %s received %d of %d matches (%s seconds)",
                      dccuser->callsign().c_str(),
                      sm.size(),
                      numReturned,
                      y);

    dccuser->datapoint("num.op.last.weather");
    return CMDERR_SUCCESS;
  } // dccLASTWEATHERCENTER::Execute

  /**************************************************************************
   ** dccLASTWEATHERBBOX Class                                             **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  /**
   *
   * BBOX:		<lat1>,<lon1>,<lat2>,<lon2>
   * AG:		Age in seconds
   *
   * SYNTAX: .WB BB:<lat1>,<lon1>,<lat2>,<lon2>[|AG:age|ST:start]
   *
   */
  const int dccLASTWEATHERBBOX::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DBI::setMapType sm;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    DCCUser *dccuser;                                     // pointer to a user
    Stopwatch sw;
    StringToken st;
    char y[10];
    double lat1, lon1, lat2, lon2;
    map<int, regexMatch> rl;
    string parseMe;
    string callsign, sf;
    string field;
    stringstream s;
    time_t start_ts = 3600;
    time_t end_ts = time(NULL);
    unsigned int numReturned;
    unsigned int i;
    unsigned int limit = kDefaultResultLimit;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    if (IsDCCUserLogin(dccuser) == false)
      return CMDERR_ACCESS;

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    if (!v.isName("bb")) {
      dccuser->reply("errMissingBBOX");
      return CMDERR_SUCCESS;
    } // if

    st.setDelimiter(',');
    st = v["bb"];

    if (st.size() != 4) {
      dccuser->reply("errInvalidBBOX");
      return CMDERR_SUCCESS;
    } // if

    lat1 = atof(st[0].c_str());
    lon1 = atof(st[1].c_str());
    lat2 = atof(st[2].c_str());
    lon2 = atof(st[3].c_str());

    if (!APRS::isValidLatLong(lat1,lon1) || !APRS::isValidLatLong(lat2,lon2)) {
      dccuser->reply("errInvalidBBOX");
      return CMDERR_SUCCESS;
    } // if

    if (v.isName("st"))
      end_ts = time(NULL) - time_t(atoi(v["st"].c_str()));

    if (v.isName("ag"))
      start_ts = end_ts - time_t(atoi(v["ag"].c_str()));

    if (v.isName("lm")) {
      if (!ereg("^[0-9]+$", v["lm"], rl)) {
        dccuser->reply("errInvalidLimit");
        return CMDERR_SUCCESS;
      } // if

      limit = atoi(v["lm"].c_str());

      if (limit > 1000) {
        dccuser->reply("errInvalidLimit");
        return CMDERR_SUCCESS;
      } // if
    } // if

    sf = "";            
    if (v.isName("sf"))
      sf = v["sf"];

    // initialize variables
    callsign = v["cl"];

    dccuser->reply("replyOk");

    // PROFILING: START
    sw.Start();

    numReturned = dbi->weather->LastByLatLong(lat1,
                                                      lon1,
                                                      lat2,
                                                      lon2,
                                                      start_ts, 
                                                      end_ts, 
                                                      limit, 
                                                      sm);

    snprintf(y, sizeof(y), "%.4f", sw.Time());

    for(i=0; i < sm.size(); i++) {
      rm = sm[i].resultMap;

      // Build results
      s.str("");
      for(ptr = rm.begin(); ptr != rm.end(); ptr++) {
        if (!dccuser->findFieldByName(ptr->first, field, sf))
          continue;

        if (s.str().length() > 0)
          s << "|";

        s << field << ":" << v.Escape(ptr->second);
      } // if

      dccuser->sendf("308 %s\n", s.str().c_str());
    } // for

    dccuser->sendf("309 RS:%d|MH:%d|SW:%s|MS:%d of %d matches returned (%s seconds).\n", 
                sm.size(),
                numReturned,
                y, 
                sm.size(),
                numReturned,
                y);

    app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (WB) %s received %d of %d matches (%s seconds)",
                      dccuser->callsign().c_str(),
                      sm.size(),
                      numReturned,
                      y);

    dccuser->datapoint("num.op.last.weather");
    return CMDERR_SUCCESS;
  } // dccLASTWEATHERBBOX::Execute
} // namespace dcc

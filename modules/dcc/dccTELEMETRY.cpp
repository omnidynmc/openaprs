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
 $Id: dccTELEMETRY.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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
#include "DBI.h"
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
   ** dccLASTTELEMETRY Class                                               **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  /**
   *
   * CL:		Callsign
   * AG:		Age in seconds
   *
   * SYNTAX: .LT CL:<callsign(wildcards[*,?])>[|AG:age|ST:start]
   *
   */
  const int dccLASTTELEMETRY::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DBI::setMapType sm, sm2;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    DCCUser::fieldsMapType fieldsMap;
    DCCUser *dccuser;                                     // pointer to a user
    Stopwatch sw;
    map<int, regexMatch> rl;
    char y[10];
    string parseMe;
    string callsign, sf;
    string field;
    string fields;
    stringstream s;
    time_t start_ts = 3600;
    time_t end_ts = time(NULL);
    unsigned int numReturned;
    unsigned int i, j;
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

    numReturned = dbi->telemetry->LastByCallsign(callsign, start_ts, end_ts, limit, sm);

    for(i=0; i < sm.size(); i++) {
      // First, try and find any labels
      dbi->telemetry->LabelsByCallsign(sm[i].resultMap["source"], 0, time(NULL), 1, sm2);
      dccuser->fieldMe(sm2, fieldsMap, sf);

      for(j=0; j < fieldsMap.size(); j++)
        dccuser->sendf("312 %s\n", fieldsMap[j].c_str());

      // Next find EQNS
      dbi->telemetry->EqnsByCallsign(sm[i].resultMap["source"], 0, time(NULL), 1, sm2);
      dccuser->fieldMe(sm2, fieldsMap, sf);

      for(j=0; j < fieldsMap.size(); j++)
        dccuser->sendf("313 %s\n", fieldsMap[j].c_str());

      // Then find Units
      dbi->telemetry->UnitsByCallsign(sm[i].resultMap["source"], 0, time(NULL), 1, sm2);
      dccuser->fieldMe(sm2, fieldsMap, sf);

      for(j=0; j < fieldsMap.size(); j++)
        dccuser->sendf("314 %s\n", fieldsMap[j].c_str());

      // Then find Bits
      dbi->telemetry->BitsByCallsign(sm[i].resultMap["source"], 0, time(NULL), 1, sm2);
      dccuser->fieldMe(sm2, fieldsMap, sf);

      for(j=0; j < fieldsMap.size(); j++)
        dccuser->sendf("315 %s\n", fieldsMap[j].c_str());

      fields = dccuser->fieldMe(sm[i].resultMap, sf);
      dccuser->sendf("310 %s\n", fields.c_str());
    } // for

    snprintf(y, sizeof(y), "%.4f", sw.Time());

    dccuser->sendf("311 RS:%d|MH:%d|SW:%s|MS:%d of %d matches returned (%s seconds).\n", 
                sm.size(),
                numReturned,
                y, 
                sm.size(),
                numReturned,
                y);

    app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (LT) %s received %d of %d matches (%s seconds)",
                      dccuser->callsign().c_str(),
                      sm.size(),
                      numReturned,
                      y);

    dccuser->datapoint("num.op.last.telemetry");
    return CMDERR_SUCCESS;
  } // dccLASTTELEMETRY::Execute

  /**************************************************************************
   ** dccTELEMETRYHISTORY Class                                            **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  /**
   *
   * CL:		Callsign
   * AG:		Age in seconds
   *
   * SYNTAX: .TH CL:<callsign(wildcards[*,?])>[|AG:age|ST:start]
   *
   */
  const int dccTELEMETRYHISTORY::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DBI::setMapType sm, sm2;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    DCCUser::fieldsMapType fieldsMap;
    DCCUser *dccuser;                                     // pointer to a user
    Stopwatch sw;
    map<int, regexMatch> rl;
    char y[10];
    string parseMe;
    string callsign, sf;
    string field;
    string fields;
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

    numReturned = dbi->telemetry->HistoryByCallsign(callsign, start_ts, end_ts, limit, sm);

    dccuser->fieldMe(sm, fieldsMap, sf);

    for(i=0; i < fieldsMap.size(); i++)
      dccuser->sendf("310 %s\n", fieldsMap[i].c_str());

    snprintf(y, sizeof(y), "%.4f", sw.Time());

    dccuser->sendf("311 RS:%d|MH:%d|SW:%s|MS:%d of %d matches returned (%s seconds).\n", 
                sm.size(),
                numReturned,
                y,
                sm.size(),
                numReturned,
                y);

    app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (TH) %s received %d of %d matches (%s seconds)",
                      dccuser->callsign().c_str(),
                      sm.size(),
                      numReturned,
                      y);

    dccuser->datapoint("num.op.telemetry.history");
    return CMDERR_SUCCESS;
  } // dccTELEMETRYHISTORY::Execute

} // namespace dcc

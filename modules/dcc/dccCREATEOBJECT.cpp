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
 $Id: dccCREATEOBJECT.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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
#include "StringToken.h"
#include "Command.h"
#include "Server.h"
#include "Vars.h"

#include "dccCommands.h"

#include "match.h"
#include "openaprs_string.h"

namespace dcc {

using namespace std;
using namespace openaprs;
using namespace aprs;

  /**************************************************************************
   ** dccCREATEOBJECT Class                                                **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  /**
   *
   *
   * NM:		Name
   * LN:		Longitude
   * LA:		Latitude
   * LL:??		Create Locally Only
   * AT:		Altitude
   * SP:		Speed
   * CR:		Course
   * TB:		Symbol Table
   * CD:		Symbol Code
   * AM:		Ambiguity
   * CM:		Comment
   * CP:		Compress
   * BC:		Beacon Frequency
   * EX:		Expire timestamp
   */
  const int dccCREATEOBJECT::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DCCUser *dccuser;			// pointer to a user
    bool compress;
    bool local = false;
    double latitude, longitude;
    double altitude, speed;
    int course;
    int ambiguity, testAmbiguity;
    map<int, regexMatch> regexList;
    string comment;
    string name;
    string parseMe;
    string source;
    string symbol_table;
    string symbol_code;
    time_t beacon;
    time_t expire_ts;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    if (IsDCCUserLogin(dccuser) == false)
      return CMDERR_ACCESS;

    if (IsDCCUserGuest(dccuser)) {
      dccuser->reply("errAccessGuest");
      return CMDERR_SUCCESS;
    } // if

    if (!IsDCCUserVerified(dccuser)) {
      dccuser->reply("errLoginVerify");
      return CMDERR_SUCCESS;
    } // if

    if (IsDCCUserLocal(dccuser))
      local = true;

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    // initialize variables
    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    // Check for missing variables first,
    // after we're doing with this we'll check each ones
    // validity.
    if (!v.isName("nm")) {
      dccuser->reply("errMissingObjectName");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("la")) {
      dccuser->reply("errMissingLatitude");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("ln")) {
      dccuser->reply("errMissingLongitude");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("tb")) {
      dccuser->reply("errMissingSymTable");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("cd")) {
      dccuser->reply("errMissingSymCode");
      return CMDERR_SUCCESS;
    } // if

    // initialize variables
    ambiguity = 0;
    comment = "";
    speed = -1.0;
    altitude = 0.0;
    course = 0;
    compress = false;
    beacon = 0;
    expire_ts = 0;

    if (v.isName("am")) {
      testAmbiguity = atoi(v["am"].c_str());
      if (testAmbiguity < 0 && testAmbiguity > 4) {
        dccuser->reply("errInvalidAmbiguity");
        return CMDERR_SUCCESS;
      } // if

      ambiguity = testAmbiguity;
    } // if

    if (v.isName("sp")) {
      speed = atof(v["sp"].c_str());

      if (speed < 0) {
        dccuser->reply("errInvalidSpeed");
        return CMDERR_SUCCESS;
      } // if
    } // if

    if (v.isName("cr")) {
      course = atoi(v["cr"].c_str());

      if (course > 360) {
        dccuser->reply("errInvalidCourse");
        return CMDERR_SUCCESS;
      } // if
    } // if

    if (v.isName("cp")) {
      if (ereg("^(YES|NO)$", strtoupper(v["cp"]), regexList) < 1) {
        dccuser->reply("errInvalidCompress");
        return CMDERR_SUCCESS;
      } // if

      if (strtoupper(v["cp"]) == "YES")
        compress = true;
    } // if

    // Check for beacon frequency variables.
    if (v.isName("bc")) {
      if (ereg("^([0-9]+)$", v["bc"], regexList) < 1) {
        dccuser->reply("errInvalidBeacon");
        return CMDERR_SUCCESS;
      } // if

      // If a beacon frequency is set, an expiration
      // must also be set.
      if (!v.isName("ex")) {
        dccuser->reply("errMissingExpire");
        return CMDERR_SUCCESS;
      } // if

      // Expiration cannot be more than 1 year from
      // current date.
      expire_ts = time_t(atoi(v["ex"].c_str()));
      if (expire_ts > 31536000) {
        dccuser->reply("errInvalidExpire");
        return CMDERR_SUCCESS;
      } // if

      expire_ts += time(NULL);

      // Beacon cannot be less than 30 mintues or
      // greater than 1 year.
      beacon = time_t(atoi(v["bc"].c_str()));
      if (beacon < 1800 || beacon > 31536000) {
        dccuser->reply("errInvalidBeacon");
        return CMDERR_SUCCESS;
      } // if
    } // if

    if (v.isName("at"))
      altitude = atof(v["at"].c_str());

    latitude = atof(v["la"].c_str());
    longitude = atof(v["ln"].c_str());

    if (!APRS::isValidLatLong(latitude, longitude)) {
      dccuser->reply("errInvalidLatLong");
      return CMDERR_SUCCESS;
    } // if

    if (ereg("^([\x20-\x7e]{1,9})$", v["nm"], regexList) < 1) {
      dccuser->reply("errInvalidName");
      return CMDERR_SUCCESS;
    } // if

    if (ereg("^([A-Z0-9\x2f\x5c]{1})$", v["tb"], regexList) < 1) {
      dccuser->reply("errInvalidSymbolTable");
      return CMDERR_SUCCESS;
    } // if

    if (ereg("^([\x20-\x7e]{1})$", v["cd"], regexList) < 1) {
      dccuser->reply("errInvalidSymbolCode");
      return CMDERR_SUCCESS;
    } // if

    if (v.isName("cm")) {
      if (ereg("^([\x20-\x7e]+)$", v["cm"], regexList) < 1) {
        dccuser->reply("errInvalidComment");
        return CMDERR_SUCCESS;
      } // if

      comment = v["cm"];
    } // if

    if (v.isName("ll")) {
      if (ereg("^([YyNn])$", v["ll"], regexList) < 1) {
        dccuser->reply("errInvalidLocal");
        return CMDERR_SUCCESS;
      } // if
    
      local = (strtoupper(v["ll"]) == "Y") ? true : false;
    } // if


    name = v["nm"];
    source = dccuser->callsign();
    symbol_table = v["tb"];
    symbol_code = v["cd"];

    if (!dbi->create->Object(name,
                                     source,
                                     "APRS",
                                     latitude,
                                     longitude,
                                     symbol_table,
                                     symbol_code,
                                     speed,
                                     course,
                                     altitude,
                                     compress,
                                     ambiguity,
                                     comment,
                                     beacon,
                                     expire_ts,
                                     local)) {
      dccuser->reply("errNoCreateObject");
      return CMDERR_SUCCESS;
    } // if

    dccuser->datapoint("num.create.object");
    dccuser->reply("replyObjectCreated");

    return 1;
  } // dccCREATEOBJECT::Execute
} // namespace dcc

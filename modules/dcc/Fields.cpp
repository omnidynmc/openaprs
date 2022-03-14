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
 $Id: Fields.cpp,v 1.12 2003/09/05 22:23:41 omni Exp $
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
#include <sstream>

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

#include "Fields.h"

#include "openframe/StringToken.h"
#include "openframe/StringTool.h"

#include "openaprs_string.h"

namespace dcc {
  using openframe::StringToken;
  using openframe::StringTool;
  using namespace std;

/**************************************************************************
 ** Fields Class                                                         **
 **************************************************************************/

  Fields::Fields() {
    fieldMap.clear();

    add("A0", "analog_0", "Telemetry Analog 0");
    add("A1", "analog_1", "Telemetry Analog 1");
    add("A2", "analog_2", "Telemetry Analog 2");
    add("A3", "analog_3", "Telemetry Analog 3");
    add("A4", "analog_4", "Telemetry Analog 4");
    add("AD", "addressee", "Message Addressee");
    add("ADM", "admin", "Is user admin?");
    add("AM", "ambiguity", "Ambiguity (0 to 4)");
    add("AG", "age", "Age (in seconds)");
    add("AL", "alternate", "Alternate Frequency");
    add("AT", "altitude", "aprs.packet.altitude", "Altitude (in meters)");
    add("AV", "active", "Account Active?");
    add("AY", "afrs_type", "AFRS Type");
    add("BA", "barometer", "aprs.packet.weather.pressure", "Pressure");
    add("BB", "bbox", "Latitude/Longitude Bounding Box");
    add("BC", "beacon", "Beacon Frequency (in seconds)");
    add("BS", "bitsense", "Telemetry Bitsense");
    add("CD", "symbol_code", "aprs.packet.symbol.code", "Symbol Code");
    add("CHG", "", "Change User");
    add("CI", "city", "ULS City");
    add("CT", "create_ts", "Creation Timestamp");
    add("CL", "callsign", "Callsign (wildcards accepted)");
    add("CLA", "class", "ULS Class");
    add("CLI", "", "DCC Client Authentication");
    add("CM", "status", "aprs.packet.comment", "Comment");
    add("CN", "", "Center Point");
    add("CP", "compress", "Compress");
    add("CR", "course", "aprs.packet.dirspd.direction", "Course");
    add("D0", "d_0", "Telemetry Digital Label 0");
    add("D1", "d_1", "Telemetry Digital Label 1");
    add("D2", "d_2", "Telemetry Digital Label 2");
    add("D3", "d_3", "Telemetry Digital Label 3");
    add("D4", "d_4", "Telemetry Digital Label 4");
    add("D5", "d_5", "Telemetry Digital Label 5");
    add("D6", "d_6", "Telemetry Digital Label 6");
    add("D7", "d_7", "Telemetry Digital Label 7");
    add("DG", "digital", "Telemetry Digital");
    add("DT", "", "Device Token");
    add("ED", "expired_date", "Expired Date");
    add("EFD", "effective_date", "Effective Date");
    add("EA0", "a0", "Telemetry EQNS a0");
    add("EB0", "b0", "Telemetry EQNS b0");
    add("EC0", "c0", "Telemetry EQNS c0");
    add("EA1", "a1", "Telemetry EQNS a1");
    add("EB1", "b1", "Telemetry EQNS b1");
    add("EC1", "c1", "Telemetry EQNS c1");
    add("EA2", "a2", "Telemetry EQNS a2");
    add("EB2", "b2", "Telemetry EQNS b2");
    add("EC2", "c2", "Telemetry EQNS c2");
    add("EA3", "a3", "Telemetry EQNS a3");
    add("EB3", "b3", "Telemetry EQNS b3");
    add("EC3", "c3", "Telemetry EQNS c3");
    add("EA4", "a4", "Telemetry EQNS a4");
    add("EB4", "b4", "Telemetry EQNS b4");
    add("EC4", "c4", "Telemetry EQNS c4");
    add("EM", "", "Email Address");
    add("EN", "entity_name", "ULS Entity Name");
    add("EX", "expire_ts", "Expire Timestamp");
    add("FNM", "first_name", "First name");
    add("FQ", "frequency", "Frequency");
    add("GD", "grant_date", "Grant Date");
    add("HU", "humidity", "aprs.packet.weather.humidity", "Humidity");
    add("IC", "icon", "Station Symbol Icon Name");
    add("ID", "packet_id", "aprs.packet.id", "Unique Packet ID");
    add("IDR", "id", "Unique Record ID");
    add("IE", "ie", "IE enabled?");
    add("KY", "", "DCC Client Key");
    add("L0", "a_0", "Telemetry Label 0");
    add("L1", "a_1", "Telemetry Label 1");
    add("L2", "a_2", "Telemetry Label 2");
    add("L3", "a_3", "Telemetry Label 3");
    add("L4", "a_4", "Telemetry Label 4");
    add("LA", "latitude", "aprs.packet.position.latitude.decimal", "Latitude");
    add("LAD", "last_action_date", "ULS Last Action Date");
    add("LL", "", "", "Create Locally Only");
    add("LN", "longitude", "aprs.packet.position.longitude.decimal", "Longitude");
    add("LNM", "last_name", "Last name");
    add("LM", "limit", "Limit");
    add("LT", "login_ts", "Last Login Timestamp");
    add("LU", "luminosity", "aprs.packet.weather.luminosity.wsm", "Luminosity");
    add("MI", "middle_initial", "ULS Middle Initial");
    add("MS", "text", "Message");
    add("MH", "", "Matches");
    add("NM", "name", "aprs.packet.object.name", "Name");
    add("NMA", "", "", "Live Interface in NMEA Mode");
    add("ON", "", "Is On");
    add("PT", "postype", "aprs.packet.position.type", "Position Type");
    add("PC", "previous_callsign", "ULS Previous Callsign");
    add("RD", "rain_calendar_day", "aprs.packet.weather.rain.midnight", "Rain Calendar Day");
    add("RE", "range_east", "Range East");
    add("RG", "range", "Range");
    add("RH", "rain_hour", "aprs.packet.weather.rain.hour", "Rain in Last Hour");
    add("RM", "rain_24hour_day", "aprs.packet.weather.rain.24hour", "Rain Last 24 Hours");
    add("RS", "", "Number of Results Returned");
    add("RX", "receive", "Receive");
    add("SA", "street_address", "ULS Street Address");
    add("SD", "", "Hash seed for client authentication");
    add("SN", "", "Message Sent From User");
    add("SP", "speed", "aprs.packet.dirspd.speed", "Message Sent From User");
    add("SQ", "sequence", "Telemetry Sequence");
    add("SR", "source", "aprs.packet.source", "Source");
    add("ST", "", "Start Timestamp");
    add("STA", "state", "ULS State");
    add("SU", "suffix", "ULS Suffix");
    add("SW", "", "Stopwatch (in seconds)");
    add("SY", "", "Symbol Name");
    add("TG", "", "Target Messages");
    add("TO", "", "Message To");
    add("TB", "symbol_table", "aprs.packet.symbol.table", "Symbol Table");
    add("TM", "temperature", "aprs.packet.weather.temperature.celcius", "Temperature");
    add("TN", "tone", "PL Tone");
    add("TY", "type", "Type");
    add("UID", "", "UID");
    add("VT", "verified_ts", "Verified Timestamp");
    add("VY", "verified", "User verified?");
    add("WD", "wind_direction", "aprs.packet.weather.wind.direction", "Wind Direction");
    add("WG", "wind_gust", "aprs.packet.weather.wind.gust", "Wind Gust (in kilometers/h)");
    add("WS", "wind_speed", "aprs.packet.weather.wind.speed", "Wind Speed (in kilometers/h)");
    add("WU", "wind_sustained", "Wind Sustained (in kilometers/h)");
    add("WX", "weather", "Has weather data?");
    add("ZC", "zip_code", "ULS Zip Code");

    return;
  } // Fields::Fields

  Fields::~Fields() {
    return;
  } // Fields::~Fields

  const bool Fields::add(const string &token,
                         const string &name,
                         const string &descrip) {
    return add(token, name, "", descrip);
  } // Fields::add

  const bool Fields::add(const string &token,
                         const string &name,
                         const string &event,
                         const string &descrip) {
    fieldMapType::iterator ptr;
    Field f;

    if ((ptr = fieldMap.find(token)) != fieldMap.end())
      return false;

    f.token = token;
    f.name = name;
    f.event = event;
    f.descrip = descrip;

    fieldMap.insert(pair<string, Field>(token, f));
    fieldEventMap.insert(pair<string, Field>(event, f));

    return true;
  } // Fields::add

  const bool Fields::remove(const string &token) {
    fieldMapType::iterator ptr;

    if ((ptr = fieldMap.find(token)) == fieldMap.end())
      return false;

    fieldMap.erase(ptr);

    return true;
  } // Fields::remove

  const unsigned int Fields::clear() {
    unsigned int numRemoved;			// number of users removed

    // initialize variables
    numRemoved = fieldMap.size();

    // clear client list
    fieldMap.clear();

    return numRemoved;
  } // Fields:clear

  const bool Fields::find(const string &token, string &name) {
    fieldMapType::iterator ptr;

    if ((ptr = fieldMap.find(name)) == fieldMap.end())
      return false;

    name = ptr->second.name;

    return true;
  } // Fields::find

  const bool Fields::findByName(const string &name, 
                                  string &token) {
    fieldMapType::iterator ptr;

    for(ptr = fieldMap.begin();
        ptr != fieldMap.end();
        ptr++) {
      if (ptr->second.name == name) {
        token = ptr->second.token;

        return true;
      } // if
    } // for

    return false;
  } // Fields::findByName

  const bool Fields::findByName(const string &name, 
                                string &token,
                                const string &filter) {

    StringToken st;
    bool wasFound;
    fieldMapType::iterator ptr;
    unsigned int i;

    st.setDelimiter(',');
    st = filter;

    for(ptr = fieldMap.begin();
        ptr != fieldMap.end();
        ptr++) {
      if (ptr->second.name == name) {
        if (st.size() > 0) {
          wasFound = false;
          for(i=0; i < st.size() && !wasFound; i++) {
            if (StringTool::toUpper(ptr->second.token) == StringTool::toUpper(st[i]))
              wasFound = true;
          } // for

          if (wasFound == false)
            return false;
        } // if

        token = ptr->second.token;

        return true;
      } // if
    } // for

    return false;
  } // Fields::findByName

  const bool Fields::findByEvent(const string &event, string &token) {
    fieldMapType::iterator ptr;

    ptr = fieldEventMap.find(event);
    if (ptr == fieldEventMap.end()) return false;
    token = ptr->second.token;

    return true;
  } // Fields::findByEvent

} // namespace dcc

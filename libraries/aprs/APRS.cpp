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
 $Id: APRS.cpp,v 1.2 2005/12/13 21:07:03 omni Exp $
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

#include <assert.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "APRS.h"
#include "StringToken.h"
#include "StringTool.h"

#include "md5wrapper.h"

#define pi 3.14159265358979323846

namespace aprs {

using std::string;
using std::list;
using std::ofstream;
using std::ostream;
using std::stringstream;
using std::ios;
using std::endl;
using std::cout;

static const char *symbol_table[] = {
 "/!", "\\!", "/\"", "\\\"", "/#", "\\#", "/$", "\\$", "/%", "\\%", "/&", "\\&",
 "/\'", "\\\'", "/(", "\\(", "/)", "\\)", "/*", "\\*", "/+", "\\+", "/,", "\\,",
 "/-", "\\-", "/.", "\\.", "//", "\\/", "/0", "\\0", "/1", "\\1", "/2", "\\2",
 "/3", "\\3", "/4", "\\4", "/5", "\\5", "/6", "\\6", "/7", "\\7", "/8", "\\8",
 "/9", "\\9", "/:", "\\:", "/;", "\\;", "/<", "\\<", "/=", "\\=", "/>", "\\>",
 "/?", "\\?", "/@", "\\@", "/A", "\\A", "/B", "\\B", "/C", "\\C", "/D", "\\D",
 "/E", "\\E", "/F", "\\F", "/G", "\\G", "/H", "\\H", "/I", "\\I", "/J", "\\J",
 "/K", "\\K", "/L", "\\L", "/M", "\\M", "/N", "\\N", "/O", "\\O", "/P", "\\P",
 "/Q", "\\Q", "/R", "\\R", "/S", "\\S", "/T", "\\T", "/U", "\\U", "/V", "\\V",
 "/W", "\\W", "/X", "\\X", "/Y", "\\Y", "/Z", "\\Z", "/[", "\\[", "/\\", "\\\\",
 "/]", "\\]", "/^", "\\^", "/_", "\\_", "/`", "\\`", "/a", "\\a", "/b", "\\b",
 "/c", "\\c", "/d", "\\d", "/e", "\\e", "/f", "\\f", "/g", "\\g", "/h", "\\h",
 "/i", "\\i", "/j", "\\j", "/k", "\\k", "/l", "\\l", "/m", "\\m", "/n", "\\n",
 "/o", "\\o", "/p", "\\p", "/q", "\\q", "/r", "\\r", "/s", "\\s", "/t", "\\t",
 "/u", "\\u", "/v", "\\v", "/w", "\\w", "/x", "\\x", "/y", "\\y", "/z", "\\z",
 "/{", "\\{", "/|", "\\|", "/}", "\\}", "/~", "\\~", NULL
};

static const char *tone_table[] = {
  "67.0",  "69.3",  "71.9",  "74.4",  "77.0",  "79.7",  "82.5",  "88.5",  "91.5",
  "94.8",  "97.4",  "100.0", "103.5", "107.2", "110.9", "114.8", "118.8", "123.0",
  "127.3", "131.8", "136.5", "141.3", "146.2", "151.4", "156.7", "162.2", "167.9",
  "173.8", "179.9", "186.2", "192.8", "203.5", "206.5", "210.7", "218.1", "225.7",
  "229.1", "233.6", "241.8", "250.3", "254.1", NULL
};

static const char *gpsxyz_table[] = {
  "BB", "OB", "BC", "OC", "BD", "OD", "BE", "OE", "BF", "OF", "BG", "OG", "BH",
  "OH", "BI", "OI", "BJ", "OJ", "BK", "OK", "BL", "OL", "BM", "OM", "BN", "ON",
  "BO", "OO", "BP", "OP", "P0", "A0", "P1", "A1", "P2", "A2", "P3", "A3", "P4",
  "A4", "P5", "A5", "P6", "A6", "P7", "A7", "P8", "A8", "P9", "A9", "MR", "NR",
  "MS", "NS", "MT", "NT", "MU", "NU", "MV", "NV", "MW", "NW", "MX", "NX", "PA",
  "AA", "PB", "AB", "PC", "AC", "PD", "AD", "PE", "AE", "PF", "AF", "PG", "AG",
  "PH", "AH", "PI", "AI", "PJ", "AJ", "PK", "AK", "PL", "AL", "PM", "AM", "PN",
  "AN", "PO", "AO", "PP", "AP", "PQ", "AQ", "PR", "AR", "PS", "AS", "PT", "AT",
  "PU", "AU", "PV", "AV", "PW", "AW", "PX", "AX", "PY", "AY", "PZ", "AZ", "HS",
  "DS", "HT", "DT", "HU", "DU", "HV", "DV", "HW", "DW", "HX", "DX", "LA", "SA",
  "LB", "SB", "LC", "SC", "LD", "SD", "LE", "SE", "LF", "SF", "LG", "SG", "LH",
  "SH", "LI", "SI", "LJ", "SJ", "LK", "SK", "LL", "SL", "LM", "SM", "LN", "SN",
  "LO", "SO", "LP", "SP", "LQ", "SQ", "LR", "SR", "LS", "SS", "LT", "ST", "LU",
  "SU", "LV", "SV", "LW", "SW", "LX", "SX", "LY", "SY", "LZ", "SZ", "J1", "Q1",
  "J2", "Q2", "J3", "Q3", "J3", "Q4", NULL
};

static const char *ssid_table[] = {
  "", "/1", "/U", "/f", "/b", "/Y", "/X", "/\'", "/s", "/>", "/<", "/O",
  "/j", "/R", "/k", "/v", NULL
};

static int dirs[] = { -1, 45, 90, 135, 180, 225, 270, 315, 0 };

/**
 * Distance Units
 *
 * Metres, Millimetres, Centimetres, Kilometres, Inches, Feet, Yards, Fathoms
 * 0       1            2            3           4       5     6      7
 * Statute Miles, Nautiecal Miles, Light Years (9460000000000000)
 * 8              9                10
 */

static double distunits[] = { 1, 0.001, 0.01, 1000, 0.0254, 0.3048, 0.9144,
                              1.8288, 1609, 1852, -1};

/**
 * Speed Units
 *
 * Metres/s, Kilometre/hr, Feet/s, Feet/m, Yard/m, Statute Mile/hr, Knot
 * 0         1             2       3       4       5                6
 */

static double speedunits[] = { 1, 0.277777777777777777777777777777777777,
                              0.3048, 0.00508, 0.01524, 0.44704, 0.514444444444, -1};

// ASCII Hexadecimal to integer
// Similar to atoi() but assumes Hexadecimal (base-16)
int axtoi( const char *digits ){
  unsigned int retval,i;

  // Initialize
  i = retval = 0;

  // While we have a digit 0-9 or A-F.  Meaning, we break on the first non-hexadecimal character just like atoi.
  while( (digits[i] >= '0' && digits[i] <= '9') || (digits[i] >= 'A' && digits[i] <= 'F') ){
    // Multiply value by 16. No effect if this is our first pass
    retval = retval * 16;

    // Convert ASCII into Integer by subtracting either 0x37 from A-F or 0x30 from 0-9
    retval += (unsigned int)( (digits[i] > '9') ? digits[i] - 0x37 : digits[i] - 0x30 );

    i++;
  } // while

  return retval;
} // axtoi

/**************************************************************************
 ** APRS Class                                                           **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

APRS::APRS(const string &packet, const time_t timestamp) : _packet(packet), _timestamp(timestamp) {
  stringstream s;
  bool isOK;

  // set defaults
  _position = false;
  _telemetry = false;
  _weather = false;
  _latitude = 0.0;
  _longitude = 0.0;
  _symbolTable = '0';
  _symbolCode = '0';
  _status = "";

  s << _timestamp;

  addString("aprs.packet.timestamp", s.str());

  isOK = _parse();

  if (!isOK) {
    clearStrings();
    throw APRS_Exception(_error);
  } // if
} // APRS::APRS

APRS::~APRS() {
  // cleanup
  clearStrings();
} // APRS::~APRS

/***************
 ** Variables **
 ***************/

const bool APRS::_parse() {
  StringToken aToken;			// string tokenizer
  StringToken aprsPath;			// Tokenize the aprs path by comma
  bool isOK;
  bool tryAsBeacon = false;		// last resort try packet as beacon
  char date_format[20];			// Format a date.
  regexMatchListType regexList;
  md5wrapper md5;
  stringstream s;			// New string stream
  string aprsMessage;
  string parseMe;
  string parseSource;
  struct tm *gm;			// Time structure.
  time_t now;				// Current time.
  size_t pos;				// current position in packet
  size_t last;				// next length
  unsigned int i;			// Generic counter.
  timeval tv;

  // first strip off any third party junk
  parseMe = _packet;
  stripcrlf(parseMe);

  while(ereg("^([^:]+:[}])", parseMe, regexList) > 0)
    parseMe.erase(regexList[1].matchBegin, regexList[1].matchEnd);

  if (parseMe.length() < 1) {
    _error = "invalid packet, 0 length";
    return false;
  } // if

  addString("aprs.packet.raw", parseMe);

  // initialize variables
  aprsPath.setDelimiter(',');

  // find source SSID
  for(pos = 0; pos < parseMe.length() && parseMe[pos] != '>'; pos++);

  if (pos >= parseMe.length()) {
    _error = "invalid packet, 0 length after source parsing";
    return false;
  } // if

  parseSource = parseMe.substr(0, pos++);
  _source = parseSource;
  addString("aprs.packet.source", parseSource);
  addString("aprs.packet.source.callsign", _extractCallsign(parseSource));

  // find path
  for(last = pos; pos < parseMe.length() && parseMe[pos] != ':'; pos++);

  if (pos >= parseMe.length()) {
    _error = "invalid packet, 0 length searching path";
    return false;
  } // if

  _path = parseMe.substr(last, (pos++ - last));
  addString("aprs.packet.path", _path);

  if (pos >= parseMe.length()) {
    _error = "invalid packet, 0 length after path";
    return false;
  } // if

  _body = parseMe.substr(pos, parseMe.length());
  addString("aprs.packet.body", _body);
  _command = parseMe.substr(pos, 1)[0];
  addString("aprs.packet.command", parseMe.substr(pos++, 1));

  // There should be no path's over 7 if there are
  // skip, it's an invalid packet!
  if (aprsPath.size() > 7) {
    _error = "invalid packet, over 7 path targets";
    return false;
  } // if

  /**
   * Valid Packet
   *
   * Up till this point we've covered the only
   * errors that would require totally dumping the packet
   * and skipping sending it to the raw table.
   *
   * There should be no reason to "return 0" after this point.
   * but if there is don't forget to delete Packet or we'll
   * have a nasty memory leak.
   *
   */

  // ## PROFILING: START
  //sw.Start();

  // initialize variables
  aprsMessage = parseMe.substr(pos, parseMe.length());
  aprsPath = getString("aprs.packet.path");

  for(i = 0; i < aprsPath.size(); i++) {
    // initialize variables
    s.str("");
    s << "aprs.packet.path" << i;
    addString(s.str(), aprsPath[i]);
  } // for

  // set our iGate
  addString("aprs.packet.igate", aprsPath[i-1]);

  // let's parse out some common APRS extensions first
  _extractAltSymbol(aprsMessage);

  addString("aprs.packet", parseMe);

  // first create unique id
  gettimeofday(&tv, NULL);
  s.str("");
  s << tv.tv_sec
    << "."
    << tv.tv_usec
    << getString("aprs.packet.source");

  _id = md5.getHashFromString(s.str());
  addString("aprs.packet.md5.id", _id);

  /**
   * Date Information
   *
   * Add the current date information to the packet event
   * for later use in the SQL injection stages.
   *
   */
  now = time(NULL);
  gm = localtime(&now);
  snprintf(date_format, sizeof(date_format), "%04d", (gm->tm_year + 1900));
  addString("aprs.current.date.year", date_format);
  snprintf(date_format, sizeof(date_format), "%02d", (gm->tm_mon + 1));
  addString("aprs.current.date.month", date_format);
  snprintf(date_format, sizeof(date_format), "%02d", gm->tm_mday);
  addString("aprs.current.date.day", date_format);
  snprintf(date_format, sizeof(date_format), "%02d", gm->tm_hour);
  addString("aprs.current.date.hour", date_format);
  snprintf(date_format, sizeof(date_format), "%02d", gm->tm_min);
  addString("aprs.current.date.minute", date_format);
  snprintf(date_format, sizeof(date_format), "%02d", gm->tm_sec);
  addString("aprs.current.date.second", date_format);
  snprintf(date_format, sizeof(date_format),
           "%04d-%02d-%02d %02d:%02d:%02d", (gm->tm_year + 1900), (gm->tm_mon + 1), gm->tm_mday, gm->tm_hour,
           gm->tm_min, gm->tm_sec);
  addString("aprs.current.date", date_format);

  isOK = false;

  addString("aprs.packet.capabilities.messaging", "0");

  switch(_command) {
    // DX		dx spot
    // {{		expirimental
    case '!':			// position; no messaging
      // check to see if packet is peet logging
      if (aprsMessage[0] == '!') {
        aprsMessage.erase(0, 1);
        replaceString("aprs.packet.command", "!!");
        addString("aprs.packet.type", "peetlogging");
        _packetType = APRS_PACKET_PEETLOGGING;
        isOK = _parsePeetLogging(aprsMessage);
        break;
      } // if
    case '/':			// position; no messaging; timestamp
    case '=':			// position; messaging
    case '@':			// position; messaging; timestamp
    case ';':			// object
    case ')':			// item
      if (_command == '=' || _command == '@')
        replaceString("aprs.packet.capabilities.messaging", "1");

      _packetType = APRS_PACKET_POSITION;
      addString("aprs.packet.type", "position");
      isOK = _parsePosition(aprsMessage);
      break;
    case '`':			// MIC_E
    case '\'':			// MIC_E
      addString("aprs.packet.type.subtype", "mic_e");
      _packetType = APRS_PACKET_POSITION;
      isOK = _parseMIC_E(aprsMessage);
      break;
    case '$':			// NMEA
      addString("aprs.packet.type.subtype", "nmea");
      _packetType = APRS_PACKET_POSITION;
      isOK = _parseNmea(aprsMessage);
      break;
    case ':':			// message
      _packetType = APRS_PACKET_MESSAGE;
      addString("aprs.packet.type", "message");
      replaceString("aprs.packet.capabilities.messaging", "1");
      isOK = _parseMessage(aprsMessage);
      break;
    case 'T':			// telemetry
      if (aprsMessage[0] == '#') {
        addString("aprs.packet.type", "telemetry");
        _packetType = APRS_PACKET_TELEMETRY;
        isOK = _parseTelemetry(aprsMessage);
      } // if
      else
        tryAsBeacon = true;
      break;
    case '>':			// status report
      addString("aprs.packet.type", "status");
      _packetType = APRS_PACKET_STATUS;
      isOK = _parseStatus(aprsMessage);
      break;
    case '<':			// station capabilities
      addString("aprs.packet.type", "capabilities");
      _packetType = APRS_PACKET_CAPABILITIES;
      isOK = _parseCapabilities(aprsMessage);
      break;
    case '_':			// positionless weather report
      addString("aprs.packet.type", "weather");
      _packetType = APRS_PACKET_WEATHER;
      isOK = _parseWeather(aprsMessage);
      break;
    case '[':			// maidenhead beacon
      //isOK = false;
      //addString("aprs.packet.type", "maidenhead");
      //_packetType = APRS_PACKET_MAIDENHEAD;
      //addString("aprs.packet.error.message", "known but unsuppored; maidenhead beacon");
      tryAsBeacon = true;
      break;
    case '{':			// experimental
      if (aprsMessage[0] == '{') {
        aprsMessage.erase(0, 1);
        replaceString("aprs.packet.command", "{{");
        addString("aprs.packet.type", "experimental");
        _packetType = APRS_PACKET_EXPERIMENTAL;
        isOK = _parseExperimental(aprsMessage);
      } // if
      else
        tryAsBeacon = true;
      break;
    default:			// unknown packet
      // everything else is simply a beacon
      //cout << "packet(" << _command << ")" << endl;
      //addString("aprs.packet.error.message", "unknown packet");
      tryAsBeacon = true;
      break;
  } // switch

  if (tryAsBeacon) {
    _packetType = APRS_PACKET_BEACON;
    addString("aprs.packet.type", "beacon");
    isOK = _parseBeacon(aprsMessage);
  } // if

  // throw an exception if something failed to parse
  // aprs.packet.error.message should be set if it did
  if (!isOK) {
    assert(isString("aprs.packet.error.message"));	// assert on bug
    _error = getString("aprs.packet.error.message");
    return false;
  } // if

  return true;
} // APRS::_parse

/**********************
 ** Altitude Members **
 **********************/

const bool APRS::_extractAltitude(string &aprsMessage) {
  if (aprsMessage.length() < 9) return false;

  std::string::size_type offset = 0;
  bool ok;
  while(offset+9 <= aprsMessage.length()) {
    char ch1 = aprsMessage[offset];
    char ch2 = aprsMessage[offset+1];
    char ch3 = aprsMessage[offset+2];
    ok = (ch1 == '/'
          && (ch2 == 'a' || ch2 == 'A')
          && ch3 == '=');
    for(size_t i=offset+3; i < offset+6 && ok; i++) {
      char ch = aprsMessage[i];
      if ((ch >= '0' && ch <= '9') || ch == '-') continue;
      ok = false;
      break;
    } // for

    if (ok) {
      if (_extractAltitudeHelper(aprsMessage, offset)) return true;
    } // if
    offset++;
  } // while

  // /A=000000
  //if (ereg("(/[aA]{1}=([0-9-]{6}))", aprsMessage, regexList) < 1)
  //  return false;

  return false;
} // APRS::_extractAltitude

const bool APRS::_extractAltitudeHelper(string &aprsMessage, const std::string::size_type offset) {
  float altitude;
  char y[10];

  if (aprsMessage.length()-offset < 9) return false;

  // convert altitude to meters
  std::string alt_str = aprsMessage.substr(offset+3, 6);

  if (alt_str.find('-') != std::string::npos) {
    // can't use remove the altitude
    aprsMessage.erase(offset, 9);
    return true;
  } // if

  altitude = atof( alt_str.c_str() ) * 0.3048;
  snprintf(y, sizeof(y), "%.2f", altitude);

  // sweet, set our altitude variable and remove from aprsMessage.
  addString("aprs.packet.altitude", y);
  aprsMessage.erase(offset, 9);

  return true;
} // APRS::_extractAltitudeHelper

/*********************************
 ** Direction and Speed Members **
 *********************************/

const bool APRS::_extractDfr(string &aprsMessage) {
  if (aprsMessage.length() < 15) return false;

  std::string::size_type offset = 0;
  bool ok;
  while(offset+15 <= aprsMessage.length()) {
    char ch1 = aprsMessage[offset+3];
    char ch2 = aprsMessage[offset+7];
    char ch3 = aprsMessage[offset+11];
    ok = (ch1 == '/' && ch2 == '/' && ch3 == '/');
    if (ok) {
      if (_extractDfrHelper(aprsMessage, offset)) return true;
    } // if
    offset++;
  } // while

  //          2        3 4        5 6        7 8         9         10
  //          0        3 4        7 8        11 12      13         14
  // 000/000/000/000
  //if (ereg("(([0-9]{3})/([0-9]{3})/([0-9]{3})/([0-9]{1})([0-9]{1})([0-9]{1}))",
  //                         aprsMessage, regexList) < 1)
  //  return false;

  return false;
} // APRS::_extractDfr

const bool APRS::_extractDfrHelper(string &aprsMessage, const std::string::size_type offset) {
  double base_range;				// Base range.
  double range;					// Range.
  double speed;
  stringstream s;				// Temp string stream for converting numbers.
  char y[10];

  if (aprsMessage.length()-offset < 15) return false;

  std::string::size_type pos = offset;
  bool ok;
  char ch = aprsMessage[pos];
  // [0-9]{3}
  for(size_t i=0; i < 3; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '0' && ch <= '9');
    if (!ok) return false;
  } // for
  pos += 4; // skip / we already know it's there

  // [0-9]{3}
  for(size_t i=0; i < 3; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '0' && ch <= '9');
    if (!ok) return false;
  } // for
  pos += 4; // skip / we already know it's there

  // [0-9]{3}
  for(size_t i=0; i < 3; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '0' && ch <= '9');
    if (!ok) return false;
  } // for
  pos += 4; // skip / we already know it's there

  // [0-9]{3}
  for(size_t i=0; i < 3; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '0' && ch <= '9');
    if (!ok) return false;
  } // for

  //            2          3          4           5        6         7
  //            0        3 4        7 8        11 12      13         14
  // 000/000/000/000
  //if (ereg("(([0-9]{3})/([0-9]{3})/([0-9]{3})/([0-9]{1})([0-9]{1})([0-9]{1}))",
  //                         aprsMessage, regexList) < 1)
  //  return false;

  // convert our range value
  base_range = atoi( aprsMessage.substr(offset+12, 1).c_str() );
  speed = KnotToKmh(atof( aprsMessage.substr(offset+4, 3).c_str() ));
  range = MphToKmh(pow(2.00, base_range));
  s.str(""); s << range;

  // sweet, set our altitude variable and remove from aprsMessage.
  addString("aprs.packet.dirspd", aprsMessage.substr(offset, 7) );
  addString("aprs.packet.dirspd.direction", aprsMessage.substr(offset, 3));
  snprintf(y, sizeof(y), "%.2f", speed);
  addString("aprs.packet.dirspd.speed", y);
  addString("aprs.packet.dirspd.bearing", aprsMessage.substr(offset+8, 3));
  addString("aprs.packet.dirspd.hits", aprsMessage.substr(offset+12, 1));
  snprintf(y, sizeof(y), "%.2f", range);
  addString("aprs.packet.dirspd.range", y);
  addString("aprs.packet.dirspd.accuracy", aprsMessage.substr(offset+14, 1));
  aprsMessage.erase(offset, 15);

  return true;
} // APRS::_extractDfrHelper

const bool APRS::_extractDirSpd(string &aprsMessage) {
  double speed;
  char y[10];

  if (aprsMessage.length() < 7) return false;

  // first call internal DF Report function, if it fails
  // we can proceed.
  if (_extractDfr(aprsMessage)) return true;

  std::string buf = aprsMessage.substr(0, 7);
  if (buf == ".../..." || buf == "   /   ") {
    // sir/spd unknown, remove it
    aprsMessage.erase(0, 7);
  } // if

  std::string::size_type pos = 0;
  char ch = aprsMessage[pos];
  bool ok;
  // [0-9]{3} - dir
  for(size_t i=0; i < 3; i++) {
    ch = aprsMessage[pos+i];
    ok = ch >= '0' && ch <= '9';
    if (!ok) return false;
  } // for
  pos += 3;

  ch = aprsMessage[pos];
  if (ch != '/') return false;
  ++pos;

  // [0-9]{3} - dir
  for(size_t i=0; i < 3; i++) {
    ch = aprsMessage[pos+i];
    ok = ch >= '0' && ch <= '9';
    if (!ok) return false;
  } // for

//              2           3
//              0         3 4
//  if (ereg("^(([0-9]{3})/([0-9]{3}))", aprsMessage, regexList) < 1)
//    return false;

  std::string dir_str = aprsMessage.substr(0, 3);
  std::string speed_str = aprsMessage.substr(4, 3);

  speed = KnotToKmh(atof( speed_str.c_str() ));

  // sweet, set our altitude variable and remove from aprsMessage.
  addString("aprs.packet.dirspd", aprsMessage.substr(0, 7) );
  addString("aprs.packet.dirspd.direction", dir_str);
  snprintf(y, sizeof(y), "%.2f", speed);
  addString("aprs.packet.dirspd.speed", y);
  aprsMessage.erase(0, 7);

  return true;
} // APRS::_extractDirSpd

/*****************
 ** DFS Members **
 *****************/

const bool APRS::_extractDfs(string &aprsMessage) {
  if (aprsMessage.length() < 7) return false;

  std::string::size_type offset = 0;
  bool ok;
  while(offset+7 <= aprsMessage.length()) {
    char ch1 = aprsMessage[offset];
    char ch2 = aprsMessage[offset+1];
    char ch3 = aprsMessage[offset+2];
    ok = (ch1 == 'D' && ch2 == 'F' && ch3 == 'S');

    for(size_t i=offset+3; i < offset+4 && ok; i++) {
      char ch = aprsMessage[i];
      if (ch >= '0' && ch <= '9') continue;
      ok = false;
      break;
    } // for

    if (ok) {
      if (_extractDfsHelper(aprsMessage, offset)) return true;
    } // if
    offset++;
  } // while

//
//  if (ereg("(DFS(([0-9]{1})(.{1})([0-9]{1})([0-9]{1})))", aprsMessage, regexList) < 1)
//    return false;

  return false;
} // APRS::_extractDfs

const bool APRS::_extractDfsHelper(string &aprsMessage, const std::string::size_type offset) {
  stringstream s;			// Temp stringstream for converting int to string.
  double base_power;			// Base power.
  double base_haat;			// Base height.
  double base_gain;			// Base gain.
  double power;				// Calculated power.
  double haat;				// Calculated height.
  double gain;				// Callculated gain.
  double range;				// Calculated range.
  int directivity;

  if (aprsMessage.length()-offset < 7) return false;

//  if (ereg("(DFS(([0-9]{1})(.{1})([0-9]{1})([0-9]{1})))", aprsMessage, regexList) < 1)
//    return false;

  // intialize variables
  // haat = height-above-average-terrain
  base_power = atoi( aprsMessage.substr(offset+3, 1).c_str() );
  base_haat = ( aprsMessage[offset+4] - 48);
  base_gain = atoi( aprsMessage.substr(offset+5, 1).c_str());

  power = pow(base_power, 2.00);
  haat = (10 * pow(2.00, base_haat));
  gain = pow(10.00, (base_gain / 10));
  range = MphToKmh(sqrt((2 * haat * sqrt((power / 10) * (gain / 2)))));

  haat = FtToM((10 * pow(2.00, base_haat)));

  directivity = dirs[atoi( aprsMessage.substr(offset+6, 1).c_str() )];

  // sweet, set our altitude variable and remove from aprsMessage.
  addString("aprs.packet.dfs", aprsMessage.substr(3, 4) );
  s.str(""); s << power;
  addString("aprs.packet.dfs.strength", s.str());
  s.str(""); s << haat;
  addString("aprs.packet.dfs.haat", s.str());
  s.str(""); s << gain;
  addString("aprs.packet.dfs.gain", s.str());
  s.str(""); s << range;
  addString("aprs.packet.dfs.range", s.str());
  s.str(""); s << directivity;
  addString("aprs.packet.dfs.directivity", s.str());
  aprsMessage.erase(offset, 7);

  return true;
} // APRS::_extractDfsHelper

/************************
 ** ARESOBJECT Members **
 ************************/

const bool APRS::_extractAreaObject(string &aprsMessage) {
  regexMatchListType regexList;         // Map of regex matches.
  char y[10];
  int color;
  int yy, xx;

  char *objectColors[] = { (char *) "black", (char *) "blue", (char *) "green", (char *) "cyan",
                           (char *) "red", (char *) "violet", (char *) "yellow", (char *) "grey",
                           NULL
                         };

  // only parse this for objects
  if (!isString("aprs.packet.object.name"))
    return false;

  if (aprsMessage.length() < 1)
    return false;

  // area objects are only symbol table \ and code f
  if (symbolTable() != '\\' || symbolCode() != 'l')
    return false;

  if (ereg("^([0-9])([0-9]{2})([1\x2f][0-9])([0-9]{2})", aprsMessage, regexList) < 1)
    return false;

  if (regexList[3].matchString[0] == '/')
    color = atoi(regexList[3].matchString.substr(1,1).c_str());
  else
    color = atoi(regexList[3].matchString.c_str());

  // valid color?
  if (color > 15)
    return false;

  if (color > 7) {
    addString("aprs.packet.object.area.color.intensity", "low");
    color -= 8;
  } // if
  else
    addString("aprs.packet.object.area.color.intensity", "high");

  yy = atoi(regexList[2].matchString.c_str());
  xx = atoi(regexList[4].matchString.c_str());

  snprintf(y, sizeof(y), "%.6f", pow(yy, 2.00));
  addString("aprs.packet.object.area.offset.latitude", y);
  snprintf(y, sizeof(y), "%.6f", pow(xx, 2.00));
  addString("aprs.packet.object.area.offset.longitude", y);

  addString("aprs.packet.object.area.color", objectColors[color]);
  addString("aprs.packet.object.area.type", regexList[1].matchString);

  aprsMessage.erase(regexList[0].matchBegin, regexList[0].matchLength);

  if ((regexList[1].matchString[0] == '1' || regexList[1].matchString[0] == '6')
      && ereg("^[\x7b]([0-9]{1,3})[\x7d]",
                               aprsMessage, regexList) > 0) {

    addString("aprs.packet.object.area.corridor", regexList[1].matchString);
    aprsMessage.erase(regexList[0].matchBegin, regexList[0].matchLength);
  } // if

  return true;
} // APRS::_extractAreaObject

/******************
 ** AFRS Members **
 ******************/

const bool APRS::_extractAfrs(string &aprsMessage) {
  regexMatchListType regexList;         // Map of regex matches.
  regexMatchListType rl2; 	        // Map of regex matches.
  char type, unit1, unit2, y[10];
  double range, rangeE, rangeW;
  stringstream s;			// Temp stringstream for converting int to string.
  string frequency;
  string rest, tone;
  int i;

  if (aprsMessage.length() < 1)
    return false;

  if (ereg("([0-9]{3,4}[.][0-9]{2}[0-9 ][Mm][Hh][Zz])(.*)", aprsMessage, regexList) < 1)
    return false;

  //cout << "[" << aprsMessage << "]" << endl;

  frequency = regexList[1].matchString;
  rest = regexList[2].matchString;

  addString("aprs.packet.afrs.frequency", frequency);

  //cout << "ARGS: " << rest << endl;

  // intialize variables
  rangeE = rangeW = range = 0.00;

  // Attempt to parse any optional 10-byte sets
  // ...try and find the first 5 bytes
  if (ereg("^([ ]([TtDCE])(([0-9]{2})([0-9mk])))(.*)", rest, regexList) > 0
      || ereg("^([ ]([TtDCE])(off))(.*)", rest, regexList) > 0
      || ereg("^([ ](1750))(.*)", rest, regexList) > 0) {

    //for(int i=0; i < regexList.size(); i++)
    //  cout << i << ":F(" << regexList[i].matchString << ") " << rest << endl;

    type = regexList[2].matchString[0];

    rest = "";
    switch(regexList.size()) {
      // matched normal format
      case 7:
        tone  = regexList[3].matchString;
        unit1 = regexList[5].matchString[0];
        rest = regexList[6].matchString;
        break;
      // matched Toff format
      case 5:
        rest = regexList[4].matchString;
        tone  = regexList[3].matchString;
        break;
      // matched 1750 tone burst format
      case 4:
        tone  = regexList[2].matchString;
        rest = regexList[3].matchString;
        break;
    } // switch

    // only snag range if it's there
    if (rest.length() > 0
        && ereg("^([ ]([RW])([0-9]{2})([mk]))", rest, rl2) > 0) {

      //for(int i=0; i < rl2.size(); i++)
      //  cout << i << ":S(" << rl2[i].matchString << ") " << rest << endl;

      unit2 = rl2[4].matchString[0];
      range = atoi(rl2[3].matchString.c_str());

      if (unit2 == 'm')
        range = MphToKmh(range);

      snprintf(y, sizeof(y), "%.2f", range);
      addString("aprs.packet.afrs.range", y);

      // handle east/west range
      // Exxm Wxxm
      if (type == 'E') {
        rangeE = atoi(regexList[4].matchString.c_str());

        // convert ranges if needed
        if (unit1 == 'm')
          rangeE = MphToKmh(rangeE);

        snprintf(y, sizeof(y), "%.2f", rangeE);
        addString("aprs.packet.afrs.range.east", y);
        snprintf(y, sizeof(y), "%.2f", range);
        addString("aprs.packet.afrs.range.west", y);
      } // if

      addString("aprs.packet.afrs.optional.extended", rl2[1].matchString);
    } // if

    // handle tone or
    // Txxx RXXm
    // Cxxx RXXm
    // Dxxx Rxxk
    // txxx Rxxk
    if (type != 'E') {
      // try ad figure out the actual tone frequency
      if (type != 't') {
        for(i=0; tone_table[i] != NULL; i++) {
          if (atoi(tone_table[i]) == atoi(tone.c_str()))
            tone = tone_table[i];
        } // for
      } // if

      addString("aprs.packet.afrs.tone", tone);

      switch(type) {
        case 'T':
          addString("aprs.packet.afrs.type", "pl");
          break;
        case 'C':
          addString("aprs.packet.afrs.type", "ctcss");
          break;
        case 'D':
          addString("aprs.packet.afrs.type", "dcs");
          break;
        case 't':
          addString("aprs.packet.afrs.type", "narrow");
          break;
        default:
          addString("aprs.packet.afrs.type", "burst");
          break;
      } // switch
    } // else if

    addString("aprs.packet.afrs.optional", regexList[1].matchString);
  } // if
  // Attempt to parse any optional 10-byte sets
  else if (ereg("^([ ]([0-9]{3,4}[.][0-9]{2}[0-9 ])(rx|[ ][+]))", rest, regexList) > 0) {

    if (regexList[3].matchString == "rx")
      addString("aprs.packet.afrs.frequency.receive", trim(regexList[2].matchString));
    else
      addString("aprs.packet.afrs.frequency.alternate", trim(regexList[2].matchString));

    addString("aprs.packet.afrs.optional", regexList[1].matchString);
    aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);
  } // else if

  addString("aprs.packet.afrs.post", aprsMessage);

  return true;
} // APRS::_extractAfrs

/**********************
 ** SIGNPOST Members **
 **********************/

const bool APRS::_extractSignpost(string &aprsMessage) {
  regexMatchListType regexList;         // Map of regex matches.

  if (aprsMessage.length() < 1)
    return false;

  if (ereg("([\x7b]([0-9]{1,3})[\x7d])", aprsMessage, regexList) < 1)
    return false;

  //cout << "[" << aprsMessage << "]" << endl;

  addString("aprs.packet.symbol.signpost", regexList[2].matchString);
  aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);

  return true;
} // APRS::_extractSignpost

const string APRS::_extractCallsign(const string &aprsCallsign) {
  regexMatchListType regexList;         // Map of regex matches.
  string call;

  if (ereg("^([A-Z0-9]{3,7})([-][0-9]{1,2})$", aprsCallsign, regexList) > 0) {
    call = regexList[1].matchString;
  } // if
  else
    call = aprsCallsign;

  return call;
} // APRS::_extractCallsign

/**********************
 ** SAFE Members     **
 **********************/

const string APRS::_makeBufferSafe(const string &parseMe) {
  StringTool st;
  string safeBuffer;
  stringstream s;
  unsigned int i;

  // make the buffer safe
  for(i = 0; i < parseMe.length(); i++) {
    if ((int) parseMe[i] < 32 ||
        (int) parseMe[i] > 126)
      s << "\\x" << st.char2hex(parseMe[i]);
    else
      s << parseMe[i];
  } // for

  safeBuffer = s.str();

  return safeBuffer;
} // APRS::_makeBufferSafe

const string APRS::_cleanupComment(const string &cleanMe) {
  StringTool st;
  char ch;
  string comment;
  string ret;
  stringstream s;
  unsigned int i;

  comment = trim(cleanMe);

  // make the buffer safe
  for(i = 0; i < comment.length(); i++) {
    ch = comment[i];
    if ((ch >= '\x20' && ch <= '\x7e') || (ch >= '\x80' && ch <= '\xfe'))
      s << ch;
  } // for

  ret = s.str();

  return ret;
} // APRS::_cleanupComment

/**********************
 ** Location Members **
 **********************/

const bool APRS::_extractLocation(string &aprsMessage) {
  stringstream s;			// Generic string stream.
  float latitude;			// Latitude
  float longitude;			// Longitude
  float latoff, longoff;		// DAO offsets
  int ambiguity=0;
  map<string, string> lng;
  map<string, string> lat;
  map<string, float> deg;
  map<string, float> min;

  /**
   * Parse Extensions
   *
   * I don't believe these extensions are used in any other messages
   * but I could be wrong.
   *
   */
  _extractAltitude(aprsMessage);
  _extractDfs(aprsMessage);
  _extractRng(aprsMessage);
  _extractPhg(aprsMessage);
  _extractDao(aprsMessage, latoff, longoff);
  _extractAfrs(aprsMessage);

  if (_extractLocationCompressed(aprsMessage) == true)
    return true;

  _extractTimestamp(aprsMessage);

  if (aprsMessage.length() < 19) {
    addString("aprs.packet.error.message", "position: invalid total length (too short < 19)");
    return false;
  } // if

  // validate
  std::string::size_type pos = 0;
  // [0-9 .]{7}
  char ch = aprsMessage[pos];
  bool ok;
  for(size_t i=0; i < 6; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '0' && ch <= '9')
         || ch == ' '
         || ch == '.';
    if (!ok) {
      addString("aprs.packet.error.message", "position: invalid character in lat");
      return false;
    } // if
  } // for

  pos += 7;
  // [NnSs]{1}
  char lat_dir = ch = aprsMessage[pos];
  ok = (ch == 'N' || ch == 'n' || ch == 'S' || ch == 's');
  if (!ok) {
    addString("aprs.packet.error.message", "position: invalid lat direction");
    return false;
  } // if

  ++pos;
  ++pos; // symbol code
  for(size_t i=0; i < 8; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '0' && ch <= '9')
         || ch == ' '
         || ch == '.';
    if (!ok) {
      addString("aprs.packet.error.message", "position: invalid character in longitude");
      return false;
    } // if
  } // for

  pos += 8;
  char lng_dir = ch = aprsMessage[pos];
  ok = (ch == 'E' || ch == 'e' || ch == 'W' || ch == 'w');
  if (!ok) {
    addString("aprs.packet.error.message", "position: invalid lng direction");
    return false;
  } // if

//               2           3          4     5           6          7
//               0           7          8     9           17         18
//  if (ereg("^(([0-9 .]{7})([NnSs]{1})(.{1})([0-9 .]{8})([EeWw]{1})(.{1}))", aprsMessage, regexList) > 0)
//    offset = -2;
//  else {
//    addString("aprs.packet.error.message", "invalid uncompressed location");
//    return false;
//  } // else

  lat["match"] = aprsMessage.substr(0, 7);
  lat["degrees"] = lat["match"].substr(0, 2);
  lat["minutes"] = lat["match"].substr(2, 5);

  lng["match"] = aprsMessage.substr(9, 8);
  lng["degrees"] = lng["match"].substr(0, 3);
  lng["minutes"] = lng["match"].substr(3, 5);

  deg["lat"] = atoi(lat["degrees"].c_str());
  deg["lng"] = atoi(lng["degrees"].c_str());

  // count the number of spaces at the end if any.
  for(pos = lat["match"].length(); pos != 0; pos--) {
    if (lat["match"][pos] == ' ')
      ambiguity++;
  } // for

  if (ambiguity > 4) {
    addString("aprs.packet.error.message", "position: invalid ambiguity");
    return false;
  } // if

  switch(ambiguity) {
    case 4:
      // minutes aren't used add 0.5 to degree value
      latitude = deg["lat"] + 0.5;
      longitude = deg["long"] + 0.5;

      break;
    case 3:
      min["lat"] = atof(lat["minutes"].substr(0, 1).c_str()) * 10 + 5;
      min["lng"] = atof(lng["minutes"].substr(0, 1).c_str()) * 10 + 5;

      latitude = deg["lat"] + (min["lat"] / 60.000000);
      longitude = deg["lng"] + (min["lng"] / 60.000000);
      break;
    case 2:
      min["lat"] = atof(lat["minutes"].substr(0, 2).c_str());
      min["lng"] = atof(lng["minutes"].substr(0, 2).c_str());

      latitude = deg["lat"] + ((min["lat"] + 0.5) / 60.000000);
      longitude = deg["lng"] + ((min["lng"] + 0.5) / 60.000000);
      break;
    case 1:
      min["lat"] = atof(lat["minutes"].substr(0, 4).c_str());
      min["lng"] = atof(lng["minutes"].substr(0, 4).c_str());

      latitude = deg["lat"] + ((min["lat"] + 0.5) / 60.000000);
      longitude = deg["lng"] + ((min["lng"] + 0.5) / 60.000000);

      break;
   /**
     * No ambiguity
     *
     */
    case 0:
      min["lat"] = atof(lat["minutes"].c_str());
      min["lng"] = atof(lng["minutes"].c_str());

      latitude = deg["lat"] + (min["lat"] / 60.000000);
      longitude = deg["lng"] + (min["lng"] / 60.000000);

      break;
  } // switch

  s.str("");
  s << ambiguity;
  addString("aprs.packet.position.ambiguity", s.str());
  addString("aprs.packet.position.raw.latitude", lat["match"]);
  addString("aprs.packet.position.raw.longitude", lng["match"]);

  // initialize variables
  s.str("");
  s << latitude;
  addString("aprs.packet.position.latitude", s.str());

  s.str("");
  s << longitude;
  addString("aprs.packet.position.longitude", s.str());

  if (lat_dir == 'S')
    latitude = latitude * -1;

  if (lng_dir == 'W')
    longitude = longitude * -1;

  s.str(""); s << latitude;
  addString("aprs.packet.position.latitude.nodao.decimal", s.str());
  s.str(""); s << longitude;
  addString("aprs.packet.position.longitude.nodao.decimal", s.str());

  /**
   * DAO Support
   *
   * Add DAO offsets if they exist.
   *
   */
  if (isString("aprs.packet.dao") == true) {
    if (latitude < 0)
      latitude -= latoff;
    else
      latitude += latoff;

    if (longitude < 0)
      longitude -= longoff;
    else
      longitude += longoff;
  } // if

  _latlng(latitude, longitude);

  addString("aprs.packet.position.latitude.direction", aprsMessage.substr(7, 1));
  addString("aprs.packet.position.longitude.direction", aprsMessage.substr(17, 1));
  _symbol(aprsMessage.substr(8, 1), aprsMessage.substr(18, 1));
  addString("aprs.packet.symbol.type", "location");

  // remove latitude/longitude message and update packet message
//  aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);
  aprsMessage.erase(0, 19);

  _extractAreaObject(aprsMessage);
  // Should be relatively last, relies on the having
  // the course be the first item in string.  Kinda of
  // ghetto but I bet it works.
  _extractDirSpd(aprsMessage);

  addString("aprs.packet.position.type", "uncompressed");

  return true;
} // APRS::_extractLocation

const bool APRS::_extractTimestamp(string &aprsMessage) {
  // 000000z or 000000/ timestamp
  if (aprsMessage.length() < 7) return false;

  for(string::size_type pos=0; pos < 7; pos++) {
    char ch = aprsMessage[pos];
    if (pos < 6) {
      if (ch < '0' || ch > '9') return false;
    } // if
    if (pos == 6) {
      if (ch != 'z' && ch != '/' && ch != 'h') return false;
    } // if
  } // for

  std::string timestamp = aprsMessage.substr(0, 6);
  addString("aprs.packet.timestamp.decoded", timestamp);
  aprsMessage.erase(0, 7);

  return true;
} // APRS::_extractTimestamp

const bool APRS::_extractWeatherTimestamp(string &aprsMessage) {
  // 0000000 timestamp
  if (aprsMessage.length() < 8) return false;

  for(string::size_type pos=0; pos < 8; pos++) {
    char ch = aprsMessage[pos];
    if (ch < '0' || ch > '9') return false;
  } // for

  std::string timestamp = aprsMessage.substr(0, 8);
  addString("aprs.packet.weather.timestamp.decoded", timestamp);
  aprsMessage.erase(0, 8);

  return true;
} // APRS::_extractWeatherTimestamp

const bool APRS::_extractLocationCompressed(string &aprsMessage) {
  string tempBuf;			// Temporary buffer.
  stringstream s;			// Generic string stream.
  float latitude = 0.0;			// Latitude
  float longitude = 0.0;		// Longitude
  float latoff, longoff;		// lat/long offsets for DAO
  float x1, x2, x3, x4;			// Int's for decoding longitude.
  float y1, y2, y3, y4;			// Int's for decoding latitude.
  double cs, c, spd, course;		// Course and c/s for calculations.
  double speed, range, altitude;	// Speed, range and altitude for calculations.
  char y[10];

  // 000000z or 000000/ timestamp
  _extractTimestamp(aprsMessage);

////////
// This regex is inefficient
//  if (ereg("^(([0-9]{6}(z|/|h))([a-jA-Z\\/]{1})([\x21-\x7b]{4})([\x21-\x7b]{4})([\x21-\x7b\x7d]{1})([\x20-\x7b]{3}))",
//                           aprsMessage, regexList)) {
//    // remove nasty timestamp, it's useless anyways.
//    aprsMessage.erase(regexList[2].matchBegin, regexList[2].matchLength);
//  } // if

  if (aprsMessage.length() < 13) return false;

  string::size_type pos = 0;
  char ch = aprsMessage[pos];
  // [a-jA-Z\\/]{1}
  bool ok = (ch >= 'a' && ch <= 'j')
            || (ch >= 'A' && ch <= 'J')
            || ch == '\\'
            || ch == '/';
  if (!ok) return false;

  ++pos;
  // [\x21-\x7b]{4}[\x21-\x7b]{4}
  for(size_t i=0; i < 8; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '\x21' && ch <= '\x7b');
    if (!ok) return false;
  } // for

  pos += 8;
  // [\x21-\x7b\x7d]{1}
  ch = aprsMessage[pos];
  ok = (ch >= '\x21' && ch <= '\x7b')
       || ch == '\x7d';
  if (!ok) return false;

  ++pos;
  for(size_t i=0; i < 3; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '\x20' && ch <= '\x7b');
    if (!ok) return false;
  } // for


//  if (ereg("^(([a-jA-Z\\/]{1})([\x21-\x7b]{4})([\x21-\x7b]{4})([\x21-\x7b\x7d]{1})([\x20-\x7b]{3}))",
//                           aprsMessage, regexList) < 1) {
//    return false;
//  } // if

  // calculate course/speed or range
  c = (double) (aprsMessage[10] - 33);
  spd = (double) (aprsMessage[11] - 33);

  // Course/Speed or Pre-Calculated Radio Range
  if (c >= 0 && c <= 89) {
    course = c * 4;
    speed = KnotToKmh(pow(1.08, spd) - 1);
    s.str(""); s << course;
    // addString("aprs.message.compressed.course", s.str());
    addString("aprs.packet.dirspd.direction", s.str());

    snprintf(y, sizeof(y), "%.2f", speed);
    // addString("aprs.message.compressed.speed", y);
    addString("aprs.packet.dirspd.speed", y);
  } // if
  else if (aprsMessage[1] == '{') {
    range = 2 * pow(1.08, spd);
    s.str(""); s << range;
    addString("aprs.message.compressed.range", s.str());
  } // else
  else if ((aprsMessage[3] & 24) == 16) {
    cs = c * 91 + spd;
    altitude = pow(1.002, cs) * 0.3048;
    s.str(""); s << altitude;
    addString("aprs.message.compressed.altitude", s.str());
    addString("aprs.packet.altitude", s.str());
  } // else if

  // initialize variables
  y1 = aprsMessage[1] - 33;
  y2 = aprsMessage[2] - 33;
  y3 = aprsMessage[3] - 33;
  y4 = aprsMessage[4] - 33;

  x1 = aprsMessage[5] - 33;
  x2 = aprsMessage[6] - 33;
  x3 = aprsMessage[7] - 33;
  x4 = aprsMessage[8] - 33;

  latitude = 90 - ((y1 * pow(91,3) + y2 * pow(91,2) + y3 * 91 + y4) / 380926);
  longitude = -180 + ((x1 * pow(91,3) + x2 * pow(91,2) + x3 * 91 + x4) / 190463);


  // snag any DAO precision
  _extractDao(aprsMessage, latoff, longoff);
  if (isString("aprs.packet.dao") == true) {
    if (latitude < 0)
      latitude -= latoff;
    else
      latitude += latoff;

    if (longitude < 0)
      longitude -= longoff;
    else
      longitude += longoff;
  } // if

  _latlng(latitude, longitude);

  s.str("");
  if (aprsMessage[0] >= 97 && aprsMessage[0] <= 106)
    s << (int) (aprsMessage[0] - 49);
  else
    s << aprsMessage[0];

  std::string symbol;
  symbol.assign(1, aprsMessage[9]);
  _symbol(s.str(), symbol);
  addString("aprs.packet.symbol.type", "location_compressed");
  addString("aprs.packet.position.type", "compressed");

  // remove latitude/longitude message and update packet message
//  aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);
  aprsMessage.erase(0, 13);

  // Should be relatively last, relies on the having
  // the course be the first item in string.  Kinda of
  // ghetto but I bet it works.
  _extractDirSpd(aprsMessage);

  return true;
} // APRS::_extractLocationCompressed

const bool APRS::_parsePosition(const string &parseMe) {
  string aprsMessage = parseMe;
  stringstream s;			// Generic string stream.
  string object;			// Object name
  regexMatchListType regexList;		// List of regex matches returned.

  // let's see if this is an object or an item.
  if (_command == ';') {
    // minimum length for an object is 31 characters
    if (aprsMessage.length() < 31) {
      addString("aprs.packet.error.message", "object: length < 31");
      return false;
    } // if

    if (ereg("^(([\x20-\x7e]{9})([*]|_))", aprsMessage, regexList) > 0) {
      object = trim(regexList[2].matchString);

      // don't allow objects that have all spaces for a name.
      if (object.length() < 1) {
        addString("aprs.packet.error.message", "invalid object name (all spaces)");
        return false;
      } // if

      // Parse signpost data
      if (symbolTable() == '\\' && symbolCode() == 'm')
        _extractSignpost(aprsMessage);

      addString("aprs.packet.object.name", object.c_str());
      addString("aprs.packet.object.action", regexList[3].matchString.c_str());
      aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);
      addString("aprs.packet.object.info", _makeBufferSafe(aprsMessage));
      addString("aprs.packet.object.type", "O");
    } // if
    // detect items
    else if (ereg("^(([\x20\x22-\x5e\x60-\x7e]{3,9})([!]|_))", aprsMessage, regexList) > 0) {
      object = regexList[2].matchString;

      // don't allow objects that have all spaces for a name.
      if (object.length() < 1) {
        addString("aprs.packet.error.message", "invalid object name (all spaces)");
        return false;
      } // if

      addString("aprs.packet.object.name", object.c_str());
      addString("aprs.packet.object.action", regexList[3].matchString.c_str());
      aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);
      addString("aprs.packet.object.info", _makeBufferSafe(aprsMessage));
      addString("aprs.packet.object.type", "I");
    } // if
    else {
      addString("aprs.packet.error.message", "object/item: invalid");
      return false;
    } // else
  } // if
  else
    addString("aprs.packet.object.type", "P");

  if (!_extractItem(aprsMessage) && isString("aprs.packet.error.message"))
    // only give up if an error message was added
    return false;

  // snag our location since this should have one
  if (!_extractLocation(aprsMessage))
    return false;

  // weather station maybe?
  if (symbolCode() == '_') {
    /**
     * Weather Report
     *
     * Since this is a  weather report form an actual station
     * Unless otherwise parsed out in WeatherPOS the direction
     * and speed are taken from DirSpd.
     */
    addString("aprs.packet.weather.wind.speed", getString("aprs.packet.dirspd.speed"));
    addString("aprs.packet.weather.wind.direction", getString("aprs.packet.dirspd.direction"));

    removeString("aprs.packet.dirspd.speed");
    removeString("aprs.packet.dirspd.direction");

    _parseWeather(aprsMessage);
  } // if

  status(_cleanupComment(aprsMessage));

  /**
   * Invalid Coordinates
   *
   * Don't allow invalid coordinates.
   *
   */
  if (!isValidLatLong(_latitude, _longitude)) {
    addString("aprs.packet.error.message", "position: invalid longitude/latitude");
    return false;
  } // if

  return true;
} // APRS::_parsePosition

const bool APRS::_parseStatus(const string &parseMe) {
  string aprsMessage = parseMe;
  stringstream s;			// Generic string stream.
  regexMatchListType regexList;         // Map of regex matches.
  regexMatchListType rl;         // Map of regex matches.

  _extractTimestamp(aprsMessage);

  status(_cleanupComment(aprsMessage));

  return true;
} // APRS::_parseStatus

const bool APRS::_parsePeetLogging(const string &parseMe) {
  string aprsMessage = parseMe;

  status(_cleanupComment(aprsMessage));

  return true;
} // APRS::_parsePeetLogging

const bool APRS::_parseExperimental(const string &parseMe) {
  string aprsMessage = parseMe;

  status(_cleanupComment(aprsMessage));

  return true;
} // APRS::_parseExperimental

const bool APRS::_parseBeacon(const string &parseMe) {
  string aprsMessage = parseMe;

  status(_cleanupComment(aprsMessage));

  return true;
} // APRS::_parseBeacon

// AD7AJ-11>APU25N,TCPIP*,qAC,T2SJC:<IGATE,MSG_CNT=6,LOC_CNT=1
const bool APRS::_parseCapabilities(const string &parseMe) {
  StringToken st;			// string token
  string aprsMessage = parseMe;
  stringstream s;			// Generic string stream.
  regexMatchListType regexList;         // Map of regex matches.
  regexMatchListType rl;         // Map of regex matches.
  unsigned int i;
  unsigned int tokens = 0;

  trim(aprsMessage);

  if (aprsMessage.length() < 0) {
    addString("aprs.packet.error.message", "invalid capbilities: length is 0 after trim");
    return false;
  } // if

  st.setDelimiter(',');
  st = aprsMessage;

  if (st.size() < 0) {
    addString("aprs.packet.error.message", "invalid capbilities: no tokens to store");
    return false;
  } // if

  for(i=0; i < st.size(); i++) {
    // name=value
    if (ereg("^\\s*([^=]+)\\s*=\\s*(.*)\\s*$", st[i], rl) > 0) {
      addString("aprs.packet.capabilities." + rl[1].matchString, rl[2].matchString);
      tokens++;
    } // if
    // just name
    else if (ereg("^\\s*([^=]+)\\s*$", st[i], rl) > 0) {
      addString("aprs.packet.capabilities." + rl[1].matchString, "undef");
      tokens++;
    } // else if
  } // for

  status(_cleanupComment(aprsMessage));

  if (!tokens) {
    addString("aprs.packet.error.message", "invalid capbilities: no tokens could be parsed");
    return false;
  } // if

  return true;
} // APRS::_parseCapabilities

/*********************
 ** Message Members **
 *********************/

const bool APRS::_parseMessage(const string &parseMe) {
  string aprsMessage = parseMe;
  stringstream s;			// Generic string stream.
  regexMatchListType regexList;         // Map of regex matches.
  regexMatchListType rl;         // Map of regex matches.

  _parseMessageTelemetry(aprsMessage);

  if (ereg("^(([A-Z0-9\\-]{1,9})[ ]*:(.+))(\\{(.{1,5}))$", aprsMessage, regexList) > 0) {
    if (ereg("^([0-9A-Z]{2})[}]([0-9A-Z]{2}|)$", regexList[5].matchString, rl) > 0) {
      addString("aprs.packet.message.id", rl[1].matchString);

      if (rl[2].matchString.length() > 0)
        addString("aprs.packet.message.ack", rl[2].matchString);
    } // if
    else
      addString("aprs.packet.message.id", regexList[5].matchString);

    addString("aprs.packet.message.id.reply", regexList[5].matchString);
    aprsMessage.erase(regexList[4].matchBegin, regexList[4].matchLength);
  } // if
  else if (ereg("^(([A-Z0-9\\-]{1,9})[ ]*:(.+))",
                           aprsMessage, regexList) > 0);
  else {
    addString("aprs.packet.error.message", "unable to parse message: invalid ack id");
    return false;
  } // else

  if (ereg("^(.*)ack([a-zA-Z0-9}]{1,5})[ ]*$", regexList[3].matchString, rl)) {
    if (rl[2].matchString.length() > 2
        && rl[2].matchString[2] == '}')
      addString("aprs.packet.message.ack", rl[2].matchString.substr(0, 2));
    else
      addString("aprs.packet.message.ack", rl[2].matchString);

    if (rl[1].matchString.length() < 1)
      addString("aprs.packet.message.ackonly", "yes");

    addString("aprs.packet.message.ack.raw", rl[2].matchString);
    addString("aprs.packet.message.text.noack", rl[1].matchString);
  } // if

  addString("aprs.packet.message.target", regexList[2].matchString);
  addString("aprs.packet.message.text", _cleanupComment(regexList[3].matchString));

  // remove latitude/longitude message and update packet message
  aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);

  status(_cleanupComment(aprsMessage));

  return true;
} // APRS::_parseMessage

/*******************
 ** MIC_E Members **
 *******************/
const bool APRS::_parseMIC_E(const string &parseMe) {
  string aprsMessage = parseMe;
  stringstream s;                       // Generic string stream.

  if (!_convertMIC_EToDecimal(aprsMessage))
    return false;

  addString("aprs.packet.object.type", "P");

  /**
   * Invalid Coordinates
   *
   * Don't allow invalid coordinates.
   *
   */
  if (!isValidLatLong(_latitude, _longitude)) {
    addString("aprs.packet.error.message", "position: invalid longitude/latitude");
    return false;
  } // if

  return true;
} // APRS::_parseMIC_E

const bool APRS::_convertMIC_EToDecimal(string &aprsMessage) {
  string destination;			// Destination field.
  string destcall;
  string information;			// Information field.
  string command;			// String of command used.
  string mbits;				// mbits string
  string longtmp;
  string longminutes;
  string rest, resttmp;
  stringstream s;			// Generic string stream.
  regexMatchListType regexList;         // Map of regex matches.
  map<string, float> deg;
  map<string, float> min;
  unsigned int i;			// Generic counter
  int course, coursespeed;
  int amount;				// Amount of position abiguity
  int first, second, third;
  double latitude, longitude, speed,
         coursespeedtmp,
         altitude;
  float latoff, longoff;
  char longoffsetchar;
  char dirchar;
  char y[10];

  command = getString("aprs.packet.command");
  destcall = destination = getString("aprs.packet.path0");
  information = aprsMessage;

  addString("aprs.packet.mic_e.raw.destination", destination);
  addString("aprs.packet.mic_e.raw.information", information);

  // get destination address only, no tactical ssid
  if (ereg("^(([a-zA-Z0-9]+)[-][0-9]+)$", destination, regexList))
    destination = regexList[2].matchString;

  if (aprsMessage.length() < 8 || destination.length() != 6) {
    addString("aprs.packet.error.message", "mice: packet or destination too short");
    addString("aprs.packet.mic_e.error.message", "mice: packet or destination too short");
    return false;
  } // if

  if (ereg("^([0-9A-LP-Z]{3}[0-9LP-Z]{3})$", destination, regexList) < 1) {
    addString("aprs.packet.error.message", "mice: invalid packet");
    addString("aprs.packet.mic_e.error.message", "mice: invalid packet");
    return false;
  } // if

  if (ereg("^([\x26-\x7f][\x26-\x61][\x1c-\x7f]{2}[\x1c-\x7d][\x1c-\x7f][\x21-\x7b\x7d][A-Z0-9\x2f\x5c])",
                           information, regexList) < 1) {

    if (ereg("^([\x26-\x7f][\x26-\x61][\x1c-\x7f]{2})[\x20]([\x21-\x7b\x7d][A-Z0-9\x2f\x5c])(.*)",
                             information, regexList) < 1) {
      addString("aprs.packet.error.message", "mice: invalid info");
      addString("aprs.packet.mic_e.error.message", "mice: invalid info");
      return false;
    } // if

    information = regexList[1].matchString + string("  ") + regexList[2].matchString
                  + regexList[3].matchString;
    addString("aprs.packet.mic_e.fixed", "yes");
  } // if

  // first parse destination
  // swap out chars
  for(i=0; i < destination.length(); i++) {
    if (destination[i] >= 'A' && destination[i] <= 'J')
      destination[i] = destination[i] - 17;
    else if (destination[i] >= 'P' && destination[i] <= 'Y')
      destination[i] = destination[i] - 32;
    else if (destination[i] == 'K' || destination[i] == 'L' || destination[i] == 'Z')
      destination[i] = ' ';
  } // for

  addString("aprs.packet.mic_e.raw.latitude", destination);

  // check for position ambiguity
  if (ereg("^([0-9]+)([ ]*)$", destination, regexList)) {
    amount = 0;
    if (regexList.size() > 1)
      amount = 6 - regexList[1].matchString.length();

    if (amount > 4) {
      addString("aprs.packet.error.message", "mice: invalid ambiguity, too large");
      addString("aprs.packet.mic_e.error.message", "mice: invalid ambiguity, too large");
      return false;
    } // if

    s.str("");
    s << amount;
    addString("aprs.packet.mic_e.position.ambiguity", s.str());
  } // if
  else {
    addString("aprs.packet.error.message", "mice: invalid ambiguity");
    addString("aprs.packet.mic_e.error.message", "mice: invalid ambiguity");
    return false;
  } // else

  if (amount == 4)
    destination[destination.length() - amount] = '3';
  else {
    destination[destination.length() - amount] = '5';
  } // else

  for(i=0; i < destination.length(); i++) {
      if (destination[i] == ' ')
        destination[i] = '0';
  } // for

  latitude = atof(destination.substr(0, 2).c_str());
  min["lat"] = atof((destination.substr(2, 2) + string(".") + destination.substr(4, 2)).c_str());
  latitude += (min["lat"] / 60);

  if (destcall[3] <= 0x4c)
    latitude = latitude * -1;

  // snag the mbits
  mbits = destcall.substr(0, 3);
  for(i=0; i < mbits.length(); i++) {
    if (mbits[i] >= '0' && mbits[i] <= '9')
      mbits[i] = '0';
    else if (mbits[i] == 'L')
      mbits[i] = '0';
    else if (mbits[i] >= 'P' && mbits[i] <= 'Z')
      mbits[i] = '1';
    else if (mbits[i] >= 'A' && mbits[i] <= 'K')
      mbits[i] = '2';
  } // for

  // pull out the begining of the longitude, should
  // be the first 3 bytes after type indicator
  longitude = information[0] - 28;
  longoffsetchar = destcall[4];
  if (longoffsetchar >= 0x50)
    longitude += 100;

  if (longitude >= 180 && longitude <= 189)
    longitude -= 80;
  else if (longitude >= 190 && longitude <= 199)
    longitude -= 190;

  // grab longitude minutes
  min["lng"] = information[1] - 28;
  if (min["lng"] >= 60)
    min["lng"] -= 60;

  snprintf(y, sizeof(y), "%02d.%02d", (int) min["lng"], (information[2] - 28));

  longminutes = y;

  // process position ambiguity
  switch(amount) {
    case 4:
      longitude += 0.5;
      break;
    case 3:
      longtmp = longminutes.substr(0, 1) + "5";
      longitude += (atof(longtmp.c_str()) / 60);
      break;
    case 2:
      longtmp = longminutes.substr(0, 2) + ".5";
      longitude += (atof(longtmp.c_str()) / 60);
      break;
    case 1:
      longtmp = longminutes.substr(0, 4) + "5";
      longitude += (atof(longtmp.c_str()) / 60);
      break;
    case 0:
      longitude += (atof(longminutes.c_str()) / 60);
      break;
    default:
      addString("aprs.packet.error.message", "mice: anonamlous ambiguity");
      addString("aprs.packet.mic_e.error.message", "mice: anonamlous ambiguity");
      return false;
      break;
  } // switch

  dirchar = destcall[5];
  if (dirchar >= 0x50)
    longitude = longitude * -1;

  // grab speed and course
  speed = ((information[3] - 28) * 10);
  coursespeed = information[4] - 28;
  coursespeedtmp = int(coursespeed / 10);

  speed += coursespeedtmp;

  coursespeed -= int(coursespeedtmp * 10.00);
  course = coursespeed * 100;
  course += (information[5] - 28);

  // adjust for maximums
  if (speed >= 800)
    speed -= 800;

  if (course >= 400)
    course -= 400;

  // convert speed to KPH
  speed = KnotToKmh(speed);

  // check for possible altitude and comment data
  // Altitude is base-91 in the format "xxx}" where
  // x is the base-91 digits in meters, origin is 10000 meters
  if (information.length() > 8) {
    // try and find altitude
    rest = information.substr(8, (information.length()-8));
//cout << rest << endl;

    // Check for altitude or additional bits, looks like it can be
    // anywhere in the status text.
    if (ereg("(([\x21-\x7b][\x21-\x7b][\x21-\x7b])[\x7d])", rest, regexList)) {
      first = (int) regexList[2].matchString[0];
      second = (int) regexList[2].matchString[1];
      third = (int) regexList[2].matchString[2];
      altitude = ((first - 33) * pow(91.00, 2.00) + (second - 33) * 91 + (third - 33)) - 10000;

      // remove altitude from rest
      rest.erase(regexList[1].matchBegin, regexList[1].matchLength);

      s.str("");
      s << altitude;
      addString("aprs.packet.altitude", s.str());
      addString("aprs.packet.mic_e.position.altitude", s.str());
    } // if

    // Eventually I'll add code to parse this junk but for now
    // I just want to detect it and remove it.
    switch(rest[0]) {
      // Telemetry `: 2 hex characters follow, channels 1 & 3
      case '\x60':
        rest.erase(0, 1);
        break;
      // Telemetry ': 5 hex chatacters follow, channels 1-5
      case '\x27':
      // Telemetry 5 binary telemetry values follow
      case '\x1d':
        rest.erase(0, 1);
        break;
      // Kenwod TH-D7
      case '>':
      // Kenwood TH-D700
      case ']':
        rest.erase(0, 1);
        break;
    } // switch

//cout << rest << endl;

    // snag any DAO precision
    _extractDao(rest, latoff, longoff);
    if (isString("aprs.packet.dao") == true) {
      if (latitude < 0)
        latitude -= latoff;
      else
        latitude += latoff;

      if (longitude < 0)
        longitude -= longoff;
      else
        longitude += longoff;
    } // if

    // remove any non printable characters
    resttmp = rest;
    rest = "";
    for(i=0; i < resttmp.length(); i++) {
      if (resttmp[i] >= '\x20' && resttmp[i] <= '\x7e')
        rest += resttmp.substr(i, 1);

    } // for

    rest = trim(rest);
    addString("aprs.packet.status", rest);
    addString("aprs.packet.mic_e.status", rest);
  } // if

  _latlng(latitude, longitude);

  snprintf(y, sizeof(y), "%.8f", latitude);
  addString("aprs.packet.mic_e.position.latitude", y);
  snprintf(y, sizeof(y), "%.8f", longitude);
  addString("aprs.packet.mic_e.position.longitude", y);

  s.str("");
  s << speed;
  addString("aprs.packet.mic_e.position.speed", s.str());
  addString("aprs.packet.dirspd.speed", s.str());

  s.str("");
  s << course;
  addString("aprs.packet.mic_e.position.course", s.str());
  addString("aprs.packet.dirspd.direction", s.str());

  _symbol(information.substr(7, 1), information.substr(6, 1));
  addString("aprs.packet.mic_e.symbol.code", information.substr(6, 1));
  addString("aprs.packet.mic_e.symbol.table", information.substr(7, 1));

  addString("aprs.packet.mic_e.raw.mbits", mbits);

  addString("aprs.packet.position.type", "mic_e");

  status(_cleanupComment(rest));

  return true;
} // APRS::_convertMIC_EToDecimal

/******************
 ** NMEA Members **
 ******************/

const bool APRS::_parseNmea(const string &parseMe) {
  string aprsMessage = parseMe;
  stringstream s;                       // Generic string stream.

  if (_extractNmea(aprsMessage) == false)
    return false;

  status(_cleanupComment(aprsMessage));
  addString("aprs.packet.object.type", "P");

  /**
   * Invalid Coordinates
   *
   * Don't allow invalid coordinates.
   *
   */
  if (!isValidLatLong(_latitude, _longitude)) {
    addString("aprs.packet.error.message", "nmea: invalid longitude/latitude");
    return false;
  } // if

  return true;
} // APRS::_parseNmea

const bool APRS::_extractItem(string &aprsMessage) {
  regexMatchListType regexList;         // Map of regex matches.
  string object;

  if (_command != ')')
    return false;

  if (ereg("^(([\x20\x22-\x5e\x60-\x7e]{3,9})([!]|_))", aprsMessage, regexList) < 1) {
    // no item here?
    addString("aprs.packet.error.message", "invalid item");
    return false;
  } // if

  object = regexList[2].matchString;

  trim(object);

  // don't allow items that have all spaces for a name.
  if (object.length() < 1) {
    addString("aprs.packet.error.message", "invalid item name; all spaces");
    return false;
  } // if

  addString("aprs.packet.object.name", object.c_str());
  addString("aprs.packet.object.action", regexList[3].matchString.c_str());
  aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);
  addString("aprs.packet.object.info", _makeBufferSafe(aprsMessage));
  replaceString("aprs.packet.object.type", "I");

  return true;
} // APRS::_extractItem

const bool APRS::_extractNmea(string &aprsMessage) {
  StringToken nmeaToken;		// NMEA tokenized.
  map<string, string> returnPOS;	// Position returned from convertPOS.

  if (_parseUltimeter(aprsMessage) == true)
    return true;

  // initialize variables
  nmeaToken.setDelimiter(',');
  nmeaToken = aprsMessage;

  if (nmeaToken.size() < 1) {
    addString("aprs.packet.error.message", "invalid nmea size");
    return false;
  } // if

  addString("aprs.packet.position.type", "nmea");

  if (nmeaToken[0] == "GPGGA") {

    if (nmeaToken.size() != 15) {
      addString("aprs.packet.error.message", "invalid nmea: GPGGA length must be 15");
      return false;
    } // if

    if (!_convertPos(nmeaToken[2], nmeaToken[3], nmeaToken[4], nmeaToken[5], returnPOS)) {
      addString("aprs.packet.error.message", "invalid nmea: unable to convert GPGGA to position");
      return false;
    } // if

    if (nmeaToken[9].size() > 0 && nmeaToken[10] == "M")
      addString("aprs.packet.position.altitude", nmeaToken[9].c_str());

    _latlng(returnPOS["latitude.decimal"], returnPOS["longitude.decimal"]);

    addString("aprs.packet.position.nmea.type", nmeaToken[0]);
    addString("aprs.packet.position.nmea.raw.latitude", returnPOS["raw.latitude"]);
    addString("aprs.packet.position.nmea.raw.longitude", returnPOS["raw.longitude"]);
    addString("aprs.packet.position.latitude", returnPOS["latitude"]);
    addString("aprs.packet.position.latitude.direction", nmeaToken[3].c_str());
    addString("aprs.packet.position.longitude", returnPOS["longitude"]);
    addString("aprs.packet.position.longitude.direction", nmeaToken[5].c_str());

    aprsMessage = "";

    return true;
  } // if

  if (nmeaToken[0] == "GPRMC") {

    if (nmeaToken.size() < 12) {
      addString("aprs.packet.error.message", "invalid nmea: GPRMC under 12 in length");
      return false;
    } // if

    if (_convertPos(nmeaToken[3], nmeaToken[4], nmeaToken[5], nmeaToken[6], returnPOS) == false) {
      addString("aprs.packet.error.message", "invalid nmea: unable to convert GPRMC to position");
      return false;
    } // if
    _latlng(returnPOS["latitude.decimal"],
                            returnPOS["longitude.decimal"]);

    addString("aprs.packet.position.nmea.type", nmeaToken[0]);
    addString("aprs.packet.position.nmea.raw.latitude", returnPOS["raw.latitude"]);
    addString("aprs.packet.position.nmea.raw.longitude", returnPOS["raw.longitude"]);
    addString("aprs.packet.position.latitude", returnPOS["latitude"]);
    addString("aprs.packet.position.latitude.direction", nmeaToken[4].c_str());
    addString("aprs.packet.position.longitude", returnPOS["longitude"]);
    addString("aprs.packet.position.longitude.direction", nmeaToken[6].c_str());

    aprsMessage = "";

    return true;
  } // if

  if (nmeaToken[0] == "GPGLL") {

    if (nmeaToken.size() < 5) {
      addString("aprs.packet.error.message", "invalid nmea: GPGLL must not be under 5 in length");
      return false;
    } // if

    if (_convertPos(nmeaToken[1], nmeaToken[2], nmeaToken[3], nmeaToken[4], returnPOS) == false) {
      addString("aprs.packet.error.message", "invalid nmea: unable to convert GPGLL to position");
      return false;
    } // if

    _latlng(returnPOS["latitude.decimal"], returnPOS["longitude.decimal"]);

    addString("aprs.packet.position.nmea.type", nmeaToken[0]);
    addString("aprs.packet.position.nmea.raw.latitude", returnPOS["raw.latitude"]);
    addString("aprs.packet.position.nmea.raw.longitude", returnPOS["raw.longitude"]);
    addString("aprs.packet.position.latitude", returnPOS["latitude"]);
    addString("aprs.packet.position.longitude", returnPOS["longitude"]);

    aprsMessage = "";

    return true;
  } // if

  addString("aprs.packet.error.message", "invalid nmea: unsupported sentence");

  return false;
} // APRS::_parseNmea

const bool APRS::_convertPos(const string &cnvLat, const string &latDir,
                            const string &cnvLong, const string &longDir,
                            map<string, string> &returnPOS) {
  float degrees;                        // Degrees
  float minutes;                        // Minutes
  float seconds;                        // Seconds
  float latitude;                       // Latitude
  float longitude;                      // Longitude
  stringstream s;			// Temp string buffer.

  if (cnvLat.length() < 8 || cnvLong.length() < 9 ||
      latDir.length() != 1 || longDir.length() != 1)
    return false;

  if ((latDir != "N" && latDir != "S") ||
      (longDir != "E" && longDir != "W"))
    return false;

  // initialize variables
  returnPOS.clear();

  // convert our degrees into decimal
  degrees = atoi(cnvLat.substr(0, 2).c_str());
  minutes = atoi(cnvLat.substr(2, 2).c_str());
  seconds = atoi(cnvLat.substr(5, 2).c_str());
  seconds = (seconds / 100) * 60;
  
  latitude = degrees + (minutes / 60.000000) + (seconds / 3600.000000);
  
  degrees = atoi(cnvLong.substr(0, 3).c_str());
  minutes = atoi(cnvLong.substr(3, 2).c_str());
  seconds = atoi(cnvLong.substr(6, 2).c_str());
  seconds = (seconds / 100) * 60;
 
  longitude = degrees + (minutes / 60.000000) + (seconds / 3600.000000);
  
  returnPOS["raw.latitude"] = cnvLat;
  returnPOS["raw.longitude"] = cnvLong;

  // initialize variables
  s.str("");   
  s << latitude;
  returnPOS["latitude"] = s.str();

  s.str("");
  s << longitude;
  returnPOS["longitude"] = s.str();
  
  s.str("");
  s << ((latDir == "S") ? (latitude * -1) : latitude);
  returnPOS["latitude.decimal"] = s.str();

  s.str("");
  s << ((longDir == "W") ? (longitude * -1) : longitude);
  returnPOS["longitude.decimal"] = s.str();

  return true;
} // APRS::_convertPos

/*****************
 ** RNG Members **
 *****************/

const bool APRS::_extractRng(string &aprsMessage) {
  regexMatchListType regexList;         // Map of regex matches.
  double range;					// Range
  stringstream s;

  if (aprsMessage.length() < 1)
    return false;

  if (ereg("(RNG([0-9-]{4}))", aprsMessage, regexList) < 1)
    return false;

  range = MphToKmh(atoi(regexList[2].matchString.c_str()));

  // sweet, set our altitude variable and remove from aprsMessage.
  s.str(""); s << range;
  addString("aprs.packet.rng", s.str());
  aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);

  return true;
} // APRS::_extractRng

/***********************
 ** AltSymbol Members **
 ***********************/

const bool APRS::_extractAltSymbol(string &aprsMessage) {
  regexMatchListType regexList;         // Map of regex matches.
  StringToken ssidToken;			// Tokenize the SSID
  string symbol;				// Temp symbol storage.
  int ssid;					// SSID
  unsigned int i;				// Counter

  // initialize variables
  ssidToken.setDelimiter('-');
  ssidToken = getString("aprs.packet.source");
  if (ssidToken.size() == 2) {
    ssid = atoi(ssidToken[1].c_str());

    if (ssid >= 0 && ssid <= 15) {
      symbol = ssid_table[ssid];

      if (symbol.length() == 2) {
        // sweet, set our altitude variable and remove from aprsMessage.
        _symbol(symbol.substr(0, 1), symbol.substr(1, 1));
        addString("aprs.packet.symbol.type", "ssid");
      } // if
    } // if

  } // if

  if (ereg("(GPS|SPC|SYM)([A-Z0-9]{2})([A-Z0-9]?)",
                           getString("aprs.packet.path0"), regexList) < 1)
    return false;

  // Try and find a match in the tables
  for(i = 0; gpsxyz_table[i] != NULL; i++) {
    if (string(gpsxyz_table[i]) != regexList[2].matchString.c_str())
      continue;

    symbol = symbol_table[i];

    if (symbol.length() != 2)
      break;

    // sweet, set our altitude variable and remove from aprsMessage.
    _symbol(symbol.substr(0, 1), symbol.substr(1, 1));
    addString("aprs.packet.symbol.type", "GPSxyz");

    return true;
  } // for

  return false;
} // APRS::_extractAltSymbol

/***********************
 ** Telemetry Members **
 ***********************/

const bool APRS::_parseTelemetry(const string &parseMe) {
  string aprsMessage = parseMe;
  stringstream s;                       // Generic string stream.

  _packetType = APRS_PACKET_TELEMETRY;

  if (_extractTelemetry(aprsMessage) == false)
    return false;

  status(_cleanupComment(aprsMessage));
  addString("aprs.packet.object.type", "P");

  return true;
} // APRS::_parseTelemtry

const bool APRS::_extractTelemetry(string &aprsMessage) {
  stringstream s;			// Generic string stream.
  regexMatchListType regexList;         // Map of regex matches.
  unsigned int i;			// Generic counter.
  char y[10];
  float f;

  // cout << "!!!TELEMETRY!!! " << aprsMessage << endl;

  if (ereg("^((#|#MIC)([0-9.]{1,6}),([0-9.]{1,6}),([0-9.]{1,6}),([0-9.]{1,6}),([0-9.]{1,6}),([0-9.]{1,6}),([0-9.]{1,8}),)(.+)$",
                           aprsMessage, regexList) > 0) {
    addString("aprs.packet.telemetry.comment", regexList[10].matchString);
    aprsMessage.erase(regexList[10].matchBegin, regexList[10].matchLength);
  } // if
  else if (ereg("^((#|#MIC)([0-9.]{1,6}),([0-9.]{1,6}),([0-9.]{1,6}),([0-9.]{1,6}),([0-9.]{1,6}),([0-9.]{1,6}),([0-9]{1,8}))",
                           aprsMessage, regexList) > 0);
  else {
    addString("aprs.packet.error.message", "invalid telemetry: unsupported format");
    return false;
  } // else

  addString("aprs.packet.telemetry.sequence", regexList[3].matchString);

  for(i = 0; i < 5; i++) {
    s.str("");
    s << "aprs.packet.telemetry.analog" << i;

    f = atof(regexList[(i + 4)].matchString.c_str());

    if (f > 999999 || f < -999999) {
      addString("aprs.packet.error.message", "invalid telemetry: telemetry data");
      return false;
    } // if

    snprintf(y, sizeof(y), "%.2f", f);
    addString(s.str(), y);
  } // for

  // exapand to 8 bits if needed
  s.str("");
  s << regexList[9].matchString;
  while(s.str().length() < 8)
    s << "0";

  _telemetry = true;
  addString("aprs.packet.telemetry.digital", s.str());

  // remove latitude/longitude message and update packet message
  aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);

  return true;
} // APRS::_extractTelemtry

/*****************
 ** PHG Members **
 *****************/

const bool APRS::_extractPhg(string &aprsMessage) {
  char sep;
  regexMatchListType regexList;         // Map of regex matches.
  stringstream s;			// Temp stringstream for converting int to string.
  double base_power;			// Base power.
  double base_haat;			// Base height.
  double base_gain;			// Base gain.
  double power;				// Calculated power.
  double haat;				// Calculated height.
  double gain;				// Callculated gain.
  double range;				// Calculated range.
  int directivity;
  int beacon;

  if (aprsMessage.length() < 1)
    return false;

  if (ereg("(PHG(([0-9]{1})(.{1})([0-9]{1})([0-9]{1})(.{1})))",
                           aprsMessage, regexList) < 1)
    return false;

  // intialize variables
  // haat = height-above-average-terrain
  base_power = atoi(regexList[3].matchString.c_str());
  base_haat = (regexList[4].matchString[0] - 48);
  base_gain = atoi(regexList[5].matchString.c_str());

  power = pow(base_power, 2.00);
  haat = (10.00 * pow(2.00, base_haat));
  gain = pow(10.00, (base_gain / 10.00));
  range = MphToKmh(sqrt((2 * haat * sqrt((power / 10.00) * (gain / 2.00)))));

  // convert HAAT to meters for storage.
  haat = FtToM((10.00 * pow(2.00, base_haat)));

  directivity = dirs[atoi(regexList[6].matchString.c_str())];
  sep = regexList[7].matchString[0];

  // look for beacon time
  if (sep >= '2' && sep <= '9') {
    beacon = int(60/(int(sep)-48));
    s.str(""); s << beacon;
    addString("aprs.packet.phg.beacon", s.str());
  } // if

  // sweet, set our altitude variable and remove from aprsMessage.
  addString("aprs.packet.phg", regexList[2].matchString);
  s.str(""); s << power;
  addString("aprs.packet.phg.power", s.str());
  s.str(""); s << haat;
  addString("aprs.packet.phg.haat", s.str());
  s.str(""); s << gain;
  addString("aprs.packet.phg.gain", s.str());
  s.str(""); s << range;
  addString("aprs.packet.phg.range", s.str());
  s.str(""); s << directivity;
  addString("aprs.packet.phg.directivity", s.str());
  aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);

  return true;
} // APRS::_extractPhg

/*****************
 ** DAO Members **
 *****************/

const bool APRS::_extractDao(string &aprsMessage, float &latoff, float &longoff) {
  regexMatchListType regexList;         // Map of regex matches.
  regexMatchListType regexList2;         // Map of regex matches.
  string daoCandidate;			// Temp storage for daoCandidate.
  stringstream s;			// Temp stringstream for converting int to string.
  char x,y;				// Characters

  if (aprsMessage.length() < 1)
    return false;

  if (ereg("(\\!([\x21-\x7b][\x20-\x7b]{2})\\!)",
                           aprsMessage, regexList) < 1)
    return false;

  daoCandidate = regexList[2].matchString;
  addString("aprs.packet.dao", regexList[2].matchString);

  if (ereg("^([\x41-\x5a])([0-9])([0-9])$",
                           daoCandidate, regexList2) > 0) {
    addString("aprs.packet.dao.datum", regexList2[1].matchString);
    addString("aprs.packet.dao.resolution", "3");

    latoff = atoi(regexList2[2].matchString.c_str());
    longoff = atoi(regexList2[3].matchString.c_str());
    latoff = (latoff * 0.001 / 60.00);
    longoff = (longoff * 0.001 / 60.00);
  } // if
  else if (ereg("^([\x61-\x7a])([\x21-\x7b])([\x21-\x7b])$",
                           daoCandidate, regexList2) > 0) {
    addString("aprs.packet.dao.datum", regexList2[1].matchString);
    addString("aprs.packet.dao.resolution", "4");

    x = regexList2[2].matchString.c_str()[0];
    y = regexList2[3].matchString.c_str()[0];
    latoff = ((x - 33.00) / 91.00 * 0.01 / 60.00);
    longoff = ((y - 33.00) / 91.00 * 0.01 / 60.00);
  } // else if
  else
    return false;

  // sweet, set our altitude variable and remove from aprsMessage.
  addString("aprs.packet.dao.x", regexList2[2].matchString);
  addString("aprs.packet.dao.y", regexList2[3].matchString);
  s.str(""); s << latoff;
  addString("aprs.packet.dao.latitude", s.str());
  s.str(""); s << longoff;
  addString("aprs.packet.dao.longitude", s.str());

  aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);

  return true;
} // APRS::_extractDao

/**********************
 ** DISTANCE Members **
 **********************/

const double APRS::calcDistance(double lat1, double lon1, double lat2,
                            double lon2, char unit) {
  double theta;
  double distance;


  if (lat1 > 90 || lat1 < -90 ||
      lat2 > 90 || lat2 < -90)
    return -1;

  if (lon1 > 180 || lon1 < -180 ||
      lon2 > 180 || lon2 < -180)
    return -1;

  theta = lon1 - lon2;

  distance = (sin(deg2rad(lat1)) * sin(deg2rad(lat2))) + (cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta)));
  distance = acos(distance);
  distance = rad2deg(distance);
  distance = distance * 60 * 1.1515;

  switch(unit) {
    case 'M':
      break;
    case 'K':
      distance = distance * 1.609344;
      break;
    case 'N':
      distance = distance * 0.8684;
      break;
  } // switch

  return distance;
} // APRS::calcDistance

double APRS::deg2rad(double deg) {
  return (deg * pi / 180);
} // APRS::deg2rad

double APRS::rad2deg(double rad) {
  return (rad * 180 / pi);
} // APRS::rad2deg

/*
 * calculate destination point given start point, initial bearing (deg) and distance (km)
 *   see http://williams.best.vwh.net/avform.htm#LL
 */
const bool APRS::calcDestPoint(const int bearing, 
                           const double d, 
                           const double lat, 
                           const double lon, 
                           double &lat2, 
                           double &lon2) {
  int R = 6371; // earth's mean radius in km
  double lat1 = deg2rad(lat), lon1 = deg2rad(lon);
  double brng = deg2rad(bearing);

  lat2 = asin(sin(lat1)*cos(d/R) + 
                        cos(lat1)*sin(d/R)*cos(brng));
  lon2 = lon1 + atan2(sin(brng)*sin(d/R)*cos(lat1), 
                               cos(d/R)-sin(lat1)*sin(lat2));
  lon2 = fmod((lon2+pi),(2*pi)) - pi;  // normalise to -180...+180

  //if (isNaN(lat2) || isNaN(lon2)) return null;

  //return new LatLon(lat2.toDeg(), lon2.toDeg());
  lat2 = rad2deg(lat2);
  lon2 = rad2deg(lon2);

  return true;
} // APRS::calcDestPoint

/****************************
 ** isValidLatLong Members **
 ****************************/

const bool APRS::isValidLatLong(const double lat, const double lon) {
  /**
   * Invalid Positions
   *
   * Don't store invalid positions.
   *
   * Latitude > 90 || Latitude < -90 [INVALID]
   * Longitude > 180 || Latitude < -180 [INVALID]
   * Latitude == 0 || Longitude == 0 [INVALID]
   */
  if (lon > 180 || lon < -180 ||
      lat > 90 || lat < -90)
    return false;

  return true;
} // APRS::isValidLatLong

const bool APRS::isValidMessage(const string &aprsMessage) {
  map<int, regexMatch> regexList;               // Map of regex matches.

  //if (ereg("^[\x20-\x7e]+$", aprsMessage, regexList) < 1)
  //  return false;

  if (ereg("^[\x00-\x1F]+$", aprsMessage, regexList))
    return false;

  return true;
} // APRS::isValidMessage

const bool APRS::isValidCallsign(const string &aprsCallsign) {
  map<int, regexMatch> regexList;               // Map of regex matches.

  if (ereg("^[A-Z0-9-]+$", aprsCallsign, regexList) < 1
      || aprsCallsign.length() > 9)
    return false;

  return true;
} // APRS::isValidCallsign

/*******************
 ** SPEED Members **
 *******************/

const double APRS::calcSpeed(const double dist, const time_t seconds, 
                         const int distUnits, const int speedUnits) {
  double speed;

  if (dist < 1 || seconds < 1)
    return -1;

  speed = ((dist * distunits[distUnits])  / (seconds * speedunits[speedUnits])); 

  return speed;
} // APRS::calcSpeed

/*******************************
 ** MESSAGE_TELEMETRY Members **
 *******************************/

const bool APRS::_parseMessageTelemetry(string &aprsMessage) {
  StringToken st;			// String Token to split packet.
  regexMatchListType regexList;         // Map of regex matches.
  unsigned int i;
  stringstream s;			// Temp stringstream for converting int to string.

  if (aprsMessage.length() < 1)
    return false;

  if (ereg("^(([A-Z0-9\\-]{1,9})[ ]*:(BITS|PARM|UNIT|EQNS)[.](.*))",
                           aprsMessage, regexList) < 1)
    return false;

  st.setDelimiter(',');
  st = regexList[4].matchString;

  if (regexList[3].matchString == "PARM" || regexList[3].matchString == "UNIT") {
    for(i=0; i < st.size(); i++) {
      s.str("");
      s << "aprs.packet.telemetry.";

      if (i < 5) {
        s << "analog" << i;
        addString(s.str(), st[i]);
      } // if
      else {
        s << "digital" << i-5;
        addString(s.str(), st[i]);
      } // else
    } // for
  } // if
  else if (regexList[3].matchString == "BITS") {
    if (st.size() < 2) {
      addString("aprs.packet.error.message", "telemetry: not enough fields in bits count");
      return false;
    } // if

    s.str("");
    s << st[0];
    for(i=0; s.str().length() < 8; i++)
      s << "0";

    addString("aprs.packet.telemetry.bitsense", s.str());
    addString("aprs.packet.telemetry.project", st[1]);
  } // else if
  else if (regexList[3].matchString == "EQNS") {
    if ((st.size() % 3) != 0) {
      addString("aprs.packet.error.message", "telemtry: EQNS fields not modulus of 3");
      return false;
    } // if

    for(i=0; i < st.size(); i++) {
      s.str("");
      s << "aprs.packet.telemetry."
        << "a"
        << (i/3)
        << ".";

      switch((i%3)) {
        case 0:
          s << "a";
          break;
        case 1:
          s << "b";
          break;
        case 2:
          s << "c";
          break;
      } // switch

      addString(s.str(), st[i]);
    } // for
  } // else if

  addString("aprs.packet.telemetry.message", aprsMessage);
  addString("aprs.packet.telemetry.message.type", regexList[3].matchString);
  addString("aprs.packet.telemetry.message.data", regexList[4].matchString);

  return true;
} // APRS::_parseMessageTelemtry

/*********************
 ** ULTIMER Members **
 *********************/

const bool APRS::_parseUltimeter(string &aprsMessage) {
  StringToken st;			// Token string of each variable
  regexMatchListType regexList;         // Map of regex matches.
  stringstream s;			// Temp stringstream for converting int to string.
  float x;				// For Calculations
  char y[10];				// Temp storage
  unsigned int i;

  if (aprsMessage.length() < 1)
    return false;

  if (ereg("^((ULTW)([0-9A-F]{4}|----){6})", aprsMessage, regexList) < 1)
    return false;

  addString("aprs.packet.ultimeter.string", aprsMessage);

  // remove ULTW prefix
  aprsMessage.erase(regexList[2].matchBegin, regexList[2].matchLength);

  addString("aprs.packet.ultimeter.string.short", aprsMessage);

  while(ereg("^([0-9A-F]{4}|----)", aprsMessage, regexList)) {
    st.add(regexList[1].matchString);
    aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);
  } // while

  for(i=0; i < st.size(); i++) {
    s.str(""); s << i;
    addString("aprs.packet.ultimeter.string.parse" + s.str(), st[i]);
  } // for

  if (st[0] != "----") {
    x = axtoi(st[0].c_str()) * 0.1;
    snprintf(y, sizeof(y), "%.1f", x);
    addString("aprs.packet.ultimeter.wind.gust", y);
  } // if

  if (st[1] != "----") {
    x = axtoi(st[1].c_str()) * 1.41176;
    snprintf(y, sizeof(y), "%.0f", x);
    addString("aprs.packet.ultimeter.wind.direction", y);
  } // if

  if (st[2] != "----") {
    x = axtoi(st[2].c_str()) * 0.1;
    snprintf(y, sizeof(y), "%.1f", x);
    addString("aprs.packet.ultimeter.temperature", y);
  } // if

  if (st[3] != "----") {
    x = axtoi(st[3].c_str()) * 0.01;
    snprintf(y, sizeof(y), "%.1f", x);
    addString("aprs.packet.ultimeter.rain", y);
  } // if

  if (st[4] != "----") {
    x = axtoi(st[4].c_str()) * 0.1;
    snprintf(y, sizeof(y), "%.1f", x);
    addString("aprs.packet.ultimeter.pressure", y);
  } // if

  if (st[5] != "----") {
    x = axtoi(st[5].c_str()) * 0.1;
    snprintf(y, sizeof(y), "%.1f", x);
    addString("aprs.packet.ultimeter.barometer", y);
  } // if

  if (st.size() > 6 &&
      st[6] != "----") {
    x = axtoi(st[6].c_str()) * 0.1;
    snprintf(y, sizeof(y), "%.1f", x);
    addString("aprs.packet.ultimeter.barometer.lsw", y);
  }

  if (st.size() > 7 &&
      st[7] != "----") {
    x = axtoi(st[7].c_str()) * 0.1;
    snprintf(y, sizeof(y), "%.1f", x);
    addString("aprs.packet.ultimeter.barometer.msw", y);
  }

  if (st.size() > 8 &&
      st[8] != "----") {
    x = axtoi(st[8].c_str()) * 0.1;
    snprintf(y, sizeof(y), "%.1f", x);
    addString("aprs.packet.ultimeter.humidity", y);
  }

  return true;
} // APRS::_parseUltimeter

/*********************
 ** Weather Members **
 *********************/

const bool APRS::_parseWeather(string &aprsMessage) {
  regexMatchListType regexList;         // Map of regex matches.
  static const char delimiters[] = "rpPhbLls#";	// Field delimiters.
  static const char *names[] = {
    "aprs.packet.weather.rain.hour",
    "aprs.packet.weather.rain.24hour",
    "aprs.packet.weather.rain.midnight",
    "aprs.packet.weather.humidity",
    "aprs.packet.weather.pressure",
    "aprs.packet.weather.luminosity.wsm",
    "aprs.packet.weather.luminosity.wsm1000",
    "aprs.packet.weather.snow.24hour",
    "aprs.packet.weather.rain.raw"
  };
  stringstream s;				// String stream for converting numbers.
  size_t pos;				// Position in string.
  size_t last;				// Last position in string.
  size_t i;				// Generic counter.
  double conv;
  char y[10];

  // remove leading timestamp?
  if (!_extractTimestamp(aprsMessage))
    _extractWeatherTimestamp(aprsMessage);

  if (aprsMessage.length() < 16) {
    addString("aprs.packet.error.message", "invalid length parsing weather");
    return false;
  } // if

  std::string weather_str = aprsMessage.substr(0, 16);

//  if (ereg("^([0-9]{8})c", aprsMessage, regexList) > 0)
//    aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchEnd);

  // find wind direction / speed if it's there...
  _extractWindDirSpd(aprsMessage);
//  if (ereg("^(c([0-9. ]{3})s([0-9. ]{3}))", aprsMessage, regexList) > 0) {

//    wind_speed = MphToKmh(atof(regexList[3].matchString.c_str()));
//    snprintf(y, sizeof(y), "%.1f", wind_speed);

//    addString("aprs.packet.weather.wind.direction", regexList[2].matchString);
//    addString("aprs.packet.weather.wind.speed", y);
//    aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);
//  } // if

    bool ok = _extractGustTemp(aprsMessage);
    if (!ok) {
      addString("aprs.packet.error.message", "invalid weather packet, missing gust and temp");
      return false;
    } // if
//  if (ereg("^(g([0-9. ]{3})t([0-9. ]{3}))", aprsMessage, regexList) < 1) {
//    addString("aprs.packet.error.message", "invalid weather packet");
//    return false;
//  } // if

  // sweet, set our altitude variable and remove from aprsMessage.
  addString("aprs.packet.weather", weather_str);
  _weather = true;

  // initialize variables
  pos = 0;

  while(true) {
    for(i = 0; i < strlen(delimiters) && delimiters[i] != aprsMessage[pos]; i++);

    // no more delimiters to parse let's bail!
    if (i == strlen(delimiters)) break;

    // let's snag our digits...
    for(last = ++pos; isdigit(aprsMessage[pos]) || aprsMessage[pos] == '.'; pos++);

    // any more data to process?
    if (pos == last)
      // guess not
      break;

    s.str("");
    switch(delimiters[i]) {
      case 'r':
      case 'p':
      case 'P':
        conv = atof(aprsMessage.substr(last, (pos - last)).c_str());
        snprintf(y, sizeof(y), "%.1f", HInchToMm(conv));
        s << y;
        break;
      case 'b':
        conv = atof(aprsMessage.substr(last, (pos - last)).c_str()) / 10;
        //conv = atof(aprsMessage.substr(last, (pos - last)).c_str());
        snprintf(y, sizeof(y), "%.1f", conv);
        s << y;
        break;
      default:
        s << aprsMessage.substr(last, (pos - last));
        break;
    } // switch

    // add our new field
    addString(names[i], s.str());
  } // while

  // dump the rest of the weather report we snagged
  aprsMessage.erase(0, pos);

  if (ereg("^([a-zA-Z0-9_-]{3,5})$", aprsMessage, regexList) > 0) {
    addString("aprs.packet.weather.version", regexList[1].matchString);
    aprsMessage.erase(regexList[1].matchBegin, regexList[1].matchLength);
  } // if

  status(aprsMessage);

  return true;
} // APRS::_parseWeather

const bool APRS::_extractWindDirSpd(string &aprsMessage) {
  double wind_speed;
  char y[10];

  if (aprsMessage.length() < 8
      || aprsMessage[0] != 'c') return false;

  std::string buf = aprsMessage.substr(1, 7);

  std::string::size_type pos = 1;
  char ch = aprsMessage[pos];
  bool ok;
  // [0-9]{3} - dir
  bool invalid_dir = false;
  for(size_t i=0; i < 3; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '0' && ch <= '9')
         || ch == '.'
         || ch == ' ';
    if (!ok) return false;
    invalid_dir |= (ch == '.' || ch == ' ');
  } // for
  pos += 3;

  ch = aprsMessage[pos];
  if (ch != 's') return false;
  ++pos;

  // [0-9]{3} - spd
  bool invalid_spd = false;
  for(size_t i=0; i < 3; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '0' && ch <= '9')
         || ch == '.'
         || ch == ' ';
    if (!ok) return false;
    invalid_spd |= (ch == '.' || ch == ' ');
  } // for

//                 2            3
//              0  1         4  5
//  if (ereg("^(c([0-9. ]{3})s([0-9. ]{3}))", aprsMessage, regexList) > 0) {

  if (!invalid_dir) {
    std::string dir_str = aprsMessage.substr(1, 3);
    addString("aprs.packet.weather.wind.direction", dir_str);
  } // if

  if (!invalid_spd) {
    std::string speed_str = aprsMessage.substr(5, 3);
    wind_speed = MphToKmh(atof( speed_str.c_str() ));
    snprintf(y, sizeof(y), "%.1f", wind_speed);
    addString("aprs.packet.weather.wind.speed", y);
  } // if

  aprsMessage.erase(0, 8);

  return true;
} // APRS::_extractWindDirSpd

const bool APRS::_extractGustTemp(string &aprsMessage) {
  if (aprsMessage.length() < 8
      || aprsMessage[0] != 'g') return false;

  std::string buf = aprsMessage.substr(1, 7);

  std::string::size_type pos = 1;
  char ch;
  bool ok;
  bool invalid_gust = false;
  // [0-9]{3} - dir
  for(size_t i=0; i < 3; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '0' && ch <= '9')
         || ch == '.'
         || ch == ' ';
    if (!ok) return false;
    invalid_gust |= (ch == '.' || ch == ' ');
  } // for
  pos += 3;

  ch = aprsMessage[pos];
  if (ch != 't') return false;
  ++pos;

  // [0-9]{3} - temp
  bool invalid_temp = false;
  for(size_t i=0; i < 3; i++) {
    ch = aprsMessage[pos+i];
    ok = (ch >= '0' && ch <= '9')
         || ch == '.'
         || ch == ' ';
    if (!ok) return false;
    invalid_temp |= (ch == '.' || ch == ' ');
  } // for

  if (!invalid_gust) {
    std::string gust_str = aprsMessage.substr(1, 3);
    double wind_gust = MphToKmh(atof( gust_str.c_str() ));
    addString("aprs.packet.weather.wind.gust", float2string(wind_gust, 1));
  } // if

  if (!invalid_temp) {
    std::string temp_str = aprsMessage.substr(5, 3);
    const int temp = atoi( temp_str.c_str() );
    addString("aprs.packet.weather.temperature.fahrenheit", int2string(temp));
    addString("aprs.packet.weather.temperature.celcius", far2cel(temp));
  } // if

  aprsMessage.erase(0, 8);

  return true;
} // APRS::_extractGustTemp

  void APRS::_latlng(const double lat, const double lng) {
    char float_lat[10] = {0};
    char float_long[10] = {0};

    _position = true;

    _latitude = lat;
    _longitude = lng;

    snprintf(float_lat, sizeof(float_lat), "%.6f", _latitude);
    snprintf(float_long, sizeof(float_lat), "%.6f", _longitude);

    addString("aprs.packet.position.latitude.decimal", float_lat);
    addString("aprs.packet.position.longitude.decimal", float_long);
  } // APRS::_latlng

} // namespace aprs

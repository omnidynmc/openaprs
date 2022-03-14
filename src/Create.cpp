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
 $Id: Create.cpp,v 1.2 2005/12/13 21:07:03 omni Exp $
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
#include "Create.h"
#include "Log.h"
#include "Server.h"

#include "config.h"
#include "openaprs_string.h"

namespace aprs {

using std::string;
using std::list;
using std::ofstream;
using std::ostream;
using std::stringstream;
using std::ios;
using std::endl;
using std::cout;

/**************************************************************************
 ** Create Class                                                         **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

Create::Create() {
  // set defaults
}

Create::~Create() {
  // cleanup
}

/***************  
 ** Variables **
 ***************/

/***********************
 ** Timestamp Members **
 ***********************/

const string Create::Timestamp(const time_t myTimestamp, const bool myZulu) {
  struct tm *gm;
  time_t timestamp;
  char y[8];

  if (myTimestamp == 0)
    timestamp = time(NULL);
  else
    timestamp = myTimestamp;

  gm = gmtime(&timestamp);

  if (myZulu)
    snprintf(y, sizeof(y), "%02d%02d%02dz", gm->tm_mday, gm->tm_hour, gm->tm_min);
  else
    snprintf(y, sizeof(y), "%02d%02d%02dh", gm->tm_hour, gm->tm_min, gm->tm_sec);

  return string(y);
} // Create::Timestamp

/******************
 ** Hash Members **
 ******************/

const bool Create::Hash(const string &callsign, short &ret) {
  map<int, regexMatch> regexList;
  string call;
  short i,j, hash;

  if (callsign.length() > 9 || callsign.length() < 1)
    return false;

  // check for a valid callsign
  if (openaprs_string_regex("^([A-Z0-9]{1,6})(-[0-9]{1,2}|)$",
                            callsign, regexList) < 1)
    return false;  

  call = regexList[1].matchString;

  // initialize variables
  hash = kKey;
  j = 0;

  //cout << "CALL: " << call << endl;
  //cout << "PRE: " << hash << endl;

  for(i=0; i < call.length();) {
    hash ^= call[j++]<<8;	// xor high byte with accumulated hash
    hash ^= call[j++];		// xor low byte with accumulated hash
    i += 2;
  } // for

  hash = (hash & 0x7fff);

  //cout << "POST: " << hash << endl;

  ret = hash;

  return true;
} // Create::Hash

/**********************
 ** Position Members **
 **********************/

const bool Create::Position(const double latitude, const double longitude, 
             const double oSpeed, const int oCourse, const double oAltitude, 
             const char oTable, const char oCode,
             const bool oCompress, const int oAmbiguity, 
             string &ret) {
  bool isNorth, isEast;
  char table;					// Symbol table 
  double latval, longval;			// Calculated latitude/longitude values
  int i, val;
  string t;					// Temporary buffer
  stringstream lats, longs;
  map<string, int> deg;
  map<string, int> min;
  char y[10];

  // check for invalid latitude/longitude pairs
  if (latitude < -89.99999
      || latitude > 89.99999
      || longitude < -179.99999
      || longitude > 179.99999)
    return false;

  // check for valid symbol code
  if (!((oTable >= 33 && oTable <= 123)
        || (oTable == 125)))
    return false;

  // passed checks, initialize variables
  ret == "";
  table = oTable;
  lats.str("");
  longs.str("");
  latval = latitude;
  longval = longitude;

  if (oCompress == true) {
    latval = 380926 * (90 - latitude);
    longval = 190463 * (180 + longitude);

    for(i=3; i >= 0; i--) {
      // create latitude characters
      val = int(latval / pow(91, i));
      latval = int(latval) % int(pow(91, i));
      lats << char(val + 33);

      // create longitude characters
      val = int(longval / pow(91, i));
      longval = int(longval) % int(pow(91, i));
      longs << char(val + 33);      
    } // for

    // convert symbol table from 0-9 to a-z
    if (table >= '0' && table <= '9')
      table = table + 49;

    ret = table + lats.str() + longs.str() + oCode;

    // encode our speed, course and altitude
    if (oSpeed >= 0 && oCourse > 0 && oCourse <= 360) {
      // In APRS spec unknown course is zero normally (and north is 360),
      // but in compressed aprs north is zero and there is no unknown course.
      // So round course to nearest 4-degree section and remember
      // to do the 360 -> 0 degree transformation.
      val = (oCourse + 2) / 4;
      if (val > 89)
        val = 0;

      ret += char(val + 33);

      // speed is in knots in compressed form, round to nearest integer
      //val = int(log(oSpeed + 1.0) / log(1.08) + 0.5);
      val = int(log(oSpeed + 1.0) / log(1.08) + 0.5);
      if (val > 89)
        // limit top speed
        val = 89;

      ret += char(val + 33) + string("A");
    } // if
    else
      ret += "  A";

    return true;
  } // if

  // uncompressed format
  isNorth = true;

  if (latitude < 0) {
    latval = 0 - latitude;
    isNorth = false;
  } // if

  deg["lat"] = int(latval);
  min["lat"] = int((latval - deg["lat"]) * 6000);
  snprintf(y, sizeof(y), "%04d", min["lat"]);
  t = y;
  snprintf(y, sizeof(y), "%02d%s.%s", deg["lat"], t.substr(0, 2).c_str(), 
                                        t.substr(2, 2).c_str());

  t = y;
  switch(oAmbiguity) {
    case 4:
      t = t.substr(0, 2) + "  .  ";
      break;
    case 3:
      t = t.substr(0, 3) + " .  ";
      break;
    case 2:
      t = t.substr(0, 4) + ".  ";
      break;
    case 1:
      t = t.substr(0, 5) + "  ";
      break;
  } // switch

  if (isNorth)
    t += "N";
  else
    t += "S";

  lats << t;

  // now process longitude
  isEast = true;

  if (longitude < 0) {
    longval = 0 - longitude;
    isEast = false;
  } // if

  deg["long"] = int(longval);
  min["long"] = int((longval - deg["long"]) * 6000);
  snprintf(y, sizeof(y), "%04d", min["long"]);
  t = y;
  snprintf(y, sizeof(y), "%03d%s.%s", deg["long"], t.substr(0, 2).c_str(), 
                                        t.substr(2, 2).c_str());

  t = y;
  switch(oAmbiguity) {
    case 4:
      t = t.substr(0, 3) + "  .  ";
      break;
    case 3:
      t = t.substr(0, 4) + " .  ";
      break;
    case 2:
      t = t.substr(0, 5) + ".  ";
      break;
    case 1:
      t = t.substr(0, 6) + "  ";
      break;
  } // switch

  if (isEast)
    t += "E";
  else
    t += "W";

  longs << t;

  ret = lats.str() + table + longs.str() + oCode;

  // add course/speed
  if (oSpeed >= 0 && oCourse >= 0) {
    //convert speed to knots
    val = KphToKnot(oSpeed);
    if (val > 999)
      val = 999;

    snprintf(y, sizeof(y), "%03d/%03d", ((oCourse > 360) ? 0 : oCourse),
                                        val);

    ret += y;
  } // if

  return true;
} // Create::Position

const bool Create::Object(const string &oName, 
                          const time_t oTimestamp,
                          const double oLatitude, 
                          const double oLongitude, 
                          const char oTable,
                          const char oCode, 
                          const double oSpeed, 
                          const int oCourse,
                          const double oAltitude, 
                          const bool oAlive,
                          const bool oCompress, 
                          const int oAmbiguity, 
                          const string &oComment,
                          string &ret) {
  stringstream s;			// Generic string stream.
  stringstream p;			// Body of packet
  string aprsMessage;			// Message of aprs string
  string name;
  string position;
  map<int, regexMatch> regexList;	// List of regex matches returned.
  time_t timestamp;

  // initialize variables
  p.str("");
  p << ";";

  //app->writeLog(OPENAPRS_LOG_DEBUG, "msgAPRS::Execute> [%s] Position with timestamp (with APRS messaging)", 
  //                  ePacket->getString("aprs.packet.source").c_str());

  if (openaprs_string_regex("^([\x20-\x7e]{1,9})$", oName, regexList) < 1)
    return false;

  for(name=oName; name.length() < 9;)
    name += " ";

  if (oTimestamp == 0)
    timestamp = time(NULL);

  p << name << ((oAlive) ? "*" : "_");

  // add our timestamp
  // For some reason aprs.fi is hosing our packets with timestamps.
  p << Timestamp(timestamp, true);

  if (!Position(oLatitude, oLongitude, oSpeed, oCourse, oAltitude, 
                oTable, oCode, oCompress, oAmbiguity, position))
    return false;

  p << position << oComment;

  // cout << p.str() << endl;

  ret = p.str();

  return true;
} // Create::Object

const bool Create::Position(const time_t oTimestamp,
                            const double oLatitude, 
                            const double oLongitude, 
                            const char oTable,
                            const char oCode, 
                            const double oSpeed, 
                            const int oCourse,
                            const double oAltitude, 
                            const bool oCompress, 
                            const int oAmbiguity, 
                            const string &oComment,
                            string &ret) {
  stringstream s;			// Generic string stream.
  stringstream p;			// Body of packet
  string aprsMessage;			// Message of aprs string
  string name;
  string position;
  map<int, regexMatch> regexList;	// List of regex matches returned.
  time_t timestamp;

  // initialize variables
  p.str("");
  p << "=";

  if (oTimestamp == 0)
    timestamp = time(NULL);

  // add our timestamp
  // For some reason aprs.fi is hosing our packets with timestamps.
  // p << Timestamp(timestamp, true);

  if (!Position(oLatitude, oLongitude, oSpeed, oCourse, oAltitude,
                oTable, oCode, oCompress, oAmbiguity, position))
    return false;

  p << position << oComment;

  // cout << p.str() << endl;

  ret = p.str();

  return true;
} // Create::Position

const bool Create::Message(const string &source, const string &target,
                           const string &message, string &ret) {
  stringstream s;			// String stream
  map<int, regexMatch> regexList;	// List of regex matches returned.
  string padTarget;			// Padded target.


  // check for invalid source
  if (openaprs_string_regex("^([A-Z0-9-]{1,9})$",
                           target, regexList) < 1)
    return false;

  // check for invalid target
  if (openaprs_string_regex("^([A-Z0-9-]{1,9})$",
                           target, regexList) < 1)
    return false;

  // check for invalid message
  //if (openaprs_string_regex("^([\x20-\x7e]+)$",
  //                         message, regexList) < 1)
  //  return false;
  if (!aprs::APRS::isValidMessage(message))
    return false;

  padTarget = target;
  while(padTarget.length() < 9)
    padTarget += " ";

  // initialize variables
  s.str("");
  s << ":" << padTarget << ":" << message;

  ret = s.str();

  return true;
} // Create::Message

const bool Create::encodeMessageID(const unsigned int id, string &ret) {
  static char charTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\0";
  unsigned int min, max;
  int delta;
  int first, second;
  unsigned int size;
  stringstream s;

  // initializev ariables
  ret = "";
  min = 0;
  max = 1295;
  delta = max - min;
  size = sizeof(charTable);

  if (id > max || id < min)
    return false;

  s.str("");

  first = floor(id / (size-2));
  second = id % 36;
  s << charTable[first] << charTable[second];

  ret = s.str();

  return true;
} // Create::encodeMessageID

}

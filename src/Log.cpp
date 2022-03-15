/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Daniel Robert Karrels99                           **
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
 $Id: Log.cpp,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <ios>
#include <iostream>

#include <time.h>
#include <unistd.h>

#include "Log.h"

#include "openaprs.h"

namespace openaprs {

using std::stringstream;
using std::string;

/**************************************************************************
 ** Log Class                                                            **
 **************************************************************************/

/**************************************
 ** Constructor / Destructor Members **
 **************************************/

Log::Log(const logId addId, const string &addMatch, LogHandler *addLogHandler, const bool shouldCache) {
  myId = addId;
  myMatch = addMatch;
  myLogHandler = addLogHandler;
  myShouldCache = shouldCache;
  myCacheTime = 0;
  myCacheCount = 0;
  myAddTime = true;
} // Log::Log

Log::~Log() {
  closeLog();
} // Log::~Log

/*****************
 ** Log Members **
 *****************/

const bool Log::openLog(const string &openFilename, ios_base::openmode openModes) {
  /*
   * 06/20/2003: Try and open a file stream from fileName.
   */

  if (myFile.is_open())
    return true;

  myFile.open(openFilename.c_str(), openModes);

  if (myFile.is_open())
    return true;

  return false;
} // Log::openLog

const bool Log::closeLog() {
  if (myFile.is_open() == true)
    myFile.close();

  return true;
} // Log::closeLog

void Log::flushLog() {
  if (myFile.is_open() == true)
    myFile.flush();
} // Log::flushLog

const unsigned int Log::appendLog(const string &appendMe) {
  if (myFile.is_open() != true)
    return 0;

  if (!isCached(appendMe)) {
    if (getAddTime())
      myFile << getTime();

    myFile << appendMe;
  } // if

  return appendMe.length();
} // Log::appendLog

const unsigned int Log::appendLog(const stringstream &appendMe) {
  if (myFile.is_open() != true)
    return 0;

  if (!isCached(appendMe.str()))
    if (getAddTime())
      myFile << getTime();

    myFile << appendMe.str();

  return appendMe.str().length();
} // Log::appendLog

const unsigned int Log::checkCache() {
  stringstream s;

  if (myCacheCount == 0)
    return 0;

  s.str("");

  if (getAddTime())
      s << getTime();

  s << "(last message repeated " << myCacheCount << " time";

  if (myCacheCount > 1)
  s << "s";

  s << ")" << endl;  

  myFile << s.str();

  resetCache();

  return s.str().length();
} // checkCache

const string Log::getTime() {
  struct tm *tm;                                // struct tm for localtime
  time_t now;                                   // current time
  char timestamp[24];                           // Currne timestamp.

  now = time(NULL);
  tm = localtime(&now);

  snprintf(timestamp, sizeof(timestamp), "[%04d-%02d-%02d %02d:%02d:%02d] ",
           (tm->tm_year + 1900),
           (tm->tm_mon + 1),
           tm->tm_mday,
           tm->tm_hour,
           tm->tm_min,
           tm->tm_sec);

  return string(timestamp);
} // Log::getTime

/**********************
 ** Operator Members **
 **********************/

Log &Log::operator<<(const char &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Log &Log::operator<<

Log &Log::operator<<(const string &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Log &Log::operator<<

Log &Log::operator<<(const int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Log &Log::operator<<

Log &Log::operator<<(const unsigned int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Log &Log::operator<<

Log &Log::operator<<(const long int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Log &Log::operator<<

Log &Log::operator<<(const unsigned long int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Log &Log::operator<<

Log &Log::operator<<(const short int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Log &Log::operator<<

Log &Log::operator<<(const unsigned short int &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Log &Log::operator<<

Log &Log::operator<<(const stringstream &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe.str();
  return *this;
} // Log &Log::operator<<

Log &Log::operator<<(const double &appendMe) {
  if (myFile.is_open() == true)
    myFile << appendMe;
  return *this;
} // Log &Log::operator<<

/**************************************************************************
 ** Log Class                                                            **
 **************************************************************************/

 /*
  * 06/20/2003: There will probably never be anything here for the log
  *             log class but just in case we'll leave it space. -GCARTER
  */
}

/**************************************************************************
 ** OpenFrame Library                                                    **
 **************************************************************************
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
 **************************************************************************/

#include <openframe/config.h>

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
#include <ctime>
#include <cassert>

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "openframe/Stopwatch.h"

namespace openframe {

/**************************************************************************
 ** Stopwatch Class                                                      **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

Stopwatch::Stopwatch() {
  Lock();
  _timeLastExpire = time(NULL);
  _timeNextInterval = time(NULL);
  _timeInterval = 0;
  Unlock();
} // Stopwatch::Stopwatch

Stopwatch::Stopwatch(const time_t timeInterval) : _timeInterval(timeInterval) {
  Lock();
  _timeLastExpire = time(NULL);
  _timeNextInterval = time(NULL);
  Unlock();
} // Stopwatch::Stopwatch

Stopwatch::~Stopwatch() {
  Lock();
  _clear();
  Unlock();
} // Stopwatch::~Stopwatch

const double Stopwatch::Start() {
  double now;

  Lock();
  now = start = _now();
  Unlock();

  return now;
} // Stopwatch::Start

const double Stopwatch::Time() {
  double total;

  Lock();
  total = (_now() - start);
  Unlock();

  return total;
} // Stopwatch::Time

const double Stopwatch::Now() {
  double now;

  Lock();
  now = _now();
  Unlock();

  return now;
} // Stopwatch::Now

const bool Stopwatch::Next() {
  bool ret;

  Lock();
  ret = _next();
  Unlock();

  return ret;
} // Stopwatch::Next

const bool Stopwatch::_next() {
  if (_timeNextInterval < time(NULL)) {
    _timeNextInterval = time(NULL) + _timeInterval;
    return true;
  } // if

  return false;
} // Stopwatch::_next

const double Stopwatch::_now() {
  timeval tv;
  double ret;

  // do not lock here, only lock on public functions

  gettimeofday(&tv, NULL);

  ret = (double) tv.tv_usec;
  ret /= 1e6;
  ret += (double) tv.tv_sec;

  return ret;
} // Stopwatch::_now

const bool Stopwatch::add(const std::string &name, const time_t expire) {
  bool ret;

  Lock();
  ret = _add(name, expire, 10);
  Unlock();

  return ret;
} // Stopwatch::add

const bool Stopwatch::add(const std::string &name, const time_t expire, const time_t resolution) {
  bool ret;

  Lock();
  ret = _add(name, expire, resolution);
  Unlock();

  return ret;
} // Stopwatch::add

const bool Stopwatch::_add(const std::string &name, const time_t expire, const time_t resolution) {
  SWAverageMapType::iterator ptr;
  SWAverage *avg;

  assert(resolution > 0);

  ptr = averageMap.find(name);
  if (ptr != averageMap.end())
    return false;

  avg = new SWAverage(expire, resolution);

  averageMap.insert(pair<string, SWAverage *>(name, avg));

  return true;
} // Stopwatch::_add

const bool Stopwatch::remove(const std::string &name) {
  bool ret;

  Lock();
  ret = _remove(name);
  Unlock();

  return ret;
} // Stopwatch::remove

const bool Stopwatch::_remove(const std::string &name) {
  SWAverage *avg;
  SWAverageMapType::iterator ptr;

  ptr = averageMap.find(name);
  if (ptr == averageMap.end())
    return false;

  avg = ptr->second;
  avg->clear();

  averageMap.erase(ptr);
  delete avg;

  return true;
} // Stopwatch::_remove

const unsigned int Stopwatch::clear() {
  unsigned int numRows;

  Lock();
  numRows = _clear();
  Unlock();

  return numRows;
} // clear

const unsigned int Stopwatch::_clear() {
  SWAverage *avg;
  SWAverageMapType::iterator ptr;
  unsigned int numRows = 0;

  numRows = averageMap.size();

  for(ptr = averageMap.begin(); ptr != averageMap.end(); ptr++) {
    avg = ptr->second;
    avg->clear();
    delete avg;
  } // for

  averageMap.clear();

  return numRows;
} // Stopwatch::clear

const unsigned int Stopwatch::expire() {
  unsigned int numRows;

  Lock();
  numRows = _expire();
  Unlock();

  return numRows;
} // Stopwatch::expire

const unsigned int Stopwatch::_expire() {
  SWAverage *avg;
  SWRecord *rec;
  SWAverageMapType::iterator ptr;
  SWAverage::SWRecordListType::iterator rptr;
  SWAverage::SWRecordListType newList;
  time_t now = time(NULL);
  time_t expire;
  unsigned int numRows = 0;

  if (now <= _timeLastExpire+10)
    return 0;

  _timeLastExpire = now;

  for(ptr = averageMap.begin(); ptr != averageMap.end(); ptr++) {
    avg = ptr->second;
    newList.clear();
    expire = (now - avg->_expire);

    for(rptr = avg->_recordList.begin(); rptr != avg->_recordList.end(); rptr++) {
      rec = *rptr;
      if (rec->create_ts < expire) {
        avg->_total -= rec->average;
        delete rec;
        numRows++;
//cout << "E";
        continue;
      } // if

      newList.push_back(rec);
    } // for

    avg->_recordList = newList;
  } // for

  return numRows;
} // Stopwatch::_expire

const double Stopwatch::average(const std::string &name, const double sw) {
  double av;

  Lock();
  av = _average(name, sw);
  Unlock();

  return av;
} // Stopwatch::average

const double Stopwatch::average(const std::string &name) {
  double av;

  Lock();
  av = _average(name);
  Unlock();

  return av;
} // Stopwatch::average

const double Stopwatch::_average(const std::string &name, const double sw) {
  bool addNew = false;
  SWRecord *rec;
  SWAverage *avg;
  SWAverageMapType::iterator ptr;
  SWAverage::SWRecordListType::iterator rptr;
  SWRecord *newRecord;
  time_t now = time(NULL);
  double av = -1.0;

  _expire();

  ptr = averageMap.find(name);
  if (ptr == averageMap.end())
    return av;

  avg = ptr->second;

  if (avg->_recordList.size() < 1)
    addNew = true;
  else {
    rec = avg->_recordList.back();
    if ((rec->create_ts + avg->_resolution) < now)
      addNew = true;
  } // else

  // do this if we're the first record
  if (addNew) {
    avg->_total += sw;
    newRecord = new SWRecord;
    newRecord->seconds = sw;
    newRecord->average = sw;
    newRecord->create_ts = now;
    newRecord->condensed = 1;
    avg->_recordList.push_back(newRecord);
//cout << "N" << avg->_recordList.size();
  } // if
  // or try and condense the record
  else {
//cout << "C" << rec->condensed;
    rec->condensed++;
    avg->_total -= rec->average;
    rec->seconds += sw;
    rec->average = ( rec->seconds / double(rec->condensed) );
    avg->_total += rec->average;
  } // else

  av = (avg->_total / double(avg->_recordList.size()));

  return av;
} // Stopwatch::_average

const double Stopwatch::_average(const std::string &name) {
  SWAverage *avg;
  SWAverageMapType::iterator ptr;
  SWAverage::SWRecordListType::iterator recordPtr;
  double av = -1.0;

  _expire();

  ptr = averageMap.find(name);
  if (ptr == averageMap.end())
    return av;

  avg = ptr->second;

  av = (avg->_total / double(avg->_recordList.size()));

  return av;
} // Stopwatch::_average


}

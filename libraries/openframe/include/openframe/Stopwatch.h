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

#ifndef __LIBOPENFRAME_STOPWATCH_H
#define __LIBOPENFRAME_STOPWATCH_H

#include <map>
#include <string>
#include <list>
#include <ctime>

#include "OFLock.h"

namespace openframe {

using std::string;
using std::map;
using std::list;
using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class SWRecord {
  public:
    double seconds;
    double average;
    time_t create_ts;
    unsigned long int condensed;
};

class SWAverage {
  public:
    SWAverage(const time_t expire, const time_t resolution) : _expire(expire), _total(0.0), _resolution(resolution) { }
    virtual ~SWAverage() { }

    typedef list<SWRecord *> SWRecordListType;

    friend class Stopwatch;

    const unsigned int clear() {
      SWRecord *rec;
      unsigned int numRows = _recordList.size();

      while(!_recordList.empty()) {
        rec = _recordList.front();
        delete rec;
        _recordList.pop_front();
      } // while

      return numRows;
    } // clear

  protected:
    time_t _expire;
    SWRecordListType _recordList;
    double _total;
    time_t _resolution;
};

class Stopwatch : public OFLock {
  public:
    Stopwatch();
    Stopwatch(const time_t);
    ~Stopwatch();

    typedef map<string, SWAverage *> SWAverageMapType;

    const double Start();
    const double Time();

    const bool add(const string &, const time_t);
    const bool add(const string &, const time_t, const time_t);
    const bool remove(const string &);
    const double average(const string &, const double);
    const double average(const string &);
    const double Now();
    const bool Next();
    const unsigned int expire();
    const unsigned int clear();

  private:
    const bool _add(const string &, const time_t, const time_t);
    const bool _remove(const string &);
    const double _now();
    const bool _next();
    const unsigned int _expire();
    const double _average(const string &, const double);
    const double _average(const string &);
    const unsigned int _clear();

    double start;
    SWAverageMapType averageMap;
    time_t _timeLastExpire;
    time_t _timeNextInterval;
    time_t _timeInterval;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

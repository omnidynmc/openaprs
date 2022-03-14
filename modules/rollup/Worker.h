/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, mySQL APRS Injector                                        **
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
 $Id: DCC.h,v 1.8 2003/09/04 00:22:00 omni Exp $
 **************************************************************************/

#ifndef __MODULE_ROLLUP_WORKER_H
#define __MODULE_ROLLUP_WORKER_H

#include <set>

#include <netdb.h>
#include <unistd.h>

#include "Rollup.h"
#include "Rollup_Abstract.h"

#include "openframe/OFLock.h"

namespace rollup {
  using namespace aprs;
  using openframe::OFLock;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class Worker : public Rollup_Abstract {
  public:
    Worker(const time_t, const time_t, const time_t, const time_t, const bool, const string &, const int);
    virtual ~Worker();

    // ### Type Definitions ###
    typedef map<string, string> partitionMapType;
    typedef partitionMapType::size_type partitionMapSizeType;
    typedef vector<string> optimizeType;
    typedef optimizeType::size_type optimizeSizeType;

    // ### Members ###
    void run();
    const double average(const string &name) { return _profile->average(name); }

    // ### Variables ###

  protected:
    void _writeStats();
    void _rollupWeather();
    void _rollupWeatherForCallsignId(const string &, const string &, const string &);
    void _partition();
    const partitionMapSizeType _parsePartitions(const string &, partitionMapType &);
    void _optimize();
    const optimizeSizeType _parseOptimize(const string &, optimizeType &);

  private:
    DBI_Rollup *_dbi;			// database handler
    Stopwatch *_profile;		// stopwatch profiler
    Stopwatch *_timerRollupInterval;	// interval for rollup
    Stopwatch *_timerPartitionInterval;	// interval for partition
    Stopwatch *_timerOptimizeInterval;  // optimize interval
    time_t _lastStatsTs;		// last time we printed stats
    time_t _statsInterval;		// interval to print stats
    time_t _rollupInterval;		// rollup interval
    time_t _partitionInterval;		// partition interval
    time_t _optimizeInterval;		// optimize interval
    time_t _timeNextMemcachedTry;	// next time to try memcached after error
    time_t _timeMemcachedExpire;	// time in seconds before memcached entries expire
    bool _deleteWeather;		// delete weather after rollup?
    unsigned int _numRollup;		// number of rollups performed
    string _interval;
    int _limit;
    DBI::resultSizeType _numLastSelection;	// number of last selected
}; // Worker

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

extern "C" {
} // extern

}
#endif

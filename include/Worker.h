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

#ifndef OPENAPRS_WORKER_H
#define OPENAPRS_WORKER_H

#include <set>

#include <netdb.h>
#include <unistd.h>

#include <openframe/OFLock.h>
#include <openframe/Stopwatch.h>
#include <openframe/scoped_lock.h>
#include <openstats/StatsClient_Interface.h>

#include "APRS.h"
#include "OpenAPRS_Abstract.h"
#include "md5wrapper.h"

namespace openaprs {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
class MemcachedController;
class Insert;
class DBI;
class Feed;
class Store;

class Work {
  public:
    Work(time_t timestamp, const std::string packet) : _timestamp(timestamp), _packet(packet) { }

    const std::string packet() const { return _packet; }
    const time_t timestamp() const { return _timestamp; }

  private:
    time_t _timestamp;
    std::string _packet;
};

class Result {
  public:
    Result(const std::string &packet, const time_t now) :
           _aprs(NULL), _duplicate(false), _ack(false), _reject(false),
           _retain(false), _parseTime(0.0), _packet(packet), _timestamp(now) { }
    virtual ~Result() {
      if (_aprs != NULL && !retain())
        delete _aprs;
    } // Result

    friend class Worker;

    const time_t timestamp() const { return _timestamp; }
    const double parseTime() const { return _parseTime; }
    const std::string error() const { return _error; }
    const std::string packet() const { return _packet; }
    const bool duplicate() const { return _duplicate; }
    const bool reject() const { return _reject; }
    const bool retain() const { return _retain; }
    const bool ack() const { return _ack; }
    aprs::APRS *aprs() const { return _aprs; }

  private:
    aprs::APRS *_aprs;
    bool _duplicate;
    bool _ack;
    bool _reject;
    bool _retain;
    double _parseTime;
    std::string _packet;
    std::string _error;
    time_t _timestamp;
};

class Worker : public OpenAPRS_Abstract,
               public openstats::StatsClient_Interface {
  public:
    static const char *kStompDestErrors;
    static const char *kStompDestRejects;
    static const char *kStompDestDuplicates;

    Worker(const std::string &id,
           const std::string &stomp_hosts,
           const std::string &stomp_login,
           const std::string &stomp_passcode,
           const time_t, const time_t, const bool, const time_t);
    virtual ~Worker();
    Worker &init();
    void try_stats();
    void onDescribeStats();
    void onDestroyStats();

    // ### Type Definitions ###
    typedef set<Work *> workSetType;
    typedef set<Result *> resultSetType;

    // ### Members ###
    const bool run();
    const double average(const std::string &name) { return _profile->average(name); }
    void timeMessageSessionExpire(const time_t seconds) { _timeMessageSessionExpire = seconds; }
    const time_t timeMessageSessionExpire() const { return _timeMessageSessionExpire; }

    // stats
    const unsigned int numPackets() const { return _numPackets; }

    void add(Work *);
    const workSetType::size_type clear();
    const workSetType::size_type workSize() const { return _workSet.size(); }

    const resultSetType::size_type resultSize() const { return _resultSet.size(); }
    const resultSetType::size_type results(resultSetType &, const int);

    // ### Variables ###

  protected:
    void try_stompstats();
    Result *parse(Work *);
    void processResult(Result *);
    void writeStats();
    void checkForDuplicates(Result *);
    void checkForPositionErrors(Result *);
    void checkForMessageAcks(Result *);
    void insertSqlFromQueue();
    void post_error(const char *dest, const std::string &packet, const Result *result);

  private:
    DBI *_dbi;				// database handler
    Insert *_sqlq;			// SQL insert queue
    MemcachedController *_memcached;	// memcached controller instance
    openframe::Stopwatch *_profile;		// stopwatch profiler
    workSetType _workSet;		// work set
    resultSetType _resultSet;		// result set
    openframe::OFLock _resultSet_l;		// mutex for result set
    openframe::OFLock _workSet_l;			// mutex for workset
    std::string _id;
    std::string _stomp_hosts;
    std::string _stomp_login;
    std::string _stomp_passcode;
    bool _useMemcached;			// should we use memcached for dups?
    unsigned int _numPackets;		// num packets
    unsigned int _numSqlInserted;	// num of successful sql insert
    unsigned int _numSqlFailed;		// num failed sql inserts
    time_t _insertInterval;		// time in seconds before sql inserts happen
    time_t _lastInsertTs;		// last time we inserted data
    time_t _lastStatsTs;		// last time we printed stats
    time_t _statsInterval;		// interval to print stats
    time_t _timeMemcachedExpire;	// time in seconds before memcached entries expire
    time_t _timeMessageSessionExpire;	// time in seconds to timeout message sessions

    Feed *_feed;
    Store *_store;

    struct aprs_stats_t {
      unsigned int packet;
      unsigned int position;
      unsigned int message;
      unsigned int telemetry;
      unsigned int status;
      unsigned int capabilities;
      unsigned int peet_logging;
      unsigned int weather;
      unsigned int dx;
      unsigned int experimental;
      unsigned int beacon;
      unsigned int unknown;
      unsigned int reject_invparse;
      unsigned int reject_duplicate;
      unsigned int reject_tosoon;
      unsigned int reject_tofast;
    }; // aprs_stats_t

    struct obj_stompstats_t {
      aprs_stats_t aprs_stats;
      time_t report_interval;
      time_t last_report_at;
      time_t created_at;
    } _stompstats;
    void init_stats(obj_stompstats_t &stats, const bool startup = false);
}; // Worker

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
}
#endif

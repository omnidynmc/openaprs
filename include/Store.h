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

#ifndef OPENAPRS_STORE_H
#define OPENAPRS_STORE_H

#include "OpenAPRS_Abstract.h"
#include "md5wrapper.h"

#include <openframe/openframe.h>
#include <openstats/StatsClient_Interface.h>

namespace openaprs {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
class MemcachedController;

class Store : public OpenAPRS_Abstract,
               public openstats::StatsClient_Interface {
  public:
    static const time_t kDefaultReportInterval;

    Store(std::string memcached_host,
          const time_t expire_interval,
          const time_t report_interval=kDefaultReportInterval);
    virtual ~Store();
    Store &init();
    void onDescribeStats();
    void onDestroyStats();

    void try_stats();

    bool getCallsignId(const std::string &source, std::string &ret_id);
    bool getPathId(const std::string &hash, std::string &);
    bool getStatusId(const std::string &hash, std::string &);
    bool getPacketId(const std::string &callsignId, std::string &ret_id);
    bool getDuplicateFromMemcached(const std::string &hash, std::string &buf);
    bool setDuplicateInMemcached(const std::string &hash, const std::string &buf);
    bool getPositionFromMemcached(const std::string &hash, std::string &buf);
    bool setPositionInMemcached(const std::string &hash, const std::string &buf);
    bool getAckFromMemcached(const std::string &hash, std::string &buf);
    bool setAckInMemcached(const std::string &hash, const std::string &buf, const time_t expire);

    // ### Variables ###

  protected:
    void try_stompstats();
    bool isMemcachedOk() const { return _last_cache_fail_at < time(NULL) - 60; }
    bool getCallsignIdFromMemcached(const std::string &source, std::string &ret_id);
    bool setCallsignIdInMemcached(const std::string &source, const std::string &id);
    bool getPathIdFromMemcached(const std::string &hash, std::string &ret_id);
    bool setPathIdInMemcached(const std::string &hash, const std::string &id);
    bool getStatusIdFromMemcached(const std::string &hash, std::string &ret_id);
    bool setStatusIdInMemcached(const std::string &hash, const std::string &id);

  private:
    DBI *_dbi;				// database handler
    MemcachedController *_memcached;	// memcached controller instance
    openframe::Stopwatch *_profile;

    std::string _memcached_host;
    time_t _expire_interval;
    time_t _last_cache_fail_at;

    struct memcache_stats_t {
      unsigned int hits;
      unsigned int misses;
      unsigned int tries;
      unsigned int stored;
    }; // memcache_stats_t

    struct sql_stats_t {
      unsigned int hits;
      unsigned int misses;
      unsigned int tries;
      unsigned int inserted;
      unsigned int failed;
    };

    struct obj_stats_t {
      memcache_stats_t cache_store;
      memcache_stats_t cache_ack;
      memcache_stats_t cache_callsign;
      memcache_stats_t cache_packet;
      memcache_stats_t cache_path;
      memcache_stats_t cache_status;
      memcache_stats_t cache_duplicates;
      memcache_stats_t cache_positions;
      sql_stats_t sql_store;
      sql_stats_t sql_ack;
      sql_stats_t sql_callsign;
      sql_stats_t sql_packet;
      sql_stats_t sql_path;
      sql_stats_t sql_status;
      time_t last_report_at;
      time_t report_interval;
      time_t created_at;
    } _stats;
    obj_stats_t _stompstats;

    void init_stats(obj_stats_t &stats, const bool startup=false);
}; // Store

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
}
#endif

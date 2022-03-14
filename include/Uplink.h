/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
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
 $Id: Server.h,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_UPLINK_H
#define __OPENAPRS_UPLINK_H

#include <string>
#include <map>
#include <set>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "Cfg.h"
#include "DBI.h"
#include "OpenAPRS_Abstract.h"
#include "ThreadMessage.h"

#include <openframe/Stopwatch.h>
#include <openframe/StringToken.h>
#include <openstats/StatsClient_Interface.h>

namespace openaprs {
  using openframe::Stopwatch;
  using openframe::StringToken;
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

  class UplinkPacket {
    public:
      UplinkPacket(const time_t created_at, const std::string &str) : _created_at(created_at), _str(str) { }
      virtual ~UplinkPacket() { }

      time_t created_at() const { return _created_at; }
      std::string str() const { return _str; }

    private:
      time_t _created_at;
      std::string _str;
  }; // UplinkPacket

  class Uplink_Exception : public OpenAPRSAbstract_Exception {
    public:
      Uplink_Exception(const string message) throw() : OpenAPRSAbstract_Exception(message) {
      } // Uplink_Exception

    private:
  }; // class Uplink_Exception

  class Decay;
  class Feed;
  class Store;
  class Uplink : public openframe::OpenFrame_Abstract,
                 public openstats::StatsClient_Interface {
    public:
      static const time_t kLogstatsInterval;
      static const time_t kDecayRetry;
      static const time_t kDecayTimeout;
      static const size_t kAckLimit;

      Uplink(const std::string &id,
             const std::string &hosts,
             const string &vhost,
             const string &stomp_hosts,
             const string &stomp_login,
             const string &stomp_passcode,
             const bool use_drop_dups);
      virtual ~Uplink();
      Uplink &init();
      void stop();
      void onDescribeStats();
      void onDestroyStats();

      // ### Type Definitions ###
      typedef std::list<UplinkPacket *> queueListType;

      // ### Variables ###

      // ### Configures ###
      inline Uplink &set_logstats_interval(const time_t value) {
        _logstats_interval = value;
        return *this;
      } // set_logstats_interval

      inline Uplink &set_decay_retry(const time_t value) {
        _decay_retry = value;
        return *this;
      } // set_decay_retry

      inline Uplink &set_decay_timeout(const time_t value) {
        _decay_timeout = value;
        return *this;
      } // set_decay_timeout

      // ### Members ###
      unsigned int send(const char *sendFormat, ...);
      void create(const bool create) { _create = create; }
      const bool create() const { return _create; }
      const string &vhost() const { return _vhost; }
      void burst(const string &filename) { _burst = filename; }
      const string &burst() const { return _burst; }
      const time_t timeConnected() const { return _timeConnected; }
      const time_t timeout() const { return _timeout; }
      const time_t interval() const { return _intervalConnect; }
      void interval(const time_t intervalConnect) { _intervalConnect = intervalConnect; }
      const bool run();
      const time_t elapsed() { return (_timeConnected ? (time(NULL) - _timeConnected) : 0); }
      const double numBytesPerSecond() const { return _numBytesPerSecond; }
      const queueListType::size_type dequeue(queueListType &, const int);
      void message(ThreadMessage *);

      // stats
      const queueListType::size_type queueSize() const { return _queueList.size(); }
      const unsigned int bytesRead() const { return _totalBytesRead; }
      const double bps() const { return _numBytesPerSecond; }
      const double kbps() const { return (_numBytesPerSecond / 1024.0); }
      const double pps() const { return _numPacketsPerSecond; }
      const double average(const string &name) { return _profile->average(name); }
      const unsigned int totalPackets() const { return _totalPackets; }

    protected:
      void try_logstats();
      bool try_ack(const bool force=false);
      const unsigned int _createPositions();
      const unsigned int _createObjects();
      const unsigned int _createMessages();
      void _decayNext();
      void _processMessage(ThreadMessage *);
      const bool _setMessageSessionInMemcached(const string &);

    private:
      DBI *_dbi;			// dbi handler
      Decay *_decay;			// decay alg
      Feed *_feed;			// stomp feed
      Stopwatch *_profile;		// average profile
      Store *_store;
      std::string _id;
      std::string _hosts;		// stomp host list
      bool _create;			// enable creation
      double _numBytesPerSecond;	// bps
      double _numPacketsPerSecond;	// pps
      queueListType _queueList;		// output queue
      size_t _queueSize;
      std::string _vhost;		// vhost to use
      std::string _stomp_hosts;
      std::string _stomp_login;
      std::string _stomp_passcode;
      std::string _burst;		// burst file
      time_t _timeout;			// server timeout
      time_t _timeConnected;		// time server connected
      time_t _timeLastRead;		// last time read data from server
      time_t _intervalConnect;		// interval to reconnect
      time_t _timeNextConnect;		// next time to try to connect
      time_t _last_logstats;		// last time logstats ran
      time_t _logstats_interval;	// logstats output interval
      time_t _decay_retry;		// decay retry
      time_t _decay_timeout;		// decay timeout
      bool _useMemcached;		// enable memcached
      unsigned int _totalBytesRead;	// number bytes read from server
      unsigned int _totalPackets;	// number of packets (lines) read

      std::string _last_message_id;	// last received message id for acking
      time_t _last_ack;
      unsigned int _num_messages;
      unsigned int _num_frames_in;
      unsigned int _num_frames_out;
  };

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openaprs
#endif

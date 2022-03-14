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

#ifndef __MODULE_DCC_WORKER_H
#define __MODULE_DCC_WORKER_H

#include <set>

#include <netdb.h>
#include <unistd.h>

#include <openframe/OFLock.h>
#include <openstats/StatsClient_Interface.h>

#include "APRS.h"
#include "DCC_Command.h"
#include "DCC_Abstract.h"
#include "MemcachedController.h"
#include "Network.h"
#include "Vars.h"

#include "md5wrapper.h"

namespace dcc {
  using openaprs::MemcachedController;
  using openaprs::Vars;
  using openframe::OFLock;
  using namespace aprs;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
class DCCUser;
class Fields;
class Symbols;

class Work {
  public:
    enum workEnumType {
      WORK_CONNECTION,
      WORK_MESSAGE,
      WORK_SEND,
      WORK_SEND_ALL,
      WORK_WALLOPS,
      WORK_WALLOPS_ALL,
      WORK_WALLOPS_REPLY_ALL
    };

    Work(Connection *connection, const time_t timestamp) :
         _connection(connection), _timestamp(timestamp), _type(WORK_CONNECTION) { }
    Work(const string &to, const string &message, const workEnumType type, const time_t timestamp) :
         _connection(NULL), _message(message),_timestamp(timestamp),
         _type(type) { assert(type == WORK_SEND || type == WORK_MESSAGE || type == WORK_WALLOPS); }
    Work(const string &message, const workEnumType type, const time_t timestamp) :
         _connection(NULL), _message(message),_timestamp(timestamp),
         _type(type) { assert(type == WORK_SEND_ALL || type == WORK_WALLOPS_ALL || type == WORK_WALLOPS_REPLY_ALL); }
    ~Work() { }

    Connection *connection() const { return _connection; }
    const string message() const { return _message; }
    const time_t timestamp() const { return _timestamp; }
    const workEnumType type() const { return _type; }
    const string to() const { return _to; }

  private:
    Connection *_connection;		// connection
    string _to;				// who message is for
    string _message;			// message for user
    time_t _timestamp;			// timestamp of work
    workEnumType _type;			// work type
};

class Worker : public DCC_Abstract,
               public openstats::StatsClient_Interface {
  public:
    Worker(DCC_Log *, const time_t, const bool, const time_t, const time_t);
    virtual ~Worker();
    void onDescribeStats();
    void onDestroyStats();

    // ### Type Definitions ###
    typedef set<Work *> workSetType;
    typedef set<DCCUser *> resultSetType;
    typedef set<DCCUser *> dccuserSetType;
    typedef map<string, Server::eventId> eventMapType;

    // ### Members ###
    void run();
    const double average(const string &name) { return _profile->average(name); }
    void clearWork();
    void clearUsers();
    void send(const string &);
    void sendTo(const string &, const string &);
    void sendToLive(Vars &, const string &);
    void wallops(const string &);
    void message(const string &, const string &);
    const bool wallopsReply(const string &);
    void send(const int, const string &);
    Fields *fields() { return _fields; }

    void add(Work *);
    const workSetType::size_type clear();
    const workSetType::size_type workSize() const { return _workSet.size(); }
    const resultSetType::size_type results(resultSetType &, const int);
    const resultSetType::size_type resultSize() const { return _resultSet.size(); }

    // setters
    Worker &maint_mode(const bool maint) {
      _maint = maint;
      return *this;
    } // Worker
    const bool maint_mode() const { return _maint; }

    // ### Variables ###

  protected:
    void _initializeSystem();
    void _deinitializeSystem();
    void _initializeEvents();
    void _deinitializeEvents();
    void _writeStats();
    void _process(DCCUser *);
    void _processWork(Work *);
    void _send(const string &);
    void _sendTo(const string &, const string &);
    void _sendToLive(Vars &, const string &);
    void _wallops(const string &);
    void _message(const string &, const string &);
    const bool _wallopsReply(const string &);
    void _clearWork();
    void _clearUsers();

  private:
    OFLock _dccuserSet_l;		// user set mutex
    DCC_Command *_command;		// command parser
    DBI *_dbi;				// database handler
    Fields *_fields;			// fields
    MemcachedController *_memcached;	// memcached controller instance
    Network *_network;			// network
    Reply *_reply;			// replies
    Stopwatch *_profile;		// stopwatch profiler
    Symbols *_symbols;			// symbols
    bool _useMemcached;			// should we use memcached for dups?
    bool _maint;			// maintenance mode?
    dccuserSetType _dccuserSet;		// dcc user set
    eventMapType _eventMap;		// event map of ids
    workSetType _workSet;		// work set
    resultSetType _resultSet;		// work set
    time_t _lastStatsTs;		// last time we printed stats
    time_t _statsInterval;		// interval to print stats
    time_t _timeNextMemcachedTry;	// next time to try memcached after error
    time_t _timeMemcachedExpire;	// time in seconds before memcached entries expire
    time_t _timeout;			// client timeout
    unsigned int _totalUsersServed;	// total users served
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

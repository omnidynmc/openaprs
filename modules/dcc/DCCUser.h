/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** HAL9000, Internet Relay Chat Bot                                     **
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
 $Id: DCCUser.h,v 1.8 2005/02/01 22:10:38 omni Exp $
 **************************************************************************/

#ifndef __MODULE_DCC_DCCUSER_H
#define __MODULE_DCC_DCCUSER_H

#include <string>
#include <list>

#include <time.h>
#include <ctype.h>

#include <openframe/OFLock.h>
#include <openframe/VarController.h>
#include <openstats/StatsClient_Interface.h>

#include "DBI.h"
#include "DCC_Abstract.h"
#include "Fields.h"
#include "LineBuffer.h"
#include "MemcachedController.h"
#include "Options.h"
#include "Timer.h"

#include "noCaseCompare.h"

#include "openaprs.h"

namespace dcc {
  using namespace std;
  using openaprs::DBI;
  using openaprs::MemcachedController;
  using openframe::OFLock;
  using openframe::VarController;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
class Reply;
class Symbols;
class DCC_Command;
class Worker;

class DCCUser : public DCC_Abstract,
                public OFLock,
                public openstats::StatsClient_Interface {
  public:
    DCCUser(Worker *, const int, const std::string &, DBI *, MemcachedController *, DCC_Command *, Reply *, Fields *, Symbols *,
            const time_t, const time_t);
    virtual ~DCCUser();
    void onDescribeStats();
    void onDestroyStats();

    /**********************
     ** Type Definitions **
     **********************/
    typedef unsigned int flagType;
    typedef Log::logId logId;
    typedef list<logId> logListType;
    typedef map<unsigned int, string> fieldsMapType;

    /***************
     ** Variables **
     ***************/
    static const flagType FLAG_LOGIN;
    static const flagType FLAG_REMOVE;
    static const flagType FLAG_LISTEN;
    static const flagType FLAG_ADMIN;
    static const flagType FLAG_MESSAGING;
    static const flagType FLAG_VERIFIED;
    static const flagType FLAG_LIVE;
    static const flagType FLAG_NMEA;
    static const flagType FLAG_GUEST;
    static const flagType FLAG_LOCAL;

    /*********************
     ** Connect Members **
     *********************/

    void Connect();
    void Kill(const std::string &);
    void chkFlood();
    const bool chkTimeout();
    const bool maint_mode();

    /******************
     ** Flag Members **
     ******************/

    const bool addFlag(const flagType addMe) {
      if (_flags & addMe)
        return false;

      _flags |= addMe;

      return true;
    } // addFlag

    const bool removeFlag(const flagType removeMe) {
      if (!(_flags & removeMe))
        return false;

      _flags &= ~removeMe;

      return true;
    } // removeFlag

    const bool findFlag(const flagType findMe) const {
      return _flags & findMe ? true : false;
    } // findFlag

    const unsigned int getFlags(string &);

    /******************
     ** Line Members **
     ******************/

    void addLine(const std::string addMe)
      { aLine.add(addMe); }

    void flushLine(const std::string addMe)
      { aLine.flush(); }

    const bool readLine(string &returnMe)
      { return aLine.readLine(returnMe); }

    /*******************
     ** OnLog Members **
     *******************/

    const int OnLog(Log *, const logId, const std::string &);

    const unsigned int Parse(const std::string &);
    void Quit(const std::string &);
    const bool Login(const std::string &, const std::string &, const std::string &);
    void Logout();
    void Welcome();
    const unsigned int sendf(const char *, ...);
    const unsigned int send(const std::string &);
    const time_t Pong() {
      _timePong = time(NULL);

      return _timePong;
    } // Pong

    const std::string seed() const { return _seed; }
    const time_t timeFlood() const { return _timeFlood; }
    void timeFlood(const time_t setMe) { _timeFlood = setMe; }
    const int socket() const { return _socket; }
    void dbi(DBI *dbi) { _dbi = dbi; }

    void callsign(const std::string &callsign) { _callsign = callsign; }
    const std::string callsign() const { return _callsign; }
    void client(const std::string &client) { _client = client; }
    const std::string client() const { return _client; }
    void id(const std::string &id) { _id = id; }
    const std::string id() const { return _id; }

    // return ts
    const time_t timePing() const { return _timePing; }
    const time_t timePong() const { return _timePong; }

    VarController *var() { return _var; }

    const int OnTimer(Timer *);

    const std::string ip() const { return _ip; }
    const int bytes() const { return _bytes; }
    const bool reply(const std::string &);
    const bool setMessageSessionInMemcached(const std::string &);
    const unsigned int fieldMe(DBI::setMapType &, fieldsMapType &, const std::string &);
    const std::string fieldMe(DBI::resultMapType &, const std::string &);
    const bool findFieldByName(const std::string &name, std::string &token, const std::string &filter) {
      return _fields->findByName(name, token, filter);
    } // findFieldByName

    const bool findFieldByName(const std::string &name, std::string &token) {
      return _fields->findByName(name, token);
    } // findFieldByName

  protected:
    // set the users bytes
    void __bytes(const int setMe) { _bytes = setMe; }
    void _incBytes(const int setMe) { _bytes = _bytes + setMe; }
    const bool _getLoginFromMemcached(const std::string &, const std::string &, DBI::resultMapType &);
    const bool _setLoginInMemcached(const std::string &, const std::string &, DBI::resultMapType &);
    const bool _isMemcachedOk() { return (_memcached != NULL); }

  private:
    DCC_Command *_command;		// command tree
    DBI *_dbi;				// dbi connection
    Fields *_fields;			// fields
    MemcachedController *_memcached;	// memcached instance
    LineBuffer aLine;			// line
    Reply *_reply;			// common replies
    Symbols *_symbols;			// common symbols
    VarController *_var;		// variable controller
    Worker *_worker;			// callsign worker
    flagType _flags;			// flags
    int _bytes;				// number of bytes
    int _socket;			// client socket
    std::string _client;			// client type
    std::string _id;				// Id login gave.
    std::string _ip;				// ip address
    std::string _callsign;			// my nick
    std::string _seed;			// Seed hash for connection.
    time_t _timeCreated;		// timestamp
    time_t _timePing;			// Ping TS
    time_t _timePong;			// Pong TS
    time_t _timeFlood;			// last flood check
    time_t _timeout;			// ping timeout time
};
/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

#define IsDCCUserAdmin(x)		x->findFlag(DCCUser::FLAG_ADMIN)
#define IsDCCUserGuest(x)		x->findFlag(DCCUser::FLAG_GUEST)
#define IsDCCUserListen(x)		x->findFlag(DCCUser::FLAG_LISTEN)
#define IsDCCUserLogin(x)		x->findFlag(DCCUser::FLAG_LOGIN)
#define IsDCCUserMessaging(x)		x->findFlag(DCCUser::FLAG_MESSAGING)
#define IsDCCUserRemove(x)		x->findFlag(DCCUser::FLAG_REMOVE)
#define IsDCCUserVerified(x)		x->findFlag(DCCUser::FLAG_VERIFIED)
#define IsDCCUserLive(x)		x->findFlag(DCCUser::FLAG_LIVE)
#define IsDCCUserNMEA(x)		x->findFlag(DCCUser::FLAG_NMEA)
#define IsDCCUserLocal(x)		x->findFlag(DCCUser::FLAG_LOCAL)

#define IsDCCUserNormal(x)              (IsDCCUserListen(x) == false && IsDCCUserRemove(x) == false)

#define SetDCCUserAdmin(x)		x->addFlag(DCCUser::FLAG_ADMIN)
#define SetDCCUserGuest(x)		x->addFlag(DCCUser::FLAG_GUEST)
#define SetDCCUserListen(x)		x->addFlag(DCCUser::FLAG_LISTEN)
#define SetDCCUserLogin(x)		x->addFlag(DCCUser::FLAG_LOGIN)
#define SetDCCUserMessaging(x)		x->addFlag(DCCUser::FLAG_MESSAGING)
#define SetDCCUserRemove(x)		x->addFlag(DCCUser::FLAG_REMOVE)
#define SetDCCUserVerified(x)		x->addFlag(DCCUser::FLAG_VERIFIED)
#define SetDCCUserLive(x)		x->addFlag(DCCUser::FLAG_LIVE)
#define SetDCCUserNMEA(x)		x->addFlag(DCCUser::FLAG_NMEA)
#define SetDCCUserLocal(x)		x->addFlag(DCCUser::FLAG_LOCAL)

#define UnsetDCCUserAdmin(x)		x->removeFlag(DCCUser::FLAG_ADMIN)
#define UnsetDCCUserGuest(x)		x->removeFlag(DCCUser::FLAG_GUEST)
#define UnsetDCCUserListen(x)		x->removeFlag(DCCUser::FLAG_LISTEN)
#define UnsetDCCUserLogin(x)		x->removeFlag(DCCUser::FLAG_LOGIN)
#define UnsetDCCUserMessaging(x)	x->removeFlag(DCCUser::FLAG_MESSAGING)
#define UnsetDCCUserRemove(x)		x->removeFlag(DCCUser::FLAG_REMOVE)
#define UnsetDCCUserVerified(x)		x->removeFlag(DCCUser::FLAG_VERIFIED)
#define UnsetDCCUserLive(x)		x->removeFlag(DCCUser::FLAG_LIVE)
#define UnsetDCCUserNMEA(x)		x->removeFlag(DCCUser::FLAG_NMEA)
#define UnsetDCCUserLocal(x)		x->removeFlag(DCCUser::FLAG_LOCAL)

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

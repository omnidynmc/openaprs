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

#ifndef __MODULE_DCCCLASS_H
#define __MODULE_DCCCLASS_H

#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>

#include "DCC_Abstract.h"
#include "DCC_Log.h"
#include "DCCUser.h"
#include "Fields.h"
#include "Log.h"
#include "Reply.h"
#include "Server.h"
#include "Symbols.h"
#include "Timer.h"
#include "Vars.h"

#include <openframe/OFLock.h>
#include <openstats/StatsClient_Interface.h>

#include "noCaseCompare.h"

namespace dcc {
  using openaprs::LogHandler;
  using openaprs::TimerHandler;
  using openframe::OFLock;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define kDefaultResultLimit	200

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
class Work;
class DCC : public DCC_Abstract,
            public LogHandler,
            public TimerHandler,
            public OFLock,
            public openstats::StatsClient_Interface {
  public:
    DCC(DCC_Log *);
    virtual ~DCC();
    void onDescribeStats();
    void onDestroyStats();

    // ### Type Definitions ###
    typedef Timer::timerId timerId;
    typedef Log::logId logId;
    typedef map<string, Server::eventId> onEventMapType;
    typedef list<Work *> workListType;
    typedef map<pthread_t, workListType *> workMapType;
    typedef set<dcc::Work *> resultSetType;
    typedef map<string, logId> onLogMapType;
    typedef set<pthread_t> threadSetType;

    const bool initializeLogs();
    const bool initializeTimers();
    const bool initializeSystem();
    const bool initializeThreads();
    const bool initializeEvents();
    const bool deinitializeLogs();
    const bool deinitializeSystem();
    const bool deinitializeTimers();
    const bool deinitializeEvents();
    const bool deinitializeThreads();

    static void *WorkerThread(void *);
    static void *ListenerThread(void *);

    void done() {
      Lock();
      _done = true;
      Unlock();
    } // done

    const bool isDone() {
      bool ret;

      Lock();
      ret = _done;
      Unlock();

      return ret;
    } // isDone

    const int OnLog(Log *, const logId, const string &);
    const int OnTimer(Timer *);

    const unsigned int Send(const char *, ...);
    const unsigned int wallopsf(const char *, ...);
    const unsigned int wallops(const string &);
    const unsigned int wallopsReply(const string &);

    void connected(Connection *);
    void addWorkToThread(const pthread_t, dcc::Work *);
    void addWork(dcc::Work *);
    const bool work(pthread_t, workListType &, const int);
    void clearWork();
    const threadSetType::size_type workers(threadSetType &);
    void clearResults();
    const workListType::size_type workSize(pthread_t tid) {
      workListType::size_type ret;
      _workMap_l.Lock();
      ret = _workSize(tid);
      _workMap_l.Unlock();
      return ret;
    } // workSize

    Fields *fields();

  protected:
    const bool _work(pthread_t, workListType &, const int);
    void _clearResults();
    void _addWork(dcc::Work *);
    void _addWorkToThread(const pthread_t, dcc::Work *);
    void _clearWork();
    const workListType::size_type _workSize(pthread_t tid) {
      workMapType::iterator ptr;

      ptr = _workMap.find(tid);
      if (ptr == _workMap.end())
        return 0;

      return ptr->second->size();
    } // workSize

  private:
    Fields *_fields;
    OFLock _workMap_l;
    OFLock _resultSet_l;
    bool _done;
    onLogMapType myOnLogList;		// List of Log Ids.
    onEventMapType myOnEventMap;
    resultSetType _resultSet;		// set of results
    threadSetType _workerThreads;	// worker thread set
    pthread_t _listenerThread_tid;	// listener thread
    workMapType _workMap;		// set of work to be done
    unsigned int _numLastThread;	// last thread work was sent to
}; // DCC

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

extern "C" {
  extern DCC *dcc;				// declare global variable for DCC
} // extern

}
#endif

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

#ifndef __MODULE_CLASS_APNS_H
#define __MODULE_CLASS_APNS_H

#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>

#include "ApnsMessage.h"
#include "Log.h"
#include "Server.h"
#include "Timer.h"

#include "openframe/OFLock.h"
#include "openframe/Stopwatch.h"

#include "noCaseCompare.h"

namespace apns {
  using openaprs::LogHandler;
  using openaprs::TimerHandler;
  using openframe::OFLock;
  using openframe::Stopwatch;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class APNS : public LogHandler, public TimerHandler, public OFLock {
  public:
    APNS();
    virtual ~APNS();

    /**********************
     ** Type Definitions **
     **********************/

    typedef Timer::timerId timerId;
    typedef Log::logId logId;
    typedef map<string, Server::eventId> onEventMapType;
    typedef map<string, logId> onLogMapType;
    typedef set<ApnsMessage *> messageQueueType;
    typedef set<pthread_t> threadSetType;

    /***************
     ** Variables **
     ***************/

    /***********************************************
     ** Initialization / Deinitialization Members **
     ***********************************************/

    const bool initializeLogs();
    const bool initializeTimers();
    const bool initializeSystem();
    const bool initializeSsl();
    const bool initializeThreads();
    const bool initializeCommands();
    const bool initializeEvents();
    const bool deinitializeLogs();
    const bool deinitializeSystem();
    const bool deinitializeThreads();
    const bool deinitializeTimers();
    const bool deinitializeCommands();
    const bool deinitializeEvents();

    /*******************
     ** OnLog Members **
     *******************/

    const int OnLog(Log *, const logId, const string &);

    /*********************
     ** OnTimer Members **
     *********************/

    const int OnTimer(Timer *);

    void Push(ApnsMessage *);
    const unsigned int getPushQueue(messageQueueType &, const int limit);
    const unsigned int getPushDevelQueue(messageQueueType &, const int limit);
    static void *SslThread(void *);
    static void *SslDevelThread(void *);
    static void *FeedbackThread(void *);

    const bool die() {
      bool die;

      Lock();
      die = _die;
      Unlock();

      return die;
    } // die()

    void die(const bool die) {
      Lock();
      _die = die;
      Unlock();
    } // die

    Stopwatch profile;

    const unsigned int numSslThreads() {
      unsigned int ret = 0;

      _numSslThreads_l.Lock();
      ret = _numSslThreads++;
      _numSslThreads_l.Unlock();

      return ret;
    } // numThreads

    const unsigned int numSslDevelThreads() {
      unsigned int ret = 0;

      _numSslDevelThreads_l.Lock();
      ret = _numSslDevelThreads++;
      _numSslDevelThreads_l.Unlock();

      return ret;
    } // numThreads

    const unsigned int numFeedbackThreads() {
      unsigned int ret = 0;

      _numFeedbackThreads_l.Lock();
      ret = _numFeedbackThreads++;
      _numFeedbackThreads_l.Unlock();

      return ret;
    } // numFeedbackThreads

  protected:
    const unsigned int _updateRegister();
  private:
    bool _die;
    onLogMapType myOnLogList;			// List of Log Ids.
    onEventMapType myOnEventMap;
    threadSetType _sslThreads;			// ssl thread ids
    unsigned int _numSslThreads;		// number of threads to count
    OFLock _numSslThreads_l;			// mutex for threadcount
    unsigned int _numSslDevelThreads;		// number of threads to count
    OFLock _numSslDevelThreads_l;		// mutex for threadcount
    unsigned int _numFeedbackThreads;		// number of threads to count
    OFLock _numFeedbackThreads_l;		// mutex for threadcount
    messageQueueType _messageQueue;		// message queue for apns push
    messageQueueType _messageDevelQueue;	// message queue for apns push devel
}; // APNS

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

extern "C" {
  extern APNS *aAPNS;				// declare global variable for APNS
} // extern

}
#endif

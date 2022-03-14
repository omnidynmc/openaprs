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
 $Id: APNS.cpp,v 1.12 2003/09/05 22:23:41 omni Exp $
 **************************************************************************/

#include <fstream>
#include <string>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <list>
#include <map>
#include <new>
#include <iostream>
#include <fstream>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include "Rollup_Log.h"
#include "Cfg.h"
#include "Rollup.h"
#include "DBI_Rollup.h"
#include "Server.h"
#include "StringTool.h"
#include "ThreadMessage.h"
#include "Worker.h"
#include "Vars.h"

#include "rollupCommands.h"
#include "stdinCommands.h"

#include "openframe/stringify.h"

#include "rollup.h"

namespace rollup {
  using namespace handler;
  using namespace putmysql;
  using namespace std;
  using aprs::StringTool;
  using openframe::stringify;

/**************************************************************************
 ** APNS Class                                                           **
 **************************************************************************/

  Rollup::Rollup(Rollup_Log *log) : _die(false) {
    logger(log);

    initializeSystem();

    return;
  } // Rollup::Rollup

  Rollup::~Rollup() {
    deinitializeSystem();

    return;
  } // Rollup::~Rollup

  /***********************************************
   ** Initialization / Deinitailization Members **
   ***********************************************/
  const bool Rollup::initializeSystem() {
    initializeLogs();
    initializeCommands();
    initializeThreads();

    _consolef("*** Module Rollup: System Initialized");
    _logf("*** Module Rollup: System Initialized");
    _rolluplogf("*** Module Rollup: System Initialized");

    return true;
  } // Rollup::initializeSystem

  const bool Rollup::initializeLogs() {
    _logList[MODULE_ROLLUP_LOG_NORMAL] = app->addLog(MODULE_ROLLUP_LOG_NORMAL, this, true);

    _consolef("*** Module Rollup: Logs Initialized");
    _logf("*** Module Rollup: Logs Initialized");
    _rolluplogf("*** Module Rollup: Logs Initialized");

    return true;
  } // Rollup::initializeLogs

  const bool Rollup::initializeCommands() {
//    app->addCommand("MGMT", "STDIN", "/MGMT", -1, Command::FLAG_CLEANUP, new stdinDTREE);
//      app->addCommand("ACTIVATE", "STDIN/MGMT", "/MGMT ACTIVATE <id|callsig|email>", -1, Command::FLAG_CLEANUP, new stdinACTIVATE);

    _logf("*** Module Rollup: Commands Initialized");
    _consolef("*** Module Rollup: Commands Initialized");
    _rolluplogf("*** Module Rollup: Commands Initialized");

    return true;
  } // Rollup::initializeCommands

  const bool Rollup::initializeThreads() {
    pthread_t newThread_tid;
    unsigned int numWorkerThreads = app->cfg->get_int("module.rollup.threads.worker", 1);

    for(unsigned int i=0; i < numWorkerThreads; i++) {
      ThreadMessage *tm = new ThreadMessage(i+1);
      tm->var->push_void("app", app);
      tm->var->push_void("rollup", this);
      pthread_create(&newThread_tid, NULL, Rollup::WorkerThread, tm);
      _consolef("*** Module Rollup: WorkerThread %u Initialized", newThread_tid);
      _logf("*** Module Rollup: WorkerThread %u Initialized", newThread_tid);
      _rolluplogf("*** Module Rollup: WorkerThread %u Initialized", newThread_tid);
      _workerThreads.insert(newThread_tid);
    } // for

    _logf("*** Module Rollup: Threads Initialized");
    _rolluplogf("*** Module Rollup: Threads Initialized");
    _consolef("*** Module Rollup: Threads Initialized");

    return true;
  } // Rollup::initializeThreads

  const bool Rollup::deinitializeCommands() {
//    app->removeCommand("STDIN/ROLLUP");

    _logf("*** Module Rollup: Commands Deinitialized");
    _rolluplogf("*** Module Rollup: Commands Deinitialized");
    _consolef("*** Module Rollup: Commands Deinitialized");

    return true;
  } // Rollup::deinitializeCommands

  const bool Rollup::deinitializeLogs() {
    app->removeLog(_logList[MODULE_ROLLUP_LOG_NORMAL]);

    _logf("*** Module Rollup: Logs Deinitialized");
    _rolluplogf("*** Module Rollup: Logs Deinitialized");
    _consolef("*** Module Rollup: Logs Deinitialized");

    return true;
  } // Rollup::deinitializeLogs

  const bool Rollup::deinitializeThreads() {
    threadSetType::iterator ptr;
    pthread_t tid;

    while(!_workerThreads.empty()) {
      ptr = _workerThreads.begin();
      tid = (*ptr);
      _consolef("*** Module Rollup: Waiting for WorkerThread %u to Deinitialized", tid);
      _logf("*** Module Rollup: Waiting for WorkerThread %u to Deinitialized", tid);
      _rolluplogf("*** Module Rollup: Waiting for WorkerThread %u to Deinitialized", tid);

      pthread_join(tid, NULL);
      _workerThreads.erase((*ptr));
    } // while

    _logf("*** Module Rollup: Threads Deinitialized");
    _consolef("*** Module Rollup: Threads Deinitialized");
    _rolluplogf("*** Module Rollup: Threads Deinitialized");

    return true;
  } // Rollup::deinitializeThreads

  const bool Rollup::deinitializeSystem() {
    die(true);
    deinitializeThreads();
    deinitializeCommands();
    deinitializeLogs();

    app->cfg->pop_match("openaprs.count.module.rollup.*");

    _logf("*** Module Rollup: System Deinitialized");
    _consolef("*** Module Rollup: System Deinitialized");
    _rolluplogf("*** Module Rollup: System Deinitialized");

    return true;
  } // Rollup::deinitializeSystem

  void *Rollup::WorkerThread(void *arg) {
    ThreadMessage *tm = (ThreadMessage *) arg;
    Server *a = (Server *) tm->var->get_void("app");
    Rollup *r = (Rollup *) tm->var->get_void("rollup");
    string id = stringify<unsigned int>(tm->id());
    Stopwatch sw;
    time_t intervalStatsCfg = 1;
    time_t timeNextStatsCfg = (time(NULL) + intervalStatsCfg);
    rollup::Worker *worker;

    try {
      worker = new rollup::Worker(
                               a->cfg->get_int("module.rollup.threads.worker.stats.interval", 3600),
                               a->cfg->get_int("module.rollup.threads.worker.rollup.interval", 10),
                               a->cfg->get_int("module.rollup.threads.worker.partition.interval", 86400),
                               a->cfg->get_int("module.rollup.threads.worker.optimize.interval", 86400),
                               a->cfg->get_bool("module.rollup.threads.worker.delete.weather", false),
                               a->cfg->get_string("module.rollup.weather.interval", "10 DAY"),
                               a->cfg->get_int("module.rollup.threads.worker.limit", 100)
                              );
      worker->logger(new Rollup_Log("Worker t:"+id));
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    while(!r->die()) {
      worker->run();
      sleep(10);
    } // while

    delete worker;
    delete tm;

    return NULL;
  } // Rollup::WorkerThread

  const int Rollup::OnLog(Log *logInfo, const logId callId, const string &logBuffer) {
    Vars v;
    string tempBuf;                               // temp local buffer
    string safeBuffer;                            // Screen safe buffer.
    stringstream s;                               // temp string stream.
    unsigned int i;                               // tmp counter.

    s.str("");

    // make the buffer safe
    for(i = 0; i < logBuffer.length(); i++) {
      if ((int) logBuffer[i] < 32 ||
          (int) logBuffer[i] > 126)
        s << "\\x" << v.char2hex(logBuffer[i]);
      else
        s << logBuffer[i];
    } // for

    safeBuffer = s.str();

    if (callId == _logList[MODULE_ROLLUP_LOG_NORMAL]) {
      /*
       * 06/20/2003: If log file isn't open, try to open it using hal's default
       *             log path. -GCARTER
       */
      if (logInfo->isLogOpen() == false)
        logInfo->openLog((OPENAPRS_PATH_LOGS + MODULE_ROLLUP_PATH_LOGS_NORMAL), ofstream::out | ofstream::app);

      logInfo->appendLog((safeBuffer + "\n"));

      if (OPENAPRS_LOG_FLUSH == true)
        logInfo->flushLog();
    } // if

    // return length of what we wrote
    return logBuffer.length();
  } // Rollup::OnLog
} // namespace rollup

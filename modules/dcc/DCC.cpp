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
 $Id: DCC.cpp,v 1.12 2003/09/05 22:23:41 omni Exp $
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

#include <pthread.h>

#include <openframe/openframe.h>

#include "DCC.h"
#include "DCC_Log.h"
#include "DCCUser.h"
#include "Fields.h"
#include "Log.h"
#include "Listener.h"
#include "Network.h"
#include "PutMySQL.h"
#include "Server.h"
#include "Timer.h"
#include "ThreadMessage.h"
#include "Worker.h"

#include "dccCommands.h"

#include "dcc.h"
#include "openaprs.h"
#include "openaprs_string.h"

namespace dcc {
  using openaprs::Server;
  using openaprs::ThreadMessage;
  using namespace handler;
  using namespace putmysql;
  using namespace std;

/**************************************************************************
 ** DCC Class                                                            **
 **************************************************************************/

  DCC::DCC(DCC_Log *log) {
    logger(log);
    _done = false;

    _fields = new Fields();
    _numLastThread = 0;

    // snag the ip address so we can set it
    //this->in.s_addr = ntohl(app->getConnectionNetworkNumber(app->getSocket()));

    // create listener and setup first user to accept
    //Listen(MODULE_DCC_PORT, MODULE_DCC_MAXCONNECTIONS);

    initializeSystem();

    return;
  } // DCC::DCC

  DCC::~DCC() {
    clearWork();
    //clearResults();

    deinitializeSystem();

    delete _fields;

    return;
  } // DCC::~DCC

  void DCC::onDescribeStats() {
    describe_stat("worker.dccuser.num.op.find.uls", "dccuser/op/num find uls", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.op.check.messages", "dccuser/op/num check messages", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.op.last.position", "dccuser/op/num last position", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.op.last.weather", "dccuser/op/num last weather", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.op.last.telemetry", "dccuser/op/num last telemetry", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.op.position.history", "dccuser/op/num position history", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.op.weather.history", "dccuser/op/num weather history", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.op.telemetry.history", "dccuser/op/num telemetry history", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.create.object", "dccuser/actions/create object", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.create.position", "dccuser/actions/create position", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.create.message", "dccuser/actions/create message", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.signup.success", "dccuser/account/signup success", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.signup.upgrade", "dccuser/account/signup upgrade", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.signup.fail", "dccuser/account/signup failure", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.login.success", "dccuser/account/login success", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("worker.dccuser.num.login.fail", "dccuser/account/login failure", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
  } // DCC::onDescribeStats

  void DCC::onDestroyStats() {
    destroy_stat("*");
  } // DCC::onDestroyStats

  /***********************************************
   ** Initialization / Deinitailization Members **
   ***********************************************/
  const bool DCC::initializeSystem() {
    initializeLogs();
    initializeTimers();
    initializeEvents();

    _dcclogf("*** Module DCC: System Initialized");
    _logf("*** Module DCC: System Initialized");
    _consolef("*** Module DCC: System Initialized");

    initializeThreads();

    return true;
  } // DCC::initializeSystem

  const bool DCC::initializeLogs() {
    myOnLogList[MODULE_DCC_LOG_NORMAL] = app->addLog(MODULE_DCC_LOG_NORMAL, this, true);
    //myOnLogList[MODULE_DCC_LOG_BOTNET_PROTOCOL] = app->addLog(MODULE_DCC_LOG_BOTNET_PROTOCOL, false, this);

    _dcclogf("*** Module DCC: Logs Initialized");
    _logf("*** Module DCC: Logs Initialized");
    _consolef("*** Module DCC: Logs Initialized");

    return true;
  } // DCC::initializeLogs

  const bool DCC::initializeEvents() {
    _dcclogf("*** Module DCC: Events Initialized");
    _logf("*** Module DCC: Events Initialized");
    _consolef("*** Module DCC: Events Initialized");

    return true;
  } // DCC::initializeEvents

  const bool DCC::deinitializeEvents() {
    _dcclogf("*** Module DCC: Events Deinitialized");
    _logf("*** Module DCC: Events Deinitialized");
    _consolef("*** Module DCC: Events Deinitialized");
    return true;
  } // DCC::deinitializeEvents

  const bool DCC::initializeTimers() {

    _dcclogf("*** Module DCC: Timers Initialized");
    _logf("*** Module DCC: Timers Initialized");
    _consolef("*** Module DCC: Timers Initialized");

    return true;
  } // DCC::initializeTimers

  /**
   * Server::initializeThreads
   *
   * Adds all commands to system.
   *
   * Returns: true if successful, false if unsuccessful.
   */
  const bool DCC::initializeThreads() {
    openframe::ConfController::matchListType matchList;
    ThreadMessage *tm;
    pthread_t newThread_tid;
    unsigned int i;
    unsigned int numWorkerThreads = app->cfg->get_int("module.dcc.threads.worker", 2);

    tm = new ThreadMessage(1);
    tm->var->push_void("app", app);
    tm->var->push_void("dcc", this);
    pthread_create(&_listenerThread_tid, NULL, DCC::ListenerThread, tm);
    _consolef("*** Module DCC: Listener %u Initialized", newThread_tid);
    _logf("*** Module DCC: Listener %u Initialized", newThread_tid);
    _dcclogf("*** Module DCC: Listener %u Initialized", newThread_tid);

    for(i=0; i < numWorkerThreads; i++) {
      tm = new ThreadMessage(i+1);
      tm->var->push_void("app", app);
      tm->var->push_void("dcc", this);
      pthread_create(&newThread_tid, NULL, DCC::WorkerThread, tm);
      _consolef("*** Module DCC: Worker %u Initialized", newThread_tid);
      _logf("*** Module DCC: Worker %u Initialized", newThread_tid);
      _dcclogf("*** Module DCC: Worker %u Initialized", newThread_tid);

      _workerThreads.insert(newThread_tid);

      _workMap_l.Lock();
      _workMap.insert( make_pair(newThread_tid, new workListType) );
      _workMap_l.Unlock();
    } // for

    _consolef("*** Module DCC: Threads Initialized");
    _logf("*** Module DCC: Threads Initialized");

    return true;
  } // DCC::initializeThreads

  const bool DCC::deinitializeSystem() {
    deinitializeThreads();
    deinitializeTimers();
    deinitializeLogs();
    deinitializeEvents();

    _dcclogf("*** Module DCC: System Deinitialized");
    _logf("*** Module DCC: System Deinitialized");
    _consolef("*** Module DCC: System Deinitialized");

    return true;
  } // DCC::deinistializeSystem

  const bool DCC::deinitializeLogs() {
    app->removeLog(myOnLogList[MODULE_DCC_LOG_NORMAL]);
    //app->removeLog(myOnLogList[MODULE_DCC_LOG_BOTNET_PROTOCOL]);

    _logf("*** Module DCC: Logs Deinitialized");
    _dcclogf("*** Module DCC: Logs Deinitialized");
    _consolef("*** Module DCC: Logs Deinitialized");

    return true;
  } // DCC::deinitializeLogs

  const bool DCC::deinitializeTimers() {
    list<string> optionList;		// List of options matched.
    string optionName;			// Name of the looped to option.

   if (app->cfg->match("openaprs.timer.dcc.*", optionList) > 0) {
      for(; !optionList.empty(); optionList.pop_front()) {
        optionName = optionList.front();

        if (app->cfg->is_int(optionName) == true) {
          app->removeTimer(app->cfg->get_int(optionName));
          app->cfg->pop(optionName);
        } // if
      } // for
    } // if

    _dcclogf("*** Module DCC: Timers Deinitialized");
    _logf("*** Module DCC: Timers Deinitialized");
    _consolef("*** Module DCC: Timers Deinitialized");

    return true;
  } // DCC::deinitializeTimers

  /**
   * DCC::deinitializeThreads
   *
   * Adds all commands to system.
   *
   * Returns: true if successful, false if unsuccessful.
   */
  const bool DCC::deinitializeThreads() {
    threadSetType::iterator ptr;
    pthread_t tid;

    done();

    _consolef("*** Module DCC: Waiting for Listener %u to Deinitialized", _listenerThread_tid);
    _logf("*** Module DCC: Waiting for Listener %u to Deinitialized",_listenerThread_tid);
    _dcclogf("*** Module DCC: Waiting for Listener %u to Deinitialized", _listenerThread_tid);
    pthread_join(_listenerThread_tid, NULL);

    while(!_workerThreads.empty()) {
      ptr = _workerThreads.begin();
      tid = (*ptr);
      _consolef("*** Module DCC: Waiting for Worker %u to Deinitialized", tid);
      _logf("*** Module DCC: Waiting for Worker %u to Deinitialized", tid);
      _dcclogf("*** Module DCC: Waiting for Worker %u to Deinitialized", tid);

      pthread_join(tid, NULL);
      _workerThreads.erase((*ptr));

      _workMap_l.Lock();
      delete _workMap[tid];
      _workMap.erase(tid);
      _workMap_l.Unlock();
    } // while

    _consolef("*** Module DCC: Threads Deinitialized");
    _logf("*** Module DCC: Threads Deinitialized");
    _dcclogf("*** Module DCC: Threads Deinitialized");

    return true;
  } // DCC::deinitializeThreads

  void *DCC::WorkerThread(void *arg) {
    ThreadMessage *tm = (ThreadMessage *) arg;
    Server *a = (Server *) tm->var->get_void("app");
    DCC *d = (DCC *) tm->var->get_void("dcc");
    bool wantSleep;
    dcc::Worker::resultSetType resultSet;
    dcc::Work *work;
    dcc::Worker *worker;
    workListType workList;
    string id = openframe::stringify<unsigned int>(tm->id());
    Stopwatch sw;
    time_t intervalStatsCfg = 1;
    time_t timeNextStatsCfg = (time(NULL) + intervalStatsCfg);
    int maxQueue = a->cfg->get_int("module.dcc.threads.worker.maxqueue", 25);
    pthread_t tid = pthread_self();

    try {
      worker = new dcc::Worker(
                               new DCC_Log("Worker t:"+id),
                               a->cfg->get_int("module.dcc.threads.worker.stats.interval", 3600),
                               a->cfg->get_int("module.dcc.threads.worker.memcached", true),
                               a->cfg->get_int("module.dcc.threads.worker.memcached.expire", 900),
                               a->cfg->get_int("module.dcc.threads.worker.client.timeout", 90)
                              );
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    bool maint_mode = app->cfg->get_bool("module.dcc.maint.enable", false);
    worker->replace_stats(d->stats(), "openaprs.dcc.worker");
    worker->maint_mode(maint_mode);

    while(!d->isDone()) {
      d->work(tid, workList, maxQueue);

      wantSleep = (workList.empty()) ? true : false;

      while(!workList.empty()) {
        work = workList.front();
        worker->add(work);
        workList.pop_front();
      } // while

      worker->run();

      if (timeNextStatsCfg < time(NULL)) {
        a->cfg->replace_uint("openaprs.count.module.dcc.threads.worker"+id+".size.work", d->workSize(tid));
        a->cfg->replace_double("openaprs.count.module.dcc.threads.worker"+id+".average.user.command", worker->average("user.command"));
        a->cfg->replace_double("openaprs.count.module.dcc.threads.worker"+id+".average.run.loop", worker->average("run.loop"));
        timeNextStatsCfg = time(NULL) + intervalStatsCfg;
      } // if

      if (wantSleep)
        usleep(10000);
    } // while

    delete worker;
    delete tm;

    return NULL;
  } // DCC::WorkerThread

  const bool DCC::work(pthread_t tid, workListType &workList, const int limit) {
    bool ret;
    _workMap_l.Lock();
    ret = _work(tid, workList, limit);
    _workMap_l.Unlock();
    return ret;
  } // DCC::work

  const bool DCC::_work(pthread_t tid, workListType &workList, const int limit) {
    workMapType::iterator ptr;
    int i = 0;

    ptr = _workMap.find(tid);
    if (ptr == _workMap.end())
      return false;

    while(!ptr->second->empty()) {
      if (i++ > limit)
        break;

      workList.push_back(ptr->second->front());
      ptr->second->pop_front();
    } // while

    return true;
  } // DCC::_work

  const DCC::threadSetType::size_type DCC::workers(threadSetType &threadSet) {
    threadSet = _workerThreads;
    return threadSet.size();
  } // DCC::workers

  void DCC::addWorkToThread(const pthread_t id, Work *work) {
    _workMap_l.Lock();
    _addWorkToThread(id, work);
    _workMap_l.Unlock();
  } // DCC::addWork

  void DCC::addWork(Work *work) {
    _workMap_l.Lock();
    _addWork(work);
    _workMap_l.Unlock();
  } // DCC::addWork

  void DCC::_addWork(Work *work) {
    workMapType::iterator ptr;
    pthread_t id;
    unsigned int i = 0;

    // probably shutting down?
    if (!_workMap.size())
      return;

    if (++_numLastThread >= _workMap.size())
      _numLastThread = 0;

    // find the worker with the least amount of work
    for(ptr = _workMap.begin(); ptr != _workMap.end(); ptr++) {
      if (i == _numLastThread) {
        id = ptr->first;
        break;
      } // if

      i++;
    } // for

    _addWorkToThread(id, work);
  } // DCC::_addWork

  void DCC::_addWorkToThread(const pthread_t id, Work *work) {
    workMapType::iterator ptr;

    ptr = _workMap.find(id);
    assert(ptr != _workMap.end());

    ptr->second->push_back(work);

  } // DCC::_addWorkToThread

  void *DCC::ListenerThread(void *arg) {
    Connection *connection;
    Listener *listener;
    ThreadMessage *tm = (ThreadMessage *) arg;
    Server *a = (Server *) tm->var->get_void("app");
    DCC *d = (DCC *) tm->var->get_void("dcc");
    string id = openframe::stringify<unsigned int>(tm->id());
    Stopwatch *profile;
    time_t intervalStatsCfg = 1;
    time_t timeNextStatsCfg = (time(NULL) + intervalStatsCfg);

    try {
      listener = new Listener(MODULE_DCC_PORT, MODULE_DCC_MAXCONNECTIONS);
      profile = new Stopwatch(1);
    } // try
    catch(Listener_Exception e) {
      app->writeLog(MODULE_DCC_LOG_NORMAL, "fatal error starting listener; %s", e.message());
      app->writeLog(OPENAPRS_LOG_NORMAL, "fatal error starting listener; %s", e.message());
      app->writeLog(OPENAPRS_LOG_STDOUT, "fatal error starting listener; %s", e.message());
      return NULL;
    } // catch

    profile->add("run.loop", 300);

    while(!d->isDone()) {
      profile->Start();
      connection = listener->listen();
      if (connection != NULL)
        d->connected(connection);

      if (profile->Next())
        a->cfg->replace_double("openaprs.count.thread.worker"+id+".run.loop", profile->average("run.loop", profile->Time()) );
    } // while

    delete profile;
    delete listener;
    delete tm;

    return NULL;
  } // DCC::ListenerThread

  void DCC::connected(Connection *connection) {
    addWork(new Work(connection, time(NULL)));
  } // connected

  /*******************
   ** OnLog Members **
   *******************/

  const int DCC::OnLog(Log *logInfo, const logId callId, const string &logBuffer) {
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

    if (callId == myOnLogList[MODULE_DCC_LOG_NORMAL]) {
      /*
       * 06/20/2003: If log file isn't open, try to open it using hal's default
       *             log path. -GCARTER
       */
      if (logInfo->isLogOpen() == false)
        logInfo->openLog((OPENAPRS_PATH_LOGS + MODULE_DCC_PATH_LOGS_NORMAL), ofstream::out | ofstream::app);

      logInfo->appendLog((safeBuffer + "\n"));

      if (OPENAPRS_LOG_FLUSH == true)
        logInfo->flushLog();
    } // if

    // return length of what we wrote
    return logBuffer.length();
  } // DCC::OnLog

  /*********************
   ** OnTimer Members **
   *********************/

  /**
   * DCC::OnTimer
   *
   * Triggered when a timer event has passed.
   *
   * Returns: 1
   */
  const int DCC::OnTimer(Timer *aTimer) {

    // initialize variables

    return 1;
  } // DCC::OnTimer

  /**
   * DCC::Send
   *
   * Sends a message to all clients.
   *
   * Returns: Number of bytes sent.
   */
  const unsigned int DCC::Send(const char *sendFormat, ...) {
    Work *work;
    char sendBuffer[MAXBUF + 1] = {0};		// Buffer to send to clients.
    pthread_t tid;
    threadSetType::iterator ptr;
    va_list va;					// Va arguments to compile for vsnprintf.

    // initialize variables
    va_start(va, sendFormat);
    vsnprintf(sendBuffer, sizeof(sendBuffer), sendFormat, va);
    va_end(va);

    for(ptr = _workerThreads.begin(); ptr != _workerThreads.end(); ptr++) {
      tid = *ptr;
      work = new Work(
                      sendBuffer,
                      Work::WORK_SEND_ALL,
                      time(NULL)
                     );

      addWorkToThread(tid, work);
    } // for


    return strlen(sendBuffer);
  } // DCC::Send

  const unsigned int DCC::wallopsf(const char *sendFormat, ...) {
    Work *work;
    char sendBuffer[MAXBUF + 1] = {0};		// Buffer to send to clients.
    pthread_t tid;
    threadSetType::iterator ptr;
    va_list va;					// Va arguments to compile for vsnprintf.

    // initialize variables
    va_start(va, sendFormat);
    vsnprintf(sendBuffer, sizeof(sendBuffer), sendFormat, va);
    va_end(va);

    for(ptr = _workerThreads.begin(); ptr != _workerThreads.end(); ptr++) {
      tid = *ptr;
      work = new Work(
                      sendBuffer,
                      Work::WORK_WALLOPS_ALL,
                      time(NULL)
                     );

      addWorkToThread(tid, work);
    } // for


    return strlen(sendBuffer);
  } // DCC::wallopsf

  const unsigned int DCC::wallops(const string &buf) {
    Work *work;
    pthread_t tid;
    threadSetType::iterator ptr;

    for(ptr = _workerThreads.begin(); ptr != _workerThreads.end(); ptr++) {
      tid = *ptr;
      work = new Work(
                      buf,
                      Work::WORK_WALLOPS_ALL,
                      time(NULL)
                     );

      addWorkToThread(tid, work);
    } // for

    return buf.length();
  } // DCC::wallops

  const unsigned int DCC::wallopsReply(const string &buf) {
    Work *work;
    pthread_t tid;
    threadSetType::iterator ptr;

    for(ptr = _workerThreads.begin(); ptr != _workerThreads.end(); ptr++) {
      tid = *ptr;
      work = new Work(
                      buf,
                      Work::WORK_WALLOPS_REPLY_ALL,
                      time(NULL)
                     );

      addWorkToThread(tid, work);
    } // for

    return buf.length();
  } // DCC::wallopsReply

  void DCC::clearWork() {
    _workMap_l.Lock();
    _clearWork();
    _workMap_l.Unlock();
  } // DCC::clearWork

  void DCC::_clearWork() {
    Work *work;
    workListType *workList;
    workMapType::iterator ptr;

    for(ptr = _workMap.begin(); ptr != _workMap.end(); ptr++) {
      workList = ptr->second;
      while(!workList->empty()) {
        work = workList->front();
        delete work;
        workList->pop_front();
      } // while

      delete workList;
    } // for

    _workMap.clear();

  } // clearWork

  void DCC::clearResults() {
    _resultSet_l.Lock();
    _clearResults();
    _resultSet_l.Unlock();
  } // DCC::clearResults

  void DCC::_clearResults() {
//    dcc::Work *work;

 //   while(!_resultSet.empty()) {
 //     work = _resultSet.front();
 //     _resultSet.erase(work);
 //     delete work;
 //   } // while

  } // clearResult

} // namespace dcc

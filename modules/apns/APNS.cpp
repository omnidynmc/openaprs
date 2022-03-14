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

#include "ApnsLog_OpenAPRS.h"
#include "Cfg.h"
#include "APNS.h"
#include "FeedbackController.h"
#include "Log.h"
#include "PutMySQL.h"
#include "Server.h"
#include "StringTool.h"
#include "Timer.h"
#include "Vars.h"

#include "apnsCommands.h"
#include "stdinCommands.h"

#include "apns.h"
#include "openaprs.h"
#include "openaprs_string.h"

namespace apns {

  using namespace openaprs;
  using namespace handler;
  using namespace putmysql;
  using namespace std;
  using aprs::StringTool;

/**************************************************************************
 ** APNS Class                                                           **
 **************************************************************************/

  APNS::APNS() : _die(false) {
    _numSslThreads = 0;
    _numSslDevelThreads = 0;
    _numFeedbackThreads = 0;
    initializeSystem();

    return;
  } // APNS::APNS

  APNS::~APNS() {
    deinitializeSystem();

    return;
  } // APNS::~APNS

  /***********************************************
   ** Initialization / Deinitailization Members **
   ***********************************************/
  const bool APNS::initializeSystem() {
    initializeLogs();
    initializeSsl();
    initializeThreads();
    initializeTimers();
    initializeCommands();
    initializeEvents();

    profile.add("module.apns.push", 300);

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: System Initialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: System Initialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: System Initialized");

    return true;
  } // APNS::initializeSystem

  const bool APNS::initializeSsl() {
    // initialize OpenSSL library
    SSL_library_init();

    // Load SSL error strings
    SSL_load_error_strings();

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Ssl Library Initialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: Ssl Library Initialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: Ssl Library Initialized");

    return true;
  } // initializeSsl

  const bool APNS::initializeLogs() {
    myOnLogList[MODULE_APNS_LOG_NORMAL] = app->addLog(MODULE_APNS_LOG_NORMAL, this, true);
    myOnLogList[MODULE_APNS_LOG_DEBUG] = app->addLog(MODULE_APNS_LOG_DEBUG, this, true);

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Logs Initialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: Logs Initialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: Logs Initialized");

    return true;
  } // APNS::initializeLogs

  const bool APNS::initializeEvents() {
    myOnEventMap["module.apns.event.message.push"] = app->addEvent("aprs.message", Event::FLAG_CLEANUP, new eventMESSAGETOPUSH);

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Events Initialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: Events Initialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: Events Initialized");

    return true;
  } // APNS::initializeEvents

  const bool APNS::deinitializeEvents() {
    app->removeEvent(myOnEventMap["module.apns.event.message.push"]);

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: System Deinitialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: System Deinitialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: System Deinitialized");

    return true;
  } // APNS::deinitializeEvents

  const bool APNS::initializeThreads() {
    pthread_t sslThread_tid;
    unsigned int i;
    unsigned int numSslThreads = app->cfg->get_int("module.apns.ssl.threads", 2);

    // create our signal handling thread
    if (app->cfg->get_bool("module.apns.prod.enable", true)) {
      for(i=0; i < numSslThreads; i++) {
        pthread_create(&sslThread_tid, NULL, APNS::SslThread, this);
        app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Started Production SSL Thread %u", sslThread_tid);
        _sslThreads.insert(sslThread_tid);
      } // for
    } // if

    if (app->cfg->get_bool("module.apns.devel.enable", true)) {
      pthread_create(&sslThread_tid, NULL, APNS::SslDevelThread, this);
      app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Started Devel SSL Thread %u", sslThread_tid);
      _sslThreads.insert(sslThread_tid);
    } // if

    pthread_create(&sslThread_tid, NULL, APNS::FeedbackThread, this);
    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Started FeedBack Thread %u", sslThread_tid);
    _sslThreads.insert(sslThread_tid);

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Threads Initialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: Threads Initialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: Threads Initialized");

    return true;
  } // APNS::initializeThreads

  const bool APNS::initializeTimers() {
    app->cfg->push_int("openaprs.timer.apns.update.register.id",
                       app->addTimer(0, TIMER_CONTINUOUS, true, false, this));

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Timers Initialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: Timers Initialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: Timers Initialized");

    return true;
  } // APNS::initializeTimers

  const bool APNS::initializeCommands() {
    app->addCommand("APNS", "STDIN", "/APNS", -1, Command::FLAG_CLEANUP, new stdinDTREE);
      app->addCommand("TEST", "STDIN/APNS", "/APNS TEST", -1, Command::FLAG_CLEANUP, new stdinDTREE);
        app->addCommand("PUSH", "STDIN/APNS/TEST", "/APNS TEST PUSH <device token>", -1, Command::FLAG_CLEANUP, new stdinAPNSTESTPUSH);
        app->addCommand("THROUGHPUT", "STDIN/APNS/TEST", "/APNS TEST THROUGHPUT [max create] [timer interval]", -1, Command::FLAG_CLEANUP, new stdinAPNSTESTTHROUGHPUT);
      app->addCommand("PUSH", "STDIN/APNS", "/APNS PUSH <token> <message>", -1, Command::FLAG_CLEANUP, new stdinAPNSPUSH);
      app->addCommand("PUSHFAKE", "STDIN/APNS", "/APNS PUSHFAKE <message>", -1, Command::FLAG_CLEANUP, new stdinAPNSPUSHFAKE);

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Commands Initialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: Commands Initialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: Commands Initialized");

    return true;
  } // APNS::initializeCommands

  const bool APNS::deinitializeCommands() {
    app->removeCommand("STDIN/APNS");

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Commands Deinitialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: Commands Deinitialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: Commands Deinitialized");

    return true;
  } // APNS::deinitializeCommands

  const bool APNS::deinitializeSystem() {
    die(true);
    deinitializeThreads();
    deinitializeCommands();
    deinitializeTimers();
    deinitializeLogs();
    deinitializeEvents();

    profile.clear();
    app->cfg->pop_match("openaprs.count.module.apns.*");

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: System Deinitialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: System Deinitialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: System Deinitialized");

    return true;
  } // APNS::deinistializeSystem

  const bool APNS::deinitializeLogs() {
    app->removeLog(myOnLogList[MODULE_APNS_LOG_NORMAL]);
    app->removeLog(myOnLogList[MODULE_APNS_LOG_DEBUG]);
    //app->removeLog(myOnLogList[MODULE_APNS_LOG_BOTNET_PROTOCOL]);

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Logs Deinitialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: Logs Deinitialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: Logs Deinitialized");

    return true;
  } // APNS::deinitializeLogs

  const bool APNS::deinitializeTimers() {
    list<string> optionList;		// List of options matched.
    string optionName;			// Name of the looped to option.

   if (app->cfg->match("openaprs.timer.apns.*", optionList) > 0) {
      for(; !optionList.empty(); optionList.pop_front()) {
        optionName = optionList.front();

        if (app->cfg->is_int(optionName) == true) {
          app->removeTimer(app->cfg->get_int(optionName));
          app->cfg->pop(optionName);
        } // if
      } // for
    } // if

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Timers Deinitialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: Timers Deinitialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: Timers Deinitialized");

    return true;
  } // APNS::deinitializeTimers

  const bool APNS::deinitializeThreads() {
    threadSetType::iterator ptr;
    pthread_t tid;

    // create our signal handling thread
    //pthread_cancel(_sslThread_tid);
    // because deinitializeSystem will set die(), we just join the other thread
    // and let it break correctly in order to unload.
    while(!_sslThreads.empty()) {
      ptr = _sslThreads.begin();
      tid = (*ptr);
      app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Waiting for Thread %u to Deinitialize", tid);
      app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: Waiting for Thread %u to Deinitialized", tid);
      app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: Waiting for Thread %u to Deinitialized", tid);

      pthread_join(tid, NULL);
      _sslThreads.erase(tid);
    } // while

    app->writeLog(MODULE_APNS_LOG_NORMAL, "*** Module APNS: Threads Deinitialized");
    app->writeLog(OPENAPRS_LOG_NORMAL, "*** Module APNS: Threads Deinitialized");
    app->writeLog(OPENAPRS_LOG_STDOUT, "*** Module APNS: Threads Deinitialized");

    return true;
  } // APNS::deinitializeLogs

  /*******************
   ** OnLog Members **
   *******************/

  const int APNS::OnLog(Log *logInfo, const logId callId, const string &logBuffer) {
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

    if (callId == myOnLogList[MODULE_APNS_LOG_NORMAL]) {
      /*
       * 06/20/2003: If log file isn't open, try to open it using hal's default
       *             log path. -GCARTER
       */
      if (logInfo->isLogOpen() == false)
        logInfo->openLog((OPENAPRS_PATH_LOGS + MODULE_APNS_PATH_LOGS_NORMAL), ofstream::out | ofstream::app);

      logInfo->appendLog((safeBuffer + "\n"));

      if (OPENAPRS_LOG_FLUSH == true)
        logInfo->flushLog();
    } // if
    else if (callId == myOnLogList[MODULE_APNS_LOG_DEBUG] &&
             IsServerDebug == true) {
      /*
       * 06/20/2003: If log file isn't open, try to open it using openaprs's default
       *             log path. -GCARTER
       */
      if (logInfo->isLogOpen() == false)
        logInfo->openLog((OPENAPRS_PATH_LOGS + MODULE_APNS_PATH_LOGS_DEBUG), ofstream::out | ofstream::app);

      logInfo->appendLog((safeBuffer + "\n"));
      if (OPENAPRS_LOG_FLUSH == true)
        logInfo->flushLog();
    } // else if

    // return length of what we wrote
    return logBuffer.length();
  } // APNS::OnLog

  /*********************
   ** OnTimer Members **
   *********************/

  /**
   * APNS::OnTimer
   *
   * Triggered when a timer event has passed.
   *
   * Returns: 1
   */
  const int APNS::OnTimer(Timer *aTimer) {
    unsigned int onTimerId;			// timer id

    // initialize variables
    onTimerId = aTimer->getId();

    if (onTimerId == (unsigned) app->cfg->get_int("openaprs.timer.apns.update.register.id", 0))
      _updateRegister();

    return 1;
  } // APNS::OnTimer

  const unsigned int APNS::_updateRegister() {
    return 0;
  } // APNS::_updateRegister

  void APNS::Push(ApnsMessage *aMessage) {
    assert(aMessage != NULL);

    Lock();
    switch(aMessage->environment()) {
      case ApnsMessage::APNS_ENVIRONMENT_PROD:
        if (app->cfg->get_bool("module.apns.prod.enable", false))
          _messageQueue.insert(aMessage);
        break;
      case ApnsMessage::APNS_ENVIRONMENT_DEVEL:
      default:
        if (app->cfg->get_bool("module.apns.devel.enable", false))
          _messageDevelQueue.insert(aMessage);
        break;
    } // switch
    Unlock();
  } // APNS::Push

  const unsigned int APNS::getPushQueue(messageQueueType &messageQueue, const int limit) {
    messageQueueType::iterator ptr;
    int numRows = 0;

    Lock();
    while(!_messageQueue.empty()) {
      if (numRows > limit)
        break;

      ptr = _messageQueue.begin();
      messageQueue.insert(*ptr);
      _messageQueue.erase(*ptr);
      numRows++;
    } // while
    Unlock();

    return numRows;
  } // APNS::getPushQueue

  const unsigned int APNS::getPushDevelQueue(messageQueueType &messageQueue, const int limit) {
    messageQueueType::iterator ptr;
    int numRows = 0;

    Lock();
    while(!_messageDevelQueue.empty()) {
      if (numRows > limit)
        break;

      ptr = _messageDevelQueue.begin();
      messageQueue.insert(*ptr);
      _messageDevelQueue.erase(*ptr);
      numRows++;
    } // while
    Unlock();

    return numRows;
  } // APNS::getPushDevelQueue

  void *APNS::SslThread(void *apns) {
    APNS *a = (APNS *) apns;
    messageQueueType messageQueue;
    messageQueueType::iterator ptr;
    PushController *push;		// prod version
    int maxQueue = app->cfg->get_int("module.apns.ssl.maxqueue", 100);
    int logStatsInterval = app->cfg->get_int("module.apns.ssl.stats.interval", PushController::DEFAULT_STATS_INTERVAL);
    unsigned int numThreads = a->numSslThreads()+1;

    push = new PushController(PROD_APPLE_HOST, PROD_APPLE_PORT, PROD_RSA_CLIENT_CERT, PROD_RSA_CLIENT_KEY, PROD_CA_CERT_PATH, PROD_APPLE_TIMEOUT);

    push->logger(new ApnsLog_OpenAPRS("SslThread #" + StringTool::uint2string(numThreads)));
    push->logStatsInterval(logStatsInterval);

    while(true) {
      if (a->die())
        break;

      pthread_testcancel();

      if (push->sendQueueSize() < (unsigned int) maxQueue)
        a->getPushQueue(messageQueue, maxQueue - push->sendQueueSize());

      while(!messageQueue.empty()) {
        ptr = messageQueue.begin();
        push->add(*ptr);
        messageQueue.erase(*ptr);
      } // while

      push->run();

      usleep(100000);
    } // while

    delete push;

    return NULL;
  } // APNS::SslThread

  void *APNS::SslDevelThread(void *apns) {
    APNS *a = (APNS *) apns;
    messageQueueType messageQueue;
    messageQueueType::iterator ptr;
    PushController *push;		// prod version
    int maxQueue = app->cfg->get_int("module.apns.ssl.maxqueue", 100);
    int logStatsInterval = app->cfg->get_int("module.apns.ssl.stats.interval", PushController::DEFAULT_STATS_INTERVAL);
    unsigned int numThreads = a->numSslDevelThreads()+1;

    push = new PushController(DEVEL_APPLE_HOST, DEVEL_APPLE_PORT, DEVEL_RSA_CLIENT_CERT, DEVEL_RSA_CLIENT_KEY, DEVEL_CA_CERT_PATH, DEVEL_APPLE_TIMEOUT);

    push->logger(new ApnsLog_OpenAPRS("SslDevelThread #" + StringTool::uint2string(numThreads)));
    push->logStatsInterval(logStatsInterval);

    while(true) {
      if (a->die())
        break;

      pthread_testcancel();

      if (push->sendQueueSize() < (unsigned int) maxQueue)
        a->getPushDevelQueue(messageQueue, maxQueue - push->sendQueueSize());

      while(!messageQueue.empty()) {
        ptr = messageQueue.begin();
        push->add(*ptr);
        messageQueue.erase(*ptr);
      } // while

      push->run();

      usleep(100000);
    } // while

    delete push;

    return NULL;
  } // APNS::SslDevelThread

  void *APNS::FeedbackThread(void *apns) {
    APNS *a = (APNS *) apns;
    FeedbackController::messageQueueType removeRegisterQueue;
    FeedbackController::messageQueueType::iterator rptr;
    FeedbackController *feedback;
    PutMySQL *sql;
    unsigned int threadNum = a->numFeedbackThreads()+1;

    if (CFG_STRING("module.apns.mode", "devel") == "prod") {
      feedback = new FeedbackController(PROD_APPLE_FEEDBACK_HOST, PROD_APPLE_FEEDBACK_PORT, PROD_RSA_CLIENT_CERT, PROD_RSA_CLIENT_KEY, PROD_CA_CERT_PATH, PROD_APPLE_FEEDBACK_TIMEOUT);
    } // if
    else {
      feedback = new FeedbackController(DEVEL_APPLE_FEEDBACK_HOST, DEVEL_APPLE_FEEDBACK_PORT, DEVEL_RSA_CLIENT_CERT, DEVEL_RSA_CLIENT_KEY, DEVEL_CA_CERT_PATH, DEVEL_APPLE_FEEDBACK_TIMEOUT);
    } // else

    CREATE_PUTMYSQL(sql, OPENAPRS_SQL_LOGIN);

    feedback->logger(new ApnsLog_OpenAPRS("FeedbackThread #" + apns::StringTool::uint2string(threadNum)));

    while(true) {
      if (a->die())
        break;

      pthread_testcancel();

      feedback->run();

      if (feedback->getQueue(removeRegisterQueue)) {
        while(!removeRegisterQueue.empty()) {
          rptr = removeRegisterQueue.begin();
          sql->queryf("INSERT INTO apns_feedback (apns_timestamp, device_token, create_ts) VALUES ('%d', '%s', UNIX_TIMESTAMP())", 
              (*rptr)->timestamp(),
              sql->Escape((*rptr)->deviceToken()).c_str()
            );
          removeRegisterQueue.erase(rptr);
          delete *rptr;
        } // while
      } // if

      sleep(10);
    } // while

    delete feedback;
    delete sql;

    return NULL;
  } // APNS::FeedbackThread
} // namespace apns


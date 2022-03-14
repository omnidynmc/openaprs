#include <new>
#include <string>

#include <stdarg.h>
#include <stdio.h>

#include <openframe/openframe.h>

#include "Server.h"
#include "Stomp.h"
#include "StompServer.h"
#include "Command.h"

#include "config.h"
#include "stomp.h"
#include "openaprs.h"

#include "stompCommands.h"
#include "stdinCommands.h"

namespace stomp {
  using namespace std;
  using namespace openframe;
  using namespace openframe::loglevel;

  Stomp::Stomp() : _done(false) {
    //_dbi = NULL;

  } // Stomp::Stomp

  Stomp::~Stomp() {
  } // Stomp:~Stomp

  void Stomp::initializeSystem() {
    initializeCommands();
    initializeDatabase();
    initializeThreads();
    _consolef("*** Module Stomp: System Initialized");
  } // Stomp::initializeSystem

  void Stomp::initializeCommands() {
//    CommandTree *command = app->command();
    app->addCommand("STOMP", "STDIN", "/STOMP", -1, Command::FLAG_CLEANUP, new stdinDTREE);
//      command->addCommand("ESME", "STDIN/STOMP", "/STOMP ESME", -1, Command::FLAG_CLEANUP, new stdinDTREE);
//        command->addCommand("SUBMITSM", "STDIN/STOMP/ESME", "/SUBMITSM <source> <destination> <message>", -1, Command::FLAG_CLEANUP, new stdinESMESUBMITSM);
//      command->addCommand("SMSC", "STDIN/STOMP", "/STOMP SMSC", -1, Command::FLAG_CLEANUP, new stdinDTREE);
//        command->addCommand("DELIVERSM", "STDIN/STOMP/SMSC", "/DELIVERSM <source> <destination> <message>", -1, Command::FLAG_CLEANUP, new stdinSMSCDELIVERSM);

    _consolef("*** Module Stomp: Commands Initialized");
  } // Stomp::initializeCommands

  void Stomp::initializeDatabase() {
    try {
      //_dbi = new DBI_Stomp(MODULE_STOMP_SQL_DB, MODULE_STOMP_SQL_HOST, MODULE_STOMP_SQL_USER, MODULE_STOMP_SQL_PASS);
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    _consolef("*** Module Stomp: Database Initialized");
  } // Stomp::initializeDatabase

  void Stomp::deinitializeSystem() {
    done(true);
    deinitializeThreads();
    deinitializeCommands();
    deinitializeDatabase();
    _consolef("*** Module Stomp: System Deinitialized");
  } // Stomp::deinitializeSystem

  void Stomp::deinitializeThreads() {
    threadSetType::iterator ptr;
    pthread_t tid;

    while(!_stompThreads.empty()) {
      ptr = _stompThreads.begin();
      tid = (*ptr);
      _consolef("*** Module Stomp: Waiting for Thread %u to Deinitialized", tid);
      destroyThreadQ(tid);
      pthread_join(tid, NULL);
      _stompThreads.erase(tid);
    } // while

    _consolef("*** Module Stomp: Threads Denitialized");
  } // Stomp::deinitializeThreads

  void Stomp::initializeThreads() {
    pthread_t stompThread_tid;
    unsigned int i;
    unsigned int numStompThreads = app->cfg->get_int("module.stomp.threads", 1);

    // create our signal handling thread
    for(i=0; i < numStompThreads; i++) {
      pthread_create(&stompThread_tid, NULL, Stomp::StompThread, this);
      _consolef("*** Module Stomp: Started Stomp Thread %u", stompThread_tid);
      _stompThreads.insert(stompThread_tid);
      createThreadQ(stompThread_tid);
    } // for

    _consolef("*** Module Stomp: Threads Initialized");
  } // Stomp::initializeThreads

  void Stomp::deinitializeCommands() {
    app->removeCommand("STDIN/STOMP");
    _consolef("*** Module Stomp: Commands Deinitialized");
  } // Stomp::initializeCommands

  void Stomp::deinitializeDatabase() {
    //if (_dbi) delete _dbi;

    _consolef("*** Module Stomp: Database Deinitialized");
  } // Stomp::deinitializeDatabase

  void *Stomp::StompThread(void *stomp) {
    Stomp *a = (Stomp *) stomp;
    StompServer *sserv;		// prod version
    int maxQueue = app->cfg->get_int("module.stomp.maxqueue", 100);
    unsigned int numThreads =  1; //a->numStompThreads()+1;

    time_t sessionInitInterval = app->cfg->get_int("module.stomp.timer.session_init", 30);
    time_t enquireLinkInterval = app->cfg->get_int("module.stomp.timer.enquire_link", 30);
    time_t inactivityInterval = app->cfg->get_int("module.stomp.timer.inactivity", 120);
    time_t responseInterval = app->cfg->get_int("module.stomp.timer.response", 60);
    time_t logstatsInterval = app->cfg->get_int("module.stomp.timer.logstats", StompServer::DEFAULT_STATS_INTERVAL);

    sserv = new StompServer(MODULE_STOMP_PORT, MODULE_STOMP_MAX, sessionInitInterval, enquireLinkInterval, inactivityInterval, responseInterval, logstatsInterval);

    sserv->debug(true);
    sserv->logger(new OpenFrame_Log("StompThread #" + StringTool::uint2string(numThreads)));

    string logpath = app->cfg->get_string("module.stomp.log.path", "logs/openaprs.stomp.log");
    elog.open("stomp", logpath);
    int loglevel = app->cfg->get_int("module.stomp.log.level", 7);
    if (loglevel >= 0 && loglevel <= 7)
      elog.level("stomp", static_cast<openframe::loglevel::Level>(loglevel));
    else
      elog.level("stomp", openframe::loglevel::LogDebug);

    sserv->elogger(&elog, "stomp");

    sserv->debug(true).start();

    while( !a->done() ) {
      if (a->done())
        break;

      bool didWork = sserv->run();

      if (!didWork)
        usleep(5000);
    } // while

    sserv->stop();

    delete sserv;

    return NULL;
  } // Stomp::StompThread
} // namespace stomp

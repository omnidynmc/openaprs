#include <new>
#include <string>

#include <stdarg.h>
#include <stdio.h>

#include <openframe/openframe.h>

#include "Server.h"
#include "Sat.h"
#include "SatServer.h"
#include "Command.h"

#include "config.h"
#include "sat.h"
#include "openaprs.h"

#include "satCommands.h"
#include "stdinCommands.h"

namespace sat {
  using namespace std;
  using namespace openframe;

  Sat::Sat() : _done(false) {
    //_dbi = NULL;

  } // Sat::Sat

  Sat::~Sat() {
  } // Sat:~Sat

  void Sat::initializeSystem() {
    initializeCommands();
    initializeDatabase();
    initializeThreads();
    _consolef("*** Module Sat: System Initialized");
  } // Sat::initializeSystem

  void Sat::initializeCommands() {
//    CommandTree *command = app->command();
    app->addCommand("SAT", "STDIN", "/STOMP", -1, Command::FLAG_CLEANUP, new stdinDTREE);
//      command->addCommand("ESME", "STDIN/STOMP", "/STOMP ESME", -1, Command::FLAG_CLEANUP, new stdinDTREE);
//        command->addCommand("SUBMITSM", "STDIN/STOMP/ESME", "/SUBMITSM <source> <destination> <message>", -1, Command::FLAG_CLEANUP, new stdinESMESUBMITSM);
//      command->addCommand("SMSC", "STDIN/STOMP", "/STOMP SMSC", -1, Command::FLAG_CLEANUP, new stdinDTREE);
//        command->addCommand("DELIVERSM", "STDIN/STOMP/SMSC", "/DELIVERSM <source> <destination> <message>", -1, Command::FLAG_CLEANUP, new stdinSMSCDELIVERSM);

    _consolef("*** Module Sat: Commands Initialized");
  } // Sat::initializeCommands

  void Sat::initializeDatabase() {
    try {
      //_dbi = new DBI_Sat(MODULE_STOMP_SQL_DB, MODULE_STOMP_SQL_HOST, MODULE_STOMP_SQL_USER, MODULE_STOMP_SQL_PASS);
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    _consolef("*** Module Sat: Database Initialized");
  } // Sat::initializeDatabase

  void Sat::deinitializeSystem() {
    done(true);
    deinitializeThreads();
    deinitializeCommands();
    deinitializeDatabase();
    _consolef("*** Module Sat: System Deinitialized");
  } // Sat::deinitializeSystem

  void Sat::deinitializeThreads() {
    threadSetType::iterator ptr;
    pthread_t tid;

    while(!_satThreads.empty()) {
      ptr = _satThreads.begin();
      tid = (*ptr);
      _consolef("*** Module Sat: Waiting for Thread %u to Deinitialized", tid);
      destroyThreadQ(tid);
      pthread_join(tid, NULL);
      _satThreads.erase(tid);
    } // while

    _consolef("*** Module Sat: Threads Denitialized");
  } // Sat::deinitializeThreads

  void Sat::initializeThreads() {
    pthread_t satThread_tid;
    unsigned int i;
    unsigned int numSatThreads = app->cfg->get_int("module.sat.threads", 1);

    // create our signal handling thread
    for(i=0; i < numSatThreads; i++) {
      pthread_create(&satThread_tid, NULL, Sat::SatThread, this);
      _consolef("*** Module Sat: Started Sat Thread %u", satThread_tid);
      _satThreads.insert(satThread_tid);
      createThreadQ(satThread_tid);
    } // for

    _consolef("*** Module Sat: Threads Initialized");
  } // Sat::initializeThreads

  void Sat::deinitializeCommands() {
    app->removeCommand("STDIN/SAT");
    _consolef("*** Module Sat: Commands Deinitialized");
  } // Sat::initializeCommands

  void Sat::deinitializeDatabase() {
    //if (_dbi) delete _dbi;

    _consolef("*** Module Sat: Database Deinitialized");
  } // Sat::deinitializeDatabase

  void *Sat::SatThread(void *sat) {
    Sat *a = (Sat *) sat;
//    int maxQueue = app->cfg->get_int("module.sat.maxqueue", 100);
    unsigned int numThreads =  1; //a->numSatThreads()+1;

    string path = app->cfg->get_string("module.sat.path.tle", "tle/");

    SatServer *sserv = new SatServer(path);
    sserv->logger( new App_Log("SatServer:") );
    sserv->load_tle();

    while( !a->done() ) {
      if (a->done())
        break;

      bool didWork = sserv->run();

      if (!didWork)
        usleep(5000);
      sleep(1);
    } // while

    delete sserv;

    return NULL;
  } // Sat::SatThread
} // namespace sat

/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
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
 $Id: Server.cpp,v 1.9 2006/03/21 20:55:12 omni Exp $
 **************************************************************************/

#include <list>
#include <new>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "APRS.h"
#include "App_Log.h"
#include "DBI.h"
#include "Insert.h"
#include "PutMySQL.h"
#include "Log.h"
#include "LineBuffer.h"
#include "Module.h"
#include "ModuleList.h"
#include "Network.h"
#include "Server.h"
#include "Timer.h"
#include "UnitTest.h"			// libaprs
#include "Uplink.h"
#include "Vars.h"
#include "Worker.h"

#include "StdinController.h"

#include "Create.h"
#include "Event.h"
#include "Send.h"
#include "eventCommands.h"
#include "stdinCommands.h"
#include "timerCommands.h"

#include "openframe/ConfController.h"
#include "openframe/Result.h"
#include "openframe/Serialize.h"
#include "openframe/Stopwatch.h"
#include "openframe/StringTool.h"

#include "config.h"
#include "openaprs.h"
#include "openaprs_string.h"
#include "md5wrapper.h"
#include "stringify.h"

namespace openaprs {
  using openframe::ConfController;
  using openframe::Stopwatch;
  using openframe::Serialize;
  using openframe::StringTool;
  using namespace putmysql;
  using namespace handler;
  using namespace events;
  using namespace aprs;

  using std::string;
  using std::list;
  using std::ofstream;
  using std::ostream;
  using std::stringstream;
  using std::ios;
  using std::endl;
  using std::cout;

/**************************************************************************
 ** Server Class                                                         **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

Server::Server() {
  // set defaults
  myEcho = true;
  myFlags = 0;
  myUplink = "";
  _done = false;
  _disconnect = false;
  _lastCommand = "";
  _serverListIndex = 0;
  _oss.str("");
  setLinkTS(0);
  setStartTS(0);
  moduleActionList.clear();

  try {
    cfg = new ConfController();
    module = new ModuleList;
  } // try
  catch(bad_alloc xa) {
    assert(false);
  } // catch

  //_stdinController = new StdinController("command> ", 1024);
  //_stdinController->logger(new App_Log("stdinController"));
} // Server::Server

Server::~Server() {
  Result *result;
  Work *work;
  workListType::iterator wptr;
  resultListType::iterator rptr;
  logMapType::iterator logPtr;			// Iterator to a log map.

  /*
   * 06/20/2003: Clean up our logging facility list and delete any storage
   *             used. -GCARTER
   */
  _logList_l.Lock();
  while(!myLogList.empty()) {
    logPtr = myLogList.begin();
    delete logPtr->second;
    myLogList.erase(logPtr);
  } // for
  _logList_l.Unlock();

  _workList_l.Lock();
  while(!_workList.empty()) {
    wptr = _workList.begin();
    work = *wptr;
    _workList.erase(wptr);
    delete work;
  } // while
  _workList_l.Unlock();

  _resultList_l.Lock();
  while(!_resultList.empty()) {
    rptr = _resultList.begin();
    result = *rptr;
    _resultList.erase(rptr);
    delete result;
  } // while
  _resultList_l.Unlock();

  clearTimers(true);

  delete cfg;
  delete dbi;
  delete module;
  //delete _stdinController;
  // cleanup

  PutMySQL::end();
} // Server::~Server

/***************
 ** Variables **
 ***************/

const Server::flagType Server::FLAG_SHUTDOWN	=	0x0001;
const Server::flagType Server::FLAG_PREAUTH	=	0x0002;
const Server::flagType Server::FLAG_CONNECTED	=	0x0004;
const Server::flagType Server::FLAG_NOAB	=	0x0008;

const bool Server::Module(const string &path, ModuleAction::actionEnumType action) {
  ModuleAction ma;

  ma.path = path;
  ma.action = action;

  moduleActionList.push_back(ma);

  return true;
} // Server::addModule

const bool Server::Email(const string &source, const string &to, const string &message) {
  Connection *nConnection;
  int newFd;
  int port = cfg->get_int("openaprs.email.port", 25);
  string host = cfg->get_string("openaprs.email.host", "localhost");
  string from = cfg->get_string("openaprs.email.from", "daemon@openaprs.net");
  string name = cfg->get_string("openaprs.email.name", "OpenAPRS - APRS Message to Email Gateway");
  string helo = cfg->get_string("openaprs.email.helo", "openaprs.net");
  stringstream s;
  string buf;
  unsigned int len;

  s.str("");
  s << port;

  nConnection = app->addConnection(host, s.str(), getVhost(), "OPENAPRS Connection", false);

  if (nConnection == NULL)
    return false;

  newFd = nConnection->getFd();

  //addConnection(newFd, "EMAIL GATEWAY");

  s.str("");
  s << "HELO " << helo << endl
    << "MAIL FROM: " << from << endl
    << "RCPT TO: " << to << endl
    << "DATA" << endl
    << "FROM: \"" << name << "\" <" << from << ">" << endl
    << "TO: " << to << endl
    << "SUBJECT: APRS Message from " << source << endl
    << endl
    << message
    << "." << endl
    << "QUIT" << endl;

  //cout << s.str() << endl;

  //cout << s.str().length() << endl;

  buf = s.str();
  while(buf.length() != 0) {
    if (buf.length() > 512)
      len = 512;
    else
      len = buf.length();

    sendConnectionF(newFd, "%s", buf.substr(0, len).c_str());
    buf.erase(0, len);
  } // while

  writeConnections();

  removeConnection(newFd);

  return true;
} // Server::Email

/*****************
 ** Die Members **
 *****************/

void Server::Die(const string &dieReason) {
  _consolef("*** Die: %s", dieReason.c_str());
  _logf("*** Die: %s", dieReason.c_str());

  Done();

  //if (IsServerConnected(this) == true) {
  //  if (getUplink().length() > 0)
  //    Send("%s SQ %s :OpenAPRS Unloading\n", (OPENAPRS_CONFIG_SERVER_NUMERIC).c_str(), getUplink().c_str());

  //  writeConnections();
  //  removeConnection(getSocket());
  //} // if
} // Server::Die

  /*******************
   ** Event Members **
   *******************/

  /**
   * Server::addEvent
   *
   * Attempts to add an event to the event system.
   *
   * Returns: Id of the new event.
   */
  const Event::eventId Server::addEvent(const string &addMatch, const unsigned int addFlags,
                                        EventHandler *addEventHandler) {
    Event::eventId ret;

    _writeEvent_l.Lock();
    ret = _addEvent(addMatch, addFlags, addEventHandler);
    _writeEvent_l.Unlock();

    return ret;
  } // Server::findEvent

  const Event::eventId Server::_addEvent(const string &addMatch, const unsigned int addFlags, 
                                        EventHandler *addEventHandler) {
    Event *nEvent;			// pointer to a new event
    eventId newId;			// new event id

    assert(addEventHandler != NULL);

    // initialize variables
    newId = _getUniqueEventId();
    nEvent = new Event(newId, addMatch, addFlags, addEventHandler);

    _debugf("Server::addEvent> Added Id: %d", newId);

    myEventList.insert(pair<eventId, Event *>(newId, nEvent));

    return newId;
  } // Server::addEvent

  /**
   * Server::findEvent
   *
   * Attmpts to search for a target event by id.
   *
   * Returns: Pointer to an Event or NULL if it can't be found.
   */
  Event *Server::findEvent(const eventId id) {
    Event *ret;

    _writeEvent_l.Lock();
    ret = _findEvent(id);
    _writeEvent_l.Unlock();

    return ret;
  } // Server::findEvent

  Event *Server::_findEvent(const eventId findMe) {
    eventMapType::iterator eventPtr;			// pointer to an Event
  
    if ((eventPtr = myEventList.find(findMe)) != myEventList.end())
      return eventPtr->second;
  
    return NULL;
  } // Server::_findEvent

  /**
   * Server::removeEvent
   *
   * Attempts to remove a target event by id.
   *
   * Returns: True if the remove is successful, false if it is not.
   */
  const bool Server::removeEvent(const eventId id) {
    bool ret;

    _writeEvent_l.Lock();
    ret = _removeEvent(id);
    _writeEvent_l.Unlock();

    return ret;
  } // Server::removeEvent
  const bool Server::_removeEvent(const eventId removeMe) {
    Event *tEvent;					// Pointer to a target Event.
    EventHandler *tEventHandler;			// Pointer to an EventHandler.
    eventMapType::iterator eventPtr;			// Iterator to an Event map.

    if ((eventPtr = myEventList.find(removeMe)) == myEventList.end())
      return false;

    // initialize variables
    tEvent = eventPtr->second;
    tEventHandler = tEvent->getEventHandler();

    _debugf("Server::removeEvent> Removed Id: %d", removeMe);

    // cleanup
    if (tEvent->findFlag(Event::FLAG_CLEANUP) == true)
      delete tEventHandler;
    delete tEvent;

    myEventList.erase(eventPtr);

    return true;
  } // Server::removeEvent

  const unsigned int Server::clearEvents() {
    eventMapType::iterator ptr;				// Iterator to an Event map.
    unsigned int numRows = 0;

    _writeEvent_l.Lock();
    while(!myEventList.empty()) {
      ptr = myEventList.begin();
      _removeEvent(ptr->first);
      numRows++;
    } // while
    _writeEvent_l.Unlock();

    return numRows;
  } // Server::clearEvents

  /**
   * Server::writeEvent
   *
   * Attempts to post an event through the event system to be received
   * by all registered events.
   *
   * Returns: Number of events registered that received this event.
   */
  const unsigned int Server::writeEvent(const string &writeName, Packet *writePacket) {
    unsigned int ret;

    _writeEvent_l.Lock();
    ret = _writeEvent(writeName, writePacket);
    _writeEvent_l.Unlock();

    return ret;
  } // Server::writeEvent
  const unsigned int Server::_writeEvent(const string &writeName, Packet *writePacket) {
    Event *lEvent;				// Event we're looping through.
    EventHandler *lEventHandler;			// EventHandler we've looped to.
    eventMapType::iterator eventPtr;		// pointer to an Event
    unsigned int i;				// number of events posted

    // initialize variables
    i = 0;

    if ((IsServerDebug || isMonitor(writePacket->getString("aprs.packet.source")))
        && writeName == OPENAPRS_LOG_FILTER_EVENT) {
      COMMAND_DEBUG_STRINGS(writePacket, commandDebugList, "root");
      writeLog(OPENAPRS_LOG_DEBUG_EVENT, "---");
      while(!commandDebugList.empty()) {
        writeLog(OPENAPRS_LOG_DEBUG_EVENT, "  [%-30s]    %s", writeName.c_str(),
                        commandDebugList.front().c_str());
        commandDebugList.pop_front();
      } // while
    } // if

    /*
     *  06/20/2003: Loop through the list of members that would like to receive
     *             a logged event.
     */
    for(eventPtr = myEventList.begin(); eventPtr != myEventList.end(); eventPtr++) {
      // initialize variables
      lEvent = eventPtr->second;
      lEventHandler = lEvent->getEventHandler();
      if (match(lEvent->getMatch().c_str(), writeName.c_str())) {
        lEventHandler->OnEvent(lEvent, writeName, writePacket);
        i++;
      } // if
    } // for

    return i;
  } // Server::writeEvent

  /**
   * Server::OnEvent
   *
   * Called when we are registered for an event.
   *
   * Returns: 1 on success, 0 on failure.
   */
  const int Server::OnEvent(Event *eventInfo, const string &writeName, Packet *writePacket) {
    return 1;
  } // Server::OnEvent
  
  /**
   * Server::getUniqueEventId
   *
   * Find a event Id that is not currently in use and return it for
   * a new registering timer.
   *
   * Returns: Number of type eventId.
   */
  const Event::eventId Server::_getUniqueEventId() {
    eventId uniqueId;                     // Uniqure timer Id to return.

    /*
     * 06/24/2003: The idea here is to loop from 1 to find an id that is not in use.
     *             There's a bunch of different ways to find a unique id, this just
     *             seems to be the easiest and most unwastful. -GCARTER
     */
    for(uniqueId = 1; myEventList.find(uniqueId) != myEventList.end(); uniqueId++);

    return uniqueId;
  } // Server::_getUniqueEventId

/****************************
 ** Initialization Members **
 ****************************/

/**
 * Server::initializeSystem
 *
 * Starts all Timers and Logs needed to run system.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::initializeSystem() {
  // Must be first
  initializeLogs();

  if (IsServerDebug) {
    addConnection(0, "STDIN");
  } // if

  if (cfg->get_bool("openaprs.toggles.aprstest", false)) {
    aprs::UnitTest ut;

    if (!ut.run())
      Die("failed aprs unit test");
  } // if

  std::string source = app->cfg->get_string("app.stompstats.source", "/topic/stats.openaprs");
  std::string instance = app->cfg->get_string("app.stompstats.instance", "prod");
  time_t update_interval = app->cfg->get_int("app.stompstats.interval", 300);
  std::string hosts = app->cfg->get_string("app.stompstats.hosts", "localhost:61613");
  std::string login = app->cfg->get_string("app.stompstats.login", "openaprs-stompstats");
  std::string passcode = app->cfg->get_string("app.stompstats.passcode", "openaprs-stompstats");
  int maxqueue = app->cfg->get_int("app.stompstats.maxqueue", 100);
  std::string dest = app->cfg->get_string("app.stompstats.destination", "/topic/stats");

  _stats = new stomp::StompStats(source,
                                 instance,
                                 update_interval,
                                 maxqueue,
                                 hosts,
                                 login,
                                 passcode,
                                 dest);

  _stats->elogger(&elog, "uplink");
  _stats->start();

  _loadBookmarks();

  if (IsServerSQL) {
    CREATE_PUTMYSQL(Sql, OPENAPRS_SQL_LOGIN);
  } // if

  try {
    dbi = new DBI();
  } // try
  catch(bad_alloc xa) {
    assert(false);
  } // catch

  initializeCommands();
  initializeEvents();
  initializeTimers();

  initializeThreads();

  // We MUST load modules AFTER initializeThreads.
  // Modules can load threads as well that will not get the
  // block signal mask if we don't.
  loadModules();

  _consolef("*** System Initialized");
  _logf("*** System Initialized");
  return true;
} // Server::initializeSystem

/**
 * Server::initializeCommands
 *
 * Adds all commands to system.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::initializeCommands() {

  // user commands
  // addCommand("ACTION", "STDIN", "/ACTION", -1, false, new stdinACTION);
  addCommand("ADD", "STDIN", "/ADD", -1, Command::FLAG_CLEANUP, new stdinDTREE);
    addCommand("MODULE", "STDIN/ADD", "/ADD MODULE <name>", -1, Command::FLAG_CLEANUP, new stdinADDMODULE);
  addCommand("BOOKMARKS", "STDIN", "/BOOKMARKS", -1, Command::FLAG_CLEANUP, new stdinBOOKMARKS);
  addCommand("DEBUG", "STDIN", "/DEBUG", -1, Command::FLAG_CLEANUP, new stdinDEBUG);
  addCommand("DISCONNECT", "STDIN", "/DISCONNECT", -1, Command::FLAG_CLEANUP, new stdinDISCONNECT);
  addCommand("ECHO", "STDIN", "/ECHO", -1, Command::FLAG_CLEANUP, new stdinECHO);
  addCommand("FILTER", "STDIN", "/FILTER <regex>", -1, Command::FLAG_CLEANUP, new stdinFILTER);
  addCommand("HASH", "STDIN", "/HASH <callsign>", -1, Command::FLAG_CLEANUP, new stdinHASH);
  addCommand("HELP", "STDIN", "/HELP", -1, Command::FLAG_CLEANUP, new stdinHELP);
  addCommand("QUERY", "STDIN", "/QUERY <query>", -1, Command::FLAG_CLEANUP, new stdinQUERY);
  addCommand("QUIT", "STDIN", "/QUIT", -1, Command::FLAG_CLEANUP, new stdinQUIT);
  addCommand("RAW", "STDIN", "/RAW", -1, Command::FLAG_CLEANUP, new stdinRAW);
  addCommand("REMOVE", "STDIN", "/REMOVE", -1, Command::FLAG_CLEANUP, new stdinDTREE);
    addCommand("MODULE", "STDIN/REMOVE", "/REMOVE MODULE <name>", -1, Command::FLAG_CLEANUP, new stdinREMOVEMODULE);
  addCommand("STATS", "STDIN", "/STATS", -1, Command::FLAG_CLEANUP, new stdinSTATS);
  addCommand("SHOW", "STDIN", "/SHOW", -1, Command::FLAG_CLEANUP, new stdinDTREE);
    addCommand("CONFIG", "STDIN/SHOW", "/SHOW CONFIG", -1, Command::FLAG_CLEANUP, new stdinSHOWCONFIG);
  addCommand("TEST", "STDIN", "/TEST", -1, Command::FLAG_CLEANUP, new stdinDTREE);
    addCommand("APRS", "STDIN/TEST", "/TEST APRS", -1, Command::FLAG_CLEANUP, new stdinTESTAPRS);
  addCommand("LOAD", "STDIN", "/LOAD <filename>", -1, Command::FLAG_CLEANUP, new stdinLOAD);
  // addCommand("SAY", "STDIN", "/SAY <target> <message>", -1, false, new stdinSAY);
  // addCommand("WALLOPS", "STDIN", "/WALLOPS <message>", -1, false, new stdinWALLOPS);

  // client commands

  // server commands
  // addCommand("\0x1c", "SERVER", "", -1, false, new msgMIC_E);
  // addCommand("\0x1d", "SERVER", "", -1, false, new msgMIC_E);

  //addCommand("!", "SERVER", "", -1, false, new msgAPRS);
  //addCommand("/", "SERVER", "", -1, false, new msgAPRS);
  //addCommand("=", "SERVER", "", -1, false, new msgAPRS);
  //addCommand("@", "SERVER", "", -1, false, new msgAPRS);
  //addCommand(";", "SERVER", "", -1, false, new msgAPRS);

  //addCommand(":", "SERVER", "", -1, false, new msgMESSAGE);
  //addCommand("`", "SERVER", "", -1, false, new msgMIC_E);
  //addCommand("'", "SERVER", "", -1, false, new msgMIC_E);
  //addCommand("$", "SERVER", "", -1, false, new msgNMEA);
  //addCommand("T", "SERVER", "", -1, false, new msgTELEMETRY);

  _consolef("*** Commands Initialized");
  _logf("*** Commands Initialized");

  return true;
} // Server::initializeCommands

/**
 * Server::initializeEvents
 *
 * Adds all commands to system.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::initializeEvents() {

  addEvent("aprs.message", Event::FLAG_CLEANUP, new eventMESSAGETOACK);
  addEvent("aprs.message", Event::FLAG_CLEANUP, new eventMESSAGETOEMAIL);
  addEvent("aprs.message", Event::FLAG_CLEANUP, new eventMESSAGETOVERIFY);
  addEvent("aprs.message", Event::FLAG_CLEANUP, new eventMESSAGETOSQL);
  addEvent("aprs.position", Event::FLAG_CLEANUP, new eventPOSITIONTOSQL);
  addEvent("aprs.telemetry", Event::FLAG_CLEANUP, new eventTELEMETRYTOSQL);
  addEvent("aprs.raw", Event::FLAG_CLEANUP, new eventRAWTOSQL);

  _consolef("*** Events Initialized");
  _logf("*** Events Initialized");

  return true;
} // Server::initializeEvents

/**
 * Server::initializeThreads
 *
 * Adds all commands to system.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::initializeThreads() {
  ConfController::matchListType matchList;
  ThreadMessage *tm;
  sigset_t mask;
  pthread_t newThread_tid;
  unsigned int i;
  unsigned int numWorkerThreads = cfg->get_int("openaprs.threads.worker", 2);

  // Ignore all signals
  sigemptyset(&mask );
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGHUP);
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGUSR2);
  sigaddset(&mask, SIGALRM);
  sigaddset(&mask, SIGPIPE);

  // Block all signals since they will be handled by signal thread
  pthread_sigmask(SIG_BLOCK, &mask, NULL);

  // create our signal handling thread
  pthread_create(&_signalThread_tid, NULL, Server::SignalThread, this);
  _consolef("*** SignalThread Initialized");
  _logf("*** SignalThread Initialized");

  for(i=0; i < numWorkerThreads; i++) {
    tm = new ThreadMessage(i+1);
    tm->var->push_void("app", app);
    pthread_create(&newThread_tid, NULL, Server::WorkerThread, tm);
    _consolef("*** WorkerThread %u Initialized", newThread_tid);
    _logf("*** WorkerThread %u Initialized", newThread_tid);
    _workerThreads.insert(newThread_tid);
  } // for

  cfg->match("openaprs.server?", matchList);
  for(i=1; !matchList.empty(); i++) {
    bool enabled = cfg->get_bool(matchList.front()+".enabled", true);
    if (!enabled) {
      matchList.pop_front();
      continue;
    } // if

    tm = new ThreadMessage(i);
    tm->var->push_void("app", app);
    pthread_create(&newThread_tid, NULL, Server::UplinkThread, tm);
    _consolef("*** UplinkThread %u Initialized", newThread_tid);
    _logf("*** UplinkThread %u Initialized", newThread_tid);
    _uplinkThreads.insert(newThread_tid);
    matchList.pop_front();
    _dispatchMap_l.Lock();
    _dispatchMap[newThread_tid] = new dispatchMessageListType;
    _dispatchMap_l.Unlock();
  } // if

  _consolef("*** Threads Initialized");
  _logf("*** Threads Initialized");

  return true;
} // Server::initializeThreads

/**
 * Server::initializeTimers
 *
 * Starts all Timers neeed to connect and run and all other run once actions.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::initializeTimers() {
  myOnTimerList["openaprs.timer.read"] = addTimer(0, TIMER_CONTINUOUS, true, false, this);
  myOnTimerList["openaprs.timer.stats"] = addTimer(OPENAPRS_TIMER_STATS, TIMER_CONTINUOUS, false, false, this);
  myOnTimerList["openaprs.timer.module"] = addTimer(0, TIMER_CONTINUOUS, true, false, this);

  myOnTimerList["openaprs.timer.clean.database"] = addTimer(OPENAPRS_TIMER_CLEANUP, TIMER_CONTINUOUS, false, false, this);
  myOnTimerList["openaprs.timer.create.aprsmail"] = addTimer(OPENAPRS_TIMER_CREATE, TIMER_CONTINUOUS, false, true, new timerAPRSMAIL);
  myOnTimerList["openaprs.timer.log.cache"] = addTimer(20, TIMER_CONTINUOUS, true, false, this);

  cfg->push_int("openaprs.timer.read", myOnTimerList["openaprs.timer.read"]);
  cfg->push_int("openaprs.timer.stats", myOnTimerList["openaprs.timer.stats"]);
  cfg->push_int("openaprs.timer.clean.database", myOnTimerList["openaprs.timer.clean.database"]);
  cfg->push_int("openaprs.timer.insert", myOnTimerList["openaprs.timer.insert"]);
  cfg->push_int("openaprs.timer.create.objects", myOnTimerList["openaprs.timer.create.objects"]);
  cfg->push_int("openaprs.timer.create.messages", myOnTimerList["openaprs.timer.create.messages"]);
  //cfg->push_int("openaprs.timer.decay", myOnTimerList["openaprs.timer.decay"]);
  cfg->push_int("openaprs.timer.log.cache", myOnTimerList["openaprs.timer.log.cache"]);

  _consolef("*** Timers Initialized");
  _logf("*** Timers Initialized");

  return true;
} // Server::initializeTimers

/**
 * Server::deinitializeThreads
 *
 * Adds all commands to system.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::deinitializeThreads() {
  threadSetType::iterator ptr;
  pthread_t tid;
  sigset_t mask;

  while(!_workerThreads.empty()) {
    ptr = _workerThreads.begin();
    tid = (*ptr);
    _consolef("*** Waiting for WorkerThread %u to Deinitialized", tid);
    _logf("*** Waiting for WorkerThread %u to Deinitialized", tid);

    pthread_join(tid, NULL);
    _workerThreads.erase((*ptr));
  } // while

  while(!_uplinkThreads.empty()) {
    ptr = _uplinkThreads.begin();
    tid = (*ptr);
    _consolef("*** Waiting for UplinkThread %u to Deinitialized", tid);
    _logf("*** Waiting for UplinkThread %u to Deinitialized", tid);
    pthread_join(tid, NULL);
    _dispatchMap_l.Lock();
    delete _dispatchMap[tid];
    _dispatchMap_l.Unlock();
    _uplinkThreads.erase((*ptr));
  } // while

  _consolef("*** WorkerThreads Deinitialized");
  _logf("*** WorkerThreads Deinitialized");

  pthread_join(_signalThread_tid, NULL);
  _consolef("*** SignalThread Deinitialized");
  _logf("*** SignalThread Deinitialized");

  sigemptyset(&mask );
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGHUP);
  sigaddset(&mask, SIGUSR1);
  sigaddset(&mask, SIGUSR2);
  sigaddset(&mask, SIGALRM);
  sigaddset(&mask, SIGPIPE);

  // Set signals back how they were
  pthread_sigmask(SIG_UNBLOCK, &mask, NULL);

  _consolef("*** Threads Deinitialized");
  _logf("*** Threads Deinitialized");

  return true;
} // Server::deinitializeThreads

/**
 * Server::initializeLogs
 *
 * Starts all Logs needed.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::initializeLogs() {
  Log *lLog;					// Log we're looping through.

  /*
   * 06/25/2003: Start the logging facilities for the following internal
   *             logs.  These really need to be done otherwise none of
   *             out critical logs for the primary process will be written. -GCARTER
   */
  myOnLogList[OPENAPRS_LOG_STDOUT] = addLog(OPENAPRS_LOG_STDOUT, this, false);
  myOnLogList[OPENAPRS_LOG_NORMAL] = addLog(OPENAPRS_LOG_NORMAL, this, true);
  myOnLogList[OPENAPRS_LOG_APRS] = addLog(OPENAPRS_LOG_APRS, this, false);
  myOnLogList[OPENAPRS_LOG_DB_ERROR] = addLog(OPENAPRS_LOG_DB_ERROR, this, false);
  myOnLogList[OPENAPRS_LOG_VERIFY] = addLog(OPENAPRS_LOG_VERIFY, this, false);
  myOnLogList[OPENAPRS_LOG_MONITOR] = addLog(OPENAPRS_LOG_MONITOR, this, false);
  myOnLogList[OPENAPRS_LOG_SQL] = addLog(OPENAPRS_LOG_SQL, this, false);
  myOnLogList[OPENAPRS_LOG_DECAY] = addLog(OPENAPRS_LOG_DECAY, this, false);
  myOnLogList[OPENAPRS_LOG_DEBUG] = addLog(OPENAPRS_LOG_DEBUG, this, true);
  myOnLogList[OPENAPRS_LOG_DEBUG_EVENT] = addLog(OPENAPRS_LOG_DEBUG_EVENT, this, false);
  myOnLogList[OPENAPRS_LOG_DEBUG_DROPPED] = addLog(OPENAPRS_LOG_DEBUG_DROPPED, this, false);
  myOnLogList[OPENAPRS_LOG_DEBUG_PACKET] = addLog(OPENAPRS_LOG_DEBUG_PACKET, this, false);
  myOnLogList[OPENAPRS_LOG_DEBUG_SOCKET] = addLog(OPENAPRS_LOG_DEBUG_SOCKET, this, false);

  // Shut off timestamping for SOCKET and DROPPED logs
  lLog = findLog(myOnLogList[OPENAPRS_LOG_DEBUG_SOCKET]);
  if (lLog != NULL)
    lLog->setAddTime(false);

  lLog = findLog(myOnLogList[OPENAPRS_LOG_DEBUG_DROPPED]);
  if (lLog != NULL)
    lLog->setAddTime(false);

  _consolef("*** Logs Initialized");
  _logf("*** Logs Initialized");

  return true;
} // Server::initializeLogs

/**
 * Server::deinitializeSystem
 *
 * Stops all Timers and Logs needed to run system.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::deinitializeSystem() {
  module->clearAll();

  delete Sql;

  if (IsServerDebug)
    removeConnection(0);

  deinitializeThreads();
  deinitializeTimers();
  deinitializeCommands();
  deinitializeEvents();
  deinitializeLogs();

  _consolef("*** System Deinitialized");
  _logf("*** System Deinitialized");

  return true;
} // Server::deinitializeSystem

/**
 * Server::deinitializeCommands
 *
 * Removes all commands from system.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::deinitializeCommands() {

  removeCommand("SERVER");
  removeCommand("STDIN");

  _consolef("*** Commands Deinitialized");
  _logf("*** Commands Deinitialized");

  return true;
} // Server::deinitializeCommands

/**
 * Server::deinitializeEvents
 *
 * Removes all events from system.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::deinitializeEvents() {
  clearEvents();

  _consolef("*** Events Deinitialized");
  _logf("*** Events Deinitialized");

  return true;
} // Server::deinitializeEvents

/**
 * Server::deinitializeTimers
 *
 * Stops all Timers.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::deinitializeTimers() {
  list<string> optionList;            // Option list returned from OptionsList::matchOptions
  string optionName;                  // Name of the looped to option.

  if (cfg->match("openaprs.timer.*", optionList) > 0) {
    for(; !optionList.empty(); optionList.pop_front()) {
      optionName = optionList.front();

      if (cfg->exists(optionName) == true) {
        removeTimer(cfg->get_int(optionName));
        cfg->pop(optionName);
      } // if
    } // for
  } // if

  _consolef("*** Timers Deinitialized");
  _logf("*** Timers Deinitialized");

  return true;
} // Server::deinitializeTimers

/**
 * Server::deinitializeLogs
 *
 * Stops all Logs needed.
 *
 * Returns: true if successful, false if unsuccessful.
 */
const bool Server::deinitializeLogs() {

  _consolef("*** Logs Deinitialized");
  _logf("*** Logs Deinitialized");

  /*
   * 06/25/2003: Stop the logging facilities for the following internal
   *             logs. -GCARTER
   */
  removeLog(myOnLogList[OPENAPRS_LOG_STDOUT]);
  removeLog(myOnLogList[OPENAPRS_LOG_NORMAL]);
  removeLog(myOnLogList[OPENAPRS_LOG_APRS]);
  removeLog(myOnLogList[OPENAPRS_LOG_DEBUG]);
  removeLog(myOnLogList[OPENAPRS_LOG_DEBUG_SOCKET]);

  return true;
} // Server::deinitializeLogs

  void *Server::SignalThread(void *server) {
    Server *s = (Server *) server;
    Network::connectionMapType connectionList;
    Network::connectionMapType::iterator ptr;
    int oldtype;
    int result;

    sigset_t mask;
    siginfo_t info;
    struct timespec timeout;

    // Create a mask holding only SIGINT - ^C Interrupt
    sigemptyset(&mask );
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGPIPE);

    // Set our sigwait timeout
    timeout.tv_sec = 5;
    timeout.tv_nsec = 0;

    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);

    s->writeLog(OPENAPRS_LOG_STDOUT, "*** SignalThread: Initialized");
    s->writeLog(OPENAPRS_LOG_NORMAL, "*** SignalThread: Initialized");

    while(!s->isDone()) {
      result = sigtimedwait( &mask, &info, &timeout );
      if (-1 == result && EAGAIN == errno) {
        //s->writeLog(OPENAPRS_LOG_STDOUT, "*** SIGTIMEDWAIT: Timed out.");
        //s->writeLog(OPENAPRS_LOG_NORMAL, "*** SIGTIMEDWAIT: Timed out.");
        continue;
      } // if

      switch(info.si_signo) {
        case SIGHUP:
          s->writeLog(OPENAPRS_LOG_STDOUT, "*** SIGHUP Received");
          s->writeLog(OPENAPRS_LOG_NORMAL, "*** SIGHUP Received");

          //s->disconnect();
          s->hup();

          break;
        case SIGPIPE:
          s->writeLog(OPENAPRS_LOG_STDOUT, "*** SIGPIPE Received");
          s->writeLog(OPENAPRS_LOG_NORMAL, "*** SIGPIPE Received");

          break;
        case SIGALRM:
          break;
        case SIGUSR1:

          s->cfg->toggle("openaprs.toggles.debug");

          s->writeLog(OPENAPRS_LOG_STDOUT, "*** SIGUSR1 Received, debug: %s", (IsServerDebug == true) ? "ON" : "OFF");
          s->writeLog(OPENAPRS_LOG_NORMAL, "*** SIGUSR1 Received, debug: %s", (IsServerDebug == true) ? "ON" : "OFF");
          break;
        case SIGUSR2:
          s->writeStats(true);

          //app->listConnections(connectionList);
          //for(ptr = connectionList.begin(); ptr != connectionList.end(); ptr++)
          //  app->writeLog(OPENAPRS_LOG_NORMAL, "*** Connections: %d) %s [%s]", ptr->first, ptr->second->getDescription().c_str(), ptr->second->getIP().c_str());

          s->writeLog(OPENAPRS_LOG_STDOUT, "*** SIGUSR2 Received, STATS");
          s->writeLog(OPENAPRS_LOG_NORMAL, "*** SIGUSR2 Received, STATS");
          break;
        case SIGINT:
          s->writeLog(OPENAPRS_LOG_STDOUT, "*** SIGINT Received");
          s->writeLog(OPENAPRS_LOG_NORMAL, "*** SIGINT Received");
          //app->Die("SIGINT Received");
          s->Die("SIGINT Received");
          break;
        default: 
          continue;
          break;
      } // switch
    } // while

    return NULL;
  } // Server::SignalThread

  void *Server::UplinkThread(void *arg) {
    Uplink::queueListType queueList;
    ThreadMessage *tm = (ThreadMessage *) arg;
    Server *a = (Server *) tm->var->get_void("app");
    dispatchMessageListType dpl;
    string id = stringify<unsigned int>(tm->id());
    Stopwatch *sw;
    string servers;
    string defaultPort;
    string vhost = "";
    int maxQueue = a->cfg->get_int("openaprs.server"+id+".maxqueue", 100);
    pthread_t tid = pthread_self();
    time_t intervalStatsCfg = 1;
    time_t timeNextStatsCfg = (time(NULL) + intervalStatsCfg);
    time_t timeDecayRetry = a->cfg->get_int("openaprs.message.decay.retry", 15);
    time_t timeDecayTimeout = a->cfg->get_int("openaprs.message.decay.timeout", 900);

    Uplink *uplink;

    servers = a->cfg->get_string("openaprs.server"+id);
    time_t logstats_interval = a->cfg->get_int("openaprs.server"+id+".logstats.interval", 3600);

    std::string stomp_hosts = a->cfg->get_string("openaprs.server"+id+".stomp.hosts", "localhost:61613");
    std::string stomp_login = a->cfg->get_string("openaprs.server"+id+".stomp.login", "openaprs-uplink");
    std::string stomp_passcode = a->cfg->get_string("openaprs.server"+id+".stomp.passcode", "openaprs-uplink");

    try {
      uplink = new Uplink(id,
                          servers,
                          vhost,
                          stomp_hosts,
                          stomp_login,
                          stomp_passcode,
                          IsServerDropDuplicates
                         );
    } // try
    catch(Uplink_Exception e) {
      a->Die("Failed to create uplink thread; %s"+string(e.message()));
    } // catch

    uplink->elogger(&elog, "uplink");
    uplink->replace_stats( a->stats(), "openaprs.uplink" );

    uplink->create( a->cfg->get_bool("openaprs.server"+id+".create", false) );
    uplink->set_logstats_interval(logstats_interval)
           .set_decay_retry(timeDecayRetry)
           .set_decay_timeout(timeDecayTimeout)
           .init();

    sw = new Stopwatch();
    sw->add("flush", 300);

    uplink->burst(a->cfg->get_string("openaprs.server"+id+".burst", ""));
    uplink->interval(a->cfg->get_int("openaprs.server"+id+".connect.interval",60));
//    uplink->script("%V", VERSION);
//    uplink->script("%T", stringify<unsigned int>(time(NULL)) );
//    uplink->script("%%", "%");

    while(!a->isDone()) {
      pthread_testcancel();

      sw->Start();

//      if (a->isDisconnect())
//        uplink->disconnect();

      dpl.clear();
      a->dequeueDispatch(tid, dpl);
      while(!dpl.empty()) {
        uplink->message(dpl.front());
        delete dpl.front();
        dpl.pop_front();
      } // while

      bool didWork = uplink->run();

      queueList.clear();
      uplink->dequeue(queueList, maxQueue);
      while(!queueList.empty()) {
        bool ok = app->addWork(queueList.front());
        if (!ok) break;
        queueList.pop_front();
      } // while

      sw->average("flush", sw->Time());

      if (timeNextStatsCfg < time(NULL)) {
        a->cfg->replace_double("openaprs.count.thread.server"+id+".loop", sw->average("flush"));
        a->cfg->replace_double("openaprs.count.thread.server"+id+".speed.bps", uplink->average("bps") );
        a->cfg->replace_double("openaprs.count.thread.server"+id+".speed.kbps", uplink->average("bps")/1024.0 );
        a->cfg->replace_double("openaprs.count.thread.server"+id+".speed.pps", uplink->average("pps") );
        a->cfg->replace_double("openaprs.count.thread.server"+id+".run.loop", uplink->average("run.loop") );
        a->cfg->replace_uint("openaprs.count.thread.server"+id+".total.packets", uplink->totalPackets() );
        a->cfg->replace_uint("openaprs.count.thread.server"+id+".elapsed", uplink->elapsed() );
        a->cfg->replace_uint("openaprs.count.thread.server"+id+".queue", uplink->queueSize() );
//        a->cfg->replace_string("openaprs.count.thread.server"+id, uplink->server() );
        timeNextStatsCfg = time(NULL) + intervalStatsCfg;
      } // if

      //if (!didWork) sleep(1);
      usleep(10000);
    } // while

    uplink->stop();

    delete uplink;
    delete sw;
    delete tm;

    return NULL;
  } // Server::UplinkThread

  void *Server::WorkerThread(void *arg) {
    Worker *worker;
    workListType workList;
    Worker::resultSetType resultSet;
    Worker::resultSetType::iterator ptr;
    ThreadMessage *tm = (ThreadMessage *) arg;
    Server *a = (Server *) tm->var->get_void("app");
    string id = stringify<unsigned int>(tm->id());
    Stopwatch sw;
    int maxQueue = a->cfg->get_int("openaprs.threads.worker.maxqueue", 100);

    std::string stomp_hosts = a->cfg->get_string("openaprs.threads.worker.stomp.hosts", "localhost:61613");
    std::string stomp_login = a->cfg->get_string("openaprs.threads.worker.stomp.login", "openaprs-uplink");
    std::string stomp_passcode = a->cfg->get_string("openaprs.threads.worker.stomp.passcode", "openaprs-uplink");

    worker = new Worker(id,
                        stomp_hosts,
                        stomp_login,
                        stomp_passcode,
                        a->cfg->get_int("openaprs.threads.worker.stats.interval", 10),
                        a->cfg->get_int("openaprs.threads.worker.sql.insert.interval", 3),
                        IsServerDropDuplicates,
                        a->cfg->get_int("openaprs.threads.worker.memcached.expire", 3600)
                       );
    worker->logger(new App_Log("Worker t:"+id) );
    worker->replace_stats( a->stats(), "openaprs.worker"+id );
    worker->init();
    worker->timeMessageSessionExpire(a->cfg->get_int("openaprs.message.timeout", 300));


    while(!a->isDone()) {
      pthread_testcancel();

      if (worker->workSize() < (unsigned int) maxQueue)
        a->dequeueWork(workList, maxQueue - worker->workSize());

      while(!workList.empty()) {
        worker->add(workList.front());
        workList.pop_front();
      } // while

      bool didWork = worker->run();

      worker->results(resultSet, maxQueue);
      while(!resultSet.empty()) {
        ptr = resultSet.begin();
        a->queueResult((*ptr));
        resultSet.erase(ptr);
      } // while

      if (!didWork) usleep(10000);
    } // while

    delete worker;
    delete tm;

    return NULL;
  } // Server::WorkerThread

  const unsigned int Server::_processResults() {
    APRS *aprs;
    Result *result;
    bool isFireEvent = true;
    unsigned int numResults = 0;
    resultListType::iterator ptr;

    _resultList_l.Lock();
    while(!_resultList.empty()) {
      if (numResults > 100)
        break;

      ptr = _resultList.begin();
      result = *ptr;
      _resultList.erase(ptr);			// remove from result list
      aprs = result->aprs();

      if (aprs == NULL) {
        cfg->inc("openaprs.count.packets.dropped", 1);
        if (IsServerDebug)
          writeLog(OPENAPRS_LOG_DEBUG_DROPPED, "%s [%s]", result->packet().c_str(), result->error().c_str());
      } // if
      else
        cfg->inc("openaprs.count.packets.parsed", 1);

      if (cfg->get_uint("openaprs.count.packets.dropped", 0) && cfg->get_uint("openaprs.count.packets", 0))
        cfg->replace_double("openaprs.count.packets.parsed.percent",
          100-(double(cfg->get_uint("openaprs.count.packets.dropped")) / double(cfg->get_uint("openaprs.count.packets"))*100.00));

      if (result->reject())
        cfg->inc("openaprs.count.packets.rejected", 1);


      if (IsServerConsole && IsServerEcho) {
        if (aprs == NULL)
          cout << "x";
//          _consolef("x");
        else if (result->duplicate())
          cout << "=";
//          _consolef("=");
        else if (result->reject())
          cout << "r";
//          _consolef("r");
        else
          cout << ".";
//          _consolef(".");

        if (aprs && IsServerRaw)
          cout << " > " << aprs->packet();

        cout.flush();
      } // if

      if (IsServerDropDuplicates
          && (result->duplicate() || result->reject()))
        isFireEvent = false;

      // fire events
      if (aprs != NULL && isFireEvent) {
        //nPacket = new Packet();
        //nPacket->addVariable("aprs", aprs);
        //nPacket->addVariable("sqlq", InsertSQL);
        switch(aprs->packetType()) {
          case APRS::APRS_PACKET_POSITION:
            //writeEvent("aprs.position", nPacket);
            cfg->inc("openaprs.count.packets.type.position", 1);
            break;
          case APRS::APRS_PACKET_MESSAGE:
            //app->writeEvent("aprs.message", nPacket);
            cfg->inc("openaprs.count.packets.type.message", 1);
            break;
          case APRS::APRS_PACKET_TELEMETRY:
            //app->writeEvent("aprs.telemtry", nPacket);
            cfg->inc("openaprs.count.packets.type.telemetry", 1);
            break;
          case APRS::APRS_PACKET_STATUS:
            //app->writeEvent("aprs.status", nPacket);
            cfg->inc("openaprs.count.packets.type.status", 1);
            break;
          case APRS::APRS_PACKET_CAPABILITIES:
            //app->writeEvent("aprs.capabilities", nPacket);
            cfg->inc("openaprs.count.packets.type.capabilities", 1);
            break;
          case APRS::APRS_PACKET_PEETLOGGING:
            //app->writeEvent("aprs.peetlogging", nPacket);
            cfg->inc("openaprs.count.packets.type.peetlogging", 1);
            break;
          case APRS::APRS_PACKET_WEATHER:
            //app->writeEvent("aprs.weather", nPacket);
            cfg->inc("openaprs.count.packets.type.weather", 1);
            break;
          case APRS::APRS_PACKET_DX:
            //app->writeEvent("aprs.dx", nPacket);
            cfg->inc("openaprs.count.packets.type.dx", 1);
            break;
          case APRS::APRS_PACKET_EXPERIMENTAL:
            //app->writeEvent("aprs.experimental", nPacket);
            cfg->inc("openaprs.count.packets.type.experimental", 1);
            break;
          case APRS::APRS_PACKET_BEACON:
            //app->writeEvent("aprs.beacon", nPacket);
            cfg->inc("openaprs.count.packets.type.beacon", 1);
            break;
          case APRS::APRS_PACKET_UNKNOWN:
            //app->writeEvent("aprs.unknown", nPacket);
            cfg->inc("openaprs.count.packets.type.unknown", 1);
            break;
          default:
            break;
        } // switch
        //delete nPacket;
      } // if

      delete result;
      numResults++;
    } // while
    _resultList_l.Unlock();

    cfg->inc("openaprs.count.packets", numResults);

    if (cfg->get_uint("openaprs.count.time.elapsed", 0) > 0 && cfg->get_uint("openaprs.count.packets", 0) > 0) {
      double pps = double( cfg->get_uint("openaprs.count.packets") ) / double( cfg->get_uint("openaprs.count.time.elapsed") );
      cfg->replace_double("openaprs.count.time.average.pps", pps);
    } // if

    return numResults;
  } // Server::_processResults

  void Server::queueResult(Result *result) {
    assert(result != NULL);

    openframe::scoped_lock slock(&_resultList_l);
    _resultList.push_back(result);
  } // Server::queueResult

  const unsigned int Server::dequeueWork(workListType &workList, const int limit) {
    int numRows = 0;

    _workList_l.Lock();
    while(!_workList.empty()) {
      if (numRows > limit)
        break;

      workList.push_back(_workList.front());
      _workList.pop_front();
      numRows++;
    } // while
    cfg->replace_uint("openaprs.count.queue.work", workList.size() );
    _workList_l.Unlock();

    return numRows;
  } // Server::dequeueWork

  const bool Server::Run() {
    Stopwatch sw;
    double totalTime;

    // Main run loop
    while(true) {
      if (isDone())
        break;

      sw.Start();
      cfg->inc("openaprs.count.loop.main", 1);

/*
      StdinController::commandListType cl;
      _stdinController->dequeue(cl);
      while(!cl.empty()) {
        ParseSTDIN(cl.front());
        cl.pop_front();
      } // while
*/
      /******************
       ** Check Timers **
       ******************/
      checkTimer();

      _processResults();

      totalTime += sw.Time();

      if (cfg->get_uint("openaprs.count.loop.main", 0)) {
        double avgLoop = double(totalTime) / double( cfg->get_uint("openaprs.count.loop.main") );
        cfg->replace_double("openaprs.count.time.average.loop.main", avgLoop);
      } // if
    } // while

    return true;
  } // Server::Run

  void Server::Done() {
    Lock();
    _done = true;
    Unlock();
  } // Server::Done

  void Server::Writestats() {
    Lock();
    _writestats = true;
    Unlock();
  } // Server::Done

  void Server::disconnect() {
//    Lock();
//    _disconnect = true;
//    Unlock();
    enqueueDispatch(new ThreadMessage("disconnect", ""));
  } // Server::Done

  const bool Server::isDone() {
    bool isDone;

    Lock();
    isDone = _done;
    Unlock();

    return isDone;
  } // Server::isDone

  const bool Server::isWritestats() {
    bool isWritestats;

    Lock();
    isWritestats = _writestats;
    Unlock();

    return isWritestats;
  } // Server::isWritestats


  const bool Server::isDisconnect() {
    bool isDisconnect;

    Lock();
    isDisconnect = _disconnect;
    Unlock();

    return isDisconnect;
  } // Server::isDone

  /********************
   ** Module Members **
   ********************/

  /**
   * Server::loadModules
   *
   * Attempts to load all modules from the autoload table.
   *
   * Returns: Nothing.
   */
  const unsigned int  Server::loadModules() {
    list<string> optionList;                    // Option list returned from match.
    string optionName;                          // Name of the option.
    string optionString;                        // String of the option.
    unsigned int numLoaded;                     // Number of modules loaded.

    // initialize variables
    numLoaded = 0;

//    if (!cfg->is_array("openaprs.modules.load"))
//      return numLoaded;

    for(size_t i=0; i < cfg->array_size("openaprs.modules.load"); i++) {
      string path = cfg->array_string("openaprs.modules.load", i);
      module->add(path, false);
      writeLog(OPENAPRS_LOG_NORMAL, "*** Module: %s loaded.", path.c_str());
      _consolef("*** Module: %s loaded.", path.c_str());
      numLoaded++;
    } // for

    return numLoaded;
  } // Server::loadModules

  const bool Server::isMonitor(const string &source) {
    StringToken s;
    string monitor;

    if (!cfg->exists("openaprs.log.monitor.source"))
      return false;

    monitor = strtoupper(cfg->get_string("openaprs.log.monitor.source"));

    if (source.length() < 1
        || monitor.length() < 1)
      return false;

    s.setDelimiter(',');
    s = strtoupper(source);

    for(size_t i=0; i < s.size(); i++) {
      if (monitor == s[i])
        return true;
    } // for

    return false;
  } // Server::isMonitor

/*****************
 ** Log Members **
 *****************/

const Log::logId Server::addLog(const string &addMatch, LogHandler *addLogHandler, const bool shouldCache) {
  Log *nLog;				// pointer to a new log
  logId newId;				// new log id

  assert(addLogHandler != NULL);

  // initialize variables
  newId = getUniqueLogId();
  nLog = new Log(newId, addMatch, addLogHandler, shouldCache);

  _debugf("Server::addLog> Added Id: %d", newId);

  myLogList.insert(pair<logId, Log *>(newId, nLog));

  return newId;
} // Server::addLog

Log *Server::findLog(const logId findMe) {
  logMapType::iterator logPtr;			// pointer to an Log

  if ((logPtr = myLogList.find(findMe)) != myLogList.end())
    return logPtr->second;

  return NULL;
} // Server::findLog

const bool Server::removeLog(const logId removeMe) {
  logMapType::iterator logPtr;			// pointer to a log

  if ((logPtr = myLogList.find(removeMe)) == myLogList.end())
    return false;

  _debugf("Server::removeLog> Removed Id: %d", removeMe);

  // cleanup
  delete logPtr->second;

  myLogList.erase(logPtr);

  return true;
} // Server::removeLog

const unsigned int Server::writeLog(const string &writeName, const char *writeFormat, ...) {
  Log *lLog;					// Log we're looping through.
  LogHandler *lLogHandler;			// LogHandler we've looped to.
  logMapType::iterator logPtr;			// pointer to an Log
  char writeBuffer[MAXBUF + 1];
  va_list va;

  _logList_l.Lock();

  va_start(va, writeFormat);
  vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
  va_end(va);

/*
  if (writeName == OPENAPRS_LOG_STDOUT) {
    _stdinController->print(writeBuffer);
    _logList_l.Unlock();
    return strlen(writeBuffer);
  } // if
*/

  /*
   * 06/20/2003: Loop through the list of members that would like to receive
   *             a logged event.
   */
  for(logPtr = myLogList.begin(); logPtr != myLogList.end(); logPtr++) {
    // initialize variables
    lLog = logPtr->second;
    lLogHandler = lLog->getLogHandler();

    if (match(lLog->getMatch().c_str(), writeName.c_str()))
      lLogHandler->OnLog(lLog, logPtr->first, writeBuffer);
  } // for

  // myLogBuffer.add(writeBuffer);

  _logList_l.Unlock();

  return strlen(writeBuffer);
} // Server::writeLog

void Server::hup() {
  Log *log;                                     // Log we're looping through.
  LogHandler *lh;
  logMapType::iterator itr;                     // pointer to an Log

  _logList_l.Lock();

  for(itr = myLogList.begin(); itr != myLogList.end(); itr++) {
    // initialize variables
    log = itr->second;
    lh = log->getLogHandler();

    if (log->isLogOpen()) {
      lh->OnLog(log, itr->first, "*** SIGHUP Received closing log");
      log->closeLog();
    } // if
  } // for

  _logList_l.Unlock();
} // Server::hup

const unsigned int Server::writeEmail(const char *writeFormat, ...) {
  char writeBuffer[MAXBUF + 1] = {0};
  va_list va;

  if (cfg->get_bool("openaprs.email.alerts", false) == false)
    return 0;

  va_start(va, writeFormat);
  vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
  va_end(va);

  string to = cfg->get_string("openaprs.email.admin", "none");
  string from = "OpenAPRS";
  stringstream s;
  struct tm *tm;                                // struct tm for localtime
  time_t now;                                   // current time
  char timestamp[24];                           // Currne timestamp.

  now = time(NULL);
  tm = gmtime(&now);

  snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d UTC",
           (tm->tm_year + 1900),
           (tm->tm_mon + 1),
           tm->tm_mday,
           tm->tm_hour,
           tm->tm_min,
           tm->tm_sec);

    s.str("");
    s << "----------------------------------------\n"
      << "  Date     : " << timestamp << "\n"
      << "  To       : " << to << "\n"
      << "  From     : " << from << "\n"
      << "  Message  : " << writeBuffer << "\n"
      << "----------------------------------------\n"
      << "\n\n---\nOpenAPRS.Net Message\n";

  Email("OpenAPRS", to, s.str());

  return strlen(writeBuffer);
} // Server::writeEmail

const unsigned int Server::Query(const char *writeFormat, ...) {
  Stopwatch sw;
  bool isOK;
  char writeBuffer[PUTMYSQL_MAXBUF + 1] = {0};
  int query_size;
  va_list va;

  va_start(va, writeFormat);
  query_size = vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
  va_end(va);

  sw.Start();
  isOK = Sql->query(writeBuffer, query_size);

  // Causes a log deadlock
//  if (isOption("openaprs.log.filter.sql")) {
//    if (sw.Time() >= double(getOptionInt("openaprs.log.filter.sql")))
//      writeLog(OPENAPRS_LOG_SQL, "[%f>%d:%i] %s\n", double(sw.Time()), getOptionInt("openaprs.log.filter.sql"), strlen(writeBuffer), writeBuffer);
//  } // if
//  else
//    writeLog(OPENAPRS_LOG_SQL, "[%f:%i] %s\n", double(sw.Time()), strlen(writeBuffer), writeBuffer);

  return isOK;
} // Server::Query

const unsigned int Server::lQuery(PutMySQL *lSql, const char *writeFormat, ...) {
  Stopwatch sw;
  bool isOK;
  char writeBuffer[PUTMYSQL_MAXBUF + 1] = {0};
  int query_size;
  va_list va;

  va_start(va, writeFormat);
  query_size = vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
  va_end(va);

  sw.Start();
  isOK = lSql->query(writeBuffer, query_size);

//  if (isOption("openaprs.log.filter.sql")) {
//    if (sw.Time() >= double(getOptionInt("openaprs.log.filter.sql")))
//      writeLog(OPENAPRS_LOG_SQL, "l[%f>%d:%i] %s\n", double(sw.Time()), getOptionInt("openaprs.log.filter.sql"), strlen(writeBuffer), writeBuffer);
//  } // if
//  else
//    writeLog(OPENAPRS_LOG_SQL, "l[%f:%i] %s\n", double(sw.Time()), strlen(writeBuffer), writeBuffer);

  return isOK;
} // Server::lQuery

const int Server::OnLog(Log *logInfo, const logId forId, const string &logBuffer) {
  Vars v;
  string tempBuf;				// temp local buffer
  string safeBuffer;				// Screen safe buffer.
  stringstream s;				// temp string stream.
  unsigned int i;				// tmp counter.

  // make the buffer safe
  for(i = 0; i < logBuffer.length(); i++) {
    if ((int) logBuffer[i] < 32 ||
        (int) logBuffer[i] > 126)
      s << "\\x" << v.char2hex(logBuffer[i]);
    else
      s << logBuffer[i];
  } // for

  safeBuffer = s.str();

  if (forId == myOnLogList[OPENAPRS_LOG_STDOUT]) {
    //_consolef("%s", logBuffer.c_str());
    if (IsServerDebug)
      cout << safeBuffer << endl;
  } // if
  else if (forId == myOnLogList[OPENAPRS_LOG_NORMAL]) {
    if (logInfo->isLogOpen() == false)
      logInfo->openLog((OPENAPRS_PATH_LOGS + OPENAPRS_PATH_LOGS_OPENAPRS), ofstream::out | ofstream::app);

    s.str("");

    s << cfg->get_int("openaprs.process.id", -1) << " " << safeBuffer;

    logInfo->appendLog((s.str() + "\n"));
    if (OPENAPRS_LOG_FLUSH == true)
      logInfo->flushLog();
  } // if
  else if (forId == myOnLogList[OPENAPRS_LOG_DB_ERROR]
           && IsServerSQL && IsServerSQLInject) {

    dbi->writeError(safeBuffer);
  } // if
  else if (forId == myOnLogList[OPENAPRS_LOG_VERIFY]) {
    if (logInfo->isLogOpen() == false)
      logInfo->openLog((OPENAPRS_PATH_LOGS + OPENAPRS_PATH_LOGS_VERIFY), ofstream::out | ofstream::app);

    logInfo->appendLog((safeBuffer + "\n"));
    if (OPENAPRS_LOG_FLUSH == true)
      logInfo->flushLog();
  } // if
  else if (forId == myOnLogList[OPENAPRS_LOG_DECAY]) {
    if (logInfo->isLogOpen() == false)
      logInfo->openLog((OPENAPRS_PATH_LOGS + OPENAPRS_PATH_LOGS_DECAY), ofstream::out | ofstream::app);

    logInfo->appendLog((safeBuffer + "\n"));
    if (OPENAPRS_LOG_FLUSH == true)
      logInfo->flushLog();
  } // if
  else if (forId == myOnLogList[OPENAPRS_LOG_APRS]) {
    if (logInfo->isLogOpen() == false)
      logInfo->openLog((OPENAPRS_PATH_LOGS + OPENAPRS_PATH_LOGS_APRS), ofstream::out | ofstream::app);

    logInfo->appendLog((safeBuffer + "\n"));
    if (OPENAPRS_LOG_FLUSH == true)
      logInfo->flushLog();
  } // if
  else if (forId == myOnLogList[OPENAPRS_LOG_DEBUG_EVENT]) {
    if (logInfo->isLogOpen() == false)
      logInfo->openLog((OPENAPRS_PATH_LOGS + OPENAPRS_PATH_LOGS_DEBUG_EVENT), ofstream::out | ofstream::app);

    logInfo->appendLog((safeBuffer + "\n"));
    if (OPENAPRS_LOG_FLUSH == true)
      logInfo->flushLog();
  } // if
  else if (forId == myOnLogList[OPENAPRS_LOG_SQL]
           && IsServerLogSQL == true) {
    if (logInfo->isLogOpen() == false)
      logInfo->openLog((OPENAPRS_PATH_LOGS + OPENAPRS_PATH_LOGS_SQL), ofstream::out | ofstream::app);

    logInfo->appendLog((safeBuffer + "\n"));
    if (OPENAPRS_LOG_FLUSH == true)
      logInfo->flushLog();
  } // if
  else if (forId == myOnLogList[OPENAPRS_LOG_MONITOR]) {
    if (logInfo->isLogOpen() == false)
      logInfo->openLog((OPENAPRS_PATH_LOGS + OPENAPRS_PATH_LOGS_MONITOR), ofstream::out | ofstream::app);

    logInfo->appendLog((safeBuffer + "\n"));
    if (OPENAPRS_LOG_FLUSH == true)
      logInfo->flushLog();
  } // if
  else if (forId == myOnLogList[OPENAPRS_LOG_DEBUG] &&
           IsServerDebug == true) {
    /*
     * 06/20/2003: If log file isn't open, try to open it using openaprs's default
     *             log path. -GCARTER
     */
    if (logInfo->isLogOpen() == false)
      logInfo->openLog((OPENAPRS_PATH_LOGS + OPENAPRS_PATH_LOGS_DEBUG), ofstream::out | ofstream::app);

    logInfo->appendLog((safeBuffer + "\n"));
    if (OPENAPRS_LOG_FLUSH == true)
      logInfo->flushLog();
  } // else if
  else if (forId == myOnLogList[OPENAPRS_LOG_DEBUG_PACKET]) {
    /*
     * 06/20/2003: If log file isn't open, try to open it using openaprs's default
     *             log path. -GCARTER
     */
    if (logInfo->isLogOpen() == false)
      logInfo->openLog((OPENAPRS_PATH_LOGS + OPENAPRS_PATH_LOGS_DEBUG_PACKET), ofstream::out | ofstream::app);

    logInfo->appendLog((safeBuffer + "\n"));
    if (OPENAPRS_LOG_FLUSH == true)
      logInfo->flushLog();
  } // else if
  else if (forId == myOnLogList[OPENAPRS_LOG_DEBUG_DROPPED] &&
           IsServerDebug == true) {
    /*
     * 06/20/2003: If log file isn't open, try to open it using openaprs's default
     *             log path. -GCARTER
     */
    if (logInfo->isLogOpen() == false)
      logInfo->openLog((OPENAPRS_PATH_LOGS + OPENAPRS_PATH_LOGS_DEBUG_DROPPED), ofstream::out | ofstream::app);

    logInfo->appendLog((safeBuffer + "\n"));
    if (OPENAPRS_LOG_FLUSH == true)
      logInfo->flushLog();
  } // else if
  else if (forId == myOnLogList[OPENAPRS_LOG_DEBUG_SOCKET] &&
           IsServerDebug == true) {
    /*
     * 06/20/2003: If log file isn't open, try to open it using openaprs's default
     *             log path. -GCARTER
     */
    if (logInfo->isLogOpen() == false)
      logInfo->openLog((OPENAPRS_PATH_LOGS + OPENAPRS_PATH_LOGS_DEBUG_SOCKET), ofstream::out | ofstream::app);

    tempBuf = safeBuffer;
    stripLF(tempBuf);

    logInfo->appendLog((tempBuf + "\n"));

    if (OPENAPRS_LOG_FLUSH == true)
      logInfo->flushLog();
  } // else if

  return logBuffer.length();
} // Server::OnLog

/**
 * Server::getUniqueLogId
 *
 * Find a log Id that is not currently in use and return it for
 * a new registering timer.
 *
 * Returns: Number of type logId.
 */
const Log::logId Server::getUniqueLogId() {
  logId uniqueId;                     // Uniqure timer Id to return.
    
  /*
   * 06/24/2003: The idea here is to loop from 1 to find an id that is not in use.
   *             There's a bunch of different ways to find a unique id, this just
   *             seems to be the easiest and most unwastful. -GCARTER
   */
  for(uniqueId = 1; myLogList.find(uniqueId) != myLogList.end(); uniqueId++);
 
  return uniqueId;
} // Server::getUniqueLogId

/*********************
 ** OnTimer Members **
 *********************/

const int Server::OnTimer(Timer *aTimer) {
  Log *lLog;					// Log we're looping through.
  LogHandler *lLogHandler;			// LogHandler we've looped to.
  logMapType::iterator logPtr;			// pointer to an Log
  static const char *tables[] = { 
    "lastposition",
    "laststatus",
    "lastweather",
    "messages",
    "position",
    "raw",
    "weather",
    "telemetry",
    "cache_30m",
    "cache_24hr",
    "raw_errors",
    "igated",
    NULL
  };

  static const char *ident[] = { "dA", "dB", "dC", "dD", "dE", "dF", "dG", "dH", "dI", "dJ", "dK", "dL", NULL };

  ModuleAction ma;
  Stopwatch sw;
  char y[10];
  stringstream s;				// Stringstream!
  unsigned int onTimerId;			// timer id for timer event we received
  static unsigned int i=0;				// Counter
  unsigned int affected_rows[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  // initialize variables
  onTimerId = aTimer->getId();

  if (onTimerId == myOnTimerList["openaprs.timer.read"])
    Read();
  else if (onTimerId == myOnTimerList["openaprs.timer.stats"])
    writeStats(false);
  else if (onTimerId == myOnTimerList["openaprs.timer.log.cache"]) {
    for(logPtr = myLogList.begin(); logPtr != myLogList.end(); logPtr++) {
      // initialize variables
      lLog = logPtr->second;
      lLogHandler = lLog->getLogHandler();

      lLog->checkCache();
    } // for
  } // else if
  else if (onTimerId == myOnTimerList["openaprs.timer.module"]) {
    while(!moduleActionList.empty()) {
      ma = moduleActionList.front();

      if (ma.action == ModuleAction::add) {
        if (app->module->add(ma.path, true)) {
          _logf("*** Module: %s loaded.", ma.path.c_str());
        } // if
      } // if
      if (ma.action == ModuleAction::remove) {
        if (app->module->remove(ma.path)) {
          _logf("*** Module: %s unloaded.", ma.path.c_str());
        } // if
      } // if
      if (ma.action == ModuleAction::reload) {
        app->module->remove(ma.path);

        if (app->module->add(ma.path, true)) {
          _logf("*** Module: %s reloaded.", ma.path.c_str());
        } // if
      } // if

      moduleActionList.pop_front();
    } // while
  } // if
  else if (onTimerId == myOnTimerList["openaprs.timer.clean.database"]
           && cfg->get_bool("openaprs.sql.cleanup.enable", false) == true) {

    if (!IsServerSQL || !IsServerSQLInject)
      return 1;

    Query("UPDATE daemon_status SET comment = '%s', create_ts = '%d' WHERE name = '%s'",
               VERSION, time(NULL), "version");

    Query("UPDATE daemon_status SET count = '%d', create_ts = '%d' WHERE name = '%s'",
                   cfg->get_uint("openaprs.count.time.connected", 0), time(NULL), "uptime");


    if (cfg->is_bool("openaprs.count.netburst"))
      return 1;


    if (tables[i] != NULL) {
      sw.Start();

      Query("DELETE LOW_PRIORITY QUICK FROM %s WHERE create_ts < '%d' LIMIT %d",
                 tables[i], (time(NULL)- cfg->get_int("openaprs.sql.cleanup.tables."+string(tables[i]), 864000)),
                 cfg->get_int("openaprs.sql.cleanup.limit."+string(tables[i]), 100));

      affected_rows[i] = Sql->num_affected_rows();
      Query("UPDATE daemon_status SET count = (count + %d), hits=(hits+1), create_ts = '%d', time=(time+%.2f) WHERE name = '%s'",
                 affected_rows[i], time(NULL), sw.Time(), tables[i]);

      snprintf(y, sizeof(y), "%.4f", double(sw.Time()));
      cfg->replace_double("openaprs.count.time.average.sql.delete."+string(tables[i]), double(sw.Time()) );
      cfg->replace_uint("openaprs.count.sql.delete."+string(tables[i]), affected_rows[i]);

      if (IsServerConsole) {
        s << ident[i] << affected_rows[i];
        _consolef("%s", s.str().c_str());
      } // if

      i++;
    } // if
    else
      i = 0;

    return 1;

  } // else if

  return 1;
} // Server::OnTimer

/*******************
 ** Parse Members **
 *******************/

const unsigned int Server::ParseSTDIN(const string &parseMe) {
  Packet aPacket;			// packet to send to command parser
  Command *tCommand;			// pointer to a target command
  StringToken aToken;			// string tokenizer
  StringToken parseCommand;		// string tokenizer for command parsing
  bool wantExecute = false;		// execute a command?
  int returnResult;			// command return result
  string tempBuf;			// temporary buffer
  unsigned int nodeLevel;		// node level in command
  StringTool::regexMatchListType rl;

  if (parseMe.length() < 1)
    return 0;

  // initialize variables
  tempBuf = parseMe;
  aprs::StringTool::stripcrlf(tempBuf);

  bool storecommand = true;
  if (tempBuf == "!") {
    if (_lastCommand.length()) {
      tempBuf = "/" + _lastCommand;
      _consolef("*** Executing: %s", tempBuf.c_str());
      wantExecute = true;
    } // if
    else
      _consolef("*** No last command found.");
  } // if
  else if (StringTool::ereg("^[!]([0-9]+)$", tempBuf, rl)) {
    size_t n = atoi(rl[1].matchString.c_str());
    if (n >= _commandHist.size()) {
      cout << "Error, command history number must match a record." << endl;
      return 0;
    } // if

    tempBuf = "/" + _commandHist[n];
    _consolef("Executing: /%s", _commandHist[n].c_str());
    storecommand = false;
  } // else if
  else if (StringTool::ereg("^[$]([0-9]+)$", tempBuf, rl)) {
    size_t n = atoi(rl[1].matchString.c_str());
    if (n > _commandHist.size()) {
      cout << "Error, command history number must match a record." << endl;
      return 0;
    } // if

    _bookmarks.push_back(_commandHist[n]);
    _consolef("Bookmarking: %lu /%s", _bookmarks.size()-1, _commandHist[n].c_str());
    storecommand = false;
    return 0;
  } // else if
  else if (StringTool::ereg("^[!][!]([0-9]+)$", tempBuf, rl)) {
    size_t n = atoi(rl[1].matchString.c_str());
    if (n > _bookmarks.size()) {
      cout << "Error, bookmark number must match a record." << endl;
      return 0;
    } // if

    tempBuf = "/" + _bookmarks[n];
    _consolef("Executing: /%s", _bookmarks[n].c_str());
    storecommand = false;
  } // else if
  else if (tempBuf == "#") {
    bookmarks();
    return 0;
  } // else if
  else if (tempBuf == "@") {
    history();
    return 0;
  } // else if

  if (tempBuf[0] != '/') {
    cout << "Type /HELP for a list of commands." << endl;
    cout << "! = run last command, ![0-9] = run # command, @ = view command history" << endl
         << "#  = view bookmarks, !![0-9] = run # command from bookmarks, $[0-9] = save to bookmarks" << endl;

    //Send("%s", tempBuf.c_str());
    return 0;
  } // if
  else
    tempBuf.replace(0, 1, "");

  // initialize variables
  parseCommand = tempBuf;
  nodeLevel = 0;
  returnResult = time(NULL);

  // find the command
  tCommand = findCommand("STDIN", parseCommand, nodeLevel, NULL);

  if (tCommand != NULL) {
    aPacket.setSource("STDIN");
    aPacket.setArguments(parseCommand.getRange(nodeLevel, parseCommand.size()));
    aPacket.setPath(tCommand->getPath());

    returnResult = tCommand->getCommandHandler()->Execute(&aPacket);
  } // if

  switch(returnResult) {
    case CMDERR_SUCCESS:
      _lastCommand = tempBuf;
      for(commandHistoryType::size_type i=0; i < _commandHist.size(); i++) {
        if (!openframe::StringTool::compare(tempBuf.c_str(), _commandHist[i].c_str()))
          storecommand = false;
      } // for

      if (storecommand) {
        _commandHist.push_back(tempBuf);
        if (_commandHist.size() > app->cfg->get_int("opentest.limits.history", 10))
          _commandHist.erase(_commandHist.begin());
      } // if
      break;
    case CMDERR_SYNTAX:
      //cout << "Syntax Error: " << tCommand->getSyntax() << endl;
      //cout << "Type /HELP for a list of available commands." << endl;
      _consolef("Syntax Error: %s", tCommand->getSyntax().c_str());
      _consolef("Type /HELP for a list of available commands.");
      break;
    case CMDERR_ACCESS:
      break;
    default:
      //cout << "Invalid Command: " << tempBuf << endl;
      //cout << "Type /HELP for a list of available commands." << endl;
      _consolef("Invalid Command: %s", tempBuf.c_str());
      _consolef("Type /HELP for a list of available commands.");
      break;
  } // switch

  return parseMe.length();
} // Server::ParseSTDIN

/******************
 ** Read Members **
 ******************/
  bool Server::addWork(UplinkPacket *upacket) {
    openframe::scoped_lock slock(&_workList_l);
    return _addWork(upacket);
  } // addWork

  bool Server::_addWork(UplinkPacket *upacket) {
    if (_workList.size() >= 1024) return false;

    Work *work = new Work(upacket->created_at(), upacket->str());
    delete upacket;
    _workList.push_back(work);

    return true;
  } // _addWork

const unsigned int Server::Read() {
  Connection *aConnection;                      // pointer to a connection
  map<int, regexMatch> regexList;
  string tempBuf;                               // temp buffer
  unsigned numBytes;				// number of bytes parsed
  stringstream s;				// string stream for replacements

  // initialize variables
  numBytes = 0;

  writeConnections();
  readConnections();

  /*
   * 06/27/2003: Read and parse STDIN. -GCARTER
   */
  if (cfg->exists("openaprs.background.pid") == false) {
    aConnection = findConnection(0);
    if (aConnection != NULL) {
      while(aConnection->readLine(tempBuf)) {
        if (OPENAPRS_CONFIG_SCRIPTING == true) {
          s.str("");
          openaprs_string_replace("%N", OPENAPRS_CONFIG_SERVER_NUMERIC, tempBuf);
          s << time(NULL);
          openaprs_string_replace("%T", s.str(), tempBuf);
          openaprs_string_replace("%%", "%", tempBuf);
        } // if

        ParseSTDIN(tempBuf);
      } // while
    } // if
  } // if

  return numBytes;
} // Server::Read

/*********************
 ** Restart Members **
 *********************/

/**
 * Server::restart
 *
 * Attempt to cleanly restart the process with the previous
 * command line parameters.  Closes STDOUT and STDERR, and unloads
 * all modules that were previously loaded cleanly.
 *
 * Returns: Nothing void function.
 */
void Server::restart(const string &myReason) {

  // debug(HERE, "Server::restart(%s) restarting...\n", myReason.c_str());
  _logf("Restart[%s] %s", app->getServer().c_str(), myReason.c_str());

  close(1);
  close(0);

  app->writeConnections();
  app->removeConnection(getSocket());

  execv(SPATH, getArgV());

  _logf("Restart[%s] %s", app->getServer().c_str(), ERROR_REPLY_FAILED_RESTART);
  Die(ERROR_REPLY_FAILED_RESTART);
  exit(-1);
}

/******************
 ** Send Members **
 ******************/

const unsigned int Server::Send(const char *sendFormat, ...) {
  char sendBuffer[MAXBUF + 1] = {0};
  va_list va;

  va_start(va, sendFormat);
  vsnprintf(sendBuffer, sizeof(sendBuffer), sendFormat, va);
  va_end(va);

  sendConnectionF(getSocket(), "%s\r\n", sendBuffer);

  _consolef(" > %s", sendBuffer);
  writeLog(OPENAPRS_LOG_DEBUG_SOCKET, "%s\n", sendBuffer);

  return strlen(sendBuffer);
} // Server::Send

/*******************
 ** Stats Members **
 *******************/

/**
 * Server::writeStats
 *
 * Writes current stats to log files.
 *
 * Returns: No return.
 *
 */
void Server::writeStats(const bool wasSignal) {
  list<string> optionList;             		// Option List

  if (wasSignal) {
    cfg->tree("openaprs.count", optionList);

    _logf("%s", "--------------------------------------------------");
    _logf("%s", "  STATS");
    _logf("%s", "--------------------------------------------------");

    while(!optionList.empty()) {
      _logf("   %s", optionList.front().c_str());
      optionList.pop_front();
    } // while

    _logf("%s", "--------------------------------------------------");
  } // if

  return;

  if (!IsServerSQL || !IsServerStats)
    return;

  Query("INSERT INTO web_stats (name, count, create_ts) VALUES ('daemon_bps', '%d', '%d')",
             cfg->get_int("openaprs.count.bps", 0),
             time(NULL));

  Query("INSERT INTO web_stats (name, count, create_ts) VALUES ('daemon_buffer', '%d', '%d')",
             cfg->get_int("openaprs.count.buffer.size", 0),
             time(NULL));

  Query("INSERT INTO web_stats (name, count, create_ts) VALUES ('daemon_pps', '%.4f', '%d')",
             cfg->get_double("openaprs.count.time.average.pps"),
             time(NULL));

  return;
} // Server::writeStats

/*******************
 ** Timer Members **
 *******************/

/**
 * Server::addTimer
 *
 * Adds a timer to the timer queue.
 *
 * Returns: Unique Id of timer added.
 */
const Timer::timerId Server::addTimer(const time_t addInterval, 
                                      const unsigned int addRuncount, 
                                      const bool addPersistent, 
                                      const bool addDelete, 
                                      TimerHandler *addTimerHandler) {
  Timer *nTimer;				// pointer to a new time
  timerId newId;				// new timer id

  assert(addTimerHandler != NULL);

  // initialize variables
  newId = getUniqueTimerId();
  nTimer = new Timer(newId, 
                     addInterval, 
                     addRuncount, 
                     addPersistent, 
                     addDelete, 
                     addTimerHandler);

  _debugf("Server::addTimer> Added Id: %d", newId);

  myTimerList.insert(pair<timerId, Timer *>(newId, nTimer));

  return newId;
} // Server::addTimer

/**
 * Server::findTimer
 *
 * Attempts to find a timer previously added to the timer queue.
 *
 * Returns: Pointer to a Timer class if successful, NULL if failed.
 */
Timer *Server::findTimer(const timerId findMe) {
  timerMapType::iterator timerPtr;		// pointer to a timer

  if ((timerPtr = myTimerList.find(findMe)) != myTimerList.end())
    return timerPtr->second;

  return NULL;
} // Server::findTimer

/**
 * Server::removeTimer
 *
 * Remove a timer from our timer queue.
 *
 * Returns: true if successful, false if the remove failed.
 */
const bool Server::removeTimer(const timerId removeMe) {
  timerMapType::iterator timerPtr;		// pointer to a log

  if ((timerPtr = myTimerList.find(removeMe)) == myTimerList.end()) {
    _debugf("Server::removeTimer> Timer Id not found: %d", removeMe);
    return false;
  } // if

  _debugf("Server::removeTimer> Removed Id: %d", removeMe);

  /*
   * 06/25/2003: Schedule the timer for removal.
   */
  timerPtr->second->setRemove(true);

  return true;
} // Server::removeTimer

/**
 * Server::clearTimers
 *
 * Clears all timers that are not set to be persistent.  Persistent timers
 * are not cleared when we disconnect from the server.
 *
 * Returns: Number of timers cleared.
 */
const unsigned int Server::clearTimers(const bool force) {
  Timer *lTimer;				// pointer to a looped to timer
  timerMapType::iterator timerPtr;		// iterator to a timer map
  timerMapType::iterator timerErasePtr;		// iterator to a timer map
  unsigned int timerCount;			// number of timers removed

  // initialize variables
  timerCount = 0;

  for(timerPtr = myTimerList.begin(); timerPtr != myTimerList.end();) {
    // initialize variables
    lTimer = timerPtr->second;

    /*
     * 06/245/2003: Some timers needs to stay forever like our connection
     *              timer. -GCARTER
     */
    if (lTimer->getPersistent() == false || force) {
      timerErasePtr = timerPtr++;
      if (lTimer->getDelete() == true)
        delete lTimer->getTimerHandler();
      myTimerList.erase(timerErasePtr);
      delete lTimer;
      continue;
    } // if

    ++timerPtr;
  } // for

  return timerCount;
} // Server::clearTimers


/**
 * Server::checkTimer
 *
 * Loops through all added timers and either runs, ignores or removes timers.
 *
 * Returns: Number of timers ran.
 */
const unsigned int Server::checkTimer() {
  Timer *lTimer;			// Pointer to a Timer we've looped to.
  TimerHandler *lTimerHandler;		// Pointer to a TimerHandler we've looped to.
  timerMapType::iterator timerPtr;	// Iterator to a timer map.
  timerMapType::iterator timerErasePtr;	// Iterator to a timer map.
  unsigned int timerCount;		// Number of timers run.

  // initialize variables
  timerCount = 0;

  for(timerPtr = myTimerList.begin(); timerPtr != myTimerList.end(); timerPtr++) {
    // initialize variables
    lTimer = timerPtr->second;
    lTimerHandler = lTimer->getTimerHandler();

    // if it's not ready to run skip it
    if (lTimer->getNextRun() > time(NULL) ||
        lTimer->getRemove() == true)
      continue;

    if (lTimerHandler == NULL) {
      _debugf("Server::checkTimer> Timer was destroyed so I can't use it.");
      lTimer->setRemove(true);
      continue;
    } // if

    // go ahead and run the function
    lTimerHandler->OnTimer(lTimer);
    timerCount++;

    // set the next time we should run
    lTimer->setNextRun(time(NULL) + lTimer->getInterval());

    // deincrement or remove timers
    if (lTimer->getRuncount() > 0) {
      if (lTimer->getRuncount() == 1) {
        lTimer->setRemove(true);
        continue;
      } // if

      lTimer->setRuncount(lTimer->getRuncount() - 1);
    } // if
  } // for

  /*
   * 06/25/2003: Iterate through our list of timers and safely
   *             perform removals. -GCARTER
   */
  for(timerPtr = myTimerList.begin(); timerPtr != myTimerList.end();) {
    // initialize variables
    lTimer = timerPtr->second;

    if (lTimer->getRemove() == true) {
      timerErasePtr = timerPtr++;
      if (lTimer->getDelete() == true)
        delete lTimer->getTimerHandler();
      myTimerList.erase(timerErasePtr);
      delete lTimer;
      continue;
    } // if

    ++timerPtr;
  } // for

  return timerCount;
} // checkTimer

/**
 * Server::getUniqueTimerId
 *
 * Find a timer Id that is not currently in use and return it for
 * a new registering timer.
 *
 * Returns: Number of type timerId.
 */
const Timer::timerId Server::getUniqueTimerId() {
  timerId uniqueId;			// Uniqure timer Id to return.

  /*
   * 06/24/2003: The idea here is to loop from 1 to find an id that is not in use.
   *             There's a bunch of different ways to find a unique id, this just
   *             seems to be the easiest and most unwastful. -GCARTER
   */
  for(uniqueId = 1; myTimerList.find(uniqueId) != myTimerList.end(); uniqueId++);

  return uniqueId;
} // Server::getUniqueTimerId

  void Server::removeDecayById(const string &decayId) {
    threadSetType::iterator ptr;
    pthread_t tid;

    for(ptr = _uplinkThreads.begin(); ptr != _uplinkThreads.end(); ptr++) {
      tid = *ptr;
      enqueueDispatch(tid, new ThreadMessage("remove.decay", decayId));
    } // for
  } // Server::removeDecayById

  void Server::enqueueDispatch(ThreadMessage *tm) {
    threadSetType::iterator ptr;
    pthread_t tid;

    for(ptr = _uplinkThreads.begin(); ptr != _uplinkThreads.end(); ptr++) {
      tid = *ptr;
      ThreadMessage *ntm = new ThreadMessage(*tm);
      enqueueDispatch(tid, ntm);
    } // for

    delete tm;
  } // Server::enqueueDispatch

  const bool Server::enqueueDispatch(const pthread_t tid, ThreadMessage *tm) {
    bool ret;
    _dispatchMap_l.Lock();
    ret = _enqueueDispatch(tid, tm);
    _dispatchMap_l.Unlock();
    return ret;
  } // Server::enqueueDispatch

  const bool Server::_enqueueDispatch(const pthread_t tid, ThreadMessage *tm) {
    dispatchMapType::iterator ptr;

    ptr = _dispatchMap.find(tid);
    if (ptr == _dispatchMap.end())
      return false;

    ptr->second->push_back(tm);

    return true;
  } // Server::enqueueDispatch

  const Server::dispatchMessageListType::size_type Server::dequeueDispatch(const pthread_t tid, dispatchMessageListType &dpl) {
    bool ret;
    _dispatchMap_l.Lock();
    ret = _dequeueDispatch(tid, dpl);
    _dispatchMap_l.Unlock();
    return ret;
  } // Server::dequeueDispatch

  const Server::dispatchMessageListType::size_type Server::_dequeueDispatch(const pthread_t tid, dispatchMessageListType &dpl) {
    dispatchMapType::iterator ptr;
    dispatchMessageListType *dispatchMessageList;

    ptr = _dispatchMap.find(tid);
    if (ptr == _dispatchMap.end())
      return 0;

    dispatchMessageList = ptr->second;

    while(!dispatchMessageList->empty()) {
      dpl.push_back(dispatchMessageList->front());
      dispatchMessageList->pop_front();
    } // while

    return dpl.size();
  } // Server::dequeueDispatch

  const unsigned int  Server::_loadBookmarks() {
    list<string> optionList;                    // Option list returned from match.
    string optionName;                          // Name of the option.
    string optionString;                        // String of the option.
    unsigned int numLoaded;                     // Number of modules loaded.

    // initialize variables
    numLoaded = 0;

    for(size_t i=0; i < cfg->array_size("openaprs.bookmarks.load"); i++) {
      string bookmark = cfg->array_string("openaprs.bookmarks.load", i);
      _bookmarks.push_back(bookmark);
      _consolef("*** Bookmark: /%s loaded.", bookmark.c_str());
      numLoaded++;
    } // for

    return numLoaded;
  } // App::_loadBookmarks

  void Server::bookmarks() {
    cout << endl;
    openframe::Result r("#,Bookmarks");
    for(commandHistoryType::size_type i=0; i < _bookmarks.size(); i++) {
      Serialize s;
      s.add((unsigned int) i);
      s.add("/" + _bookmarks[i]);
      openframe::Row row(s.compile());
      r.add(row);
    } // for

    openframe::Result::print(r);
  } // App::bookmarks

  void Server::history() {
    cout << endl;
    openframe::Result r("#,History");
    for(commandHistoryType::size_type i=0; i < _commandHist.size(); i++) {
      openframe::Serialize s;
      s.add((unsigned int) i);
      s.add("/" + _commandHist[i]);
      openframe::Row row(s.compile());
      r.add(row);
    } // for

    openframe::Result::print(r);
  } // App::history

} // namespace openaprs

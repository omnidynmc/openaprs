/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
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
 $Id: Server.h,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_SERVER_H
#define __OPENAPRS_SERVER_H

#include <string>
#include <map>
#include <set>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "Cfg.h"
#include "Event.h"
#include "LineBuffer.h"
#include "Log.h"
#include "Network.h"
#include "Module.h"
#include "ModuleList.h"
#include "OpenAPRS_Abstract.h"
#include "Command.h"
#include "Timer.h"
#include "ThreadMessage.h"
#include "PutMySQL.h"
#include "Worker.h"

#include <openframe/ConfController.h>
#include <openframe/OFLock.h>
#include <openframe/Stopwatch.h>
#include <stomp/StompStats.h>

#include "openaprs_string.h"
#include "misc.h"

namespace openaprs {
  using openframe::Stopwatch;
  using namespace putmysql;
  using namespace handler;

  using std::string;
  using std::map;
  using std::list;
  using std::pair;
  using std::endl;
  using std::ostream;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class ModuleAction {
  public:
    enum actionEnumType { add, remove, reload };

    std::string path;
    actionEnumType action;
};


class DBI;
class Work;
class StdinController;
class UplinkPacket;

class Server : public OpenAPRS_Abstract, public LogHandler, public TimerHandler, public CommandTree, public Network,
               public openframe::OFLock {
  public:
    Server();				// constructor
    virtual ~Server();			// destructor

    /**********************
     ** Type Definitions **
     **********************/
    typedef Event::eventId eventId;
    typedef Timer::timerId timerId;
    typedef Log::logId logId;
    typedef map<logId, Log *> logMapType;
    typedef map<string, logId> onLogMapType;
    typedef map<timerId, Timer *> timerMapType;
    typedef map<string, timerId> onTimerMapType;
    typedef unsigned int flagType;
    typedef map<eventId, Event *> eventMapType;
    typedef map<string, eventId> onEventMapType;
    typedef list<ModuleAction> moduleActionListType;
    typedef set<pthread_t> threadSetType;
    typedef list<Work *> workListType;
    typedef list<Result *> resultListType;
    // ### Type Definitions ###
    typedef vector<string> commandHistoryType;


    typedef list<ThreadMessage *> dispatchMessageListType;
    typedef map<pthread_t, dispatchMessageListType *> dispatchMapType;

    /***************
     ** Variables **
     ***************/
    const static flagType FLAG_SHUTDOWN;
    const static flagType FLAG_PREAUTH;
    const static flagType FLAG_CONNECTED;
    const static flagType FLAG_NOAB;

    DBI *dbi;
    ModuleList *module;

    /******************
     ** ArgV Members **
     ******************/

    char **getArgV()
      { return myArgV; }
    void setArgV(char **setMe)
      { myArgV = setMe; }

    /*******************
     ** Burst Members **
     *******************/

    const bool Email(const std::string &, const std::string &, const std::string &);

    /********************
     ** Config Members **
     ********************/

    void setConfigPath(const std::string &setMe)
      { myConfigPath = setMe; }

    const std::string &getConfigPath()
      { return myConfigPath; }

    /*****************
     ** Die Members **
     *****************/

    void Die(const std::string &);

    /*******************
     ** Event Members **
     *******************/
 
    const unsigned int writeEvent(const std::string &, Packet *);
    const eventId addEvent(const std::string &, const unsigned int, EventHandler *);
    const bool removeEvent(const eventId);
    const unsigned int clearEvents();
    Event *findEvent(const eventId);
    const int OnEvent(Event *, const std::string &, Packet *);

    virtual const int Execute(Packet) { return 1; }


    /******************
     ** Echo Members **
     ******************/

    const bool getEcho() const
      { return myEcho; }

    void setEcho(const bool setMe)
      { myEcho = setMe; }

    /*********************
     ** Execute Members **
     *********************/

    const int Execute(COMMAND_ARGUMENTS) { return 1; }

    /******************
     ** Flag Members **
     ******************/

    const bool addFlag(const flagType addMe) {
      if (myFlags & addMe)
        return false;

      myFlags |= addMe;

      return true;
    } // addFlag

    const bool removeFlag(const flagType removeMe) {
      if (!(myFlags & removeMe))
        return false;

      myFlags &= ~removeMe;

      return true;
    } // removeFlag

    const bool findFlag(const flagType findMe) const {
      return myFlags & findMe ? true : false;
    } // findFlag

    const unsigned int getFlags(string &);

    /*****************
     ** I/O Members **
     *****************/

    void stripLF(string &stripMe) {
      size_t pos;			// pointer to a string

      for(pos = stripMe.find("\n");
          pos != string::npos;
          pos = stripMe.find("\n"))
        stripMe.replace(pos, 1, "\\n");

      for(pos = stripMe.find("\r");
          pos != string::npos;
          pos = stripMe.find("\r"))
        stripMe.replace(pos, 1, "\\r");

    } // stripLF

    /***********************************************
     ** Initialization / Deinitialization Members **
     ***********************************************/

    const bool initializeSystem();
    const bool initializeCommands();
    const bool initializeEvents();
    const bool initializeLogs();
    const bool initializeTimers();
    const bool initializeThreads();
    const bool deinitializeSystem();
    const bool deinitializeCommands();
    const bool deinitializeEvents();
    const bool deinitializeLogs();
    const bool deinitializeTimers();
    const bool deinitializeThreads();

    void bookmarks();
    void history();


    const bool Run();
    void Done();
    void disconnect();
    void hup();
    void Writestats();
    const bool isDone();
    const bool isDisconnect();
    const bool isWritestats();

    const unsigned int dequeueWork(workListType &, const int);
    void queueResult(Result *);

    /*********************
     ** Link TS Members **
     *********************/

    void setLinkTS(const time_t setMe)
      { myLinkTS = setMe; }

    const time_t getLinkTS() const
      { return myLinkTS; }

    /********************
     ** Module Members **
     ********************/

    const unsigned int loadModules();

    /********************
     ** Monitor Members **
     ********************/

    const bool isMonitor(const std::string &);

    /*******************
     ** OnLog Members **
     *******************/

    const logId getUniqueLogId();
    const unsigned int writeLog(const std::string &, const char *, ...);
    const unsigned int writeEmail(const char *, ...);
    const unsigned int Query(const char *, ...);
    const unsigned int lQuery(PutMySQL *, const char *, ...);
    const logId addLog(const std::string &, LogHandler *, const bool);
    const bool removeLog(const logId);
    Log *findLog(const logId);
    const int OnLog(Log *, const logId, const std::string &);

    /*******************
     ** Parse Members **
     *******************/

    const unsigned int ParseSTDIN(const std::string &);

    /*****************
     ** PID Members **
     *****************/

    const pid_t getPID() const
      { return myPID; }

    void setPID(const pid_t setMe)
      { myPID = setMe; }

    /********************
     ** Prompt Members **
     ********************/

    const std::string &getPrompt() const
      { return myPrompt; }

    void setPrompt(const std::string &setMe)
      { myPrompt = setMe; }

    /******************
     ** Port Members **
     ******************/

    const std::string &getPort() const
      { return myPort; }

    void setPort(const std::string &setMe)
      { myPort = setMe; }

    /******************
     ** Read Members **
     ******************/

    const unsigned int Read();

    /*********************
     ** Restart Members **
     *********************/

    void restart(const std::string &);	// restart the process

    /******************
     ** Send Members **
     ******************/

    const unsigned int Send(const char *, ...);

    /********************
     ** Server Members **
     ********************/

    const std::string rotateServer() {
      if (!_serverList.size())
        return string("");

      myServer = _serverList[(_serverListIndex++ % _serverList.size())];
      return myServer;
    } // rotateServer

    const std::string &getServer() const {
      return myServer;
    } //  getServer

    void setServer(const std::string &setMe) {
      std::string parseMe = setMe;
      openaprs_string_replace(" ", "", parseMe);
      _serverList.setDelimiter(',');
      _serverList = parseMe;
    } // setServer

    /*******************
     ** Stats Members **
     *******************/
    void writeStats(const bool);

    /**********************
     ** Start TS Members **
     **********************/

    void setStartTS(const time_t setMe)
      { myStartTS = setMe; }
    
    const time_t getStartTS() const
      { return myStartTS; }

    /********************
     ** Socket Members **
     ********************/

    void setSocket(const int setMe)
      { mySocket = setMe; }

    const int getSocket() const
      { return mySocket; }

    /*******************
     ** Timer Members **
     *******************/

    const timerId getUniqueTimerId();
    const int OnTimer(Timer *);

    const timerId addTimer(const time_t, const unsigned int, const bool, const bool, TimerHandler *);
    const bool removeTimer(const timerId);
    Timer *findTimer(const timerId);

    const unsigned int checkTimer();
    const unsigned int clearTimers(const bool);

    /***********************
     ** Timestamp Members **
     ***********************/

    const time_t getTimestamp() const
      { return myTimestamp; }

    void setTimestamp(const time_t setMe)
      { myTimestamp = setMe; }

    /*******************
     ** Vhost Members **
     *******************/

    void setVhost(const std::string &setMe)
      { myVhost = setMe; }

    const std::string &getVhost()
      { return myVhost; }

    /********************
     ** Rehash Members **
     ********************/

    void rehash();			// rehash the bot's config

    /********************
     ** Uplink Members **
     ********************/

    const std::string &getUplink() const
      { return myUplink; }

    void setUplink(const std::string & setMe)
      { myUplink = setMe; }

    const bool Module(const std::string &, ModuleAction::actionEnumType);

    static void *SignalThread(void *);
    static void *WorkerThread(void *);
    static void *UplinkThread(void *);

    /***************
     ** Variables **
     ***************/
    Network aNetwork;			// network connection
    Stopwatch profile;
    openframe::ConfController *cfg;

    bool addWork(UplinkPacket *upacket);
    void removeDecayById(const std::string &);
    void enqueueDispatch(ThreadMessage *);
    const bool enqueueDispatch(const pthread_t, ThreadMessage *);
    const dispatchMessageListType::size_type dequeueDispatch(const pthread_t, dispatchMessageListType &);
    StdinController *stdin() { return _stdinController; }
    stomp::StompStats *stats() { return _stats; }

  protected:
    const bool _enqueueDispatch(const pthread_t, ThreadMessage *);
    const dispatchMessageListType::size_type _dequeueDispatch(const pthread_t, dispatchMessageListType &);
    bool _addWork(UplinkPacket *upacket);
    const unsigned int _processResults();
    const unsigned int _loadBookmarks();

    const eventId _getUniqueEventId();
    const unsigned int _writeEvent(const std::string &, Packet *);
    const eventId _addEvent(const std::string &, const unsigned int, EventHandler *);
    const bool _removeEvent(const eventId);
    Event *_findEvent(const eventId);

  private:
    StringToken _serverList;		// my server list
    LineBuffer myLogBuffer;		// log buffer
    OFLock _writeEvent_l;		// mutex for events
    OFLock _logList_l;
    OFLock _workList_l;
    OFLock _resultList_l;
    OFLock _dispatchMap_l;
    StdinController *_stdinController;
    bool myEcho;			// echo mode?
    bool _done;
    bool _disconnect;
    bool _writestats;
    char **myArgV;			// command line arguments
    commandHistoryType _commandHist;
    commandHistoryType _bookmarks;
    eventMapType myEventList;           // Event List.
    onEventMapType myOnEventList;       // OnEvent list.
    ostringstream _oss;
    flagType myFlags;			// my flags
    int mySocket;			// my socket
    int myBurstFd;			// fd of burst file
    dispatchMapType _dispatchMap;	// thread dispatch map
    pthread_t _signalThread_tid;	// Signal thread's TID
    logMapType myLogList;		// Log list.
    moduleActionListType moduleActionList;	// Delayed module actions
    onLogMapType myOnLogList;		// List of log id's we're listening for.
    onTimerMapType myOnTimerList;	// List of timer id's we're listening for.
    workListType _workList;		// List of strings to parse
    resultListType _resultList;
    pid_t myPID;			// my pid
    std::string myConfigPath;		// my config file path
    std::string myPrompt;			// my prompt
    std::string myVhost;			// my virtual host
    std::string myPort;			// my port
    std::string myServer;			// my server
    std::string myUplink;			// my uplink
    std::string _lastCommand;		// last command run
    threadSetType _workerThreads;	// worker threads
    threadSetType _uplinkThreads;	// worker threads
    time_t myTimestamp;			// my timestamp
    time_t myLinkTS;			// my link ts
    time_t myStartTS;			// my start ts
    timerMapType myTimerList;		// timer list
    unsigned int _serverListIndex;	// Index to server list

    stomp::StompStats *_stats;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

#define IsServerShutdown(x)		x->findFlag(Server::FLAG_SHUTDOWN)
#define IsServerPreAuth(x)		x->findFlag(Server::FLAG_PREAUTH)
#define IsServerConnected(x)		x->findFlag(Server::FLAG_CONNECTED)
#define IsServerNoAutoBurst(x)		x->findFlag(Server::FLAG_NOAB)
#define IsServerDebug			(CFG_BOOL("openaprs.toggles.debug", false) == true)
#define IsServerLogSQL			(CFG_BOOL("openaprs.toggles.log.sql", false) == true)
#define IsServerSQL			(CFG_BOOL("openaprs.sql.enable", false) == true)
#define IsServerSQLInject		(CFG_BOOL("openaprs.sql.inject", false) == true)
#define IsServerStats			(CFG_BOOL("openaprs.stats.enable", false) == true)
#define IsServerDropDuplicates		(CFG_BOOL("openaprs.sql.drop_duplicates", false) == true)
#define IsServerEcho			(CFG_BOOL("openaprs.toggles.echo", false) == true)
#define IsServerRaw			(CFG_BOOL("openaprs.toggles.raw", false) == true)
#define IsServerConsole			IsServerEcho
#define SetServerShutdown(x)		x->addFlag(Server::FLAG_SHUTDOWN);
#define SetServerPreAuth(x)		x->addFlag(Server::FLAG_PREAUTH);
#define SetServerConnected(x)		x->addFlag(Server::FLAG_CONNECTED);
#define SetServerNoAutoBurst(x)		x->addFlag(Server::FLAG_NOAB);
#define UnsetServerShutdown(x)		x->removeFlag(Server::FLAG_SHUTDOWN);
#define UnsetServerPreAuth(x)		x->removeFlag(Server::FLAG_PREAUTH);
#define UnsetServerConnected(x)		x->removeFlag(Server::FLAG_CONNECTED);
#define UnsetServerNoAutoBurst(x)	x->removeFlag(Server::FLAG_NOAB);

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openaprs

extern Server *app;		// establish a global variable
extern openframe::Logger elog;

#endif

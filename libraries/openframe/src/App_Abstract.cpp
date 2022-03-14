#include <config.h>

#include <string>

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include <openframe/openframe.h>
#include <openframe/App_Abstract.h>
#include <openframe/stdinCommands.h>
#include <openframe/ModuleList.h>

#include <readline/readline.h>
#include <readline/history.h>

namespace openframe {
  using namespace loglevel;

  App_Abstract::App_Abstract(const std::string &prompt, const std::string &config, const bool console)
               : _done(false),
                 _config(config),
                 _prompt(prompt),
                 _console(console) {
    _lastCommand = "";

    try {
      _command = new CommandTree;
      _modules = new ModuleList();
      cfg = new ConfController();
    } // try
    catch(std::bad_alloc xa) {
      assert(false);
    } // catch
  } // App_Abstract::App_Abstract

  App_Abstract::~App_Abstract() {
    delete _modules, _modules=NULL;
    delete _command, _command=NULL;
    delete cfg, cfg=NULL;
  } // App_Abstract::~App_Abstract

  bool App_Abstract::write_pid(const std::string &path) {
    // write pid file
    pid_t pid = getpid();
    FILE *PID = fopen(path.c_str(), "w");
    if (PID == NULL) return false;
    fprintf(PID, "%d\n", pid);
    fclose(PID);
    return true;
  } // App_Abstract::write_pid

  bool App_Abstract::is_done() {
    scoped_lock slock(&_done_l);
    return _done;
  } // App_Abstract::is_done

  void App_Abstract::set_done(const bool done) {
    scoped_lock slock(&_done_l);
    _done = done;
  } // App_Abstract::set_done

  bool App_Abstract::is_console() const {
    return _console;
  } // App_Abstract::is_console

  void App_Abstract::run() {
    LOG(LogNotice, << "*** " << _prompt
                      << " started, version " << VERSION
                      << std::endl);

    while(!is_done()) {
      onRun();
      _read();
    } // while
 } // App_Abstract::run

  void App_Abstract::start() {
    initializeSystem();
  } // App_Abstract::start

  void App_Abstract::stop() {
    deinitializeSystem();
  } // App_Abstract::stop

  void App_Abstract::initializeSystem() {
    if (is_done()) return;

    initializeConfig();
    initializeCommands();
    initializeDatabase();
    initializeThreads();

    // We MUST load modules AFTER initializeThreads.
    // Modules can load threads as well that will not get the
    // block signal mask if we don't.
    initializeModules();

    _loadBookmarks();

    onInitializeSystem();
    LOG(LogInfo, << "*** System Initialized" << std::endl);
    if ( is_console() ) _consolef("Type /HELP for a list of commands or ! to execute that last successful command.");
  } // App_Abstract::initializeSystem

  void App_Abstract::initializeConfig() {
    if (!cfg) return;
    //if ( is_console() ) _consolef("*** Loading configuration from: %s", _config.c_str());

    try {
      cfg->load(_config);
    } // try
    catch(ConfController_Exception e) {
      _consolef("ERROR: Unable to load config %s; %s", _config.c_str(), e.what());
      set_done(true);
    } // catch

    onInitializeConfig();
    LOG(LogInfo, << "*** Config Initialized" << std::endl);
  } // App_Abstract::initializeConfig

  void App_Abstract::initializeCommands() {
    _command->addCommand("ADD", "STDIN", "/ADD", -1, Command::FLAG_CLEANUP, new stdinDTREE);
      _command->addCommand("MODULE", "STDIN/ADD", "/ADD MODULE <path>", -1, Command::FLAG_CLEANUP, new stdinADDMODULE);
    _command->addCommand("BINARY", "STDIN", "/BINARY", -1, Command::FLAG_CLEANUP, new stdinBINARY);
    _command->addCommand("BOOKMARKS", "STDIN", "/BOOKMARKS", -1, Command::FLAG_CLEANUP, new stdinBOOKMARKS);
    _command->addCommand("CAT", "STDIN", "/CAT <filename>", -1, Command::FLAG_CLEANUP, new stdinCAT);
    _command->addCommand("HELP", "STDIN", "/HELP", -1, Command::FLAG_CLEANUP, new stdinHELP);
    _command->addCommand("HISTORY", "STDIN", "/HISTORY", -1, Command::FLAG_CLEANUP, new stdinHISTORY);
    _command->addCommand("QUIT", "STDIN", "/QUIT", -1, Command::FLAG_CLEANUP, new stdinQUIT);
    _command->addCommand("SET", "STDIN", "/SET", -1, Command::FLAG_CLEANUP, new stdinDTREE);
      _command->addCommand("CONFIG", "STDIN/SET", "/SET CONFIG <name> <value> - set a std::string config value", -1, Command::FLAG_CLEANUP, new stdinSETCONFIG);
    _command->addCommand("SHOW", "STDIN", "/SHOW", -1, Command::FLAG_CLEANUP, new stdinDTREE);
      _command->addCommand("CONFIG", "STDIN/SHOW", "/SHOW QUIT", -1, Command::FLAG_CLEANUP, new stdinSHOWCONFIG);
    _command->addCommand("RELOAD", "STDIN", "/RELOAD", -1, Command::FLAG_CLEANUP, new stdinDTREE);
      _command->addCommand("MODULE", "STDIN/RELOAD", "/RELOAD MODULE <path>", -1, Command::FLAG_CLEANUP, new stdinRELOADMODULE);
    _command->addCommand("REMOVE", "STDIN", "/REMOVE", -1, Command::FLAG_CLEANUP, new stdinDTREE);
      _command->addCommand("MODULE", "STDIN/REMOVE", "/REMOVE MODULE <path>", -1, Command::FLAG_CLEANUP, new stdinREMOVEMODULE);

    onInitializeCommands();
    LOG(LogInfo, << "*** Commands Initialized" << std::endl);
  } // App_Abstract::initializeCommands

  void App_Abstract::initializeDatabase() {
    onInitializeDatabase();
    LOG(LogInfo, << "*** Database Initialized" << std::endl);
  } // App_Abstract::initializeDatabase

  void App_Abstract::initializeThreads() {
    sigset_t mask;

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
    pthread_create(&_signalThread_tid, NULL, App_Abstract::SignalThread, this);
    LOG(LogInfo, << "SignalThread Initialized" << std::endl);

    onInitializeThreads();
    LOG(LogInfo, << "*** Threads Initialized" << std::endl);
  } // App_Abstract::initializeThreads

  void App_Abstract::initializeModules() {
    _loadModules();
    onInitializeModules();
    LOG(LogInfo, << "*** Modules Initialized" << std::endl);
  } // App_Abstract::initializeModules

  void App_Abstract::deinitializeSystem() {
    set_done(true);
    onDeinitializeSystem();
    deinitializeModules();
    deinitializeCommands();
    deinitializeDatabase();
    deinitializeThreads();
    LOG(LogInfo, << "*** System Deinitialized" << std::endl);
  } // App_Abstract::deinitializeSystem

  void App_Abstract::deinitializeCommands() {
    onDeinitializeCommands();
    _command->removeCommand("STDIN");
    LOG(LogInfo, << "*** Commands Deinitialized" << std::endl);
  } // App_Abstract::initializeCommands

  void App_Abstract::deinitializeDatabase() {
    onDeinitializeDatabase();
    LOG(LogInfo, << "*** Database Deinitialized" << std::endl);
  } // App_Abstract::deinitializeDatabase

  void App_Abstract::deinitializeThreads() {
    onDeinitializeThreads();

    pthread_join(_signalThread_tid, NULL);
    LOG(LogInfo, << "SignalThread Deinitialized" << std::endl);

    sigset_t mask;
    sigemptyset(&mask );
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGPIPE);

    // Set signals back how they were
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);

    LOG(LogInfo, << "*** Threads Deinitialized" << std::endl);
  } // App_Abstract::deinitializeThreads

  void App_Abstract::deinitializeModules() {
    onDeinitializeModules();
    _modules->clear();
    LOG(LogInfo, << "*** Modules Deinitialized" << std::endl);
  } // App_Abstract::deinitializeModules

  const unsigned int App_Abstract::_read() {
    StringTool::regexMatchListType regexList;
    std::string tempBuf;                               // temp buffer
    unsigned numBytes;                            // number of bytes parsed
    stringstream s;                               // std::string stream for replacements

    // initialize variables
    numBytes = 0;

    std::string prompt = cfg->get_string("app.prompt", "command> ");

    char *line = readline( prompt.c_str() );

    if (line == NULL) {
      set_done(true);
      return 0;
    } // if

    tempBuf = line;
    s.str("");
    s << time(NULL);
    StringTool::replace("%T", s.str(), tempBuf);
    StringTool::replace("%%", "%", tempBuf);
    if (tempBuf.length()) add_history(line);
    free(line);
    _parse(tempBuf);

    return numBytes;
  } // App_Abstract::_read

  const unsigned int App_Abstract::_parse(const std::string &parseMe) {
    int returnResult;                     // command return result
    std::string tempBuf;                       // temporary buffer
    unsigned int nodeLevel;               // node level in command
    StringTool::regexMatchListType rl;
    bool storecommand = true;

    if (parseMe.length() < 1)
      return 0;

    // initialize variables
    tempBuf = parseMe;
    StringTool::stripcrlf(tempBuf);

    if (tempBuf == "!") {
      if (_lastCommand.length()) {
        tempBuf = "/" + _lastCommand;
        _consolef("Executing: /%s", _lastCommand.c_str());
        storecommand = false;
      } // if
      else
        _consolef("No last command found.");
    } // if
    else if (StringTool::ereg("^[!]([0-9]+)$", tempBuf, rl)) {
      size_t n = atoi(rl[1].matchString.c_str());
      if (n > _commandHist.size()) {
        _consolef("Error, command history number must match a record.");
        return 0;
      } // if

      tempBuf = "/" + _commandHist[n];
      _consolef("Executing: /%s", _commandHist[n].c_str());
      storecommand = false;
    } // else if
    else if (StringTool::ereg("^[$]([0-9]+)$", tempBuf, rl)) {
      size_t n = atoi(rl[1].matchString.c_str());
      if (n > _commandHist.size()) {
        _consolef("Error, command history number must match a record.");
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
        _consolef("Error, bookmark number must match a record.");
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
      _consolef("Type /HELP for a list of commands.");
      std::cout << "! = run last command, ![0-9] = run # command, @ = view command history" << std::endl
                << "#  = view bookmarks, !![0-9] = run # command from bookmarks, $[0-9] = save to bookmarks" << std::endl;
      return 0;
    } // if
    else
      tempBuf.replace(0, 1, "");

    // initialize variables
    StringToken parseCommand;
    parseCommand = tempBuf;
    nodeLevel = 0;
    returnResult = time(NULL);

    // find the command
    Command *tCommand = _command->findCommand("STDIN", parseCommand, nodeLevel, NULL);

    Packet aPacket;                       // packet to send to command parser
    if (tCommand != NULL) {
      aPacket.setSource("STDIN");
      aPacket.setArguments(parseCommand.getRange(nodeLevel, parseCommand.size()));
      aPacket.setPath(tCommand->getPath());
      aPacket.addVariable("app", this);

      returnResult = tCommand->getCommandHandler()->Execute(&aPacket);
    } // if

    switch(returnResult) {
      case CMDERR_SUCCESS:
        _lastCommand = tempBuf;
        for(commandHistoryType::size_type i=0; i < _commandHist.size(); i++) {
          if (!StringTool::compare(tempBuf.c_str(), _commandHist[i].c_str()))
            storecommand = false;
        } // for

        if (storecommand) {
          _commandHist.push_back(tempBuf);
          if (_commandHist.size() > static_cast<unsigned int>( cfg->get_int("app.limits.history", 10) ))
            _commandHist.erase(_commandHist.begin());
        } // if
        break;
      case CMDERR_SYNTAX:
        std::cout << "Syntax Error: " << tCommand->getSyntax() << std::endl;
        std::cout << "Type /HELP for a list of available commands." << std::endl;
        break;
      case CMDERR_ACCESS:
        break;
      default:
        std::cout << "Invalid Command: " << tempBuf << std::endl;
        std::cout << "Type /HELP for a list of available commands." << std::endl;
        break;
    } // switch

    return parseMe.length();
  } // App_Abstract::_parse

  void App_Abstract::bookmarks() {
    std::cout << std::endl;
    Result r("#,Bookmarks");
    for(commandHistoryType::size_type i=0; i < _bookmarks.size(); i++) {
      Serialize s;
      s.add((unsigned int) i);
      s.add("/" + _bookmarks[i]);
      Row row(s.compile());
      r.add(row);
    } // for

    Result::print(r, 80);
  } // App_Abstract::bookmarks

  void App_Abstract::history() {
    std::cout << std::endl;
    Result r("#,History");
    for(commandHistoryType::size_type i=0; i < _commandHist.size(); i++) {
      Serialize s;
      s.add((unsigned int) i);
      s.add("/" + _commandHist[i]);
      Row row(s.compile());
      r.add(row);
    } // for

    Result::print(r, 80);
  } // App_Abstract::history

  const unsigned int  App_Abstract::_loadModules() {
    list<string> optionList;                    // Option list returned from match.
    std::string optionName;                          // Name of the option.
    std::string optionString;                        // std::string of the option.
    unsigned int numLoaded;                     // Number of modules loaded.

    // initialize variables
    numLoaded = 0;

    for(size_t i=0; i < cfg->array_size("app.modules.load"); i++) {
      std::string path = cfg->array_string("app.modules.load", i);
        _modules->add(path, false);
        _consolef("*** Module: %s loaded.", path.c_str());
      numLoaded++;
    } // for

    return numLoaded;
  } // App_Abstract::_loadModules

  const unsigned int  App_Abstract::_loadBookmarks() {
    list<string> optionList;                    // Option list returned from match.
    std::string optionName;                          // Name of the option.
    std::string optionString;                        // std::string of the option.
    unsigned int numLoaded;                     // Number of modules loaded.

    // initialize variables
    numLoaded = 0;

    for(size_t i=0; i < cfg->array_size("app.bookmarks.load"); i++) {
      std::string bookmark = cfg->array_string("app.bookmarks.load", i);
      _bookmarks.push_back(bookmark);
      LOG(LogInfo, << "*** Bookmark: /" << bookmark << " loaded." << std::endl);
      numLoaded++;
    } // for

    return numLoaded;
  } // App_Abstract::_loadBookmarks

  void *App_Abstract::SignalThread(void *arg) {
    App_Abstract *a = static_cast<App_Abstract *>(arg);
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
    while( !a->is_done() ) {
      result = sigtimedwait( &mask, &info, &timeout );
      if (-1 == result && EAGAIN == errno) {
        continue;
      } // if

      switch(info.si_signo) {
        case SIGHUP:
          a->rcvSighup();
          break;
        case SIGPIPE:
          a->rcvSigpipe();
          break;
        case SIGALRM:
          break;
        case SIGUSR1:
          a->rcvSigusr1();
          break;
        case SIGUSR2:
          a->rcvSigusr2();
          break;
        case SIGINT:
          a->rcvSigint();
          break;
        default:
          continue;
          break;
      } // switch
    } // while

    return NULL;
  } // App_Abstract::SignalThread
} // namespace openframe

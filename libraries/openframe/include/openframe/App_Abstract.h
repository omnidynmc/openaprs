#ifndef OPENFRAME_APP_ABSTRACT_H
#define OPENFRAME_APP_ABSTRACT_H

#include <string>
#include <vector>

#include <pthread.h>

#include "Command.h"
#include "OpenFrame_Abstract.h"
#include "openframe.h"
#include "scoped_lock.h"

namespace openframe {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  class ModuleList;
  class App_Abstract : public OpenFrame_Abstract {
    public:
      App_Abstract(const std::string &prompt, const std::string &config, const bool console=true);
      virtual ~App_Abstract();

      // ### Type Definitions ###
      typedef std::vector<string> commandHistoryType;

      // ### Public Members ###
      static bool write_pid(const std::string &path);
      const std::string prompt() const { return _prompt; }
      void prompt(const std::string &prompt) { _prompt = prompt; }
      const std::string config() const { return _config; }
      void config(const std::string &config) { _config = config; }
      void run();
      virtual void start();
      virtual void stop();
      void bookmarks();
      void history();
      bool is_done();
      void set_done(const bool done);
      void showTree(const std::string &name, const int lvl, const std::string spacer, list<string> &list) {
        _command->showTree(name, lvl, spacer, list);
      } // showTree
      CommandTree *command() { return _command; }
      ModuleList *modules() { return _modules; }

      // ### Public Variables ###
      ConfController *cfg;

      virtual void onInitializeSystem() = 0;
      virtual void onInitializeConfig() = 0;
      virtual void onInitializeCommands() = 0;
      virtual void onInitializeDatabase() = 0;
      virtual void onInitializeModules() = 0;
      virtual void onInitializeThreads() = 0;

      virtual void onDeinitializeSystem() = 0;
      virtual void onDeinitializeCommands() = 0;
      virtual void onDeinitializeDatabase() = 0;
      virtual void onDeinitializeModules() = 0;
      virtual void onDeinitializeThreads() = 0;

      virtual void rcvSighup() = 0;
      virtual void rcvSigusr1() = 0;
      virtual void rcvSigusr2() = 0;
      virtual void rcvSigint() = 0;
      virtual void rcvSigpipe() = 0;

      virtual void onRun() = 0;

      static void *SignalThread(void *arg);

    protected:
      virtual void initializeSystem();
      virtual void initializeConfig();
      virtual void initializeCommands();
      virtual void initializeDatabase();
      virtual void initializeModules();
      virtual void initializeThreads();
      virtual void deinitializeSystem();
      virtual void deinitializeCommands();
      virtual void deinitializeDatabase();
      virtual void deinitializeModules();
      virtual void deinitializeThreads();
      const unsigned int _read();
      const unsigned int _parse(const std::string &);
      const unsigned int _loadModules();
      const unsigned int _loadBookmarks();
      bool is_console() const;

    private:
      CommandTree *_command;
      ModuleList *_modules;
      commandHistoryType _commandHist;
      commandHistoryType _bookmarks;
      bool _done;
      OFLock _done_l;
      std::string _config;
      std::string _prompt;
      bool _console;
      std::string _lastCommand;
      pthread_t _signalThread_tid;
  }; // App_Abstract

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/


} // namespace openframe

#endif

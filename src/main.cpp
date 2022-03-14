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
 $Id: openaprs.cpp,v 1.4 2006/03/21 20:55:12 omni Exp $
 **************************************************************************/

#include <fstream>
#include <string>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <new>
#include <iostream>
#include <string>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/uio.h>
#include <sys/param.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>

#include "App_Log.h"
#include "LineBuffer.h"
#include "PutMySQL.h"
#include "Server.h"
#include "Timer.h"
#include "Create.h"

#include <openframe/ConfController.h>
#include <openframe/Stopwatch.h>

#include "config.h"
#include "openaprs.h"
#include "openaprs_signal.h"
#include "openaprs_string.h"

using namespace openaprs;
using namespace putmysql;
using std::string;

// establish global variables
openaprs::Server *app;		// this will be the only global variable soon
openframe::Logger elog;
PutMySQL *Sql;

void die(const string &dieMessage) {
  cout << "*** ERROR: " << dieMessage << endl << endl;
  cout << "Please email gcarter@openaprs.net with the error your recieved." << endl;
  exit(1);
} // die

void usage() {
  fprintf(stderr, "Usage: %s -d -o -e -f [-v] [-c <config_file_path>] [-b vhost]\n", app->getPrompt().c_str());
  exit(1);
}

void header() {
  return;
}

void version() {
  printf("OpenAPRS: MySQL Injector\n");
  printf("Version: %s\n", VERSION);
  printf("Compiled: %s\n", __DATE__);
}

int main(int argc, char **argv) {
  Stopwatch sw;
  char *prompt;		// process prompt
  int ch;		// misc pointer
  pid_t pid;		// process pid
  time_t now;		// current time

  FILE *PID;

  // try to create our new variables
  try {
    app = new Server;
  } // try
  catch(bad_alloc xa) {
    assert(false);
  } // catch

  app->logger(new App_Log(""));
  app->setVhost("");
  app->setStartTS(time(NULL));

  // time I was first started
  app->setTimestamp(time(NULL));

  // save our argv
  app->setArgV(argv);

  umask(077);

  // remember prompt
  if ((prompt = strrchr(argv[0], '/')))
    prompt++;
  else
    prompt = argv[0];

  app->setPrompt(prompt);
  app->setConfigPath(OPENAPRS_PATH_CONFIG);

  // parse command line options
  while ((ch = getopt(argc, argv, "daveob:c:")) != -1) {
    switch (ch) {
      case 'c':
	app->setConfigPath(optarg);
	break;
      case 'd':
        app->cfg->replace_bool("openaprs.toggles.debug", true);
        break;
      case 'e':
        app->cfg->replace_bool("openaprs.toggles.echo", true);
        break;
      case 'o':
        app->cfg->replace_bool("openaprs.toggles.dcc.only", true);
        break;
      case 'a':
        SetServerNoAutoBurst(app);
	break;
      case 'b':
	app->setVhost(optarg);
	break;
      case 'v':
	(void) version();
	exit(0);
	break;
      case '?':
      default:
	(void) usage();
	break;
    } // switch
  } // while

  // parse mysql record id for configuration
  argc -= optind;
  argv += optind;

  // load config file
  try {
    app->cfg->load(app->getConfigPath());
  } // try
  catch(openframe::ConfController_Exception e) {
    fprintf(stdout, "%s: ERROR loading config: %s\n", app->getPrompt().c_str(), e.message());
    exit(1);
  } // catch

  app->setServer(OPENAPRS_CONFIG_SERVER);
  app->setPort(OPENAPRS_CONFIG_PORT);

  // Verify the log directory exists.
  if (!is_dir((OPENAPRS_PATH_LOGS).c_str())) {
    fprintf(stdout, "%s: Fatal error, can not find ``%s''.\n", app->getPrompt().c_str(), (OPENAPRS_PATH_LOGS).c_str());
    exit(1);
  } // if

  // fork to background unless in debug mode
  if (IsServerDebug) {
    pid = getpid();
    app->cfg->push_int("openaprs.process.id", pid);
    // version();
    // fprintf(stdout, "\[Forground(PID: %d)]\n", pid);
  } // if
  else {
    // reset the variables that affect the console.
    app->cfg->replace_bool("openaprs.toggles.echo", false);
    app->cfg->replace_bool("openaprs.toggles.raw", false);

    if ((pid = fork())) {
      // printf("[Background(PID:  %d)]\r\n", pid);
        PID = fopen((OPENAPRS_PATH_PID).c_str(), "w");

        if (PID == NULL)
          die("Could not write to PID file.");

        fprintf(PID, "%d\n", pid);

      app->cfg->replace_int("openaprs.background.pid", pid);
      app->cfg->replace_int("openaprs.process.id", pid);
      fclose(PID);

      // close stdout and stderr
      close(0);
      close(1);
      exit(0);
    } // if
  } // else

  app->cfg->replace_int("openaprs.process.id", getpid());

  /***********************
   ** Initialize System **
   ***********************/

  string logpath = app->cfg->get_string("app.uplink.log.path", "logs/openaprs.uplink.log");
  elog.open("uplink", logpath);
  int loglevel = app->cfg->get_int("app.uplink.log.level", 6);
  if (loglevel >= 0 && loglevel <= 7)
    elog.level("uplink", static_cast<openframe::loglevel::Level>(loglevel));
  else
    elog.level("uplink", openframe::loglevel::LogDebug);

  app->initializeSystem();
  app->Run();
  app->deinitializeSystem();

  // cleanup
  delete app;

  return 1;
} // main

int sectotf(time_t ts, struct timeframe *tf) {
  tf->hours = ts / 3600;
  tf->minutes = (ts - (tf->hours * 3600)) / 60;
  tf->seconds = ts - (tf->hours * 3600) - (tf->minutes * 60);
  return 1;
} // sectotf

int is_dir(const char *path) {
  int ret;
  struct stat sb;
    
  ret = lstat(path, &sb);
  if (ret == -1) {
    // debug(HERE, "is_dir(): could not stat %s, %s\n", path, strerror(errno));
    return 0;
  } // if
   
  if (sb.st_mode & S_IFDIR) {
    // debug(HERE, "is_dir(): found that %s is a directory.\n", path);
    return 1;
  } // if
   
  // debug(HERE, "is_dir(): found that %s is not a directory, aborting.\n", path);
  return 0;
} // is_dir

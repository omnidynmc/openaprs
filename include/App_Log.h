/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, mySQL APRS Injector                                        **
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
 $Id: DCC.h,v 1.8 2003/09/04 00:22:00 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_APPLOG_H
#define __OPENAPRS_APPLOG_H

#include <iostream>
#include <string>

#include <pthread.h>

#include "OpenAPRS_Log.h"
#include "Server.h"

namespace openaprs {
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  class App_Log : public OpenAPRS_Log {
    public:
      App_Log(const string &ident) : OpenAPRS_Log(ident) { }
      virtual ~App_Log() { }

      virtual void log(const string &message) {
        //cout << message << endl;
        app->writeLog(OPENAPRS_LOG_NORMAL, "%s%s%s", _ident.c_str(), (_ident.length() ? " " : ""), message.c_str());
      } // log

      virtual void slog(const string &where, const string &message) {
        //cout << message << endl;
        app->writeLog(where, "%s%s%s", _ident.c_str(), (_ident.length() ? " " : ""), message.c_str());
      } // log

      virtual void debug(const string &message) {
        //cout << message << endl;
        //app->writeLog(OPENAPRS_LOG_DEBUG, "%u %s", _tid, message.c_str());
        app->writeLog(OPENAPRS_LOG_DEBUG, "%s%s%s", _ident.c_str(), (_ident.length() ? " " : ""), message.c_str());
      } // log

      virtual void console(const string &message) {
        //cout << message << endl;
        //app->writeLog(OPENAPRS_LOG_DEBUG, "%u %s", _tid, message.c_str());
        app->writeLog(OPENAPRS_LOG_STDOUT, "%s%s%s", _ident.c_str(), (_ident.length() ? " " : ""), message.c_str());
      } // log

  protected:
  }; // class App_Log

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

extern "C" {
} // extern

}
#endif

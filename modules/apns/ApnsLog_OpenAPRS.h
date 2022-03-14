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

#ifndef __MODULE_APNSLOGOPENAPRS_H
#define __MODULE_APNSLOGOPENAPRS_H

#include "pthread.h"

#include "ApnsLog.h"
#include "Server.h"

#include "apns.h"

namespace apns {

using namespace openaprs;
using namespace std;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class ApnsLog_OpenAPRS : public ApnsLog {
  public:
    ApnsLog_OpenAPRS(const string &ident) : _ident(ident) { _tid = pthread_self(); }
    virtual ~ApnsLog_OpenAPRS() { }

    virtual void log(const string &message) {
      //app->writeLog(MODULE_APNS_LOG_NORMAL, "%u %s %s", _tid, _ident.c_str(), message.c_str());
      app->writeLog(MODULE_APNS_LOG_NORMAL, "%s %s", _ident.c_str(), message.c_str());
    } // log

    virtual void debug(const string &message) {
      app->writeLog(MODULE_APNS_LOG_DEBUG, "%s %s", _ident.c_str(), message.c_str());
    } // log

  protected:
    pthread_t _tid;
    string _ident;
}; // class ApnsLog

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

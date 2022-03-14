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

#ifndef __OPENAPRS_OPENAPRSLOG_H
#define __OPENAPRS_OPENAPRSLOG_H

#include <iostream>
#include <string>

#include <pthread.h>

namespace openaprs {
  using std::string;
  using std::cout;
  using std::endl;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define OPENAPRSLOG_MAXBUF	1024

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  class OpenAPRS_Log {
    public:
      OpenAPRS_Log(const string &ident) : _ident(ident) { _tid = pthread_self(); }
      virtual ~OpenAPRS_Log() { }

      virtual const string ident() const { return _ident; }

      virtual void log(const string &message) {
        cout << "Abstract: " << message << endl;
      } // log

      virtual void slog(const string &where, const string &message) {
        cout << "Abstract: to " << where << " " << message << endl;
      } // log

      virtual void debug(const string &message) {
        cout << "Abstract: " << message << endl;
      } // log

      virtual void console(const string &message) {
        cout << "Abstract: " << message << endl;
      } // log

  protected:
    pthread_t _tid;
    string _ident;
  }; // class OpenAPRS_Log

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

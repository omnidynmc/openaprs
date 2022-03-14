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

#ifndef __OPENAPRS_THREADMESSAGE_H
#define __OPENAPRS_THREADMESSAGE_H

#include <string>
#include <map>
#include <set>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "openframe/VarController.h"

#include "openaprs_string.h"
#include "misc.h"

namespace openaprs {
  using openframe::VarController;
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class ThreadMessage {
    public:
      typedef unsigned int threadIdType;

      ThreadMessage(const threadIdType id) : _id(id) { var = new VarController(); }
      ThreadMessage(const string &action, const string &message) : _id(0), _action(action), _message(message) {
        var = new VarController();
      } // ThreadMessage
      ThreadMessage(const ThreadMessage &tm) : _id(tm._id), _action(tm._action), _message(tm._message) {
        var = new VarController(*(tm.var));
      } // ThreadMessage

      virtual ~ThreadMessage() { delete var; }

      const threadIdType id() const { return _id; }
      const string message() const { return _message; }
      const string action() const { return _action; }
      void message(const string &message) { _message = message; }

      VarController *var;

    private:
      threadIdType _id;		// thread id
      string _action;
      string _message;		// message
  };

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

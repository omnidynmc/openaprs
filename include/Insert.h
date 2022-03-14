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
 $Id: Insert.h,v 1.2 2005/12/13 16:08:01 omni Exp $
 **************************************************************************/

#ifndef __LIBHANDLER_INSERT_H
#define __LIBHANDLER_INSERT_H

#include <string>
#include <list>
#include <map>

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

#include "openframe/OFLock.h"

#include "noCaseCompare.h"

namespace openaprs {
  using openframe::OFLock;
  using std::list;
  using std::map;
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define LIBHANDLER_OPTION_DEFAULT_INT				0
#define LIBHANDLER_OPTION_DEFAULT_STRING			""
#define LIBHANDLER_OPTION_DEFAULT_BOOL				false

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class Insert : public OFLock {
  public:
    Insert();
    virtual ~Insert();

    typedef list<string> insertListType;
    typedef map<string, string> queryListType;
    typedef map<string, insertListType, noCaseCompare> insertMapType;

    /********************
     ** Option Members **
     ********************/

    const bool Add(const string &, const char *fmt, ...);
    const bool Flush();
    const bool Dequeue(insertListType &);

  private:
    insertMapType insertList;
    queryListType queryList;
    queryListType updateList;

  protected:
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openaprs
#endif

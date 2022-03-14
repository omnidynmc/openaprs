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
 $Id: DB.h,v 1.1 2005/11/21 18:16:02 omni Exp $
 **************************************************************************/

#ifndef __MODULE_MGMT_DBI_H
#define __MODULE_MGMT_DBI_H

#include <string>
#include <map>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <mysql++.h>

#include "DBI.h"

namespace mgmt {
  using std::string;
  using std::map;
  using std::list;
  using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class MGMT_DBI : public DBI {
  public:
    MGMT_DBI();
    virtual ~MGMT_DBI();

    // ### Type Definitions ###

    // ### Public Members ###
    const DBI::resultSizeType getUserByCallsign(const string &, DBI::resultType &);
    const DBI::resultSizeType getUserById(const string &, DBI::resultType &);
    const DBI::resultSizeType getUserByEmail(const string &, DBI::resultType &);
    const DBI::resultSizeType activateUserByCallsign(const string &);
    const DBI::resultSizeType activateUserById(const string &);
    const DBI::resultSizeType activateUserByEmail(const string &);
    const DBI::resultSizeType verifyUserByCallsign(const string &);
    const DBI::resultSizeType verifyUserById(const string &);
    const DBI::resultSizeType verifyUserByEmail(const string &);

  protected:
  private:
    DBI::queryMapType _query;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

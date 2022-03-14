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
    const DBI::resultSizeType addPartition(const string &, const string &);
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with this program; if not, write to the Free Software          **
 ** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            **
 **************************************************************************
 $Id: DB.h,v 1.1 2005/11/21 18:16:02 omni Exp $
 **************************************************************************/

#ifndef __MODULE_DBI_ROLLUP_H
#define __MODULE_DBI_ROLLUP_H

#include <string>
#include <map>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <mysql++.h>

#include "DBI.h"

namespace rollup {
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

class DBI_Rollup : public DBI {
  public:
    DBI_Rollup();
    virtual ~DBI_Rollup();

    // ### Type Definitions ###

    // ### Public Members ###
    const DBI::resultSizeType getWeatherRollupCallsignId(const string &, const int, DBI::resultType &);
    const DBI::resultSizeType rollupWeatherForCallsignId(const string &, const string &);
    const DBI::resultSizeType removeWeatherAfterRollup(const string &, const string &);
    const DBI::resultSizeType setLastRollupForCallsignId(const string &);
    const DBI::resultSizeType addPartitions(const string &, const string &);
    const DBI::resultSizeType dropPartitions(const string &, const string &);
    const DBI::resultSizeType optimizeTable(const string &);

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

} // namespace rollup
#endif

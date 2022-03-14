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

#ifndef __MODULE_SAT_DBI_SAT_H
#define __MODULE_SAT_DBI_SAT_H

#include <string>
#include <map>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <mysql++.h>

#include "DBI.h"

namespace sat {
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

class DBI_Sat : public DBI {
  public:
    DBI_Sat();
    virtual ~DBI_Sat();

    // ### Type Definitions ###

    // ### Public Members ###
    const DBI::resultSizeType getSats(DBI::resultType &);
    const DBI::resultSizeType updateSats(const string &number,
                                         const time_t packet_date,
                                         const string &name,
                                         const double elevation,
                                         const double azimuth,
                                         const double latitude,
                                         const double longitude,
                                         const double altitude,
                                         const time_t create_ts);
    const DBI::resultSizeType updateNextSats(const string &number,
                                             const time_t packet_date,
                                             const string &name,
                                             const double elevation,
                                             const double azimuth,
                                             const double latitude,
                                             const double longitude,
                                             const double altitude,
                                             const time_t create_ts);
    const DBI::resultSizeType removeOldNext(const string &interval);
    const DBI::resultSizeType setLastSatForCallsignId(const string &);
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

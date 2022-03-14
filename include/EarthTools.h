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
 $Id: EarthTools.h,v 1.1 2005/11/21 18:16:02 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_EARTHTOOLS_H
#define __OPENAPRS_EARTHTOOLS_H

#include <string>
#include <map>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "openaprs.h"
#include "openaprs_string.h"
#include "misc.h"

namespace openaprs {

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

class EarthTools {
  public:
    EarthTools();				// constructor
    virtual ~EarthTools();			// destructor

    /**********************
     ** Type Definitions **
     **********************/

    /***************
     ** Variables **
     ***************/

    /**********************
     ** DISTANCE Members **
     **********************/

    const double DISTANCE(double, double, double, double, char);
    double deg2rad(double);
    double rad2deg(double);

    /****************************
     ** isValidLatLong Members **
     ****************************/

    const bool isValidLatLong(const double, const double);

    /*******************
     ** SPEED Members **
     *******************/

    const double SPEED(const double, const time_t, 
                       const int, const int);


    const string DecimalToNMEA(const double, const bool);

    /***************
     ** Variables **
     ***************/
  public:
  protected:
  private:
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

#if 0
  #define KnotToKmh(x)			(x * 1.852);
  #define	MphToKmh(x)			(x * 1.609344);
  #define KmhToMs(x)			(x * 10 / 36);
  #define MphToMs(x)			(x * 1.609344 * 10 / 36);
  #define InchToMm(x)			(x * 0.254);
#endif

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

int axtoi(const char *);

}
#endif

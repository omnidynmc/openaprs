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
 $Id: APRS.h,v 1.1 2005/11/21 18:16:02 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_APRS_H
#define __OPENAPRS_APRS_H

#include <string>
#include <map>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "LineBuffer.h"
#include "Log.h"
#include "Network.h"
#include "Options.h"
#include "Command.h"
#include "Timer.h"

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

class APRS  {
  public:
    APRS();				// constructor
    virtual ~APRS();			// destructor

    /**********************
     ** Type Definitions **
     **********************/

    /***************
     ** Variables **
     ***************/

    /**********************
     ** Altitude Members **
     **********************/

    const bool Altitude(string &, Packet *);

    /*****************
     ** DFS Members **
     *****************/

    const bool DFS(string &, Packet *);

    /******************
     ** AFRS Members **
     ******************/

    const bool AFRS(string &, Packet *);

    /************************
     ** AREAOBJECT Members **
     ************************/

    const bool AREAOBJECT(string &, Packet *);

    /******************
     ** BASE Members **
     ******************/

    const string BASE(const string);
    const string SAFE(const string &);

    /************************
     ** DUPLICATES Members **
     ************************/
    const bool DUPLICATES(Packet *);
    const bool DUPLICATES_POSITION(Packet *);

    /*********************************
     ** Direction and Speed Members **
     *********************************/

    const bool DFR(string &, Packet *);
    const bool DirSpd(string &, Packet *);

    /**********************
     ** Location Members **
     **********************/

    const bool Location(string &, Packet *);
    const bool LocationCMP(string &, Packet *);

    /*********************
     ** Message Members **
     *********************/

    const bool Message(string &, Packet *);

    /*******************
     ** MIC_E Members **
     *******************/

    const bool MIC_EToDecimal(string &, Packet *);
    const bool MIC_E(string &, Packet *);

    /******************
     ** NMEA Members **
     ******************/

    const bool NMEA(string &, Packet *);
    const bool convertPOS(const string &, const string &, const string &, const string &,
                          map<string, string> &);

    /*****************
     ** RNG Members **
     *****************/

    const bool RNG(string &, Packet *);

    /***********************
     ** AltSymbol Members **
     ***********************/

    const bool AltSymbol(string &, Packet *);

    /***********************
     ** Telemetry Members **
     ***********************/

    const bool Telemetry(string &, Packet *);

    /**********************
     ** SIGNPOST Members **
     **********************/

    const bool SIGNPOST(string &, Packet *);

    /*****************
     ** PHG Members **
     *****************/

    const bool PHG(string &, Packet *);

    /*****************
     ** DAO Members **
     *****************/

    const bool DAO(string &, Packet *, float &, float &);

    /**********************
     ** DISTANCE Members **
     **********************/

    const double DISTANCE(double, double, double, double, char);
    const bool destPoint(const int, const double, const double, const double,
                         double &, double &);
    double deg2rad(double);
    double rad2deg(double);

    /****************************
     ** isValid*       Members **
     ****************************/

    const bool isValidLatLong(const double, const double);
    const bool isValidMessage(const string &);
    const bool isValidCallsign(const string &);

    /*******************
     ** SPEED Members **
     *******************/

    const double SPEED(const double, const time_t, 
                       const int, const int);

    /***********************
     ** ULTIMETER Members **
     ***********************/

    const bool ULTIMETER(string &, Packet *);

    /*******************************
     ** MESSAGE_TELEMETRY Members **
     *******************************/

    const bool MESSAGE_TELEMETRY(string &, Packet *);

    /*********************
     ** Weather Members **
     *********************/

    const bool WeatherPOS(string &, Packet *);

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

#define KnotToKmh(x)			(x * 1.852)
#define KphToKnot(x)			(x * 0.539956803)
#define	MphToKmh(x)			(x * 1.609344)
#define KmhToMs(x)			(x * 10 / 36)
#define MphToMs(x)			(x * 1.609344 * 10 / 36)
#define HInchToMm(x)			(x * 0.254)
#define FtToM(x)			(x * 0.3048);

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

int axtoi(const char *);

}
#endif

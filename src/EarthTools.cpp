/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
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
 $Id: EarthTools.cpp,v 1.2 2005/12/13 21:07:03 omni Exp $
 **************************************************************************/

#include <list>
#include <new>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "EarthTools.h"
#include "Log.h"
#include "LineBuffer.h"
#include "Server.h"

#include "stdinCommands.h"

#include "config.h"
#include "openaprs.h"
#include "openaprs_string.h"

#define pi 3.14159265358979323846

namespace openaprs {

using std::string;
using std::list;
using std::ofstream;
using std::ostream;
using std::stringstream;
using std::ios;
using std::endl;
using std::cout;

/**
 * Distance Units
 *
 * Metres, Millimetres, Centimetres, Kilometres, Inches, Feet, Yards, Fathoms
 * 0       1            2            3           4       5     6      7
 * Statute Miles, Nautiecal Miles, Light Years (9460000000000000)
 * 8              9                10
 */
  
static double distunits[] = { 1, 0.001, 0.01, 1000, 0.0254, 0.3048, 0.9144,
                              1.8288, 1609, 1852, -1};
    
/**
 * Speed Units
 *
 * Metres/s, Kilometre/hr, Feet/s, Feet/m, Yard/m, Statute Mile/hr, Knot
 * 0         1             2       3       4       5                6
 */
 
static double speedunits[] = { 1, 0.277777777777777777777777777777777777,
                              0.3048, 0.00508, 0.01524, 0.44704, 0.514444444444, -1};


/**************************************************************************
 ** EarthTools Class                                                     **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

EarthTools::EarthTools() {
  // set defaults
}

EarthTools::~EarthTools() {
  // cleanup
}

/***************  
 ** Variables **
 ***************/

/**********************
 ** DISTANCE Members **
 **********************/

const double EarthTools::DISTANCE(double lat1, double lon1, double lat2,
                            double lon2, char unit) {
  double theta;
  double distance;


  if (lat1 > 90 || lat1 < -90 ||
      lat2 > 90 || lat2 < -90)
    return -1;

  if (lon1 > 180 || lon1 < -180 ||
      lon2 > 180 || lon2 < -180)
    return -1;

  theta = lon1 - lon2;

  distance = (sin(deg2rad(lat1)) * sin(deg2rad(lat2))) + (cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta)));
  distance = acos(distance);
  distance = rad2deg(distance);
  distance = distance * 60 * 1.1515;

  switch(unit) {
    case 'M':
      break;
    case 'K':
      distance = distance * 1.609344;
      break;
    case 'N':
      distance = distance * 0.8684;
      break;
  } // switch

  return distance;
} // EarthTools::Distance

double EarthTools::deg2rad(double deg) {
  return (deg * pi / 180);
} // EarthTools::deg2rad

double EarthTools::rad2deg(double rad) {
  return (rad * 180 / pi);
} // EarthTools::rad2deg

/****************************
 ** isValidLatLong Members **
 ****************************/

const bool EarthTools::isValidLatLong(const double lat, const double lon) {
  /**
   * Invalid Positions
   *
   * Don't store invalid positions.
   *
   * Latitude > 90 || Latitude < -90 [INVALID]
   * Longitude > 180 || Latitude < -180 [INVALID]
   * Latitude == 0 || Longitude == 0 [INVALID]
   */
  if (lat == 0 || lon == 0 || lon > 180 || lon < -180 ||
      lat > 90 || lat < -90)
    return false;

  return true;
} // EarthTools::isValidLatLong

/*******************
 ** SPEED Members **
 *******************/

const double EarthTools::SPEED(const double dist, const time_t seconds, 
                               const int distUnits, const int speedUnits) {
  double speed;
  time_t secs;

  if (dist < 1)
    return -1;

  // minimum of one second.
  if (seconds < 1)
    secs = 1;
  else
    secs = seconds;

  speed = ((dist * distunits[distUnits])  / (secs * speedunits[speedUnits])); 

  return speed;
} // EarthTools::Speed

const string EarthTools::DecimalToNMEA(const double n, const bool isLat) {
  double d, min;
  int deg;
  char y[10];

  d = fabs(n);

  deg = (int) floor(d);
  min = (d-deg)*60;

  if (isLat)
    snprintf(y, sizeof(y), "%02d%05.2f", deg, min);
  else
    snprintf(y, sizeof(y), "%03d%05.2f", deg, min);

  return string(y);
} // EarthTools::DecimalToNMEA

}

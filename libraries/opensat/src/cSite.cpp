//
// cSite.cpp
//
// Copyright (c) 2003 Michael F. Henry
//
//////////////////////////////////////////////////////////////////////////////
#include "opensat/stdafx.h"
#include "opensat/cSite.h"
#include "opensat/globals.h"
#include "opensat/Sat_Exception.h"

namespace opensat {

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
cSite::cSite(const cCoordGeo &geo) : m_geo(geo)
{}

//////////////////////////////////////////////////////////////////////////////
// c'tor accepting:
//    Latitude  in degress (negative south)
//    Longitude in degress (negative west)
//    Altitude  in km
cSite::cSite(double degLat, double degLon, double kmAlt) :
   m_geo(deg2rad(degLat), deg2rad(degLon), kmAlt)
{}

cSite::~cSite()
{}

//////////////////////////////////////////////////////////////////////////////
// setGeo()
// Set a new geographic position
void cSite::setGeo(const cCoordGeo &geo)
{
   m_geo = geo;
}

//////////////////////////////////////////////////////////////////////////////
// getPosition()
// Return the ECI coordinate of the site at the given time.
cEci cSite::getPosition(const cJulian &date) const
{
   return cEci(m_geo, date);
}

//////////////////////////////////////////////////////////////////////////////
// getLookAngle()
// Return the topocentric (azimuth, elevation, etc.) coordinates for a target
// object described by the input ECI coordinates.
cCoordTopo cSite::getLookAngle(const cEci &eci) const
{
   // Calculate the ECI coordinates for this cSite object at the time
   // of interest.
   cJulian date = eci.getDate();
   cEci eciSite(m_geo, date);

   // The Site ECI units are km-based; ensure target ECI units are same
   if (!eci.UnitsAreKm()) throw SatUnitsNotKm_Exception();
   //assert(eci.UnitsAreKm());

   cVector vecRgRate(eci.getVel().m_x - eciSite.getVel().m_x,
                     eci.getVel().m_y - eciSite.getVel().m_y,
                     eci.getVel().m_z - eciSite.getVel().m_z);

   double x = eci.getPos().m_x - eciSite.getPos().m_x;
   double y = eci.getPos().m_y - eciSite.getPos().m_y;
   double z = eci.getPos().m_z - eciSite.getPos().m_z;
   double w = sqrt(sqr(x) + sqr(y) + sqr(z));

   cVector vecRange(x, y, z, w);

   // The site's Local Mean Sidereal Time at the time of interest.
   double theta = date.toLMST(getLon());

   double sin_lat   = sin(getLat());
   double cos_lat   = cos(getLat());
   double sin_theta = sin(theta);
   double cos_theta = cos(theta);

   double top_s = sin_lat * cos_theta * vecRange.m_x + 
                  sin_lat * sin_theta * vecRange.m_y - 
                  cos_lat * vecRange.m_z;
   double top_e = -sin_theta * vecRange.m_x + 
                   cos_theta * vecRange.m_y;
   double top_z = cos_lat * cos_theta * vecRange.m_x + 
                  cos_lat * sin_theta * vecRange.m_y + 
                  sin_lat * vecRange.m_z;
   double az    = atan(-top_e / top_s);

   if (top_s > 0.0)
      az += PI;

   if (az < 0.0)
      az += 2.0*PI;

   double el   = asin(top_z / vecRange.m_w);
   double rate = (vecRange.m_x * vecRgRate.m_x + 
                  vecRange.m_y * vecRgRate.m_y + 
                  vecRange.m_z * vecRgRate.m_z) / vecRange.m_w;

   cCoordTopo topo(az,           // azimuth,   radians
                   el,           // elevation, radians
                   vecRange.m_w, // range, km
                   rate);        // rate,  km / sec

#ifdef WANT_ATMOSPHERIC_CORRECTION
   // Elevation correction for atmospheric refraction.
   // Reference:  Astronomical Algorithms by Jean Meeus, pp. 101-104
   // Note:  Correction is meaningless when apparent elevation is below horizon
   topo.m_El += deg2rad((1.02 / 
                        tan(deg2rad(rad2deg(el) + 10.3 / 
                                    (rad2deg(el) + 5.11)))) / 60.0);
   if (topo.m_El < 0.0)
      topo.m_El = el;    // Reset to true elevation

   if (topo.m_El > (PI / 2))
      topo.m_El = (PI / 2);
#endif

   return topo;
}

//////////////////////////////////////////////////////////////////////////////
// toString()
//
string cSite::toString() const
{
   const int TEMP_SIZE = 128;
   char sz[TEMP_SIZE];

   bool LatNorth = true;
   bool LonEast  = true;

   if (m_geo.m_Lat < 0.0) 
   {
      LatNorth = false;
   }

   if (m_geo.m_Lon < 0.0)
   {
      LonEast = false;
   }

   snprintf(sz, TEMP_SIZE, 
             "%06.3f%c, ", 
             fabs(rad2deg(m_geo.m_Lat)),
             (LatNorth ? 'N' : 'S'));

   string strLoc = sz;

   snprintf(sz, TEMP_SIZE, 
             "%07.3f%c, ", 
             fabs(rad2deg(m_geo.m_Lon)),
             (LonEast ? 'E' : 'W'));
   strLoc += sz;

   snprintf(sz, TEMP_SIZE, 
            "%.1fm\n", 
            (m_geo.m_Alt * 1000.0));
   strLoc += sz;

   return strLoc;
}

} // namespace opensat

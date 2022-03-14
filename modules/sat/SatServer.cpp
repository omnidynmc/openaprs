#include <string>
#include <iostream>

#include <dirent.h>

#include <math.h>

#include <openframe/openframe.h>

#include <opensat/cTle.h>
#include <opensat/cEci.h>
#include <opensat/cOrbit.h>
#include <opensat/cSite.h>
#include <opensat/globals.h>
#include <opensat/Sat_Exception.h>

#include "DBI_Sat.h"
#include "SatServer.h"

namespace sat {
  using namespace std;
  using openframe::StringTool;
  using openframe::VarController;

/**************************************************************************
 ** SatServer Class                                                      **
 **************************************************************************/

  SatServer::SatServer(const std::string &tle_path) : _tle_path(tle_path) {
    _dbi = NULL;

    try {
      _dbi = new DBI_Sat();
    } // try
    catch(bad_alloc xa) {
      assert(false);
    } // catch

    _time_next_positions = new openframe::Stopwatch(20);

    return;
  } // SatServer::SatServer

  SatServer::~SatServer() {
    if (_dbi) delete _dbi;
    return;
  } // SatServer::~SatServer

  const bool SatServer::run() {
    bool didWork = false;

    time_t start_time = time(NULL);
//    time_t end_time = time(NULL) + 120;
    time_t end_time = time(NULL) + 1;
    int step = 1;
    double latitude = 0.0;
    double longitude = 0.0;

    DBI::resultType res;
    bool ok = _dbi->getSats(res);
    if (!ok) return false;

    for(DBI::resultSizeType i=0; i < res.num_rows(); i++) {
      string num = res[i]["number"].c_str();
//      _logf("processing sat; %s", num.c_str());
      tles_itr itr = _tles.find(num);
      if (itr == _tles.end())
        continue;

      TLE *tle = &itr->second;
      satpos_t satpos;
      ok = calcPositions(tle, latitude, longitude, start_time, end_time, step, satpos);
      for(size_t j=0; j < satpos.size(); j++)
        _dbi->updateSats(num,
                         satpos[j].timestamp,
                         satpos[j].name,
                         satpos[j].elevation,
                         satpos[j].azimuth,
                         satpos[j].latitude,
                         satpos[j].longitude,
                         satpos[j].altitude,
                         satpos[j].timestamp);
    } // for

//    _logf("processed %d", res.num_rows());

    if (_time_next_positions->Next())
      didWork |= next_positions();

    return didWork;
  } // SatServer::run

  const bool SatServer::next_positions() {
    bool didWork = false;

    time_t start_time = time(NULL);
    time_t end_time = time(NULL) + 30;
    int step = 1;
    double latitude = 0.0;
    double longitude = 0.0;

    DBI::resultType res;
    bool ok = _dbi->getSats(res);
    if (!ok) return false;

    for(DBI::resultSizeType i=0; i < res.num_rows(); i++) {
      string num = res[i]["number"].c_str();
//      _logf("processing sat; %s", num.c_str());
      tles_itr itr = _tles.find(num);
      if (itr == _tles.end())
        continue;

      TLE *tle = &itr->second;
      satpos_t satpos;
      ok = calcPositions(tle, latitude, longitude, start_time, end_time, step, satpos);
      for(size_t j=0; j < satpos.size(); j++)
        _dbi->updateNextSats(num,
                         satpos[j].timestamp,
                         satpos[j].name,
                         satpos[j].elevation,
                         satpos[j].azimuth,
                         satpos[j].latitude,
                         satpos[j].longitude,
                         satpos[j].altitude,
                         satpos[j].timestamp);
    } // for

//    _logf("processed %d", res.num_rows());
    _dbi->removeOldNext("1 MINUTE");
    return didWork;
  } // SatServer::next_positions

  const bool SatServer::calcPositions(const TLE *tle,
                                      const double latitude,
                                      const double longitude,
                                      const time_t start_time,
                                      const time_t end_time,
                                      const time_t step,
                                      satpos_t &ret) {
    string str1 = tle->name;
    string str2 = tle->line[0];
    string str3 = tle->line[1];

    opensat::cTle tleSDP4(str1, str2, str3);
    opensat::cOrbit orbitSDP4(tleSDP4);
    opensat::cEci eciSDP4;
    opensat::cJulian gmt;

    for(time_t ts = start_time; ts < end_time; ts += step) {

      opensat::cJulian gmt = opensat::cJulian(ts);
      // printf("%.4f\n", (orbitSDP4.TPlusEpoch(gmt) / 60));
      orbitSDP4.getPosition((orbitSDP4.TPlusEpoch(gmt) / 60), &eciSDP4);

      // Now create a site object. Site objects represent a location on the
      // surface of the earth. Here we arbitrarily select a point on the
      // equator.
      opensat::cSite siteEquator(latitude, longitude, 0); // 0.00 N, 100.00 W, 0 km altitude

      // Now get the "look angle" from the site to the satellite.
      // Note that the ECI object "eciSDP4" contains a time associated
      // with the coordinates it contains; this is the time at which
      // the look angle is valid.
      opensat::cCoordTopo topoLook;
      try {
        topoLook = siteEquator.getLookAngle(eciSDP4);
      } // try
      catch(opensat::Sat_Exception ex) {
        //cout << "ERROR: cSite; " << ex.message() << endl;
        continue;
      } // catch

      opensat::cCoordGeo geoLook = eciSDP4.toGeo();

      // Print out the results. Note that the Azimuth and Elevation are
      // stored in the cCoordTopo object as radians. Here we convert
      // to degrees using rad2deg()
      //printf("AZ: %.6f  EL: %.6f\n",
      //       rad2deg(topoLook.m_Az),
      //       rad2deg(topoLook.m_El));

      double new_lon;
      double sat_lon = opensat::rad2deg(geoLook.m_Lon);

      sat_lon = 360.0 - sat_lon;
      if (sat_lon > 180)
        new_lon = (180 - (sat_lon - 180));
      else
        new_lon = sat_lon * -1;
/*
        printf("%d\t%d\t%s\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\n", 
               ts,
               satellite_number,
               orbitSDP4.SatName(false).c_str(),
               opensat::rad2deg(geoLook.m_Lat),
               new_lon,
               geoLook.m_Alt,
               opensat::rad2deg(topoLook.m_Az),
               opensat::rad2deg(topoLook.m_El));
*/
        SatPosition sp;
        sp.name = orbitSDP4.SatName(false);
        sp.latitude = opensat::rad2deg(geoLook.m_Lat);
        sp.longitude = new_lon;
        sp.altitude = geoLook.m_Alt;
        sp.azimuth = opensat::rad2deg(topoLook.m_Az);
        sp.elevation = opensat::rad2deg(topoLook.m_El);
        sp.timestamp = ts;
        ret.push_back(sp);

      } // for

    return true;
  } // SatServer::calcPositions

  SatServer &SatServer::load_tle(const string &path) {
    struct dirent *dit;

    DIR *dip = opendir( path.c_str() );
    if (dip == NULL) {
      _logf("unable to open; %s", path.c_str());
      return *this;
    } // if

    _logf("opened %s", path.c_str());

    _tles.clear();

    /*  struct dirent *readdir(DIR *dir);
     *
     * Read in the files from argv[1] and print */
    while((dit = readdir(dip)) != NULL) {
       if (!StringTool::compare(dit->d_name, "..") || !StringTool::compare(dit->d_name, "."))
         continue;

      //_logf("loading TLE file; %s", dit->d_name);

      string filename = path + string(dit->d_name);
      std::ifstream datafile;
      datafile.open(filename.c_str(), ifstream::in);

      if (!datafile.is_open()) {
        _logf("error could not open file; %s", filename.c_str());
        continue;
      } // if

      std::string line;
      TLE tle;
      for(int i = 0; !std::getline(datafile, line, '\n').eof(); i++) {
        while(line[line.size() - 1] == ' ' ||
              line[line.size() - 1] == '\r' ||
              line[line.size() - 1] == '\n')
          line.erase(line.size() - 1);

        openframe::StringToken st;

        switch((i % 3)) {
          case 0:
            if (line.length() < 1) {
              _logf("invalid tle line; %s", filename.c_str());
              continue;
            } // if

            tle.name = line;
            tle.line.clear();
            break;
          case 1:
            tle.line.push_back(line);
            break;
          case 2:
            tle.line.push_back(line);
            st = line;

            if (st.size() < 2) {
              _logf("invalid tle line; %s", filename.c_str());
              continue;
            } // if

            _tles[st[1]] = tle;
            break;
          default:
            _logf("error, bad tle file; %s line %d", filename.c_str(), i);
            continue;
            break;
        } // switch
      } // for

      _logf("loaded TLE file; %s", filename.c_str());
      datafile.close();
    } // while

    _logf("found a total of %d elements", _tles.size() );

    if (closedir(dip) == -1) {
      _logf("failed to close dir; %s", path.c_str());
      return *this;
    } // if

    return *this;
  } // SatServer::load_tle
} // namespace sat

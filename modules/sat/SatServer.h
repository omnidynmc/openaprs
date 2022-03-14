#ifndef __MODULE_SAT_SATSERVER_H
#define __MODULE_SAT_SATSERVER_H

#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <set>
#include <list>

#include <openframe/openframe.h>

namespace sat {
  using openframe::OpenFrame_Exception;
  using openframe::StringTool;
  using openframe::OFLock;
  using std::map;
  using std::set;
  using std::stringstream;
  using std::string;
  using std::vector;
  using std::list;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  // ### Forward Declarations ###

  class SatServer_Exception : public OpenFrame_Exception {
    public:
      SatServer_Exception(const string message) throw() : OpenFrame_Exception(message) { };
  }; // class SatServer_Exception

  class TLE {
    public:
      std::string name;
      std::vector<string> line;
  }; // class TLE

  class SatPosition {
    public:
      string name;
      double elevation;
      double azimuth;
      double latitude;
      double longitude;
      double altitude;
      time_t timestamp;
  }; // class SatPosition

  typedef vector<SatPosition> satpos_t;
  typedef satpos_t::iterator satpos_itr;
  typedef satpos_t::const_iterator satpos_citr;
  typedef satpos_t::size_type satpos_s;

  class DBI_Sat;
  class SatServer : public OpenAPRS_Abstract, public openframe::ThreadMessenger {
    public:
      SatServer(const string &);
      virtual ~SatServer();

      /**********************
       ** Type Definitions **
       **********************/
      typedef map<string, TLE> tles_t;
      typedef tles_t::iterator tles_itr;
      typedef tles_t::size_type tles_s;

      /***************
       ** Variables **
       ***************/

      // ### Core Members ###
      const bool run();
      const bool next_positions();
      const bool calcPositions(const TLE *tle,
                               const double latitude,
                               const double longitude,
                               const time_t start_time,
                               const time_t end_time,
                               const time_t step,
                               satpos_t &ret);

      // ### Options ###
      inline const bool debug() const { return _debug; }
      inline SatServer &debug(const bool debug) {
        _debug = debug;
        return (SatServer &) *this;
      } // debug
      SatServer &load_tle() { return load_tle(_tle_path); }
      SatServer &load_tle(const string &path);

    protected:
      // ### Protected Members ###

      // ### Protected Variables ###

    private:
      openframe::Stopwatch *_time_next_positions;
      DBI_Sat *_dbi;
      bool _debug;
      tles_t _tles;
      std::string _tle_path;
  }; // SatServer

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

  extern "C" {
  } // extern

} // namespace sat
#endif

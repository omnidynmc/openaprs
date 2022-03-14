#include <string>

#include <openframe/openframe.h>

#include <openstats/NullStats.h>
#include <openstats/StatsClient_Exception.h>
#include <openstats/constants.h>

namespace openstats {
  using namespace openframe::loglevel;

  /**************************************************************************
   ** NullStats Class                                                      **
   **************************************************************************/

  NullStats::NullStats() {
  } // NullStats::NullStats

  NullStats::~NullStats() {
  } // NullStats::~NullStats
} // namespace openstats


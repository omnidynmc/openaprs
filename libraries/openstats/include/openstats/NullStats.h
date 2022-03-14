#ifndef __OPENSTATS_NULLSTATS_H
#define __OPENSTATS_NULLSTATS_H

#include <string>
#include <map>

#include <pthread.h>

#include <openframe/openframe.h>

#include "StatsClient.h"
#include "constants.h"

namespace openstats {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class NullStats : public StatsClient {
    public:
      NullStats();
      virtual ~NullStats();

      virtual inline StatsClient &describe_stat(const std::string &label, const graphTypeEnum graph_type=graphTypeCounter, const dataTypeEnum data_type=dataTypeInt) { return *this; }
      virtual inline bool datapoint(const std::string &label, const statint_t value) { return true; }
      virtual inline bool datapoint_float(const std::string &label, const statfloat_t value) { return true; }

//      virtual NullStats &start() { return *this; }
      virtual void stop() { }

    protected:
      virtual inline void initializeThreads() { }
      virtual inline void deinitializeThreads() { }
    private:
  }; // NullStats

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openstats
#endif

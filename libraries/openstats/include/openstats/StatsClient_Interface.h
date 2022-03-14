#ifndef __OPENSTATS_STATSCLIENT_INTERFACE_H
#define __OPENSTATS_STATSCLIENT_INTERFACE_H

#include <string>

#include "StatsClient.h"
#include "constants.h"

namespace openstats {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class StatsClient_Interface {
    public:
      StatsClient_Interface();
      virtual ~StatsClient_Interface();

      void replace_stats(StatsClient *stats, const std::string &id="");
      void set_stat_id_prefix(const std::string &id) { _stat_id_prefix = id; }
      const std::string stat_id_prefix(const std::string &id) { return _stat_id_prefix + "." + id; }
      const std::string &stat_id_prefix() const { return _stat_id_prefix; }
      StatsClient *stats();

      virtual StatsClient &describe_stat(const std::string &id,
                                         const std::string &label,
                                         const graphTypeEnum graph_type=graphTypeGauge,
                                         const dataTypeEnum data_type=dataTypeInt,
                                         const useTypeEnum use_type=useTypeMean);
      virtual StatsClient &destroy_stat(const std::string &id);

      virtual bool datapoint(const std::string &id, const statint_t value=1);
      virtual bool datapoint_float(const std::string &id, const statfloat_t value=1.0);

      virtual void onDescribeStats() = 0;
      virtual void onDestroyStats() = 0;

    protected:
    private:
      StatsClient *_stats;
      std::string _stat_id_prefix;
  }; // StatsClient_Interface

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

#ifndef OPENSTATS_AVERAGE
#define OPENSTATS_AVERAGE(a, b)		(b ? double(a) / double(b) : 0)
#endif

#ifndef OPENSTATS_PERCENT
#define OPENSTATS_PERCENT(a, b)		(b ? (double(a) / double(b) * 100.0) : 0)
#endif

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openstats
#endif

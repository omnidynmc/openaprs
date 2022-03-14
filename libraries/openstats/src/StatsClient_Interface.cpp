#include <string>

#include <openframe/openframe.h>

#include <openstats/NullStats.h>
#include <openstats/StatsClient_Interface.h>

namespace openstats {
  using namespace openframe::loglevel;

  /**************************************************************************
   ** StatsClient_Interface Class                                          **
   **************************************************************************/

  StatsClient_Interface::StatsClient_Interface() {
    _stats = new NullStats();
  } // StatsClient_Interface::StatsClient_Interface

  StatsClient_Interface::~StatsClient_Interface() {
    if (_stats) _stats->release();
  } // StatsClient_Interface::~StatsClient_Interface

  void StatsClient_Interface::replace_stats(StatsClient *stats, const std::string &id) {
    if (_stats) _stats->release();
    _stats = stats;
    _stats->retain();
    _stat_id_prefix = id;
    onDescribeStats();
  } // StatsClient_Interface::replace_stats

  StatsClient *StatsClient_Interface::stats() {
    assert(_stats != NULL);
    return _stats;
  } // StatsClient_Interface::stats

  StatsClient &StatsClient_Interface::describe_stat(const std::string &id,
                                                    const std::string &label,
                                                    const graphTypeEnum graph_type,
                                                    const dataTypeEnum data_type,
                                                    const useTypeEnum use_type) {
    assert(_stats != NULL);
    return _stats->describe_stat( stat_id_prefix(id), label, graph_type, data_type, use_type);
  } // StatsClient

  StatsClient &StatsClient_Interface::destroy_stat(const std::string &id) {
    assert(_stats != NULL);
    return _stats->destroy_stat( stat_id_prefix(id) );
  } // StatsClient_Interface::destroy_stat

  bool StatsClient_Interface::datapoint(const std::string &id, const statint_t value) {
    assert(_stats != NULL);
    return _stats->datapoint( stat_id_prefix(id), value);
  } // StatsClient_Interface::datapoint

  bool StatsClient_Interface::datapoint_float(const std::string &id, const statfloat_t value) {
    assert(_stats != NULL);
    return _stats->datapoint_float( stat_id_prefix(id), value);
  } // Statsclient_Interface::datapoint_float

} // namespace openstats


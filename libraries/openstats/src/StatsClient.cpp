#include <string>
#include <queue>

#include <openframe/openframe.h>

#include <openstats/Data.h>
#include <openstats/Datapoint.h>
#include <openstats/StatsClient.h>
#include <openstats/StatsClient_Exception.h>
#include <openstats/constants.h>

namespace openstats {
  using namespace openframe::loglevel;

  /**************************************************************************
   ** StatsClient Class                                                    **
   **************************************************************************/

  StatsClient::StatsClient() : _thread(false) {
  } // StatsClient::StatsClient

  StatsClient::StatsClient(
    const time_t sampling_freq
  ) :
    _sampling_freq(sampling_freq),
    _thread(false) {
    initializeThreads();
  } // StatsClient::StatsClient

  StatsClient::~StatsClient() {
    for(descrip_itr itr=_descrips.begin(); itr != _descrips.end(); itr++) {
      Data_Abstract *data = itr->second;
      data->release();
    } // for
    _descrips.clear();

    done();
    deinitializeThreads();
  } // StatsClient::~StatsClient

  void StatsClient::initializeThreads() {
    pthread_create(&_thread_tid, NULL, StatsClient::UpdateThread, this);
    _thread = true;
  } // StatsClient::initializeThreads

  void StatsClient::deinitializeThreads() {
    if (!_thread) return;
    pthread_join(_thread_tid, NULL);
  } // StatsClient::deinitializeThreads

//  StatsClient &StatsClient::start() {
//    return *this;
//  } // StatsClient::start

  void StatsClient::stop() {
    done();
  } // StatsClient::stop

  StatsClient &StatsClient::describe_stat(const std::string &id,
                                          const std::string &label,
                                          const graphTypeEnum graph_type,
                                          const dataTypeEnum data_type,
                                          const useTypeEnum use_type) {
    openframe::scoped_lock slock(&_descrips_l);
    descrip_citr citr = _descrips.find(label);
    if ( citr != _descrips.end() ) throw StatsClientLabelAlreadyExists_Exception();
    if (graph_type == graphTypeCounter && data_type == dataTypeFloat) throw StatsClientUnsupportedDataType_Exception();

    Data_Abstract *data;
    switch(data_type) {
      case dataTypeInt:
        data = new Data<statint_t>(id, label, graph_type, data_type, use_type);
        break;
      case dataTypeFloat:
        data = new Data<statfloat_t>(id, label, graph_type, data_type, use_type);
        break;
      default:
        throw StatsClientUnsupportedDataType_Exception();
        break;
    } // switch

    _descrips.insert( make_pair(id, data) );
    return *this;
  } // describe_stat

  StatsClient &StatsClient::destroy_stat(const std::string &id) {
    openframe::scoped_lock slock(&_descrips_l);
    std::queue<descrip_itr> remove;
    for(descrip_itr itr=_descrips.begin(); itr != _descrips.end(); itr++) {
      bool is_match = openframe::StringTool::match(id.c_str(), itr->first.c_str() );
      if (!is_match) continue;
      remove.push(itr);
    } // for

    while( !remove.empty() ) {
      descrip_itr itr = remove.front();
      Data_Abstract *data = itr->second;
      data->release();
      _descrips.erase(itr);
      remove.pop();
    } // while
    return *this;
  } // destroy_stat

  bool StatsClient::datapoint(const std::string &id, const statint_t num) {
    openframe::scoped_lock slock(&_descrips_l);
    descrip_itr itr = _descrips.find(id);
    if ( itr == _descrips.end() ) throw StatsClientNoSuchId_Exception();
    if (!itr->second->is_data_type(dataTypeInt) ) throw StatsClientUnsupportedDataType_Exception();
    Data<statint_t> *data = dynamic_cast< Data<statint_t> *>( itr->second );
    data->point(num);
    return true;
  } // datapoint

  bool StatsClient::datapoint_float(const std::string &id, const statfloat_t num) {
    openframe::scoped_lock slock(&_descrips_l);
    descrip_itr itr = _descrips.find(id);
    if ( itr == _descrips.end() ) throw StatsClientNoSuchId_Exception();
    if (!itr->second->is_data_type(dataTypeFloat) ) throw StatsClientUnsupportedDataType_Exception();
    Data<statfloat_t> *data = dynamic_cast< Data<statfloat_t> *>( itr->second );
    data->point(num);
    return true;
  } // datapoint

  void StatsClient::onStats() {
    openframe::scoped_lock slock(&_descrips_l);
    for(descrip_itr itr = _descrips.begin(); itr != _descrips.end(); itr++ ) {
      Data_Abstract *data = itr->second;
      std::cout << "Sending stats " << data->toString()
                << ",sampling_freq=" << sampling_freq()
                << std::endl;
      data->reset();
    } // for
  } // Statsclient::onStats

  void *StatsClient::UpdateThread(void *arg) {
    StatsClient *scli = static_cast<StatsClient *>( arg );
    time_t sampling_freq = scli->sampling_freq();
    time_t next_sampling = time(NULL) + sampling_freq;

    while( !scli->is_done() ) {
      bool sample_now = next_sampling < time(NULL);
      if (!sample_now) {
        sleep(1);
        continue;
      } // if

      // send samples
      scli->onStats();
      next_sampling = time(NULL) + sampling_freq;
    } // while
    return NULL;
  } // StatsClient::UpdateThread

  std::ostream &operator<<(std::ostream &ss, const graphTypeEnum type) {
    switch(type) {
      case graphTypeCounter:
        ss << "counter";
        break;
      case graphTypeGauge:
        ss << "gauge";
        break;
      default:
        ss << "unknown";
        break;
    } // switch
    return ss;
  } // operator<<

  std::ostream &operator<<(std::ostream &ss, const dataTypeEnum type) {
    switch(type) {
      case dataTypeInt:
        ss << "integer";
        break;
      case dataTypeFloat:
        ss << "float";
        break;
      default:
        ss << "unknown";
        break;
    } // switch
    return ss;
  } // operator<<

  std::ostream &operator<<(std::ostream &ss, const useTypeEnum type) {
    switch(type) {
      case useTypeSum:
        ss << "sum";
        break;
      case useTypeMean:
        ss << "mean";
        break;
      case useTypeRate:
        ss << "rate";
        break;
      default:
        ss << "unknown";
        break;
    } // switch
    return ss;
  } // operator<<

} // namespace openstats


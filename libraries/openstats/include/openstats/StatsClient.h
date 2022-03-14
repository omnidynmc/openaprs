#ifndef __OPENSTATS_STATSCLIENT_H
#define __OPENSTATS_STATSCLIENT_H

#include <string>
#include <map>

#include <pthread.h>

#include <openframe/openframe.h>

#include "constants.h"

namespace openstats {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class Data_Abstract;
  class StatsClient : public openframe::Refcount {
    public:
      typedef std::map<std::string, Data_Abstract *> descrip_t;
      typedef descrip_t::iterator descrip_itr;
      typedef descrip_t::const_iterator descrip_citr;
      typedef descrip_t::size_type descrip_st;

      StatsClient();
      StatsClient(
        const time_t sampling_freq
      );
      virtual ~StatsClient();

      virtual StatsClient &describe_stat(const std::string &id,
                                         const std::string &label,
                                         const graphTypeEnum graph_type=graphTypeGauge,
                                         const dataTypeEnum data_type=dataTypeInt,
                                         const useTypeEnum use_type=useTypeMean);
      virtual StatsClient &destroy_stat(const std::string &id);

      virtual bool datapoint(const std::string &id, const statint_t value=1);
      virtual bool datapoint_float(const std::string &id, const statfloat_t value=1.0);

      virtual void onStats();

      inline time_t sampling_freq() const { return _sampling_freq; }

//      virtual StatsClient &start();
      virtual void stop();

      inline void done() {
        openframe::scoped_lock slock(&_done_l);
        _done = true;
      } // done
      inline bool is_done() {
        openframe::scoped_lock slock(&_done_l);
        return _done;
      } // is_done

      static void *UpdateThread(void *);
    protected:
      virtual void initializeThreads();
      virtual void deinitializeThreads();

      descrip_t _descrips;
      openframe::OFLock _descrips_l;

    private:
      time_t _sampling_freq;
      bool _thread;

      pthread_t _thread_tid;

      bool _done;
      openframe::OFLock _done_l;
  }; // StatsClient

  std::ostream &operator<<(std::ostream &ss, const graphTypeEnum type);
  std::ostream &operator<<(std::ostream &ss, const dataTypeEnum type);
  std::ostream &operator<<(std::ostream &ss, const useTypeEnum type);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openstats
#endif

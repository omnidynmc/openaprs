#ifndef __OPENSTATS_CLASS_DATA_H
#define __OPENSTATS_CLASS_DATA_H

#include <string>
#include <sstream>

#include <math.h>

#include <openframe/openframe.h>
#include <openframe/OpenFrame_Abstract.h>

#include <openstats/Datapoint.h>
#include "constants.h"

namespace openstats {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#ifndef MAX
#define MAX(a, b)	(a > b ? a : b)
#endif

#ifndef MIN
#define MIN(a, b)	(a < b ? a : b)
#endif

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class Data_Abstract : public openframe::Refcount {
    public:
      typedef unsigned long int count_t;
      Data_Abstract(const std::string &id,
                    const std::string &label,
                    const graphTypeEnum graph_type,
                    const dataTypeEnum data_type,
                    const useTypeEnum use_type=useTypeMean)
                   : _id(id),
                     _label(label),
                     _count(0),
                     _graph_type(graph_type),
                     _data_type(data_type),
                     _use_type(use_type),
                     _created( time(NULL) ) { }
      Data_Abstract(Data_Abstract &da) : _id(da._id),
                                         _label(da._label),
                                         _count(da._count),
                                         _graph_type(da._graph_type),
                                         _data_type(da._data_type),
                                         _use_type(da._use_type),
                                         _created( time(NULL) ) { }

      inline std::string id() const { return _id; }
      inline std::string label() const { return _label; }
      inline count_t count() const { return _count; }
      inline count_t inc() { _count++; return _count; }
      inline count_t inc(const count_t by) { _count += by; return _count; }
      inline graphTypeEnum graph_type() const { return _graph_type; }
      inline bool is_graph_type(const graphTypeEnum istype) const { return _graph_type == istype; }
      inline dataTypeEnum data_type() const { return _data_type; }
      inline bool is_data_type(const dataTypeEnum istype) const { return _data_type == istype; }
      inline useTypeEnum use_type() const { return _use_type; }
      inline bool is_use_type(const useTypeEnum istype) const { return _use_type == istype; }
      inline time_t since() const { return time(NULL) - _created; }
      virtual std::string toString() = 0;
      virtual std::string toJson() = 0;
      virtual inline void reset() {
        _count = 0;
        _created = time(NULL);
      } // reset

    protected:

    private:
      std::string _id;
      std::string _label;
      count_t _count;
      graphTypeEnum _graph_type;
      dataTypeEnum _data_type;
      useTypeEnum _use_type;
      time_t _created;
  }; // Data

  template <class T>
  class Data : public Data_Abstract {
    public:
      typedef Data_Abstract super;
      Data(const std::string &id,
          const std::string &label,
          const graphTypeEnum graph_type,
          const dataTypeEnum data_type,
          const useTypeEnum use_type)
        : super(id, label, graph_type, data_type, use_type),
          _value(0),
          _sum(0),
          _mean(0),
          _rate(0),
          _max(0),
          _min(0),
          _maxdex(0),
          _mindex(0),
          _pseudo_variance(0),
          _variance(0),
          _standard_deviation(0) { }

      void point(T value) {
        if ( is_graph_type(graphTypeCounter) ) {
          _value += value;
          inc();
          return;
        } // if

        statfloat_t oldmean = _mean;
        _sum += value;
        inc();
        if (value >= _max || count() == 1) {
          _max = value;
          _maxdex = count() - 1;
        } // if

        if (value <= _min || count() == 1) {
          _min = value;
          _mindex = count() - 1;
        } // if

        _sample_range = _max - _min;
        _mean += (value - oldmean) / count();
        _pseudo_variance += (statfloat_t(value) - oldmean) * (statfloat_t(value) - _mean);
        _rate = statfloat_t(_sum) / since();

        switch( use_type() ) {
          case useTypeSum:
            _value = _sum;
            break;
          case useTypeRate:
            _value = _rate;
            break;
          case useTypeMean:
            _value = _mean;
            break;
          default:
            assert(false);		// bug
            break;
        } // switch

        if (count() > 1) {
          _variance = _pseudo_variance / (count() - 1);
          _standard_deviation = sqrt(_variance);
        } // if
      } // point
      inline T sum() const { return _sum; }
      inline T max() const { return _max; }
      inline T min() const { return _min; }
      inline count_t maxdex() { return _maxdex; }
      inline count_t mindex() { return _mindex; }
      inline statfloat_t mean() { return _mean; }
      inline statfloat_t value() { return _value; }
      inline statfloat_t rate() { return _rate; }
      inline statfloat_t pseudo_variance() { return _pseudo_variance; }
      inline statfloat_t variance() { return _variance; }
      inline statfloat_t standard_deviation() { return _standard_deviation; }
      inline T sample_range() { return _sample_range; }
      std::string toString() {
        std::stringstream s;
        s << "id=" << id()
          << ",label=" << label()
          << ",graph_type=" << graph_type()
          << ",data_type=" << data_type()
          << ",timestamp=" << time(NULL)
          << ",count=" << count()
          << ",use=" << use_type();

        if (is_graph_type(graphTypeGauge)) {
          s << ",sum=" << sum()
            << ",max=" << max()
            << ",min=" << min()
            << ",maxdex=" << maxdex()
            << ",mindex=" << mindex()
            << ",pseudo_variance=" << pseudo_variance()
            << ",variance=" << variance()
            << ",standard_deviation=" << standard_deviation()
            << ",sample_range=" << sample_range()
            << ",mean=" << mean()
            << ",rate=" << rate();
        } // if

        s << ",value=" << value();

        return s.str();
      } // toString

      std::string toJson() {
        std::stringstream s;
        s << "{ \"id\":\"" << id() << "\"," << std::endl
          << "  \"label\":\"" << label() << "\"," << std::endl
          << "  \"graph_type\":\"" << graph_type() << "\"," << std::endl
          << "  \"data_type\":\"" << data_type()<< "\"," << std::endl
          << "  \"timestamp\":\"" << time(NULL) << "\"," <<std::endl
          << "  \"count\":\"" << count() << "\"," <<std::endl
          << "  \"use\":\"" << use_type() << "\"," <<std::endl;

        if (is_graph_type(graphTypeGauge)) {
          s << "  \"sum\":\"" << sum() << "\"," <<std::endl
            << "  \"max\":\"" << max() << "\"," <<std::endl
            << "  \"min\":\"" << min() << "\"," <<std::endl
            << "  \"maxdex\":\"" << maxdex() << "\"," <<std::endl
            << "  \"mindex\":\"" << mindex() << "\"," <<std::endl
            << "  \"pseudo_variance\":\"" << pseudo_variance() << "\"," <<std::endl
            << "  \"variance\":\"" << variance() << "\"," <<std::endl
            << "  \"standard_deviation\":\"" << standard_deviation() << "\"," <<std::endl
            << "  \"sample_range\":\"" << sample_range() << "\"," <<std::endl
            << "  \"mean\":\"" << mean() << "\"," <<std::endl
            << "  \"rate\":\"" << rate() << "\"," <<std::endl;
        } // if

        s << "  \"value\":\"" << value() << "\"}" <<std::endl;

        return s.str();
      } // toJson

      virtual inline void reset() {
        super::reset();
        _value = 0;
        _sum = 0;
        _mean = 0;
        _rate = 0;
        _max = 0;
        _min = 0;
        _maxdex = 0;
        _mindex = 0;
        _pseudo_variance = 0;
        _variance = 0;
        _standard_deviation = 0;
        _sample_range = 0;
      } // reset

    protected:
      T _value;
      T _sum;
      statfloat_t _mean;
      statfloat_t _rate;
      T _max;
      T _min;
      count_t _maxdex;
      count_t _mindex;
      statfloat_t _pseudo_variance;
      statfloat_t _variance;
      statfloat_t _standard_deviation;
      statfloat_t _sample_range;
    private:
  }; // class Data

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openstats
#endif

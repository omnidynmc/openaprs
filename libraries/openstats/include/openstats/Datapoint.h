#ifndef __OPENSTATS_DATAPOINT_H
#define __OPENSTATS_DATAPOINT_H

#include <string>

#include <openframe/openframe.h>
#include <openframe/OpenFrame_Abstract.h>
#include <openframe/stringify.h>

namespace openstats {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class Datapoint {
    public:
      enum dataType {
        DATAPOINT_DOUBLE,
        DATAPOINT_INT,
        DATAPOINT_UINT,
        DATAPOINT_LONG,
        DATAPOINT_ULONG
      }; // dataType

      typedef union {
        double v_double;
        int v_int;
        unsigned int v_uint;
        long int v_long;
        unsigned long int v_ulong;
      } dataValue_t;

      const dataType type() const { return _type; }

      const double doubly() {
        assert(_type == DATAPOINT_DOUBLE);  // bug
        return _value.v_double;
      } // doubly

      const int inty() {
        assert(_type == DATAPOINT_INT);  // bug
        return _value.v_int;
      } // inty

      const double uinty() {
        assert(_type == DATAPOINT_UINT);  // bug
        return _value.v_uint;
      } // doubly

      const long int longy() {
        assert(_type == DATAPOINT_LONG);  // bug
        return _value.v_long;
      } // longy

      const unsigned long int ulongy() {
        assert(_type == DATAPOINT_ULONG);  // bug
        return _value.v_ulong;
      } // ulongy

    const std::string str() {
      std::string ret;

      switch(_type) {
        case DATAPOINT_DOUBLE:
          ret = openframe::stringify<double>(_value.v_double);
          break;
        case DATAPOINT_INT:
          ret = openframe::stringify<int>(_value.v_int);
          break;
        case DATAPOINT_UINT:
          ret = openframe::stringify<unsigned int>(_value.v_uint);
          break;
        case DATAPOINT_LONG:
          ret = openframe::stringify<long int>(_value.v_long);
          break;
        case DATAPOINT_ULONG:
          ret = openframe::stringify<unsigned long int>(_value.v_ulong);
          break;
        default:
          assert(false);                // bug
          break;
      } // switch

      return ret;
    } // str

    protected:

    private:
      dataType _type;
      dataValue_t _value;
  }; // Datapoint

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openstats
#endif

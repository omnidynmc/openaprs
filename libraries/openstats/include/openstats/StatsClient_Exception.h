#ifndef __OPENSTATS_STATSCLIENT_EXCEPTION_H
#define __OPENSTATS_STATSCLIENT_EXCEPTION_H

#include <string>
#include <map>

#include <openframe/openframe.h>

#include "constants.h"

namespace openstats {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class StatsClient_Exception : public openframe::OpenFrame_Exception {
    public:
      StatsClient_Exception(const std::string message) throw() : openframe::OpenFrame_Exception(message) { };
  }; // class StatsClient_Exception

  class StatsClientNoSuchId_Exception : public StatsClient_Exception {
    public:
      typedef StatsClient_Exception super;
      StatsClientNoSuchId_Exception() throw() : super("no such id") { };
  }; // class StatsClientNoSuchId_Exception

  class StatsClientTypeNotCounter_Exception : public StatsClient_Exception {
    public:
      typedef StatsClient_Exception super;
      StatsClientTypeNotCounter_Exception() throw() : super("type not counter") { };
  }; // class StatsClientTypeNotCounter_Exception

  class StatsClientTypeNotGauge_Exception : public StatsClient_Exception {
    public:
      typedef StatsClient_Exception super;
      StatsClientTypeNotGauge_Exception() throw() : super("type not gauge") { };
  }; // class StatsClientTypeNotGauge_Exception

  class StatsClientUnsupportedDataType_Exception : public StatsClient_Exception {
    public:
      typedef StatsClient_Exception super;
      StatsClientUnsupportedDataType_Exception() throw() : super("unsupported data type") { };
  }; // class StatsClientUnsupportedDataType_Exception

  class StatsClientLabelAlreadyExists_Exception : public StatsClient_Exception {
    public:
      typedef StatsClient_Exception super;
      StatsClientLabelAlreadyExists_Exception() throw() : super("label already exists") { };
  }; // class StatsClientLabelAlreadyExists_Exception


/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openstats
#endif

#ifndef __OPENFRAME_APP_EXCEPTION_H
#define __OPENFRAME_APP_EXCEPTION_H

#include <string>

#include <openframe/openframe.h>

#include "App_Log.h"
#include "Command.h"
#include "OpenTest_Abstract.h"

namespace opensmsc {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
  class App_Exception : public OpenFrame_Exception {
    public:
      App_Exception(const std::string message) throw() : OpenTest_Exception(message) { }
  }; // class App_Exception

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openframe
#endif

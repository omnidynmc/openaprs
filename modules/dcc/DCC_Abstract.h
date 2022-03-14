/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, mySQL APRS Injector                                        **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Daniel Robert Karrels                             **
 **                    Dynamic Networking Solutions                      **
 **                                                                      **
 ** This program is free software; you can redistribute it and/or modify **
 ** it under the terms of the GNU General Public License as published by **
 ** the Free Software Foundation; either version 1, or (at your option)  **
 ** any later version.                                                   **
 **                                                                      **
 ** This program is distributed in the hope that it will be useful,      **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of       **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        **
 ** GNU General Public License for more details.                         **
 **                                                                      **
 ** You should have received a copy of the GNU General Public License    **
 ** along with this program; if not, write to the Free Software          **
 ** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            **
 **************************************************************************
 $Id: DCC.h,v 1.8 2003/09/04 00:22:00 omni Exp $
 **************************************************************************/

#ifndef __MODULE_DCC_DCCABSTRACT_H
#define __MODULE_DCC_DCCABSTRACT_H

#include <exception>
#include <string>

#include <stdio.h>
#include <stdarg.h>

#include "DCC_Log.h"
#include "OpenAPRS_Abstract.h"

namespace dcc {

using openaprs::OpenAPRS_Abstract;
using namespace std;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class DCC_Abstract : public OpenAPRS_Abstract {
  public:
    DCC_Abstract() { logger( new DCC_Log("") ); }
    virtual ~DCC_Abstract() { }

    void logger(DCC_Log *logger) {
      if (_logger != NULL)
        delete _logger;

      _logger = _dcclogger = logger;
    } // logger

  protected:
    void _dcclogf(const char *writeFormat, ...) {
      char writeBuffer[OPENAPRSLOG_MAXBUF + 1] = {0};
      va_list va;

      va_start(va, writeFormat);
      vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
      va_end(va);

      _dcclog(writeBuffer);
    } // dcclogf

    virtual void _dcclog(const string &message) {
      _dcclogger->dcclog(message);
    } // _log

    DCC_Log *_dcclogger;
  private:
}; // class DCC_Abstract

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

extern "C" {
} // extern

}
#endif

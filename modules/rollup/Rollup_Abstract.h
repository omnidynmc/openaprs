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

#ifndef __MODULE_ROLLUP_ABSTRACT_H
#define __MODULE_ROLLUP_ABSTRACT_H

#include <exception>
#include <string>

#include <stdio.h>
#include <stdarg.h>

#include "Rollup_Log.h"
#include "OpenAPRS_Abstract.h"

namespace rollup {

using openaprs::OpenAPRS_Abstract;
using namespace std;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class Rollup_Abstract : public OpenAPRS_Abstract {
  public:
    Rollup_Abstract() { logger( new Rollup_Log("") ); }
    virtual ~Rollup_Abstract() { }

    void logger(Rollup_Log *logger) {
      if (_logger != NULL)
        delete _logger;

      _logger = _rolluplogger = logger;
    } // logger

  protected:
    void _rolluplogf(const char *writeFormat, ...) {
      char writeBuffer[OPENAPRSLOG_MAXBUF + 1] = {0};
      va_list va;

      va_start(va, writeFormat);
      vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
      va_end(va);

      _rolluplog(writeBuffer);
    } // rolluplogf

    virtual void _rolluplog(const string &message) {
      _rolluplogger->rolluplog(message);
    } // _log

    Rollup_Log *_rolluplogger;
  private:
}; // class Rollup_Abstract

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

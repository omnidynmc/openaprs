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

#ifndef __OPENAPRS_OPENAPRSABSTRACT_H
#define __OPENAPRS_OPENAPRSABSTRACT_H

#include <exception>
#include <string>

#include <stdio.h>
#include <stdarg.h>

#include "OpenAPRS_Log.h"

namespace openaprs {
  using std::exception;
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/
class OpenAPRSAbstract_Exception : public std::exception {
  public:
    OpenAPRSAbstract_Exception(const string message) throw() {
      if (!message.length())
        _message = "An unknown message exception occured.";
      else
        _message = message;
    } // OpenAbstract_Exception

    virtual ~OpenAPRSAbstract_Exception() throw() { }
    virtual const char *what() const throw() { return _message.c_str(); }

    const char *message() const throw() { return _message.c_str(); }

  private:
    string _message;                    // Message of the exception error.
}; // class OpenAPRSAbstract_Exception

class OpenAPRS_Abstract {
  public:
    OpenAPRS_Abstract() { _logger = new OpenAPRS_Log(""); }
    virtual ~OpenAPRS_Abstract() {
      if (_logger != NULL)
        delete _logger;
    } // OpenAPRS_Abstract

    void logger(OpenAPRS_Log *logger) {
      if (_logger != NULL)
        delete _logger;
      _logger = logger;
    } // logger

  protected:
    void _logf(const char *writeFormat, ...) {
      char writeBuffer[OPENAPRSLOG_MAXBUF + 1] = {0};
      va_list va;

      va_start(va, writeFormat);
      vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
      va_end(va);

      _log(writeBuffer);
    } // log

    void _slogf(const string &where, const char *writeFormat, ...) {
      char writeBuffer[OPENAPRSLOG_MAXBUF + 1] = {0};
      va_list va;

      va_start(va, writeFormat);
      vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
      va_end(va);

      _slog(where, writeBuffer);
    } // log

    void _debugf(const char *writeFormat, ...) {
      char writeBuffer[OPENAPRSLOG_MAXBUF + 1] = {0};
      va_list va;

      va_start(va, writeFormat);
      vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
      va_end(va);

      _debug(writeBuffer);
    } // _debugf

    void _consolef(const char *writeFormat, ...) {
      char writeBuffer[OPENAPRSLOG_MAXBUF + 1] = {0};
      va_list va;

      va_start(va, writeFormat);
      vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
      va_end(va);

      _console(writeBuffer);
    } // _consolef

    virtual void _log(const string &message) {
      _logger->log(message);
    } // _log

    virtual void _debug(const string &message) {
      _logger->debug(message);
    } // _debug

    virtual void _console(const string &message) {
      _logger->console(message);
    } // _console

    virtual void _slog(const string &where, const string &message) {
      _logger->slog(where, message);
    } // _console

    OpenAPRS_Log *_logger;			// Pointer to set logger.
  private:

}; // class OpenAPRS_Abstract

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

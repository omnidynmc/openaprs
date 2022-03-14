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

#ifndef __MODULE_APNSABSTRACT_H
#define __MODULE_APNSABSTRACT_H

#include <exception>
#include <string>

#include <stdio.h>

#include "ApnsLog.h"

namespace apns {

using namespace std;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class ApnsAbstract_Exception : public std::exception {
  public:
    ApnsAbstract_Exception(const string message) throw() {
      if (!message.length())
        _message = "An unknown APNS Message exception occured.";
      else
        _message = message;
    } // ApnsAbstract_Exception

    virtual ~ApnsAbstract_Exception() throw() { }
    virtual const char *what() const throw() { return _message.c_str(); }

    const char *message() const throw() { return _message.c_str(); }

  private:
    string _message;                    // Message of the exception error.
}; // class ApnsAbstract_Exception

class ApnsAbstract {
  public:
    ApnsAbstract() { _logger = new ApnsLog(); }
    virtual ~ApnsAbstract() { 
      if (_logger != NULL)
        delete _logger;
    } // ApnsAbstract

    void logger(ApnsLog *logger) {
      if (_logger != NULL)
        delete _logger;
      _logger = logger;
    } // logger

    const bool testDeviceTokenTools() { return _testDeviceTokenTools(); }
    const string generateRandomDeviceToken();
    const string char2hex(const char ch) { return _char2hex(ch); }

  protected:
    void _logf(const char *writeFormat, ...) {
      char writeBuffer[APNSLOG_MAXBUF + 1] = {0};
      va_list va;

      va_start(va, writeFormat);
      vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
      va_end(va);

      _log(writeBuffer);
    } // log

    void _debugf(const char *writeFormat, ...) {
      char writeBuffer[APNSLOG_MAXBUF + 1] = {0};
      va_list va;

      va_start(va, writeFormat);
      vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
      va_end(va);

      _debug(writeBuffer);
    } // _debugf

    virtual void _log(const string &message) {
      _logger->log(message);
    } // _log

    virtual void _debug(const string &message) {
      _logger->debug(message);
    } // _log

    void _deviceTokenToBinary(char *, const string &, const size_t);
    const string _binaryToDeviceToken(const char *, const size_t);
    const string _char2hex(const char);
    const string _safeBinaryOutput(const char *, const size_t);
    const bool _testDeviceTokenTools();

  private:

    ApnsLog *_logger;			// Pointer to set logger.
}; // class ApnsLog

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

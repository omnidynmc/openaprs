/**************************************************************************
 ** OpenFrame Library                                                    **
 **************************************************************************
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
 **************************************************************************/

#ifndef LIBOPENFRAME_OPENFRAME_ABSTRACT_H
#define LIBOPENFRAME_OPENFRAME_ABSTRACT_H

#include <string>
#include <cassert>

#include <stdio.h>
#include <stdarg.h>

#include "Logger.h"
#include "OpenFrame_Log.h"
#include "serializable.h"

namespace openframe {

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

#define OPENFRAMELOG_MAXBUF	1024

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class OpenFrame_Exception : public std::exception {
    public:
      OpenFrame_Exception(const std::string message) throw() {
        if (!message.length())
          _message = "An unknown message exception occured.";
        else
          _message = message;
      } // OpenAbstract_Exception

      virtual ~OpenFrame_Exception() throw() { }
      virtual const char *what() const throw() { return _message.c_str(); }

      const char *message() const throw() { return _message.c_str(); }

    private:
      std::string _message;                    // Message of the exception error.
  }; // class OpenFrame_Exception

  class OpenFrame_Abstract : public serializable {
    public:
      OpenFrame_Abstract() {
        _logger = new OpenFrame_Log("Abstract");
        _elogger = new Logger;
      } // OpenFrame_Abstract
      virtual ~OpenFrame_Abstract() {
        if (_logger != NULL) _logger->release();
        if (_elogger != NULL) _elogger->release();
      } // OpenFrame_Abstract

      virtual void serialize(std::ostream &os) const {
        os << "OpenFrame_Abstract" << std::endl;
      } // serialize

      void logger(OpenFrame_Log *logger) {
        if (_logger != NULL) _logger->release();
        _logger->retain();
        _logger = logger;
      } // logger

      void elogger(Logger *elogger, const string &elog_name) {
        if (_elogger != NULL) _elogger->release();
        elogger->retain();
        _elog_name = elog_name;
        _elogger = elogger;
      } // elogger
      Logger *elogger() const { return _elogger; }
      const string elog_name() const { return _elog_name; }

      logstream &log(const loglevel::Level lvl) {
        assert(_elogger != NULL);
        dynamic_cast<Logger &>(*_elogger)[_elog_name] << lvl;
        return dynamic_cast<Logger &>(*_elogger)[_elog_name];
      } // elog

    protected:
      void _logf(const char *writeFormat, ...) {
        char writeBuffer[OPENFRAMELOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _log(writeBuffer);
      } // log

      void _debugf(const char *writeFormat, ...) {
        char writeBuffer[OPENFRAMELOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _debug(writeBuffer);
      } // _debugf

      void _infof(const char *writeFormat, ...) {
        char writeBuffer[OPENFRAMELOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _info(writeBuffer);
      } // _infof

      void _noticef(const char *writeFormat, ...) {
        char writeBuffer[OPENFRAMELOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _notice(writeBuffer);
      } // _noticef

      void _warnf(const char *writeFormat, ...) {
        char writeBuffer[OPENFRAMELOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _warn(writeBuffer);
      } // _warnf

      void _errorf(const char *writeFormat, ...) {
        char writeBuffer[OPENFRAMELOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _error(writeBuffer);
      } // _errorf

      void _alertf(const char *writeFormat, ...) {
        char writeBuffer[OPENFRAMELOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _alert(writeBuffer);
      } // _alertf

      void _critf(const char *writeFormat, ...) {
        char writeBuffer[OPENFRAMELOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _crit(writeBuffer);
      } // _critf

      void _emergf(const char *writeFormat, ...) {
        char writeBuffer[OPENFRAMELOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _emerg(writeBuffer);
      } // _emergf

      void _consolef(const char *writeFormat, ...) {
        char writeBuffer[OPENFRAMELOG_MAXBUF + 1] = {0};
        va_list va;

        va_start(va, writeFormat);
        vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
        va_end(va);

        _console(writeBuffer);
      } // _consolef

      virtual void _log(const std::string &message) {
        _logger->log(message);
      } // _log

      virtual void _debug(const std::string &message) {
        _logger->debug(message);
      } // _debug

      virtual void _info(const std::string &message) {
        _logger->info(message);
      } // _info

      virtual void _notice(const std::string &message) {
        _logger->notice(message);
      } // _notice

      virtual void _warn(const std::string &message) {
        _logger->warn(message);
      } // _warn

      virtual void _error(const std::string &message) {
        _logger->error(message);
      } // _error

      virtual void _alert(const std::string &message) {
        _logger->alert(message);
      } // _alert

      virtual void _crit(const std::string &message) {
        _logger->crit(message);
      } // _critical

      virtual void _emerg(const std::string &message) {
        _logger->emerg(message);
      } // _emerg

      virtual void _console(const std::string &message) {
        _logger->console(message);
      } // _console

      // ### Variables ###
      OpenFrame_Log *_logger;

    private:
      Logger *_elogger;
      std::string _elog_name;
  }; // OpenFrame_Abstract

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/
} // namespace openframe

#endif

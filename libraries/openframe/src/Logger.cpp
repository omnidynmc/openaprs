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

#include <map>
#include <iostream>
#include <iomanip>

#include <openframe/Logger.h>

namespace openframe {

  Logger::Logger() : logstream() {
  } // Logger::Logger

  Logger::~Logger() {
    for(logs_itr itr = _logs.begin(); itr != _logs.end(); itr++)
      delete itr->second;
  } // Logger::~Logger

  const bool Logger::open(const std::string &name, const std::string &path) {
    logs_itr itr = _logs.find(name);
    if (itr != _logs.end()) return false;

    filelogstreambuf *flb = new filelogstreambuf(path);
    if (!flb) return false;
    logstream *fl = new logstream(flb);
    _logs.insert( make_pair(name, fl) );
    _logcache.insert( make_pair(name, path) );
    return true;
  } // Logger::open

  const bool Logger::close(const std::string &name) {
    logs_itr itr = _logs.find(name);
    if (itr == _logs.end()) return false;
    logstream *ls = itr->second;
    delete ls;
    _logs.erase(name);
    _logcache.erase(name);
    return true;
  } // Logger::close

  void Logger::hup() {
    logcache_t lc = _logcache;
    for(logcache_itr itr = lc.begin(); itr != lc.end(); itr++) {
      std::string name = itr->first;
      std::string path = itr->second;

      // save the log level
      loglevel::Level lvl = _logs[name]->getLevel();

      close(name);
      bool ok = open(name, path);

      // restore the log level
      level(name, lvl);
    } // for
  } // Logger::hup

  const bool Logger::level(const std::string &name, loglevel::Level lvl) {
    logs_itr itr = _logs.find(name);
    if (itr == _logs.end()) return false;
    logstream *ls = itr->second;
    ls->resetLevel(lvl);
    return true;
  } // Logger::level

  logstream &Logger::operator[](const std::string &name) {
    logs_itr itr = _logs.find(name);
    if (itr == _logs.end()) return *this;
    logstream *ls = itr->second;
    return dynamic_cast<logstream &>(*ls);
  } // Logger::operator[]

  logstream &Logger::at(const std::string &name) {
    logs_itr itr = _logs.find(name);
    if (itr == _logs.end()) return *this;
    logstream *ls = itr->second;
    return dynamic_cast<logstream &>(*ls);
  } // Logger::at

  Logger &operator<<(Logger &log, loglevel::Level lvl) {
      dynamic_cast<logstream&>(log) << std::right << std::setw(6) << lvl << ": ";
      return log;
  } // Logger operator<<
} // namespace openframe

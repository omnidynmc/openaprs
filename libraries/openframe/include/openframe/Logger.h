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

#ifndef __LIBOPENFRAME_LOGGER_H
#define __LIBOPENFRAME_LOGGER_H

#include <map>
#include <string>

#include <openframe/Refcount.h>
#include <openframe/OFLock.h>

#include "openframe/logging/logstream.h"
//to be able to access cnull
#include "openframe/logging/nullstream.h"


namespace openframe {

/***
 *
 * \class NewLogger
 * \brief NewLogger manages the Logging outputs for the application.
 *
 */
  class Logger : public Refcount,
                 public OFLock,
                 public logstream {
    public:
      Logger();
      ~Logger();

      typedef std::map<std::string, logstream *> logs_t;
      typedef logs_t::iterator logs_itr;
      typedef logs_t::const_iterator logs_citr;
      typedef logs_t::size_type logs_s;

      typedef std::map<std::string, std::string> logcache_t;
      typedef logcache_t::iterator logcache_itr;
      typedef logcache_t::const_iterator logcache_citr;
      typedef logcache_t::size_type logcache_st;

      const bool open(const std::string &name, const std::string &filename);
      const bool close(const std::string &name);
      const bool level(const std::string &name, loglevel::Level lvl);
      void hup();

      friend Logger &operator<<(Logger &log, loglevel::Level lvl);

      logstream &operator[](const std::string &name);
      logstream &at(const std::string &name);

    protected:
    private:
      // ### Private Variables ###
      logs_t _logs;
      logcache_t _logcache;
  };

#define LOG(a, b) \
{ elogger()->Lock(); \
log(a) b; \
elogger()->Unlock(); }

} // namespace openframe
#endif

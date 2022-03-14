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

#ifndef __OPENFRAME_LOGSTREAM_HH
#define __OPENFRAME_LOGSTREAM_HH

#include "openframe/logging/nullstream.h"
#include "openframe/logging/logstreambuf.h"
#include "openframe/logging/loglevel.h"

//to get string << operator for logstream
#include <string>

namespace openframe {
  /**
   * \class logstream
   * \brief logstream provide filtering based on loglevel.
   *
   */
  class logstream : public std::ostringstream {
      //streambuff
      logstreambuf* pvm_lsb;

      //level of hte log stream.
      //anything less important than this level is ignored
      loglevel::Level loglvl;

    public :
      //default constructor (clog output logstreambuf )
      logstream();
      explicit logstream(logstreambuf* lsb);
      ~logstream();

      //to manage prefix
      void resetprefix(const std::string & newprefix = "");
      const std::string &getprefix() const;

      //to use logstream as streamthrough
      friend std::ostream& operator<<(std::ostream& o, logstream& l) {
        return o << l.rdbuf();
      } // operator<<

      logstreambuf *rdbuf() const {
        return pvm_lsb;
      } // rdbuf

      //set loglevel. Messages logged here will have at least this level
      void resetLevel(loglevel::Level l=loglevel::LogInfo) {
        loglvl = l;
      } // resetLevel
      loglevel::Level getLevel() {
        return loglvl;
      } // getLevel

/*
      logstream &operator<<(logstream& ( *pf )(logstream &)) {
std::cout << "intercept" << std::endl;
        return *this;
      } // operator<<
*/

      //manipulator to set *messages's* level
      friend logstream& operator<<(logstream &o, loglevel::Level lvl);
      logstream& level(loglevel::Level l);

  }; // class logstream
} // namespace openframe
#endif // __OPENFRAME_LOGSTREAM_H



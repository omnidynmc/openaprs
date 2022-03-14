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

#include <iomanip>

#include <openframe/logging/logstream.h>

namespace openframe {

  logstream::logstream() : std::ostringstream() {
    //to hook up to usual stream design
    pvm_lsb = new clogstreambuf();
    this->init(pvm_lsb);

    //setup default flags
    this->flags ( std::ios::left | std::ios::boolalpha );

    //setup default prefix
    resetprefix();
    resetLevel();
  } // logstream::logstream

  logstream::logstream(logstreambuf *lsb) : std::ostringstream() {
    //to hook up to usual stream design
    pvm_lsb = lsb;
    this->init(pvm_lsb);

    //setup default flags
    this->flags ( std::ios::left | std::ios::boolalpha );

    //setup default prefix
    resetprefix();
  } // logstream::logstream

  logstream::~logstream() {
    if (pvm_lsb != NULL) delete pvm_lsb;
  } // logstream::logstream

  //to manage prefix
  void logstream::resetprefix(const std::string& newprefix) {
    rdbuf()->resetprefix(newprefix);
  } // logstream::resetprefix

  const std::string &logstream::getprefix() const {
    return rdbuf()->getprefix();
  } // logstream::getprefix

  logstream &operator<<(logstream &ls, loglevel::Level lvl) {
    bool ok = ls.getLevel() >= lvl;
//std::cout << "LOGLEVEL(" << ls.getLevel() << ") lvl(" << lvl << ") ok(" << ok << ")" << std::endl;
    if (ok) {
      ls.rdbuf()->filterin(lvl);
      //dynamic casting to call the ostream << ( ostream, loglevel) operator
//      dynamic_cast<logstream&>(ls) << std::right << std::setw(6) << lvl << ": ";
    } // if
    else {
      ls.rdbuf()->filterout();
    } // else
    return ls;
  } // operator<<

  logstream& logstream::level(loglevel::Level l) {
    *this << l;
    return *this;
  } // logstream::level
} // namespace openframe



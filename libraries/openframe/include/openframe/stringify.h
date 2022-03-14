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

#ifndef __LIBOPENFRAME_STRINGIFY_H
#define __LIBOPENFRAME_STRINGIFY_H

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <limits>

namespace openframe {
  using std::cout;
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  template<typename T> inline std::string stringify(T const& x) {
    std::ostringstream out;
    out << x;
    return out.str();
  }

  template<> inline std::string stringify<bool>(bool const& x) {
    std::ostringstream out;
    out << std::boolalpha << x;
    return out.str();
  }

  template<> inline std::string stringify<double>(double const& x) {
    const int sigdigits = std::numeric_limits<double>::digits10;
    // or perhaps std::numeric_limits<double>::max_digits10 if that is available on your compiler
    std::ostringstream out;
    out << std::fixed << std::setprecision(sigdigits) << x;
    return out.str();
  }

  template<> inline std::string stringify<float>(float const& x) {
    const int sigdigits = std::numeric_limits<float>::digits10;
    // or perhaps std::numeric_limits<float>::max_digits10 if that is available on your compiler
    std::ostringstream out;
    out << std::setprecision(sigdigits) << x;
    return out.str();
  }

  template<> inline std::string stringify<long double>(const long double& x) {
    const int sigdigits = std::numeric_limits<long double>::digits10;
    // or perhaps std::numeric_limits<long_double>::max_digits10 if that is available on your compiler
    std::ostringstream out;
    out << std::fixed << std::setprecision(sigdigits) << x;
    return out.str();
  }


  template<typename T> inline std::string stringify(T const& x, const int &y) {
    std::ostringstream out;
    out << x;
    return out.str();
  }

  template<> inline std::string stringify<double>(double const& x, const int &sigdigits) {
    // or perhaps std::numeric_limits<double>::max_digits10 if that is available on your compiler
    std::ostringstream out;
    out << std::fixed << std::setprecision(sigdigits) << x;
    return out.str();
  }

  template<> inline std::string stringify<long double>(const long double& x, const int &sigdigits) {
    // or perhaps std::numeric_limits<long_double>::max_digits10 if that is available on your compiler
    std::ostringstream out;
    out << std::fixed << std::setprecision(sigdigits) << x;
    return out.str();
  }

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

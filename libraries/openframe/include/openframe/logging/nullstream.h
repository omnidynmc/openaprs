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

#ifndef __OPENFRAME_NULLOSTREAM_H
#define __OPENFRAME_NULLOSTREAM_H

#include "openframe/logging/nullstreambuf.h"

#include <ostream>

namespace openframe {
  namespace stdext {
    template <class cT, class traits = std::char_traits<cT> >
    class basic_onullstream: public std::basic_ostream<cT, traits> {
      public:
        basic_onullstream():
          std::basic_ios<cT, traits>(),
          std::basic_ostream<cT, traits>(0),
          m_sbuf() {
          this->init(&m_sbuf);
        } // template

      private:
        basic_nullstreambuf<cT, traits> m_sbuf;
    }; // class template

    typedef basic_onullstream<char> onullstream;
    typedef basic_onullstream<wchar_t> wonullstream;
  } //namespace stdext

  extern stdext::onullstream cnull;
  extern stdext::wonullstream wcnull;
} // namespace openframe
#endif // __OPENFRAME_NULLOSTREAM_H

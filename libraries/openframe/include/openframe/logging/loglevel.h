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

#ifndef __OPENFRAME_LOGLEVEL_H
#define __OPENFRAME_LOGLEVEL_H

//TODO
// we ned to keep an enum
// we just need to add proper operators around it
// http://www.edm2.com/0405/enumeration.html

#include <ostream>

namespace openframe {
  namespace loglevel {
    enum Level {
      LogEmerg	= 0,
      LogAlert	= 1,
      LogCrit	= 2,
      LogError	= 3,
      LogWarn	= 4,
      LogNotice	= 5,
      LogInfo	= 6,
      LogDebug	= 7
    }; // enum Level

    //beware ++ means more important that is opposite in integer value in enums
    //logleve can not be more important than FATAL (=0)
    inline void operator++(Level &lvl) {
      lvl = (lvl == LogEmerg) ? LogEmerg : Level(lvl-1);
    } // operator++
    inline void operator++(Level &lvl, int) {
      lvl = (lvl == LogEmerg) ? LogEmerg : Level(lvl-1);
    } // operator++
    //loglevel can be as uninteressant as we want.
    inline void operator--(Level &lvl) {
      lvl = Level(lvl+1);
    } // operator--
    inline void operator--(Level &lvl, int) {
      lvl = Level(lvl+1);
    } // operator--

    std::ostream& operator<<(std::ostream& os, const Level& lvl);
  } // namespace loglevel
} // namespace openframe



#endif

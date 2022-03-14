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

#include <openframe/logging/loglevel.h>

namespace openframe {
  namespace loglevel {
    std::ostream& operator<<(std::ostream &os, const Level &lvl) {
      switch (lvl) {
        case LogEmerg:
          os << "EMERG";
          break;
        case LogAlert:
          os << "ALERT";
          break;
        case LogCrit:
          os << "CRIT";
          break;
        case LogError:
          os << "ERROR";
          break;
        case LogWarn:
          os << "WARN";
          break;
        case LogNotice:
          os << "NOTICE";
          break;
        case LogInfo:
          os << "INFO";
          break;
        case LogDebug:
          os << "DEBUG";
          break;
      } // switch
      return os;
    } // operator<<
  } // namespace loglevel
} // namespace openframe

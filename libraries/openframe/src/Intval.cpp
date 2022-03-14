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

#include <openframe/config.h>

#include <ctime>

#include <openframe/Intval.h>

namespace openframe {

  /**************************************************************************
   ** Intval Class                                                         **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/
  Intval::Intval(const time_t seconds)
         : _interval(seconds),
           _last_at( time(NULL) ),
           _created_at( time(NULL) ) {
  } // Intval::Intval

  Intval::~Intval() {
  } // Intval::~Intval

  void Intval::set_interval(const time_t seconds) {
    _interval = seconds;
    _last_at = time(NULL);
  } // Intval::set_interval

  bool Intval::is_next() {
    if (_last_at > time(NULL) - _interval) return false;

    _last_at = time(NULL);
    return true;
  } // Intval::is_next
} // namespace openframe

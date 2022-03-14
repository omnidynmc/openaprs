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

#include <pthread.h>

#include <openframe/OFLock.h>

namespace openframe {
/**************************************************************************
 ** OFLock Class                                                         **
 **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  OFLock::OFLock() {
    pthread_mutex_init(&_mutex_l, NULL);
  } // OFLock::OFLock

  OFLock::~OFLock() {
  } // OFLock::~OFLock

  /**
   * OFLock::Lock
   *
   * Add a packet to our history.
   *
   * Returns: true if packet was added (actually should
   *          always return true).
   *
   */
  void OFLock::Lock() {
    pthread_mutex_lock(&_mutex_l);
  } // OFLock::Lock

  /**
   * OFLock::Unlock
   *
   * Add a packet to our history.
   *
   * Returns: true if packet was added (actually should
   *          always return true).
   *
   */
  void OFLock::Unlock() {
    pthread_mutex_unlock(&_mutex_l);
  } // OFLock::Unlock

}

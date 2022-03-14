/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, mySQL APRS Injector                                        **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
 **                    Daniel Robert Karrels                             **
 **                    Dynamic Networking Solutions                      **
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
 **************************************************************************
 $Id: DCC.h,v 1.8 2003/09/04 00:22:00 omni Exp $
 **************************************************************************/

#ifndef __MODULE_CLASSMGMT_H
#define __MODULE_CLASSMGMT_H

#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>

#include "App_Log.h"
#include "MGMT_DBI.h"
#include "OpenAPRS_Abstract.h"

#include "openframe/OFLock.h"

namespace mgmt {
  using namespace openaprs;
  using openframe::OFLock;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class MGMT : public OpenAPRS_Abstract, public OFLock {
  public:
    MGMT(App_Log *);
    virtual ~MGMT();

    // ### Type Definitions ###

    // ### Public Members ###
    const bool initializeSystem();
    const bool initializeCommands();
    const bool deinitializeSystem();
    const bool deinitializeCommands();

    const bool die() {
      bool die;

      Lock();
      die = _die;
      Unlock();

      return die;
    } // die()

    void die(const bool die) {
      Lock();
      _die = die;
      Unlock();
    } // die

    MGMT_DBI *dbi() { return _dbi; }

  protected:
  private:
    bool _die;
    MGMT_DBI *_dbi;
}; // APNS

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

extern "C" {
  extern MGMT *mgmt;				// declare global variable for mgmt
} // extern

}
#endif

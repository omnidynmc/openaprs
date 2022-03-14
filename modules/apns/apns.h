/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** HAL9000, Internet Relay Chat Bot                                     **
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
 $Id: cs.h,v 1.6 2003/09/20 18:10:52 omni Exp $
 **************************************************************************/

#ifndef __MODULE_APNS_H
#define __MODULE_APNS_H

#include <math.h>
#include <stdio.h>
#include <stdarg.h>

namespace openaprs_module_apns {

using std::string;
using std::map;   
using std::list;
using std::pair;

using namespace openaprs;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**********
 ** Logs **
 **********/

/******************
 ** SQL Settings **
 ******************/

/**************
 ** Commands **
 **************/


/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

#define MODULE_APNS_LOG_NORMAL			"module.apns.log.normal"
#define MODULE_APNS_PATH_LOGS_NORMAL		CFG_STRING("module.apns.path.logs.normal", "openaprs.apns.log")
#define MODULE_APNS_LOG_DEBUG			"module.apns.log.debug"
#define MODULE_APNS_PATH_LOGS_DEBUG		CFG_STRING("module.apns.path.logs.debug", "openaprs.apns.debug.log")

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

extern "C" {

} // extern

} // namespace module_apns

#endif

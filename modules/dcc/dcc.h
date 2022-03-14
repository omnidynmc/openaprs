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

#ifndef __MODULE_DCC_H
#define __MODULE_DCC_H

#include <math.h>
#include <stdio.h>
#include <stdarg.h>

#include "Cfg.h"

namespace dcc {
  using std::string;
  using std::map;
  using std::list;
  using std::pair;

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

#define MODULE_DCC_COMMANDS_CHARACTER           "."

#define MODULE_DCC_PORT				CFG_INT("module.dcc.port", 2620)
#define MODULE_DCC_MAXCONNECTIONS		1024

#define MODULE_DCC_NUMERIC			CFG_STRING("module.dcc.numeric", "AA")
#define MODULE_DCC_NUMERIC_SERVER_LENGTH	2
#define MODULE_DCC_NUMERIC_NUMNICK_LENGTH	5

#define MODULE_DCC_FLOOD_RATE                           512
#define MODULE_DCC_FLOOD_TIME                           10

#define MODULE_DCC_NUMERIC_NICK_LENGTH		CFG_INT("module.dcc.numeric.nick.length", 3)

#define MODULE_DCC_LOG_NORMAL			"module.dcc.log.normal"
#define MODULE_DCC_PATH_LOGS_NORMAL		CFG_STRING("module.dcc.path.logs.normal", "openaprs.dcc.log")

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

extern "C" {

} // extern

} // namespace module_dcc

#endif

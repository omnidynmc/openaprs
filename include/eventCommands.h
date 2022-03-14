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
 **************************************************************************/

#ifndef __LIBEVENTS_EVENT_COMMANDS_H
#define __LIBEVENTS_EVENT_COMMANDS_H

#include <map>
#include <string>
#include <list>

#include <time.h>
#include <sys/time.h>

namespace events {

  using std::string;
  using std::map;
  using std::list;
  using std::pair;

  using namespace handler;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  CREATE_EVENT(eventMESSAGETOACK);
  CREATE_EVENT(eventMESSAGETOEMAIL);
  CREATE_EVENT(eventMESSAGETOSQL);
  CREATE_EVENT(eventMESSAGETOVERIFY);
  CREATE_EVENT(eventPOSITIONTOSQL);
  CREATE_EVENT(eventTELEMETRYTOSQL);
  CREATE_EVENT(eventRAWTOSQL);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace commands
#endif

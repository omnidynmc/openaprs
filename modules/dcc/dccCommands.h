/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
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

#ifndef __DCCCOMMANDS_H
#define __DCCCOMMANDS_H

#include <map>
#include <string>
#include <list>

#include <time.h>
#include <sys/time.h>

#include "StringToken.h"

#include "Command.h"
#include "Event.h"

namespace dcc {
  using namespace std;
  using openaprs::CommandHandler;
  using openaprs::Packet;
  using handler::EventHandler;
  using handler::Event;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

//CREATE_COMMAND(stdinACTION);
CREATE_COMMAND(dccADDMODULE);
CREATE_COMMAND(dccCHANGEUSER);
CREATE_COMMAND(dccCHECKMESSAGES);
CREATE_COMMAND(dccCREATEMESSAGE);
CREATE_COMMAND(dccCREATEOBJECT);
CREATE_COMMAND(dccCREATEPOSITION);
CREATE_COMMAND(dccFINDULS);
CREATE_COMMAND(dccFINDUSER);
CREATE_COMMAND(dccFREQUENCY);
CREATE_COMMAND(dccKILLOBJECT);
CREATE_COMMAND(dccLASTPOSITION);
CREATE_COMMAND(dccLASTPOSITIONBBOX);
CREATE_COMMAND(dccLASTPOSITIONCENTER);
CREATE_COMMAND(dccLASTTELEMETRY);
CREATE_COMMAND(dccLASTWEATHER);
CREATE_COMMAND(dccLASTWEATHERBBOX);
CREATE_COMMAND(dccLASTWEATHERCENTER);
CREATE_COMMAND(dccLIVE);
CREATE_COMMAND(dccLOGIN);
CREATE_COMMAND(dccMESSAGING);
CREATE_COMMAND(dccPASSWORDRESET);
CREATE_COMMAND(dccPING);
CREATE_COMMAND(dccPONG);
CREATE_COMMAND(dccPOSITIONHISTORY);
CREATE_COMMAND(dccQUERYLOG);
CREATE_COMMAND(dccQUIT);
CREATE_COMMAND(dccREGISTERNOTIF);
CREATE_COMMAND(dccRELOADMODULE);
CREATE_COMMAND(dccREMOVEMODULE);
CREATE_COMMAND(dccRESENDACTIVATION);
CREATE_COMMAND(dccSEED);
CREATE_COMMAND(dccSIGNUP);
CREATE_COMMAND(dccTELEMETRYHISTORY);
CREATE_COMMAND(dccUID);
CREATE_COMMAND(dccWEATHERHISTORY);
CREATE_COMMAND(dccWALLOPS);

CREATE_EVENT(eventMESSAGETOSESSION);
CREATE_EVENT(eventPOSITIONTOLIVE);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

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
 $Id: APNS.cpp,v 1.12 2003/09/05 22:23:41 omni Exp $
 **************************************************************************/

#include <fstream>
#include <string>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <list>
#include <map>
#include <new>
#include <iostream>
#include <fstream>

#include <errno.h>

#include "Command.h"
#include "DCC_Command.h"
#include "dccCommands.h"

namespace dcc {
  using namespace std;
  using openaprs::Command;
  using openaprs::CommandTree;
  using openaprs::CommandHandler;

/**************************************************************************
 ** Worker Class                                                         **
 **************************************************************************/

  DCC_Command::DCC_Command() {
    addCommand("AM", "DCC", ".AM <name>", 1, Command::FLAG_CLEANUP, new dccADDMODULE);
    addCommand("CK", "DCC", ".CK [SR:<call with ssid>|AG:age|TG:ALL]", 1, Command::FLAG_CLEANUP, new dccCHECKMESSAGES);
    addCommand("CM", "DCC", ".CM TO:<to>|MS:<message>[SR:<from call with ssid>]", 1, Command::FLAG_CLEANUP, new dccCREATEMESSAGE);
    addCommand("CO", "DCC", ".CO <NM:name|LA:latitude|LN:longitude|TB:symbol table|CD:symbol code[|AT:altitude|SP:speed|CR:cource|AM:ambiguity|CM:comment|BC:beacon|EX:expire|CP:compress]>", 1, Command::FLAG_CLEANUP, new dccCREATEOBJECT);
    addCommand("CU", "DCC", ".CU CL:<callsign(wildcards[*,?])|email>|CHG:<A|V|I|D>", 1, Command::FLAG_CLEANUP, new dccCHANGEUSER);
    addCommand("FQ", "DCC", ".FQ CL:<callsign(wildcards[*,?])>[|AG:age|ST:start|LM:limit]", 1, Command::FLAG_CLEANUP, new dccFREQUENCY);
    addCommand("FL", "DCC", ".FU CL:<callsign>", 1, Command::FLAG_CLEANUP, new dccFINDULS);
    addCommand("FU", "DCC", ".FU CL:<callsign(wildcards[*,?])|email>", 1, Command::FLAG_CLEANUP, new dccFINDUSER);
    addCommand("KO", "DCC", ".KO NM:<name>", 1, Command::FLAG_CLEANUP, new dccKILLOBJECT);
    addCommand("CP", "DCC", ".CP <LA:latitude|LN:longitude|TB:symbol table|CD:symbol code[|AT:altitude|SP:speed|CR:cource|AM:ambiguity|CM:comment|CP:compress]>", 1, Command::FLAG_CLEANUP, new dccCREATEPOSITION);
    addCommand("QL", "DCC", ".QL <ON|OFF>", 1, Command::FLAG_CLEANUP, new dccQUERYLOG);
    addCommand("QU", "DCC", ".QU", 1, Command::FLAG_CLEANUP, new dccQUIT);
    addCommand("LN", "DCC", ".LN <username> <password> [client name]", 1, Command::FLAG_CLEANUP, new dccLOGIN);
    addCommand("LC", "DCC", ".LC CN:<lat>,<lon>|RG:<range in meters>[|AG:age|ST:start|LM:limit]", 1, Command::FLAG_CLEANUP, new dccLASTPOSITIONCENTER);
    addCommand("LB", "DCC", ".LB BB:<lat1>,<lon1>,<lat2>,<lon2>[|AG:age|ST:start|LM:limit]", 1, Command::FLAG_CLEANUP, new dccLASTPOSITIONBBOX);
    addCommand("LP", "DCC", ".LP CL:<callsign(wildcards[*,?])>[|AG:age|ST:start|LM:limit]", 1, Command::FLAG_CLEANUP, new dccLASTPOSITION);
    addCommand("LT", "DCC", ".LT CL:<callsign(wildcards[*,?])>[|AG:age|ST:start|LM:limit]", 1, Command::FLAG_CLEANUP, new dccLASTTELEMETRY);
    addCommand("LV", "DCC", ".LV", 1, Command::FLAG_CLEANUP, new dccLIVE);
    addCommand("MS", "DCC", ".MS ON:<yes/no>[|SR:<callsign with SSID>]", 1, Command::FLAG_CLEANUP, new dccMESSAGING);
    addCommand("PH", "DCC", ".PH CL:<callsign(wildcards[*,?])>[|AG:age|ST:start|LM:limit]", 1, Command::FLAG_CLEANUP, new dccPOSITIONHISTORY);
    addCommand("PG", "DCC", ".PG", 1, Command::FLAG_CLEANUP, new dccPING);
    addCommand("PN", "DCC", ".PN", 1, Command::FLAG_CLEANUP, new dccPONG);
    addCommand("PR", "DCC", ".PR EM:<email>|CLI:<client>|PW:<password>", 1, Command::FLAG_CLEANUP, new dccPASSWORDRESET);
    addCommand("RA", "DCC", ".RA EM:<email>|CLI:<client>|PW:<password>", 1, Command::FLAG_CLEANUP, new dccRESENDACTIVATION);
    addCommand("RL", "DCC", ".RL <name>", 1, Command::FLAG_CLEANUP, new dccRELOADMODULE);
    addCommand("RM", "DCC", ".RM <name>", 1, Command::FLAG_CLEANUP, new dccREMOVEMODULE);
    addCommand("RN", "DCC", ".RN CL:<callsign>|DT:<device token>|UID:<uid>", 1, Command::FLAG_CLEANUP, new dccREGISTERNOTIF);
    addCommand("SD", "DCC", ".SD", 1, Command::FLAG_CLEANUP, new dccSEED);
    addCommand("SU", "DCC", ".SU LNM:<lastname>|FNM:<firstname>|CL:<callsign>|EM:<email>|PW:<password>", 1, Command::FLAG_CLEANUP, new dccSIGNUP);
    addCommand("TH", "DCC", ".TH CL:<callsign(wildcards[*,?])>[|AG:age|ST:start|LM:limit]", 1, Command::FLAG_CLEANUP, new dccTELEMETRYHISTORY);
    addCommand("UID", "DCC", ".UID ID:<uid>", 1, Command::FLAG_CLEANUP, new dccUID);
    addCommand("WA", "DCC", ".WA MS:<message>", 1, Command::FLAG_CLEANUP, new dccWALLOPS);
    addCommand("WB", "DCC", ".WB BB:<lat1>,<lon1>,<lat2>,<lon2>[|AG:age|ST:start|LM:limit]", 1, Command::FLAG_CLEANUP, new dccLASTWEATHERBBOX);
    addCommand("WC", "DCC", ".WC CN:<lat>,<lon>|RG:<range in meters>[|AG:age|ST:start|LM:limit]", 1, Command::FLAG_CLEANUP, new dccLASTWEATHERCENTER);
    addCommand("WH", "DCC", ".WH CL:<callsign(wildcards[*,?])>[|AG:age|ST:start|LM:limit]", 1, Command::FLAG_CLEANUP, new dccWEATHERHISTORY);
    addCommand("WX", "DCC", ".WX CL:<callsign(wildcards[*,?])>[|AG:age|ST:start|LM:limit]", 1, Command::FLAG_CLEANUP, new dccLASTWEATHER);
  } // DCC_Command::DCC_Command

  DCC_Command::~DCC_Command() {
  } // DCC_Command::~DCC_Command
} // namespace dcc

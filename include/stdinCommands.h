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

#ifndef __STDINCOMMANDS_H
#define __STDINCOMMANDS_H

#include <map>
#include <string>
#include <list>

#include <time.h>
#include <sys/time.h>

#include "Command.h"

namespace openaprs {

using std::string;
using std::map;
using std::list;
using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

//CREATE_COMMAND(stdinACTION);
CREATE_COMMAND(stdinADDMODULE);
CREATE_COMMAND(stdinBOOKMARKS);
CREATE_COMMAND(stdinDEBUG);
CREATE_COMMAND(stdinDTREE);
CREATE_COMMAND(stdinDISCONNECT);
CREATE_COMMAND(stdinECHO);
CREATE_COMMAND(stdinFILTER);
CREATE_COMMAND(stdinHASH);
CREATE_COMMAND(stdinHELP);
CREATE_COMMAND(stdinLOAD);
CREATE_COMMAND(stdinQUERY);
CREATE_COMMAND(stdinQUIT);
CREATE_COMMAND(stdinRAW);
CREATE_COMMAND(stdinREMOVEMODULE);
CREATE_COMMAND(stdinSTATS);
CREATE_COMMAND(stdinSHOWCONFIG);
CREATE_COMMAND(stdinTESTAPRS);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

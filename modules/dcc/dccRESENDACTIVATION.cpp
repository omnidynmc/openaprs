/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
 ** Copyright (C) 1999 Gregory A. Carter                                 **
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
 $Id: dccCREATEPOSITION.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
 **************************************************************************/

#include <list>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <iostream>
#include <string>
#include <exception>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "APRS.h"
#include "DCC.h"
#include "DCCUser.h"
#include "StringToken.h"
#include "Command.h"
#include "Server.h"
#include "Vars.h"

#include "dccCommands.h"

#include "match.h"
#include "openaprs_string.h"

namespace dcc {

using namespace std;
using namespace openaprs;

  /**************************************************************************
   ** dccRESENDACTIVATION Class                                            **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  /**
   *
   *
   * EM: Email Address
   * CL:		Callsign
   */
  const int dccRESENDACTIVATION::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DCCUser *dccuser;			// pointer to a user
    bool autoverify;
    map<int, regexMatch> regexList;
    string parseMe;
    string email;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    // We don't have to be logged in or verified to issue a signup command.
    //if (IsDCCUserLogin(dccuser) == false)
    //  return CMDERR_ACCESS;

    //if (!IsDCCUserVerified(dccuser)) {
    //  dccuser->reply("errLoginVerify");
    //  return CMDERR_SUCCESS;
    //} // if

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    // initialize variables
    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    if (!v.isName("KY") || !v.isName("CLI")) {
      dccuser->reply("errMissingClientAuth");
      return CMDERR_SUCCESS;
    } // if

    if (!dbi->signup->isKey(v["CLI"], v["KY"], dccuser->seed(), autoverify)) {
      dccuser->reply("errAccessClientAuth");
      return CMDERR_SUCCESS;
    } // if

    // Check for missing variables first,
    // after we're doing with this we'll check each ones
    // validity.
    if (!v.isName("em")) {
      dccuser->reply("errMissingEmail");
      return CMDERR_SUCCESS;
    } // if

    // initialize variables
    email = v["em"];

    if (ereg("^([A-Za-z0-9._%+-]+[@][A-Za-z0-9.-]+[.][a-zA-Z]{2,4})$", email, regexList) < 1) {
      dccuser->reply("errInvalidEmail");
      return CMDERR_SUCCESS;
    } // if

    if (dbi->signup->isUniqueEmail(email)) {
      dccuser->reply("errNoAccount");
      return CMDERR_SUCCESS;
    } // if

    if (dbi->signup->isActivated(email)) {
      dccuser->reply("errAlreadyActivated");
      return CMDERR_SUCCESS;
    } // if

    if (!dbi->signup->ResendActivation(email)) {
      dccuser->reply("errNoResendActivation");
      return CMDERR_SUCCESS;
    } // if

    dccuser->reply("replyResendActivationComplete");

    return 1;
  } // dccRESENDACTIVATION::Execute
} // namespace dcc

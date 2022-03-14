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

#include "dcc.h"
#include "match.h"
#include "openaprs_string.h"
#include "md5wrapper.h"

namespace dcc {

using namespace std;
using namespace openaprs;

  /**************************************************************************
   ** dccSIGNUP Class                                                      **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  /**
   *
   *
   * FNM:		Firstname
   * LNM:		Lastname
   * PW:		Password
   * EM:		Email Address
   * CL:		Callsign
   */
  const int dccSIGNUP::Execute(COMMAND_ARGUMENTS) {
    DBI *dbi;
    DCCUser *dccuser;			// pointer to a user
    map<int, regexMatch> regexList;
    bool autoverify = false;
    int r;
    string parseMe;
    string lastname;
    string firstname;
    string callsign;
    string password;
    string passhash;
    string email;
    string activate;
    stringstream s;
    md5wrapper md5;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");
    dbi = (DBI *) ePacket->getVariable("dbi");

    if (ePacket->getArguments().size() < 1)
      return CMDERR_SYNTAX;

    // initialize variables
    parseMe = ePacket->getArguments().getTrailing(0);

    Vars v(parseMe);

    if (v.isName("ID")
        && dbi->signup->isBanned(v["ID"])) {
      dccuser->reply("errBanned");
      dccuser->Kill("You are banned from using this service.");
      return CMDERR_SUCCESS;
    } // if

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
    if (!v.isName("fnm")) {
      dccuser->reply("errMissingFirstname");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("lnm")) {
      dccuser->reply("errMissingLastname");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("pw")) {
      dccuser->reply("errMissingPassword");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("cl")) {
      dccuser->reply("errMissingCallsign");
      return CMDERR_SUCCESS;
    } // if

    if (!v.isName("em")) {
      dccuser->reply("errMissingEmail");
      return CMDERR_SUCCESS;
    } // if

    if (dbi->signup->isBanned(v["em"])) {
      dccuser->reply("errBanned");
      dccuser->Kill("You are banned from using this service.");
      return CMDERR_SUCCESS;
    } // if

    // initialize variables
    firstname = v["fnm"];
    lastname = v["lnm"];
    password = v["pw"];
    callsign = v["cl"];
    email = v["em"];

    if (ereg("^([\x20-\x7e]{1,64})$", firstname, regexList) < 1) {
      dccuser->reply("errInvalidFirstname");
      return CMDERR_SUCCESS;
    } // if

    if (ereg("^([\x20-\x7e]{1,64})$", lastname, regexList) < 1) {
      dccuser->reply("errInvalidLastname");
      return CMDERR_SUCCESS;
    } // if

    if (ereg("^([A-Za-z0-9._%+-]+[@][A-Za-z0-9.-]+[.][a-zA-Z]{2,4})$", email, regexList) < 1) {
      dccuser->reply("errInvalidEmail");
      return CMDERR_SUCCESS;
    } // if

    // Don't allow all characters
    if (ereg("^([a-zA-Z]+)$", callsign, regexList)) {
      dccuser->reply("errInvalidCallsign");
      return CMDERR_SUCCESS;
    } // if

    // Don't allow two numbers
    if (ereg("([0-9]{2})", callsign, regexList)) {
      dccuser->reply("errInvalidCallsign");
      return CMDERR_SUCCESS;
    } // if

    // Don't allow all numbers
    if (ereg("^([0-9]+)$", callsign, regexList)) {
      dccuser->reply("errInvalidCallsign");
      return CMDERR_SUCCESS;
    } // if

    if (ereg("^([a-zA-Z0-9]{2,7})$", callsign, regexList) < 1) {
      dccuser->reply("errInvalidCallsign");
      return CMDERR_SUCCESS;
    } // if

    // Is this an iE upgrade?
    passhash = md5.getHashFromString(password.c_str());
    //passhash = password;

    if (autoverify
       && dbi->signup->isIEUpgrade(email, password)) {
      dccuser->datapoint("num.signup.upgrade");
      dccuser->reply("replyIEUpgraded");
      return CMDERR_SUCCESS;
    } // if

    if (!dbi->signup->isUniqueCall(callsign)) {
      dccuser->reply("errAlreadyCallsign");
      return CMDERR_SUCCESS;
    } // if

    if (!dbi->signup->isUniqueEmail(email)) {
      dccuser->reply("errAlreadyEmail");
      return CMDERR_SUCCESS;
    } // if

    s.str("");

    srand(time(NULL));
    r = rand() % 1000;
    s << r << time(NULL) << email;
    activate = md5.getHashFromString(s.str());

    if (!dbi->signup->Create(firstname,
                                     lastname,
                                     email,
                                     password,
                                     activate,
                                     callsign,
                                     app->cfg->get_int("openaprs.signups.active.until", 172800),
                                     autoverify)) {
      dccuser->reply("errNoCreateSignup");
      return CMDERR_SUCCESS;
    } // if

    app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: Password hash calculated is: %s", passhash.c_str());
    dccuser->datapoint("num.signup.success", 1);
    dccuser->reply("replySignupComplete");

    return 1;
  } // dccSIGNUP::Execute
} // namespace dcc

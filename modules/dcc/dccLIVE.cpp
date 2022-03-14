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
 $Id: dccWALLOPS.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
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
#include "Command.h"
#include "Server.h"
#include "Vars.h"

#include "openframe/StringToken.h"

#include "dccCommands.h"

#include "dcc.h"
#include "match.h"
#include "openaprs_string.h"

namespace dcc {
  using namespace std;
  using namespace openaprs;
  using namespace aprs;
  using openframe::StringToken;

  /**************************************************************************
   ** dccLIVE Class                                                        **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const int dccLIVE::Execute(COMMAND_ARGUMENTS) {
    DCCUser *dccuser;                                     // pointer to a user
    bool turnOff = false;
    StringToken st;
    stringstream s;
    Vars v;
    double clat, clon, lat1, lon1, lat2, lon2, rg;
    string parseMe;
    string source;
    string filter;
    string sf;

    // initialize access processing
    dccuser = (DCCUser *) ePacket->getVariable("dccuser");

    if (IsDCCUserLogin(dccuser) == false)
      return CMDERR_ACCESS;

    filter = "";
    sf = "";
    if (ePacket->getArguments().size() > 0) {
      filter = parseMe = ePacket->getArguments().getTrailing(0);

      Vars v(parseMe);

      if (v.isName("on")) {
        if (strtoupper(v["on"]) == "NO")
          turnOff = true;
        else if (strtoupper(v["on"]) != "YES") {
          dccuser->reply("errInvalidOn");
          return CMDERR_SUCCESS;
        } // else if

        v.remove("on");
        v.compile(filter, "");
      } // if

      if (v.isName("nma")) {
        if (strtoupper(v["nma"]) == "NO")
          UnsetDCCUserNMEA(dccuser);
        else if (strtoupper(v["nma"]) == "YES")
          SetDCCUserNMEA(dccuser);
        else {
          dccuser->reply("errInvalidNMEA");
          return CMDERR_SUCCESS;
        } // else

        v.remove("nma");
        v.compile(filter, "");
      } // if

      if (v.isName("sf")) {
        sf = v["sf"];
        v.remove("sf");
        v.compile(filter, "");
      } // if

      if (v.isName("cn")) {
        st.setDelimiter(',');
        st = v["cn"];

        if (st.size() != 3) {
          dccuser->reply("errInvalidCenter");
          return CMDERR_SUCCESS;
        } // if

        clat = atof(st[0].c_str());
        clon = atof(st[1].c_str());
        rg = atof(st[2].c_str());

        if (rg < 0.0 || rg > 200.00) {
          dccuser->reply("errInvalidRange");
          return CMDERR_SUCCESS;
        } // if

        if (!APRS::isValidLatLong(clat,clon)) {
          dccuser->reply("errInvalidCenter");
          return CMDERR_SUCCESS;
        } // if

        // Try and calculate bounding box.
        APRS::calcDestPoint(315, (rg*2), clat, clon, lat1, lon1);
        APRS::calcDestPoint(135, (rg*2), clat, clon, lat2, lon2);

        v.remove("cn");
        v.remove("la");
        v.remove("ln");

        s.str("");
        s << "<" << lat1 << ",>" << lat2;
        v.add("la", s.str());

        s.str("");
        s << ">" << lon1 << ",<" << lon2;
        v.add("ln", s.str());

        v.compile(filter, "");
      } // if
    } // if

    // initialize variables
    source = dccuser->callsign();

    dccuser->var()->pop("dcc.user.live.filter");
    dccuser->var()->pop("dcc.user.live.fields");

    if (IsDCCUserLive(dccuser) && turnOff == true) {
      UnsetDCCUserLive(dccuser);
      dccuser->sendf("316 ON:no|MS:Live set to OFF.\n");
      app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (LV) %s set to `%s'",
                        source.c_str(),
                        "OFF");
    } // if
    else {
      dccuser->var()->replace_string("dcc.user.live.filter", filter);
      dccuser->var()->replace_string("dcc.user.live.fields", sf);
      app->writeLog(MODULE_DCC_LOG_NORMAL, "*** Module DCC: (LV) %s set to `%s'",
                        source.c_str(),
                        IsDCCUserLive(dccuser) ? "UPDATED" : "ON");

      SetDCCUserLive(dccuser);

      dccuser->sendf("317 ON:yes|MS:Live set to ON.\n");
    } // else

    return CMDERR_SUCCESS;
  } // dccLIVE::Execute
} // namespace dcc

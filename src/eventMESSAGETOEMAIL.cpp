/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** HAL9000, Internet Relay Chat Bot                                     **
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
 $Id: eventMESSAGETOEMAIL.cpp,v 1.2 2003/08/30 03:59:00 omni Exp $
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
#include "Command.h"
#include "Create.h"
#include "DBI.h"
#include "Send.h"
#include "Server.h"

#include "openframe/StringToken.h"

#include "eventCommands.h"

#include "openaprs.h"
#include "openaprs_string.h"

namespace events {
  using openframe::StringToken;
  using namespace handler;
  using namespace std;
  using namespace aprs;
  using aprs::Send;

/**************************************************************************
 ** eventMESSAGETOEMAIL Class                                            **
 **************************************************************************/

  /*********************
   ** OnEvent Members **
   *********************/

  const int eventMESSAGETOEMAIL::OnEvent(Event *eventInfo, const string &eventMatch, Packet *ePacket) {
    APRS *aprs = (APRS *) ePacket->getVariable("aprs");
    DBI *dbi = (DBI *) ePacket->getVariable("dbi");
    DBI::setMapType sm;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    Send sn;
    StringToken st;
    map<int, regexMatch> regexList;               // Map of regex matches.
    bool isOk = false, aliasMatch = false;
    string source = aprs->getString("aprs.packet.source");
    string to = aprs->getString("aprs.packet.message.target");
    string email;
    string cache = aprs->getString("aprs.packet.body");
    string parseMe = aprs->getString("aprs.packet.message.text");
    string shortcut;
    string message;
    string ackMessage;
    string base = aprs->getString("aprs.packet.source.callsign");
    string reply;
    stringstream s;
    struct tm *tm;                                // struct tm for localtime
    time_t now;                                   // current time
    char timestamp[24];                           // Currne timestamp.
    int numRows;
    unsigned int i;
    list<string> aliasList;

    // Is email gateway enabled?
    if (!app->cfg->get_bool("openaprs.email.enable", false))
      return CMDERR_SUCCESS;

    sn.logger(new App_Log(""));

    app->cfg->match("openaprs.email.alias*", aliasList);

    while(!aliasList.empty()) {
      if (app->cfg->get_string(aliasList.front()) == to)
        aliasMatch = true;

      aliasList.pop_front();
    } // for

    if (!aliasMatch)
      return CMDERR_SUCCESS;

    // Should I ignore this user?
    if (dbi->email->ignore(base)) {
      app->writeLog(OPENAPRS_LOG_APRS, "*** Email Gateway: Ignored %s",
                                             source.c_str());

      return CMDERR_SUCCESS;
    } // if

    // If the message was sent to the email gateway, ack it.
    if (aprs->isString("aprs.packet.message.id")) {
      s.str("");
      s << "ack" << aprs->getString("aprs.packet.message.id.reply");
      sn.Message(aprs->getString("aprs.packet.message.target"), aprs->getString("aprs.packet.source"), s.str(), false, ackMessage);
    } // if

    // Is this a resend or in cache for some reason?
    if (dbi->email->cached(source, cache)) {
      app->writeLog(OPENAPRS_LOG_APRS, "*** Email Gateway[%s]: Cached `%s`", 
                                             source.c_str(),
                                             cache.c_str());

      return CMDERR_SUCCESS;
    } // if
    else
      dbi->email->cache(source, cache);

    st = parseMe;
    if (st.size() < 2)
      return CMDERR_SUCCESS;

    // Detect alias add request.
    if (openaprs_string_regex("^([a-zA-Z0-9]+)[ ]+([a-zA-Z0-9._-]+@[a-zA-Z0-9.-]+)$", 
                               parseMe,
                               regexList)) {
      isOk = dbi->email->add(base, regexList[1].matchString, regexList[2].matchString);

      if (isOk)
        reply = "Alias added.";
      else
        reply = "Alias already exists.";

      app->writeLog(OPENAPRS_LOG_APRS "*** Email Gateway[%s]: Alias for %s to %s: %s",
                                             source.c_str(),
                                             regexList[1].matchString.c_str(),
                                             regexList[2].matchString.c_str(),
                                             reply.c_str());

      dbi->createMessage(OPENAPRS_CONFIG_CREATE, source, reply, false);

      return CMDERR_SUCCESS;
    } // if

    // Detect alias remove request.
    if (openaprs_string_regex("^([a-zA-Z0-9]+)[ ]+[rR]$",
                               parseMe,
                               regexList)) {
      isOk = dbi->email->remove(base, regexList[1].matchString);

      if (isOk)
        reply = "Alias removed.";
      else
        reply = "Alias not found.";

      app->writeLog(OPENAPRS_LOG_APRS, "*** Email Gateway[%s]: %s", source.c_str(), reply.c_str());

      dbi->createMessage(OPENAPRS_CONFIG_CREATE, aprs->getString("aprs.packet.source"), reply, false);

      return CMDERR_SUCCESS;
    } // if

    // First try and figure out if this is an alias or not.
    if (dbi->email->find(base, st[0], shortcut)) {
      app->writeLog(OPENAPRS_LOG_APRS, "*** Email Gateway[%s]: Found alias for %s to %s",
                                             source.c_str(),
                                             st[0].c_str(),
                                             shortcut.c_str());
      parseMe = shortcut + string(" ") + st.getTrailing(1);
    } // if

    // Detect alias list request.
    if (openaprs_string_regex("^([a-zA-Z0-9._-]+@[a-zA-Z0-9.-]+)[ ]+[lL]$", 
                              parseMe,
                              regexList)) {
      email = regexList[1].matchString;

      numRows = dbi->email->list(base, sm);

      s.str("");
      s << "OpenAPRS.Net - APRS Message to Email Gateway" << endl
        << endl
        << "Alias List for " << base << ":" << endl
        << endl;

      for(i=0; i < sm.size(); i++) {
        rm = sm[i].resultMap;

        for(ptr = rm.begin(); ptr != rm.end(); ptr++)
          s << ptr->second << " ";

        s << endl;
      } // for

      app->writeLog(OPENAPRS_LOG_APRS, "*** Email Gateway[%s]: Sending list to %s",
                                             source.c_str(),
                                             email.c_str());

      app->Email(source, email, s.str());

      return CMDERR_SUCCESS;
    } // if

    // Parse message sending format.
    if (!openaprs_string_regex("^([a-zA-Z0-9._-]+@[a-zA-Z0-9.-]+)[ ]+([\x20-\x7e]+)$",
                               parseMe,
                               regexList)) {
      app->writeLog(OPENAPRS_LOG_APRS, "*** Email Gateway: Failed to parse `%s' from %s",
                                             parseMe.c_str(),
                                             source.c_str());
      return CMDERR_SUCCESS;
    } // if

    email = regexList[1].matchString;
    message = regexList[2].matchString;

    if (message.length() > 67)
      message = message.substr(0, 66);

    app->writeLog(OPENAPRS_LOG_APRS, "*** Email Gateway[%s]: Sending message `%s' to %s",
                                           source.c_str(),
                                           message.c_str(),
                                           email.c_str()),


    now = time(NULL);
    tm = gmtime(&now);

    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d UTC",
             (tm->tm_year + 1900),
             (tm->tm_mon + 1),
             tm->tm_mday,
             tm->tm_hour,
             tm->tm_min,
             tm->tm_sec);

    s.str("");
    s << message << "\n\n"
      << "----------------------------------------\n"
      << "  Date     : " << timestamp << "\n"
      << "  From     : " << source << "\n"
      << "  To       : " << email << "\n"
      << "  IGATE    : " << (aprs->isString("aprs.packet.igate") ? aprs->getString("aprs.packet.igate") : "unknown") << "\n"
      << "----------------------------------------\n"
      << "\n\n---\nOpenAPRS.Net Message to Email Gateway\n";


    app->Email(source, email, s.str());

    return CMDERR_SUCCESS;
  } // eventMESSAGETOEMAIL::OnEvent
} // namespace commands

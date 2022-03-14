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
 $Id: timerAPRSMAIL.cpp,v 1.2 2003/08/30 03:59:00 omni Exp $
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
#include "Timer.h"
#include "Vars.h"

#include "timerCommands.h"

#include "openaprs.h"
#include "openaprs_string.h"

namespace handler {

  using namespace std;
  using namespace openaprs;
  using aprs::Send;
  using aprs::APRS;

/**************************************************************************
 ** timerAPRSMAIL Class                                                  **
 **************************************************************************/

  /*********************
   ** OnTimer Members **
   *********************/

class cAPRSMAIL {
  public:
    int id;
    int user_id;
    string sender;
    string from;
    string to;
    string callsign;
    string message;
    string subject;
    string title;
};

  const int timerAPRSMAIL::OnTimer(Timer *aTimer) {
    DBI::resultMapType rs;
    DBI::setMapType sm;
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    Vars v;
    cAPRSMAIL am;
    list<cAPRSMAIL> msgs;
    map<int, regexMatch> rl;               // Map of regex matches.
    int user_id;
    string msgid;
    string message;
    string source;
    string base;
    stringstream s;
    unsigned int i;
    unsigned int numCreated;
    unsigned int numReturned;

    // initialize variables
    numCreated = 0;

    if (app->cfg->get_bool("openaprs.aprsmail.enable", false) == false)
      return CMDERR_SUCCESS;

    if (!IsServerSQL)
      return CMDERR_SUCCESS;

    //if (app->Query("SELECT aprsmail_mail.*, web_users.callsign AS user_callsign FROM aprsmail_mail LEFT JOIN web_users ON web_users.id = aprsmail_mail.user_id WHERE aprsmail_mail.read_ts = '0' ORDER BY aprsmail_mail.create_ts ASC")) {
    if (app->dbi->aprsmail->getPending(sm)) {
      for(i=0; i < sm.size(); i++) {
        rm = sm[i].resultMap;

        // initialize variables
        am.from = rm["mail_from"];
        am.to = rm["mail_to"];
        am.callsign = rm["callsign"];
        am.sender = rm["sender"];
        am.subject = rm["mail_subject"];
        am.message = rm["mail_content"];
        am.message = app->dbi->handle()->unescape(am.message);
        am.user_id = atoi(rm["user_id"].c_str());
        am.id = atoi(rm["id"].c_str());

        s.str("");
        s << "Create EMAIL<=>APRS \'" << am.message << "\' to " << am.to;
        am.title = s.str();

        if (am.callsign.length() < 1)
          continue;

        // we've got to save it like this so that
        // the Sql query doesn't get squashed by
        // send reparsing messages.
        msgs.push_back(am);
      } // for

      for(; !msgs.empty(); msgs.pop_front()) {
        am = msgs.front();

        app->dbi->aprsmail->setRead(am.id, time(NULL));

        // Try and parse the message from the content.
        v = am.message;

        s.str("");
        s << "Thank you for using OpenAPRS's APRSMail interface!" << endl << endl
          << "To use this EMAIL->APRS Message gateway send an email to <your friends callsign>@aprsmail.org and in the body of the email use the following format to send a message:" << endl << endl
          << "SR:<your callsign>|PW:<your pin number>|MS:<message>" << endl << endl
          << "Please note that the use of the pipe (|) character or the backslash (\\) character in the message field must be escaped with an additional backslash character preceding them." << endl << endl
          << "The response from your request is:" << endl << endl;

        if (!v.isName("sr")) {
          s << "ERROR: Missing source (SR) field.  The SR field should be your callsign." << endl;
          app->dbi->aprsmail->writeError(am.id, "%s", "ERROR: Missing source (SR) field.  The SR field should be your callsign.");
          app->Email("OpenAPRS", am.sender, s.str());
          app->writeLog(OPENAPRS_LOG_NORMAL, "*** EMAIL<=>APRS Missing source (SR) field: %s to %s from <%s>", am.message.c_str(), am.callsign.c_str(), am.sender.c_str());
          continue;
        } // if

        if (!ereg("^([a-zA-Z0-9-]{1,10})$", v["sr"], rl)) {
          s << "ERROR: Invalid source callsign (SR) field.  Source callsign should be 10 characters or less and may contain SSID." << endl;
          app->dbi->aprsmail->writeError(am.id, "%s", "ERROR: Invalid source callsign (SR) field.  Source callsign should be 10 characters or less and may contain SSID.");
          app->Email("OpenAPRS", am.sender, s.str());
          app->writeLog(OPENAPRS_LOG_NORMAL, "*** EMAIL<=>APRS Invalid source (SR) field: %s to %s from <%s>", am.message.c_str(), am.callsign.c_str(), am.sender.c_str());
          continue;
        } // if

        source = APRS::extractCallsign(v["sr"]);

        if (!v.isName("pw")) {
          s << "ERROR: Missing PW field.  Unable to verify your account." << endl;
          app->dbi->aprsmail->writeError(am.id, "%s", "ERROR: Missing PW field.  Unable to verify your account.");
          app->Email("OpenAPRS", am.sender, s.str());
          app->writeLog(OPENAPRS_LOG_NORMAL, "*** EMAIL<=>APRS Missing pin number (PW) field: %s to %s from <%s>", am.message.c_str(), am.callsign.c_str(), am.sender.c_str());
          continue;
        } // if

        // Attempt to authenticate user.
        if (!app->dbi->getUserByCallsign(source, rs) ||
            rs.find("id") == rs.end()) {
          s << "ERROR: Could not verify your account and pin." << endl;
          app->dbi->aprsmail->writeError(am.id, "%s", "ERROR: Could not verify your account and pin.");
          app->Email("OpenAPRS", am.sender, s.str());
          app->writeLog(OPENAPRS_LOG_NORMAL, "*** EMAIL<=>APRS Callsign (%s) not found: %s to %s from <%s>", source.c_str(), am.message.c_str(), am.callsign.c_str(), am.sender.c_str());
          continue;
        } // if

        user_id = atoi(rs.find("id")->second.c_str());

        if (!app->dbi->getUserOptions(user_id, rs) ||
            rs.find("aprsmail_pin") == rs.end() ||
            rs.find("aprsmail_pin")->second != v["pw"]) {
          s << "ERROR: Could not verify your account and pin." << endl;
          app->dbi->aprsmail->writeError(am.id, "%s", "ERROR: Could not verify your account and pin.");
          app->Email("OpenAPRS", am.sender, s.str());
          app->writeLog(OPENAPRS_LOG_NORMAL, "*** EMAIL<=>APRS Pin does not match for %s: %s to %s from <%s>", source.c_str(), am.message.c_str(), am.callsign.c_str(), am.sender.c_str());
          continue;
        } // if

        if (!v.isName("ms")) {
          s << "ERROR: Missing MS field.  The MS field should be set to the message you wish to send." << endl;
          app->dbi->aprsmail->writeError(am.id, "%s", "ERROR: Missing MS field.  The MS field should be set to the message you wish to send.");
          app->Email("OpenAPRS", am.sender, s.str());
          app->writeLog(OPENAPRS_LOG_NORMAL, "*** EMAIL<=>APRS Missing message (MS) field: %s to %s from <%s>", am.message.c_str(), am.callsign.c_str(), am.sender.c_str());
          continue;
        } // if

        if (!ereg("^([\x20-\x7e]+)$", v["ms"], rl)) {
          s << "ERROR: Invalid message (MS) field.  Message should be 67 characters or less and may not contain binary data." << endl;
          app->dbi->aprsmail->writeError(am.id, "%s", "ERROR: Invalid message (MS) field.  Message should be 67 characters or less and may not contain binary data.");
          app->Email("OpenAPRS", am.from, s.str());
          app->writeLog(OPENAPRS_LOG_NORMAL, "*** EMAIL<=>APRS Invalid message (MS) field: %s to %s from <%s>", am.message.c_str(), am.callsign.c_str(), am.sender.c_str());
          continue;
        } // if

        // Check for a preferred list for the target user.
        // IMPORTANT: From this point forward user_options is now pointed
        // at our target user.
        base = APRS::extractCallsign(am.callsign);

        // Attempt to authenticate user.
        if (strtoupper(source) == strtoupper(base)) {
          s << "ERROR: To prevent the possibility of ack loops through our system you cannot send messages to your self." << endl;
          app->dbi->aprsmail->writeError(am.id, "%s", "ERROR: To prevent the possibility of ack loops through our system you cannot send messages to your self.");
          app->Email("OpenAPRS", am.sender, s.str());
          app->writeLog(OPENAPRS_LOG_NORMAL, "*** EMAIL<=>APRS Tried to send to self: %s to %s from <%s>", source.c_str(), am.message.c_str(), am.callsign.c_str(), am.sender.c_str());
          continue;
        } // if

        if (app->dbi->getUserByCallsign(base, rs) &&
            rs.find("id") != rs.end()) {
          user_id = atoi(rs.find("id")->second.c_str());

          if (app->dbi->getUserOptions(user_id, rs) &&
              rs.find("aprsmail_preferred") != rs.end() &&
              rs.find("aprsmail_preferred")->second.length() > 0) {
            am.callsign = rs.find("aprsmail_preferred")->second;

            s << "OVERRIDE: "
              << v["sr"]
              << " has a preferred callsign set, your message will be sent to "
              << am.callsign
              << " instead." << endl << endl;
          } // if
        } // if

        numCreated++;
        app->dbi->createMessage(v["sr"], am.callsign, v["ms"], false);

        s << "Your message is being sent out." << endl << endl;
        app->dbi->aprsmail->writeError(am.id, "%s", "STATUS: OK");
        app->writeLog(OPENAPRS_LOG_NORMAL, "*** EMAIL<=>APRS Sent: %s to %s from <%s>", am.message.c_str(), am.callsign.c_str(), am.sender.c_str());

        // Try and find list of active target callsigns.
        numReturned = app->dbi->position->LastByCallsign(base+"*", time(NULL)-86400, time(NULL), 10, sm);

        if (numReturned)
          s << "We have seen "
            << base
            << " sending position reports from the following callsigns today:";

        for(i=0; i < sm.size(); i++) {
          rm = sm[i].resultMap;

          if ((ptr = rm.find("source")) != rm.end()) {
            if (i > 0)
              s << ",";
            s << " " << ptr->second;
          } // if
        } // for

        if (numReturned)
          s << endl;

        numReturned = app->dbi->message->LastByCallsign(base+"*", time(NULL)-86400, time(NULL), 10, sm);

        if (numReturned)
          s << "We have seen "
            << base
            << " sending messages from the following callsigns today:";

        for(i=0; i < sm.size(); i++) {
          rm = sm[i].resultMap;

          if ((ptr = rm.find("source")) != rm.end()) {
            if (i > 0)
              s << ",";
            s << " " << ptr->second;
          } // if
        } // for

        if (numReturned)
          s << endl;

        app->Email("OpenAPRS", am.sender, s.str());

      } // for
    } // if

    if (numCreated > 0)
      app->writeLog(OPENAPRS_LOG_NORMAL, "*** EMAIL<=>APRS created: %d", numCreated);

    return CMDERR_SUCCESS;
  } // timerCREATEMESSAGES::OnTimer
} // namespace commands

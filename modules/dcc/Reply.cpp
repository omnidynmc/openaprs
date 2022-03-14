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
 $Id: Reply.cpp,v 1.12 2003/09/05 22:23:41 omni Exp $
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
#include <sstream>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include "Reply.h"

namespace dcc {

  using namespace std;

/**************************************************************************
 ** Reply Class                                                          **
 **************************************************************************/

  Reply::Reply() {
    replyMap.clear();

    add(3, "replyUidOk", "UID OK, send login information.");
    add(100, "replyLoginOk", "Login successful.");
    add(101, "errLoginAlready", "Already logged in.");
    add(102, "errLoginVerify", "Account must be verified to use this feature.");
    add(103, "errLoginActive", "Account is not active.  If you just signed up it could take up to 15 minutes to receive your activation email, when you do follow the instructions in the email to activate your account.");
    add(104, "errLogin", "Login failed, check your username and password.");
    add(105, "replyLoginAdmin", "You have admin access.");
    add(106, "sendPing", "Ping?");
    add(107, "replyPong", "Pong!");
    add(108, "replyGoodbye", "Goodbye.");
    add(109, "replyLoginStats", "Login Status");
    add(110, "replySeed", "Hash Seed");
    add(111, "replyLoginGuest", "You have guest access ONLY, you will not be allowed to send packets.");
    add(112, "warnActiveUntil", "Warning, you have not activated your account yet.  You only have a 72 hour grace period form the time you signup to respond to the activation email we sent you to activate your account.");
    add(200, "errSyntax", "Syntax error.");
    add(201, "errAccessDenied", "Access denied.");
    add(202, "errInvalidCommand", "Invalid command.");
    add(203, "errAccessClientAuth", "Client is not authorized to perform signups.  Have client vendor contact OpenAPRS staff for assistance or update your client to the latest version.");
    add(204, "errAccessGuest", "Guest account is not allowed to send packets.  Signup for an official account or if you already have an account setup your account information in Settings.");
    //add(300, "replyMessageTable", "Reserved for message table.");
    //add(301, "replyMessageCount", "Reserved for message count.");
    add(302, "replyMessagingOn", "Messaging is ON.");
    add(303, "replyMessagingOff", "Messaging is OFF.");
    //add(304, "replyPosition", "Reserved for position reply.");
    //add(305, "replyPositionCount", "Reserved for position count.");
    //add(306, "replyFrequency", "Reserved for frequency reply.");
    //add(307, "replyFrequencyCount", "Reserved for frequency count.");
    //add(308, "replyWeather", "Reserved for weather reply.");
    //add(309, "replyWeatherCount", "Reserved for weather count.");
    //add(310, "replyTelemetry", "Reserved for telemetry reply.");
    //add(311, "replyTelemetryCount", "Reserved for telemetry count.");
    //add(312, "replyTelemetryLabels", "Reserved for telemetry labels.");
    //add(313, "replyTelemetryEqns", "Reserved for telemetry eqns.");
    //add(314, "replyTelemetryUnits", "Reserved for telemetry units.");
    //add(315, "replyTelemetryBits", "Reserved for telemetry bits.");
    //add(316, "replyWallops", "Reserved for wallops messages.");
    //add(317, "replyLive", "Reserved for live reply.");
    //add(318, "replyPositionLive", "Reserved for live position reply.");
    //add(319, "replyFindUser", "Reserved for find user.");
    //add(320, "replyFindUserCount", "Reserved for find user count.");
    //add(321, "replyChangeUserComplete", "Reserved for change count.");
    //add(322, "replyPositionHistory", "Reserved for position history reply.");
    //add(323, "replyPositionHistoryCount", "Reserved for position history count.");
    //add(324, "replyFindUls", "Reserved for find uls reply.");
    //add(325, "replyFindUlsCount", "Reserved for find uls count.");
    add(400, "errUnknown", "Unknown error.");
    add(401, "errMissingObjectName", "Missing object name.");
    add(402, "errMissingLatitude", "Missing latitude.");
    add(403, "errMissingLongitude", "Missing longitude.");
    add(404, "errMissingSymTable", "Missing symbol table.");
    add(405, "errMissingSymCode", "Missing symbol code.");
    add(406, "errObjectNotFound", "Object not found.  You may only kill objects created by this application.");
    add(407, "errModuleNotFound", "Module not found.");
    add(408, "errNoCreateObject", "Could not create object.");
    add(409, "errNoCreatePosition", "Could not create position.");
    add(410, "errMissingExpire", "Missing expire timestamp, when beacon is used an expire time is required.");
    add(411, "errMissingCallsign", "Missing callsign.");
    add(412, "errMissingBBOX", "Missing bounding box.");
    add(413, "errMissingMessage", "Missing message.");
    add(414, "errMissingTo", "Missing message to field.");
    add(415, "errMissingCenter", "Missing center lat/long.");
    add(416, "errMissingRange", "Missing range.");
    add(417, "errMissingOn", "Missing on switch.");
    add(418, "errNoCreateSignup", "Could not create signup.");
    add(419, "errMissingFirstname", "Missing first name.");
    add(420, "errMissingLastname", "Missing last name.");
    add(421, "errMissingPassword", "Missing password.");
    add(422, "errMissingEmail", "Missing email.");
    add(423, "errMissingClientAuth", "Missing client authentication.");
    add(424, "errNoResendActivation", "Could not resend activation key.  Probably already waiting to send activation email.");
    add(425, "errNoAccount", "Could not find your account.");
    add(426, "errMissingIdr", "Missing record id.");
    add(427, "errNoUser", "User not found.");
    add(428, "errMissingChangeAction", "Missing change action.");
    add(428, "errMissingUid", "Missing device UID.");
    add(428, "errBanned", "Your are banned from using this service.");
    add(429, "errNoPasswordReset", "Could not reset your password.  Probably already waiting to send reset email.");
    add(430, "errMissingDeviceToken", "Missing device token.");
    add(500, "replyOk", "OK!");
    add(501, "replyMessageSent", "Message sent.");
    add(502, "replyPositionSent", "Position sent.");
    add(503, "replySignupComplete", "Signup complete.");
    add(504, "replyResendActivationComplete", "Resend activation complete.");
    add(505, "replyReloadingModule", "Reloading module.");
    add(506, "replyObjectCreated", "Object created.");
    add(507, "replyObjectKilled", "Object killed.");
    add(508, "replyPasswordResetComplete", "Password reset complete.");
    add(509, "replyRegisterOk", "Registration for notifications complete.");
    add(600, "errInvalidField", "Invalid field.");
    add(601, "errInvalidLatLong", "Invalid latitude or longitude.");
    add(602, "errInvalidCallsign", "Invalid callsign, you must be an amateur radio operator to use this app.");
    add(603, "errInvalidMessage", "Invalid message, may have invalid characters or may be trying to send message to self.");
    add(604, "errInvalidCourse", "Invalid course.");
    add(605, "errInvalidSpeed", "Invalid speed.");
    add(606, "errInvalidAmbiguity", "Invalid ambiguity.");
    add(607, "errInvalidCompress", "Invalid compress, should be yes or no.");
    add(608, "errInvalidName", "Invalid name.");
    add(609, "errInvalidBeacon", "Invalid beacon frequncy, should be in number of seconds and must be greater than 1800.");
    add(610, "errInvalidSymbolTable", "Invalid symbol table.");
    add(611, "errInvalidSymbolCode", "Invalid symbol code.");
    add(612, "errInvalidComment", "Invalid comment.");
    add(613, "errInvalidLimit", "Invalid limit. [lm > 0 && lm < 1001]");
    add(614, "errInvalidBBOX", "Invalid bounding box.");
    add(615, "errInvalidExpire", "Invalid expire timestamp, cannot be over 1 year.");
    add(616, "errInvalidSymbolName", "Invalid symbol name.");
    add(617, "errInvalidCenter", "Invalid center lat/long.");
    add(618, "errInvalidRange", "Invalid range, must be less than 201km.");
    add(619, "errInvalidSSID", "Invalid SSID, must use same base callsign as you signed up with.");
    add(620, "errInvalidOn", "Invalid ON switch, must be yes or no.");
    add(621, "errInvalidNMEA", "Invalid NMEA switch, must be yes or no.");
    add(622, "errInvalidFirstname", "Invalid first name.");
    add(623, "errInvalidLastname", "Invalid last name.");
    add(624, "errInvalidPassword", "Invalid password.");
    add(625, "errInvalidEmail", "Invalid email.");
    add(626, "errInvalidClientAuth", "Invalid client authorization.");
    add(627, "errInvalidIdr", "Invalid record id.");
    add(628, "errInvalidChangeAction", "Invalid change action.");
    add(629, "errInvalidDeviceToken", "Invalid device token.");
    add(630, "errInvalidUID", "Invalid UID.");
    add(700, "errAlreadyOn", "Already switched on.");
    add(701, "errAlreadyOff", "Already switched off.");
    add(702, "errAlreadyCallsign", "Callsign already in use, you may have previously signed up on the OpenAPRS website in the past.  Please contact support at ie-support@openaprs.net and we will assist you.");
    add(703, "errAlreadyEmail", "Email address already in use, you may have previously signed up on the OpenAPRS website in the past.  Please contact support at ie-support@openaprs.net and we will assist you.");
    add(704, "errAlreadyActivated", "Account is already activated.");
    add(705, "replyIEUpgraded", "You already had an account with OpenAPRS that matched the username and password you tried to sign up with.  Your OpenAPRS account has been upgraded for use with OpenAPRS iE!  You may still need to check for your activation email to activate your account.");
    add(900, "errConnectionTimeout", "Connection timed out.");
    add(901, "errShutdown", "Shutting down.");
    add(902, "errReload", "Reloading module, please reconnect.");
    //add(903, "errKilled", "Killed.");
    add(950, "errMaintMode", "Our servers are down for maintenance, please try again later.");
    //add(999, "errUnknown", "An unknown error has occured.");

    return;
  } // DCC::DCC

  Reply::~Reply() {
    return;
  } // Reply::~Reply

  const bool Reply::add(const unsigned int number, const string &name, const string &message) {
    bool ret;

    Lock();
    ret = _add(number, name, message);
    Unlock();

    return ret;
  } // Reply::add

  const bool Reply::_add(const unsigned int number,
                        const string &name,
                        const string &message) {
    replyMapType::iterator ptr;
    ReplyMessage r;

    if ((ptr = replyMap.find(name)) != replyMap.end())
      return false;

    r.name = name;
    r.number = number;
    r.message = message;

    replyMap.insert(pair<string, ReplyMessage>(name, r));

    return true;
  } // Reply::add

  const bool Reply::remove(const string &name) {
    bool ret;
    Lock();
    ret = _remove(name);
    Unlock();

    return ret;
  } // Reply::remove

  const bool Reply::_remove(const string &name) {
    replyMapType::iterator ptr;

    if ((ptr = replyMap.find(name)) == replyMap.end())
      return false;

    replyMap.erase(ptr);

    return true;
  } // DCC::_remove

  const unsigned int Reply::clear() {
    unsigned int ret;
    Lock();
    ret = _clear();
    Unlock();
    return ret;
  } // Reply::clear

  const unsigned int Reply::_clear() {
    unsigned int numRemoved;			// number of users removed

    // initialize variables
    numRemoved = replyMap.size();

    // clear client list
    replyMap.clear();

    return numRemoved;
  } // DCC::_clear

  const bool Reply::find(const string &name, string &message) {
    bool ret;
    Lock();
    ret = _find(name, message);
    Unlock();
    return ret;
  } // Reply::find

  const bool Reply::_find(const string &name, string &message) {
    replyMapType::iterator ptr;
    stringstream s;

    if ((ptr = replyMap.find(name)) == replyMap.end())
      return false;

    s.str("");
    s << ptr->second.number << " MS:" << ptr->second.message;

    message = s.str();

    return true;
  } // Reply::_find

  const bool Reply::findByNumeric(const unsigned int number, string &message) {
    bool ret;
    Lock();
    ret = _findByNumeric(number, message);
    Unlock();
    return ret;
  } // Reply::findByNumeric

  const bool Reply::_findByNumeric(const unsigned int number, string &message) {
    replyMapType::iterator ptr;
    stringstream s;

    for(ptr = replyMap.begin();
        ptr != replyMap.end();
        ptr++) {
      if (ptr->second.number == number) {
        s.str("");
        s << ptr->second.number << " " << ptr->second.message;

        message = s.str();

        return true;
      } // if
    } // for

    return false;
  } // Reply::_findByNumeric
} // namespace dcc

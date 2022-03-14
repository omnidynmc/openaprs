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
 $Id: DCCUser.cpp,v 1.14 2005/02/01 22:10:38 omni Exp $
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

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "APRS.h"
#include "DBI.h"
#include "DCC.h"
#include "DCC_Command.h"
#include "DCCUser.h"
#include "Fields.h"
#include "LineBuffer.h"
#include "Network.h"
#include "MemcachedController.h"
#include "PutMySQL.h"
#include "Reply.h"
#include "Server.h"
#include "Symbols.h"
#include "Vars.h"
#include "Worker.h"

#include "openframe/Stopwatch.h"
#include "openframe/StringToken.h"
#include "openframe/StringTool.h"
#include "openframe/VarController.h"

#include "dcc.h"
#include "openaprs.h"
#include "openaprs_string.h"
#include "md5wrapper.h"

namespace dcc {
  using namespace putmysql;
  using namespace std;
  using openaprs::DBI;
  using openaprs::CommandTree;
  using openaprs::Connection;
  using openframe::Stopwatch;
  using openframe::StringToken;
  using openframe::StringTool;
  using openframe::VarController;
  using ::md5wrapper;

  /*****************
   ** Constructor **
   *****************/

  DCCUser::DCCUser(Worker *worker, const int socket, const string &ip, DBI *dbi, MemcachedController *memcached, DCC_Command *command,
                   Reply *reply, Fields *fields, Symbols *symbols,
                   const time_t timeout, const time_t timeCreated) {

    _worker = worker;
    _dbi = dbi;
    _socket = socket;
    _ip = ip;
    _memcached = memcached;
    _command = command;
    _reply = reply;
    _fields = fields;
    _symbols = symbols;
    _timeout = timeout;
    _timeCreated = timeCreated;

    // set defaults
    _bytes = 0;
    _flags = 0;
    _timeFlood = _timePing = _timePong = time(NULL);

    _var = new VarController();
  } // DCCUser::DCCUser

  /****************
   ** Destructor **
   ****************/

  DCCUser::~DCCUser() {
    // unregister user message session
    if (_dbi && IsDCCUserLogin(this) && _var->exists("dcc.user.messaging.nick"))
      _dbi->unregisterMessageSession(id(), _var->get_string("dcc.user.messaging.nick"));

    delete _var;
  } // DCCUser::~DCCUser

  void DCCUser::onDescribeStats() {
  } // DCCUser::onDescribeStats()

  void DCCUser::onDestroyStats() {
  } // DCCUser::onDestroyStats

  /***************
   ** Variables **
   ***************/

  const DCCUser::flagType DCCUser::FLAG_LOGIN		=	0x0001;
  const DCCUser::flagType DCCUser::FLAG_REMOVE		=	0x0002;
  const DCCUser::flagType DCCUser::FLAG_LISTEN		=	0x0004;
  const DCCUser::flagType DCCUser::FLAG_ADMIN		=	0x0008;
  const DCCUser::flagType DCCUser::FLAG_MESSAGING	=	0x0010;
  const DCCUser::flagType DCCUser::FLAG_VERIFIED	=	0x0020;
  const DCCUser::flagType DCCUser::FLAG_LIVE		=	0x0040;
  const DCCUser::flagType DCCUser::FLAG_NMEA		=	0x0080;
  const DCCUser::flagType DCCUser::FLAG_GUEST		=	0x0100;
  const DCCUser::flagType DCCUser::FLAG_LOCAL		=	0x0200;

  /*********************
   ** Connect Members **
   *********************/

  /**
   * DCCUser::Connect
   *
   * Send connect line to User.
   *
   * Returns: Nothing.
   */
  void DCCUser::Connect() {
    _timePing = _timePong = time(NULL);

    return;
  } // DCCUser::Connect

  /**
   * DCCUser::isTimeout
   *
   * Check and see if this connection has timed out.
   *
   */
  const bool DCCUser::chkTimeout() {
    time_t timeout = time_t(_timeout);
    time_t now = time(NULL);
    time_t ping_ts = now - timeout;
    time_t pong_ts = now - (timeout*2);

    // Don't timeout the listener.
    if (!IsDCCUserNormal(this))
      return false;

    // Should we disconnect this user?
    if (_timePong < pong_ts) {
      reply("errConnectionTimeout");
      SetDCCUserRemove(this);
//      _logf("*** Module DCC: Connection timeout for %s@%s",
//                        callsign().c_str(), ip().c_str());
      _dcclogf("Connection timeout for %s@%s",
                        callsign().c_str(), ip().c_str());
      return true;
    } // if

    // Should I send a ping?
    if (_timePing < ping_ts) {
      reply("sendPing");
      _timePing = now;
    } // if

    return false;
  } // DCCUser::chkTimeout

  /**
   * DCCUser::maint_mode
   *
   * Send maint mode message to user and disconnect
   *
   */
  const bool DCCUser::maint_mode() {
    reply("errMaintMode");
      SetDCCUserRemove(this);
      _dcclogf("Maintenance mode message sent to %s@%s",
                        callsign().c_str(), ip().c_str());

    return true;
  } // DCCUser::maint_mode

  /*******************
   ** Login Members **
   *******************/

  /**
   * DCCUser::Login
   *
   * Try to authenticate the user.
   *
   * Returns: Nothing.
   */
  const bool DCCUser::Login(const string &username, const string &password,
                            const string &client) {
    Stopwatch sw;
    DBI::resultMapType userLoginMap;
    bool wasCached = false;

    if (!_dbi)
      return false;

    // Already logged in?
    if (IsDCCUserLogin(this)) {
      reply("replyLoginAlready");
      return false;
    } // if

    sw.Start();

    if ((wasCached = _getLoginFromMemcached(username, password, userLoginMap))) {
//      _logf("*** Module DCC: %s authenticated from cache (%.4f seconds)",
//                        (username.length() ? username.c_str() : ""),
//                        sw.Time());
      _dcclogf("%s authenticated from cache (%.5f seconds)",
                        (username.length() ? username.c_str() : ""),
                        sw.Time());
    } // if
    else if (!_dbi->getUserLogin(username, password, userLoginMap)) {
//      _logf("*** Module DCC: Login attempted failed for %s (%.4f seconds)",
//                        (username.length() ? username.c_str() : ""),
//                        sw.Time());
      _dcclogf("Login attempt failed for %s (%.5f seconds)",
                        (username.length() ? username.c_str() : ""),
                        sw.Time());
      reply("errLogin");
      datapoint("num.login.fail", 1);
      return false;
    } // if

    // Add login to cache
    if (wasCached == false
        && userLoginMap["active"] == "Y"
        && userLoginMap["verified"] == "Y")
      _setLoginInMemcached(username, password, userLoginMap);

    if (userLoginMap["active"] == "N"
        && atoi(userLoginMap["active_until"].c_str()) > time(NULL))
      reply("warnActiveUntil");
    else if (userLoginMap["active"] != "Y") {
      reply("errLoginActive");
      return false;
    } // if

    if (userLoginMap["verified"] == "Y") {
      //reply("errLoginVerify");
      //return false;
      SetDCCUserVerified(this);
    } // if

    if (userLoginMap["local"] == "Y") {
      //reply("errLoginVerify");
      //return false;
      SetDCCUserLocal(this);
    } // if

    SetDCCUserLogin(this);
    _callsign = strtoupper(userLoginMap["callsign"]);
    _id = userLoginMap["id"];
    _client = client;

    _dbi->updateUserLoginTS(atoi(userLoginMap["id"].c_str()));

//    _logf("*** Module DCC: %s authenticated as %s (client is %s) %.4f seconds",
//                      ip().c_str(), callsign().c_str(),
//                      this->client().c_str(), sw.Time());
    _dcclogf("%s authenticated as %s (client is %s) %.5f seconds",
                      ip().c_str(), callsign().c_str(),
                      this->client().c_str(), sw.Time());

    datapoint("num.login.success", 1);
    reply("replyLoginOk");

    if (userLoginMap["admin"] == "Y") {
      SetDCCUserAdmin(this);
      reply("replyLoginAdmin");
    } // if

    if (userLoginMap["guest"] == "Y") {
      SetDCCUserGuest(this);
      reply("replyLoginGuest");
    } // if

    sendf("109 CL:%s\n", userLoginMap["callsign"].c_str());

    return true;
  } // DCCUser::Login

  const bool DCCUser::_getLoginFromMemcached(const string &username, const string &password, DBI::resultMapType &userLoginMap) {
    MemcachedController::memcachedReturnEnum mcr;
    Vars v;
    md5wrapper md5;
    string key;
    string body;
    string buf;

    if (!_isMemcachedOk())
      return false;

    body = aprs::StringTool::toLower(username+":"+password);
    key = md5.getHashFromString(body);

    try {
      mcr = _memcached->get("dcclogin", key, buf);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
    } // catch

    if (mcr != MemcachedController::MEMCACHED_CONTROLLER_SUCCESS)
      return false;

    _debugf("memcached dcclogin found key %s", key.c_str());
    _debugf("memcached dcclogin body: %s", body.c_str());
    _debugf("memcached dcclogin data: %s", buf.c_str());

    v = buf;
    if (!v.exists("ac,ad,au,cl,gu,id,vr"))
      return false;

    userLoginMap.clear();
    userLoginMap["active"] = v["ac"];
    userLoginMap["admin"] = v["ad"];
    userLoginMap["active_until"] = v["au"];
    userLoginMap["callsign"] = v["cl"];
    userLoginMap["guest"] = v["gu"];
    userLoginMap["id"] = v["id"];
    userLoginMap["verified"] = v["vr"];
    userLoginMap["local"] = v["lc"];

    return true;
  } // DCCUser::_getLoginFromMemcached

  const bool DCCUser::_setLoginInMemcached(const string &username, const string &password, DBI::resultMapType &userLoginMap) {
    Vars *v;
    bool isOK = true;
    md5wrapper md5;
    string key;
    string body;
    string buf;

    if (!_isMemcachedOk())
      return false;

    body = aprs::StringTool::toLower(username+":"+password);
    key = md5.getHashFromString(body);

    v = new Vars();
    v->add("ac", userLoginMap["active"]);
    v->add("ad", userLoginMap["admin"]);
    v->add("au", userLoginMap["active_until"]);
    v->add("cl", userLoginMap["callsign"]);
    v->add("gu", userLoginMap["guest"]);
    v->add("id", userLoginMap["id"]);
    v->add("vr", userLoginMap["verified"]);
    v->add("lc", userLoginMap["local"]);

    v->compile(buf, "");
    delete v;

    try {
      _memcached->put("dcclogin", key, buf);
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      isOK = false;
    } // catch

    return isOK;
  } // DCCUser::_setLoginInMemcached

  void DCCUser::Kill(const string &reason) {
    sendf("903 MS:Killed: %s\n", reason.c_str());

    addFlag(DCCUser::FLAG_REMOVE);

    return;
  } // DCCUser::Kill

  void DCCUser::chkFlood() {
    if (IsDCCUserListen(this) == true)
      return;

    if ((bytes() > MODULE_DCC_FLOOD_RATE) &&
        (_timeFlood + MODULE_DCC_FLOOD_TIME) > time(NULL))
      Kill("Excess flood");
    else if ((_timeFlood + MODULE_DCC_FLOOD_TIME) < time(NULL)) {
      _bytes = 0;
      _timeFlood = time(NULL);
    } // else

  } // DCCUser::chkFlood

  /********************
   ** Logout Members **
   ********************/

  /**
   * DCCUser::Logout
   *
   * Logs a User out.
   *
   * Returns: Nothing.
   */
  void DCCUser::Logout() {
    // don't process remove in progress or listening clients
    if (IsDCCUserNormal(this) == false)
      return;

    removeFlag(DCCUser::FLAG_LOGIN);

    return;
  } // DCCUser::Logout


  /*******************
   ** OnLog Members **
   *******************/

  const int DCCUser::OnLog(Log *aLog, const logId callId, const string &logBuffer) {
    return 1;
  } // OnLog

  /*******************
   ** Parse Members **
   *******************/

  /**
   * DCCUser::Parse
   *
   * Parse input from a normal DCCUser (no IRCH).
   *
   * Returns: Number of bytes parsed.
   */
  const unsigned int DCCUser::Parse(const string &dccArguments) {
    Command *tCommand;			// a command
    Packet nPacket;			// command packet
    StringToken commandToken;		// tokenize a string
    Vars v;
    int cmdReturn;			// command return
    string dccMessage;			// body of the message
    string dccRest;			// rest
    string commandTree;			// Command tree to use.
    size_t pos;			// position in string
    unsigned int nodeLevel;		// position in token
    unsigned int numBytes;		// Number of byutes processed.

    // there will always be at least 1 \n
    if (dccArguments.size() < 1)
      return 0;

    _incBytes(dccArguments.size());

    // initialize variables
    dccMessage = dccArguments;

    numBytes = dccArguments.length();

    nPacket.addVariable("dccuser", this);
    nPacket.addVariable("dbi", _dbi);
    nPacket.addVariable("memcached", _memcached);
    nPacket.addVariable("reply", _reply);
    nPacket.addVariable("symbols", _symbols);
    nPacket.addVariable("field", _fields);

    // remove all newline and linefeeds
    for(pos = dccMessage.find("\r"); pos != string::npos; pos = dccMessage.find("\r"))
      dccMessage.replace(pos, 1, "");

    for(pos = dccMessage.find("\n"); pos != string::npos; pos = dccMessage.find("\n"))
      dccMessage.replace(pos, 1, "");

    // just in case the \n was the only character
    if (dccMessage.size() < 1)
      return numBytes;

    commandTree = "DCC";

    /******************
     ** DCC Commands **
     ******************/
    if (dccMessage[0] == '.') {
      dccMessage.replace(0, 1, "");

      // it's a command don't echo
      // we'll parse our comand list here
      nodeLevel = 0;

      commandToken = dccMessage;

      tCommand = _command->findCommand(commandTree, commandToken, nodeLevel, NULL);

      cmdReturn = time(NULL);

      if (tCommand != NULL) {
        // initialize variables
        nPacket.setArguments(commandToken.getRange(nodeLevel, commandToken.size()));
        nPacket.setPath(tCommand->getPath());
        nPacket.setName(tCommand->getName());

        cmdReturn = tCommand->getCommandHandler()->Execute(&nPacket);
      } // if

      switch(cmdReturn) {
        case CMDERR_SYNTAX:
          sendf("200 MS:Syntax error: %s\n", v.Escape(tCommand->getSyntax()).c_str());
          _dcclogf("!SYN DCC[%s@%s] %s", callsign().c_str(), ip().c_str(), (tCommand->findFlag(Command::FLAG_SUPPRESS) == true) ? tCommand->getPath().c_str() : dccMessage.c_str());
          break;
        case CMDERR_SUCCESS:
          _dcclogf("!OK DCC[%s@%s] %s", callsign().c_str(), ip().c_str(), (tCommand->findFlag(Command::FLAG_SUPPRESS) == true) ? tCommand->getPath().c_str() : dccMessage.c_str());
          break;
        case CMDERR_ACCESS:
          reply("errAccessDenied");
          _dcclogf("!ACS DCC[@%s] %s", ip().c_str(), (tCommand->findFlag(Command::FLAG_SUPPRESS) == true) ? tCommand->getPath().c_str() : dccMessage.c_str());
          break;
        default:
          sendf("202 MS:Invalid command: %s\n", v.Escape(dccMessage).c_str());
          _dcclogf("!INV DCC[%s@%s] %s", callsign().c_str(), ip().c_str(), dccMessage.c_str());
          break;
      } // switch
    } // if

    return numBytes;
  } // DCCUser::Parse

  const bool DCCUser::reply(const string &name) {
    string message;

    // Does the error exist?
    if (!_reply->find(name, message)) {
      sendf("999 MS:Unknown error please contact support.\n");
      return false;
    } // if

    sendf("%s\n", message.c_str());

    return true;
  } // DCC::reply

  /*********************
   ** Welcome Members **
   *********************/

  /**
   * DCCUser::Welcome
   *
   * Send welcome to User.
   *
   * Returns: Nothing.
   */
  void DCCUser::Welcome() {
    md5wrapper md5;
    stringstream s;
    int i, r;

    // Create a random hash value for this user now that they are connected.
    srand(time(NULL));
    s.str("");
    s << _ip << _timeCreated;
    for(i=0; i < 32; i++) {
      r = (rand() % 26) + 65;
      s << (char) r;
    } // for

    _seed = md5.getHashFromString(s.str());

    sendf("001 MS:I am a OpenAPRS v%s.\n", VERSION);
    sendf("002 MS:%s\n",
         "Please login by typing ``.LN <email> <password> [client]''");

    return;
  } // DCCUser::Welcome

  /******************
   ** Quit Members **
   ******************/

  /**
   * DCCUser::Quit
   *
   * Cause the User to quit.
   *
   * Returns: Nothing.
   */
  void DCCUser::Quit(const string &quitReason) {

    // don't process remove in progress clients
    if (IsDCCUserNormal(this) == false)
      return;

    return;
  } // DCCUser::Quit

  /******************
   ** Send Members **
   ******************/

  /**
   * DCCUser::Send
   *
   * Sends a message to the client.
   *
   * Returns: Number of bytes sent.
   */
  const unsigned int DCCUser::sendf(const char *sendFormat, ...) {
    char sendBuffer[MAXBUF + 1] = {0};          // Buffer to send to clients.
    string sendString;				// String to send to client.
    va_list va;                                 // Va arguments to compile for vsnprintf.

    // initialize variables
    va_start(va, sendFormat);
    vsnprintf(sendBuffer, sizeof(sendBuffer), sendFormat, va);
    va_end(va);

    send(sendBuffer);

    return strlen(sendBuffer);
  } // DCCUser::Send

  const unsigned int DCCUser::send(const string &buf) {
    _worker->send(_socket, buf);

    return buf.length();
  } // DCCUser::send

  const unsigned int DCCUser::fieldMe(DBI::setMapType &sm, fieldsMapType &fieldsMap,
                                      const string &filter) {
    DBI::resultMapType rm;
    DBI::resultMapType::iterator ptr;
    Vars v;
    string field;
    stringstream s;
    unsigned int i;

    fieldsMap.clear();

    for(i=0; i < sm.size(); i++) {
      rm = sm[i].resultMap;

      // Build results
      s.str("");
      for(ptr = rm.begin(); ptr != rm.end(); ptr++) {
        if (!_fields->findByName(ptr->first, field, filter))
          continue;

        if (s.str().length() > 0)
          s << "|";

        s << field << ":" << v.Escape(ptr->second);
      } // if

      //aUser->sendf("310 %s\n", s.str().c_str());
      fieldsMap.insert(pair<unsigned int, string>(i, s.str()));
    } // for

    return fieldsMap.size();
  } // DCCUser::fieldMe

  const string DCCUser::fieldMe(DBI::resultMapType &rm,
                                const string &filter) {
    DBI::resultMapType::iterator ptr;
    Vars v;
    string field;
    stringstream s;

    // Build results
    s.str("");
    for(ptr = rm.begin(); ptr != rm.end(); ptr++) {
      if (!_fields->findByName(ptr->first, field, filter))
        continue;

      if (s.str().length() > 0)
        s << "|";

      s << field << ":" << v.Escape(ptr->second);
    } // if

    return s.str();
  } // DCCUser::fieldMe

  const bool DCCUser::setMessageSessionInMemcached(const string &source) {
    string key = aprs::StringTool::toUpper(source);
    bool isOK = true;

    if (!_isMemcachedOk())
      return false;

    try {
      _memcached->put("ack", key, "1");
    } // try
    catch(MemcachedController_Exception e) {
      _log(e.message());
      //_timeNextMemcachedTry = time(NULL) + 60;
      isOK = false;
    } // catch

    return isOK;
  } // setMessageSessionInMemcached
} // namespace dcc

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
 $Id: DB.cpp,v 1.2 2005/12/13 21:07:03 omni Exp $
 **************************************************************************/

#include <list>
#include <new>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#include "App_Log.h"
#include "APRS.h"
#include "DBI.h"
#include "Create.h"
#include "Server.h"
#include "Send.h"

#include "SSQLS.h"

#include "config.h"
#include "openaprs.h"
#include "openaprs_string.h"
#include "md5wrapper.h"

namespace openaprs {

using std::string;
using std::list;
using std::ofstream;
using std::ostream;
using std::stringstream;
using std::ios;
using std::endl;
using std::cout;
using namespace putmysql;
using aprs::StringTool;

/**************************************************************************
 ** DB Class                                                             **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

DBI::DBI() {
  logger(new App_Log(""));

  try {
    _sqlpp = new mysqlpp::Connection(true);
    _sqlpp->set_option(new mysqlpp::ReconnectOption(true));
    _sqlpp->set_option(new mysqlpp::MultiResultsOption(true));
  } // try
  catch(bad_alloc xa) {
    assert(false);
  } // catch

  // initialize ssqls tables
  SQL_Callsign::table("callsign");

  connect();

  CREATE_PUTMYSQL(_sql, OPENAPRS_SQL_LOGIN);

  aprsmail = new DBI::APRSMail(this, _sql);
  inject = new DBI::Inject(this, _sql, _sqlpp);
  create = new DBI::Create(this, _sql);
  email = new DBI::Email(this, _sql);
  position = new DBI::Position(this, _sql);
  message = new DBI::Message(this, _sql);
  extra = new DBI::Extra(this, _sql);
  signup = new DBI::Signup(this, _sql);
  telemetry = new DBI::Telemetry(this, _sql);
  uls = new DBI::Uls(this, _sql);
  weather = new DBI::Weather(this, _sql);
} // DBI::DB

DBI::~DBI() {
  if (_sqlpp->connected())
    _sqlpp->disconnect();

  delete aprsmail;
  delete inject;
  delete create;
  delete email;
  delete extra;
  delete message;
  delete position;
  delete signup;
  delete uls;
  delete telemetry;
  delete weather;
  delete _sql;
  delete _sqlpp;
} // DBI::~DBI

/***************
 ** Variables **
 ***************/

  const bool DBI::ping() {
    bool ret = true;
    if (!_sqlpp->ping()) {
      _logf("*** SQL Error: Failed to ping server.");
    } // if

    return ret;
  } // DBI::ping

  const bool DBI::connect() {
    _logf("*** Connecting to MySQL server %s as %s", OPENAPRS_SQL_HOST.c_str(), OPENAPRS_SQL_USER.c_str());
    try {
      _sqlpp->connect(OPENAPRS_SQL_DB.c_str(), OPENAPRS_SQL_HOST.c_str(), OPENAPRS_SQL_USER.c_str(), OPENAPRS_SQL_PASS.c_str());
    } // try
    catch(mysqlpp::ConnectionFailed e) {
      _logf("*** SQL Error: #%d %s", e.errnum(), e.what());
      return false;
    } // if

    return true;
  } // DBI::connect()

  const DBI::resultSizeType DBI::query(const string &sqls, DBI::resultType &res) {
    queryType query = _sqlpp->query(sqls);

    try {
      res = query.store();
    } // try
    catch(mysqlpp::BadQuery e) {
      _logf("*** SQL Error: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.num_rows();
  } // DBI::query

  void DBI::print(const string &label, resultType &res) {
    string sep = "";
    string fieldSep = "";
    string name;
    string value;
    string l = "";
    size_t maxFieldLen = 0;
    size_t maxValueLen = 0;
    size_t i;
    resultSizeType j;

    for(i=0; i < res.num_fields(); i++)
      maxFieldLen = DBI_MAX(res.field_name(i).length(), maxFieldLen);

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++)
        maxValueLen = DBI_MAX(res[j][res.field_name(i).c_str()].length(), maxValueLen);
    } // for

    maxFieldLen += 1;
    maxValueLen += 1;

    sep = "+";
    StringTool::pad(sep, "-", maxFieldLen+maxValueLen+4);
    sep += "+";

    fieldSep = "+";
    StringTool::pad(fieldSep, "-", maxFieldLen+2);
    fieldSep += "+";
    StringTool::pad(fieldSep, "-", maxFieldLen+maxValueLen+4);
    fieldSep += "+";

    l = label;
    StringTool::pad(l, " ", maxFieldLen+maxValueLen+1);

    // loop through result
    stringstream s;
    s << sep << endl;
    s << "| " << l << " |" << endl;
    s << fieldSep << endl;
    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        name = res.field_name(i);
        value = res[j][res.field_name(i).c_str()].c_str();
        StringTool::pad(name, " ", maxFieldLen);
        StringTool::pad(value, " ", maxValueLen);
        s << "| " << name << "| " << value << "|" << endl;
      } // for
    } // for
    s << fieldSep << endl;

    cout << s.str();
  } // DBI::print

  void DBI::makeListFromField(const string &name, resultType &res, string &list) {
    resultSizeType j;

    list = "";

    for(j=0; j < res.num_rows(); j++) {
      if (list.length())
        list += ", ";
      list += res[j][name.c_str()].c_str();
    } // for
  } // DBI::makeListFromField

  void DBI::print(DBI::resultType &res) {
    map<size_t, size_t> maxFieldLen;
    resultSizeType j;
    size_t maxRowLen = 0;
    size_t fieldLen = 0;
    size_t i;
    string sep = "";
    string fieldSep = "";
    string field;
    string value;

    for(i=0; i < res.num_fields(); i++) {
      // populate map with head size
      fieldLen = res.field_name(i).length();
      maxFieldLen[i] = res.field_name(i).length();
    } // for

    for(j=0; j < res.num_rows(); j++) {
      for(i=0; i < res.num_fields(); i++) {
        fieldLen = res[j][res.field_name(i).c_str()].length();

        // compare existing field lengths to find max
        maxFieldLen[i] = DBI_MAX(maxFieldLen[i], fieldLen);
      } // for
    } // for

    // create the sep
    for(i=0; i < maxFieldLen.size(); i++) {
      fieldSep = "+";
      StringTool::pad(fieldSep, "-", maxFieldLen[i]+3);
      sep += fieldSep;
      maxRowLen += maxFieldLen[i];
    } // for
    sep += "+";

    maxRowLen += (maxFieldLen.size()-1) * 3;

    stringstream s;
    s << sep << endl;
    s << "| ";
    for(i=0; i < res.num_fields(); i++) {
      field = res.field_name(i);
      StringTool::pad(field, " ", maxFieldLen[i]);
      if (i)
        s << " | ";
      s << field;
    } // for
    s << " |" << endl;
    s << sep << endl;

    // now one more loop to print out stuffs
    for(j=0; j < res.num_rows(); j++) {
      // loop for newlines and try not to screw up the formatting
      size_t newlines = 0;
      for(size_t n=0; n == 0 || newlines; n++) {
        stringstream ss;
        newlines = 0;
        ss << "| ";
        for(i=0; i < res.num_fields(); i++) {
          if (lineForNewlineRow(res[j][i].c_str(), value, n))
            newlines++;

          StringTool::pad(value, " ", maxFieldLen[i]);
          if (i)
            ss << " | ";
          ss << value;
        } // for
        ss << " |" << endl;

        if (newlines)
          s << ss.str();
      } // for

    } // for
    s << sep << endl;
    s << res.num_rows() << " row" << (res.num_rows() == 1 ? "" : "s") << " in set" << endl << endl;

    cout << s.str();
  } // DBI::print

  const size_t DBI::maxNewlineLen(const string &buf) {
    StringToken st;
    size_t len = 0;

    st.setDelimiter('\n');
    st = buf;

    if (!st.size())
      return 0;

    for(size_t i=0; i < st.size(); i++)
      len = DBI_MAX(len, st[i].length());

    return len;
  } // DBI::maxNewlineLen

  const bool DBI::lineForNewlineRow(const string &buf, string &ret, const size_t i) {
    StringToken st;

    st.setDelimiter('\n');
    st = buf;

    if (i >= st.size()) {
      ret = "";
      return false;
    } // if

    ret = st[i];

    return true;
  } // DBI::lineForNewlineRow


  /**********************
   ** Wildcard Members **
   **********************/

  const string DBI::Wildcard(const string &parseMe) {
    bool isEscaped = false;
    string ret;
    size_t pos;

    ret = "";

    for(pos=0; pos < parseMe.length(); pos++) {
      if (!isEscaped && parseMe[pos] == '*')
        ret += string("%");
      else if (!isEscaped && parseMe[pos] == '?')
        ret += string("_");
      else if (!isEscaped
               && (parseMe[pos] == '%' || parseMe[pos] == '_'))
        ret += string("\\") + parseMe[pos];
      else if (!isEscaped && parseMe[pos] == (char) 92)
        isEscaped = true;
      else {
        ret += parseMe[pos];
        isEscaped = false;
      } // else
    } // for

    return ret;
  } // DBI::Wildcard

  const unsigned int DBI::update(const string &query) {
    bool isOK;
    unsigned int numRows = 0;

    isOK = _sql->query(query.c_str(), query.length());

    if (isOK)
      numRows = _sql->num_affected_rows();
    else
      _logf("*** SQL Error: #%d %s", _sql->getErrorNumber(), _sql->getErrorMessage().c_str());

    return numRows;
  } // DBI::update

  const unsigned int DBI::fetchRowsAndCount(const string &count, const string &query, setMapType &setMap) {
    Set st;
    bool isOk;
    string field, value;
    unsigned int numRows = 0;
    unsigned int i,j;

    // initialize variables
    setMap.clear();
    st.resultMap.clear();

    isOk = _sql->queryf("%s", count.c_str());

    if (isOk)
      while((*_sql)++)
        numRows = atoi((*_sql)["count"]);

    isOk = _sql->queryf("%s", query.c_str());

    if (isOk) {
      for(j=0;(*_sql)++;j++) {
        st.resultMap.clear();

        for(i=0; i < _sql->num_fields(); i++) {
          field = _sql->getFieldName(i);

          if ((*_sql)[field.c_str()] == NULL)
            continue;

          if (strlen((*_sql)[field.c_str()]) == 1)
            value = string((*_sql)[field.c_str()]);
          else
            value = _sql->Unescape((*_sql)[field.c_str()]);

          st.resultMap.insert(pair<string, string>(field, value));
        } // for

        setMap.insert(pair<unsigned int, Set>(j, st));
      } // while
    } // if
    else
      _logf("*** SQL Error: #%d %s", _sql->getErrorNumber(), _sql->getErrorMessage().c_str());

    return numRows;
  } // DBI::fetchRowsAndCount

  const unsigned int DBI::fetchRows(const string &query, setMapType &setMap) {
    Set st;
    bool isOk;
    string field, value;
    unsigned int i,j;

    // initialize variables
    setMap.clear();
    st.resultMap.clear();

    isOk = _sql->queryf("%s", query.c_str());

    if (isOk) {
      for(j=0;(*_sql)++;j++) {
        st.resultMap.clear();

        for(i=0; i < _sql->num_fields(); i++) {
          field = _sql->getFieldName(i);

          if ((*_sql)[field.c_str()] == NULL)
            continue;

          if (strlen((*_sql)[field.c_str()]) == 1)
            value = string((*_sql)[field.c_str()]);
          else
            value = _sql->Unescape((*_sql)[field.c_str()]);

          st.resultMap.insert(pair<string, string>(field, value));
        } // for

        setMap.insert(pair<unsigned int, Set>(j, st));
      } // while
    } // if
    else
      _logf("*** SQL Error: #%d %s", _sql->getErrorNumber(), _sql->getErrorMessage().c_str());

    return setMap.size();
  } // DBI::fetchRows

/*********************
 ** Message Members **
 *********************/

const unsigned int DBI::lastMessageID(const string &source, string &id) {
  int numRows;

  // try and get stations last message id
  _sql->queryf("SELECT lm.id FROM last_message lm LEFT JOIN callsign c ON c.id = lm.callsign_id WHERE c.source='%s' LIMIT 1",
              _sql->Escape(source).c_str());

  numRows = _sql->num_rows();

  while((*_sql)++) {
    if ((*_sql)["id"] != NULL)
      id = (*_sql)["id"];
    else
      id = "";
  } // while

  return (numRows > 0) ? true : false;
} // DBI::lastMessageID

const unsigned int DBI::nextSendMessageID(const string &source) {
  unsigned int id = 0;

  if (_sql->queryf("SELECT msgnum FROM create_message cm WHERE source='%s' ORDER BY msgnum DESC LIMIT 1",
              _sql->Escape(source).c_str())) {
    while((*_sql)++)
      id = atoi((*_sql)["msgnum"])+1;
  } // if

  if (id > 0)
    id = id % 1295;

  return id;
} // DBI::nextSendMessageID

const bool DBI::getMessageDecayID(const string &source,
                                  const string &target,
                                 const string &ack,
                                 string &decayId) {
  unsigned int numRows = 0;

  if (_sql->queryf("SELECT decay_id FROM create_message cm WHERE source='%s' AND target='%s' AND msgack='%s' AND ack='N' LIMIT 1",
              _sql->Escape(source).c_str(),
              _sql->Escape(target).c_str(),
              _sql->Escape(ack).c_str())) {

    numRows = _sql->num_rows();

    while((*_sql)++)
      decayId = (*_sql)["decay_id"];

  } // if

  return (numRows > 0) ? true : false;
} // DBI::getMessageDecayID

const bool DBI::registerMessageSession(const string &id, const string &callsign) {
  unsigned int numMessage = 0;
  unsigned int numAffectedRows = 0;
  time_t create_ts = time(NULL);

  numMessage = getLastMessageID(id, callsign);

  if (_sql->queryf("REPLACE INTO session_message (user_id, callsign, num_message, create_ts) VALUES ('%s', '%s', '%d', '%d')",
                 _sql->Escape(id).c_str(),
                 _sql->Escape(callsign).c_str(),
                 numMessage,
                 create_ts)) {

    numAffectedRows = _sql->num_affected_rows();
  } // if

  return (numAffectedRows > 0) ? true : false;
} // DBI::registerMessageSession

const bool DBI::unregisterMessageSession(const string &id, const string &callsign) {
  unsigned int numAffectedRows = 0;
  time_t past_ts = time(NULL) - 86400;

  if (_sql->queryf("DELETE FROM session_message WHERE user_id='%s' AND callsign='%s' AND create_ts > '%d'",
                   _sql->Escape(id).c_str(),
                   _sql->Escape(callsign).c_str(),
                   past_ts)) {

    numAffectedRows = _sql->num_affected_rows();
  } // if

  return (numAffectedRows > 0) ? true : false;
} // DBI::unregisterMessageSession

const unsigned int DBI::getLastMessageID(const string &id, const string &callsign) {
  unsigned int numMessage = 0;

  if (_sql->queryf("SELECT num_message FROM session_message WHERE user_id='%s' AND callsign='%s'",
                   _sql->Escape(id).c_str(),
                   _sql->Escape(callsign).c_str())) {

    while((*_sql)++)
      numMessage = atoi((*_sql)["num_message"]);
  } // if

  return numMessage;
} // DBI::getLastMessageID

const unsigned int DBI::incLastMessageID(const string &id, const string &callsign) {
  unsigned int numAffectedRows = 0;

  if (_sql->queryf("UPDATE session_message SET num_message=(num_message+1) WHERE user_id='%s' AND callsign='%s'",
                   _sql->Escape(id).c_str(),
                   _sql->Escape(callsign).c_str())) {

    numAffectedRows = _sql->num_affected_rows();
  } // if

  return (numAffectedRows > 0) ? true : false;
} // DBI::incLastMessageID

const unsigned int DBI::setMessageAck(const string &source, const string &target, const string &ack) {
  unsigned int numAffectedRows = 0;
  bool isOK;

  isOK = _sql->queryf("UPDATE create_message SET ack='Y', ack_ts='%d' WHERE source='%s' AND target='%s' AND msgack='%s' AND ack='N'",
                     time(NULL),
                     _sql->Escape(target).c_str(),
                     _sql->Escape(source).c_str(),
                     _sql->Escape(ack).c_str());

  if (isOK)
    numAffectedRows = _sql->num_affected_rows();

  return (numAffectedRows > 0) ? true : false;
} // DBI::setMessageAck

/**
 * DBI::getUserLogin
 *
 * Verify a username and password and return associated
 * user data from database.
 *
 * Returns: true on success, false on failure.
 *
 */
const bool DBI::getUserLogin(const string &username,
                            const string &password,
                            resultMapType &user) {
  string field;
  unsigned int numRows = 0;
  unsigned int i;

  // initialize variables
  user.clear();

  if (_sql->queryf("SELECT id, callsign, verified, active, active_until, admin, guest, local, email, md5 FROM web_users WHERE email='%s' AND md5=MD5('%s') LIMIT 1",
                   _sql->Escape(username).c_str(),
                   _sql->Escape(password).c_str())) {

    numRows = _sql->num_rows();

    while((*_sql)++) {
      for(i=0; i < _sql->num_fields(); i++) {
        field = _sql->getFieldName(i);

        if ((*_sql)[field.c_str()] == NULL)
          continue;

        user.insert(pair<string, string>(field, (*_sql)[field.c_str()]));
      } // for
    } // while
  } // if

  return (numRows > 0) ? true : false;
} // DBI::getUserLogin

/**
 * DBI::updateUserLoginTS
 *
 * Verify a username and password and return associated
 * user data from database.
 *
 * Returns: number of rows affected.
 *
 */
const unsigned int DBI::updateUserLoginTS(const int id) {
  bool isOK = false;
  unsigned int numAffectedRows = 0;
  time_t login_ts = time(NULL);

  isOK = _sql->queryf("UPDATE web_users SET login_ts = '%d' WHERE id='%d'",
                    login_ts,
                    id);

  numAffectedRows = _sql->num_affected_rows();

  return numAffectedRows;
} // DBI::updateUserLoginTS

/**
 * DBI::getUserByCallsign
 *
 * Grab user data by callsign.
 *
 * Returns: true on success, false on failure.
 *
 */
const bool DBI::getUserByCallsign(const string &callsign, resultMapType &user) {
  string field;
  unsigned int numRows = 0;
  unsigned int i;

  // initialize variables
  user.clear();

  if (_sql->queryf("SELECT id, callsign, verified, active, admin, guest FROM web_users WHERE callsign='%s' LIMIT 1",
                   _sql->Escape(callsign).c_str())) {

    numRows = _sql->num_rows();

    while((*_sql)++) {
      for(i=0; i < _sql->num_fields(); i++) {
        field = _sql->getFieldName(i);

        if ((*_sql)[field.c_str()] == NULL)
          continue;

        user.insert(pair<string, string>(field, (*_sql)[field.c_str()]));
      } // for
    } // while
  } // if

  return (numRows > 0) ? true : false;
} // DBI::getUserByCallsign

/**
 * DBI::getUserOptions
 *
 * Grab a users account options from the database
 * and default any missing options.
 *
 * Returns: true on success, false on failure.
 *
 */
const bool DBI::getUserOptions(const int id, resultMapType &user) {
  string field;
  unsigned int numRows = 0;

  // initialize variables
  user.clear();

  // Grab the current list of user options for user.
  if (_sql->queryf("SELECT name, value FROM web_users_options WHERE user_id='%d'", id)) {

    numRows = _sql->num_rows();

    while((*_sql)++)
      user.insert(pair<string, string>((*_sql)["name"], (*_sql)["value"]));
  } // if

  // Now set any options that didn't exist with default values.
  if (_sql->queryf("SELECT name, value FROM web_users_defaults",
                 id)) {

    while((*_sql)++) {
      if (user.find((*_sql)["name"]) == user.end())
        user.insert(pair<string, string>((*_sql)["name"], (*_sql)["value"]));
    } // while
  } // if

  return (numRows > 0) ? true : false;
} // DBI::getUserOptions

const bool DBI::getObjectByName(const string &name, resultMapType &object) {
  string field, value;
  unsigned int numRows = 0;
  unsigned int i;

  // initialize variables
  object.clear();

  if (_sql->queryf("SELECT c.source, lp.name, lp.destination, lp.latitude, lp.longitude, lp.symbol_table, lp.symbol_code, s.body AS status FROM lastposition lp LEFT JOIN callsign c ON c.id = lp.callsign_id LEFT JOIN statuses s ON s.id = lp.status_id WHERE lp.name='%s' AND lp.type='O' LIMIT 1",
                 _sql->Escape(name).c_str())) {

    numRows = _sql->num_rows();

    while((*_sql)++) {
      for(i=0; i < _sql->num_fields(); i++) {
        field = _sql->getFieldName(i);

        if ((*_sql)[field.c_str()] == NULL)
          continue;

        value = (*_sql)[field.c_str()];
        object.insert(pair<string, string>(field, value));
      } // for
    } // while
  } // if

  return (numRows > 0) ? true : false;
} // DBI::getObjectByName

const unsigned int DBI::getMessagesByCallsign(const string &callsign,
                                              const messageMatchEnum match,
                                              const time_t past_ts,
                                              setMapType &messages) {
  Set st;
  bool isOk;
  string field;
  unsigned int numRows = 0;
  unsigned int i,j;

  // initialize variables
  messages.clear();
  st.resultMap.clear();

  if (match == TO)
    isOk = _sql->queryf("SELECT c.source, m.addressee, m.text, m.create_ts FROM messages m LEFT JOIN callsign c ON c.id = m.callsign_id WHERE m.addressee = '%s' AND m.create_ts > '%d' ORDER BY m.create_ts ASC",
                   _sql->Escape(callsign).c_str(),
                   past_ts);
  else if (match == FROM)
    isOk = _sql->queryf("SELECT c.source, m.addressee, m.text, m.create_ts FROM messages m LEFT JOIN callsign c ON c.id = m.callsign_id WHERE c.source = '%s' AND m.create_ts > '%d' ORDER BY m.create_ts ASC",
                   _sql->Escape(callsign).c_str(),
                   past_ts);
  else
    isOk = _sql->queryf("SELECT c.source, m.addressee, m.text, m.create_ts FROM messages m LEFT JOIN callsign c ON c.id = m.callsign_id WHERE (c.source = '%s' OR m.addressee = '%s') AND m.create_ts > '%d' ORDER BY m.create_ts ASC",
                   _sql->Escape(callsign).c_str(),
                   _sql->Escape(callsign).c_str(),
                   past_ts);

  if (isOk) {
    numRows = _sql->num_rows();

    for(j=0;(*_sql)++;j++) {
      st.resultMap.clear();

      for(i=0; i < _sql->num_fields(); i++) {
        field = _sql->getFieldName(i);

        if ((*_sql)[field.c_str()] == NULL)
          continue;

        st.resultMap.insert(pair<string, string>(field, _sql->Unescape((*_sql)[field.c_str()])));
      } // for

      messages.insert(pair<unsigned int, Set>(j, st));
    } // while
  } // if

  return numRows;
} // DBI::getMessagesByCallsign

const bool DBI::doKillObject(const string &source, const string &name) {
  resultMapType object;
  string decayId;
  time_t create_ts = time(NULL);
  unsigned int numAffectedRows = 0;

  if (!getObjectByName(name, object))
    return false;

  // try and find a decay id if it exists
  decayId = "";

  if (getObjectDecayID(name, decayId));
    app->removeDecayById(decayId);


  if (_sql->queryf("REPLACE INTO create_object (name, source, destination, latitude, longitude, symbol_table, symbol_code, status, type, `kill`, decay_id, broadcast_ts, create_ts) VALUES ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 'O', 'Y', '%s', '0', '%d')",
                   _sql->Escape(name).c_str(),
                   _sql->Escape(source).c_str(),
                   _sql->Escape(object["destination"]).c_str(),
                   _sql->Escape(object["latitude"]).c_str(),
                   _sql->Escape(object["longitude"]).c_str(),
                   _sql->Escape(object["symbol_table"]).c_str(),
                   _sql->Escape(object["symbol_code"]).c_str(),
                   _sql->Escape(object["status"]).c_str(),
                   _sql->Escape(decayId).c_str(),
                   create_ts)
                  ) {

    numAffectedRows = _sql->num_affected_rows();
  } // if

  return (numAffectedRows > 0) ? true : false;
} // DBI::doKillObject

const bool DBI::writeError(const string &message) {
  unsigned int numRows = 0;

  if (_sql->queryf("INSERT INTO daemon_error (message, create_ts) VALUES ('%s', '%d')",
              _sql->Escape(message).c_str(),
              time(NULL))) {

    numRows = _sql->num_affected_rows();
  } // if

  return (numRows > 0) ? true : false;
} // DBI::writeError

const bool DBI::getObjectDecayID(const string &name, string &decayId) {
  unsigned int numRows = 0;

  if (_sql->queryf("SELECT decay_id FROM create_object WHERE name='%s' AND create_ts > '%d' LIMIT 1",
              _sql->Escape(name).c_str(), time(NULL) - 14400)) {

    numRows = _sql->num_rows();

    while((*_sql)++)
      decayId = (*_sql)["decay_id"];

  } // if

  return (numRows > 0) ? true : false;
} // DBI::getObjectDecayID

const bool DBI::tryVerify(const string &source, const string &id, const string &key, string &ret) {
  // initialize variables
  ret="Invalid arguments.";

  if (source.length() > 9 || source.length() < 1)
    return false;

  if (id.length() < 1)
    return false;

  if (key.length() != 8)
    return false;

  // try and detect resends
  if (_sql->queryf("SELECT id FROM web_users WHERE id='%s' AND msgchecksum=MD5('%s') AND callsign='%s' AND guest='N' LIMIT 1",
                  _sql->Escape(id).c_str(),
                  _sql->Escape(key).c_str()),
                  _sql->Escape(source).c_str()) {
    if (_sql->num_rows() > 0) {
      ret = "Resend detected and ignored, already replied.";
      return false;
    } // if
  } // if

  // update msgchecksum to prevent future resend problems
  _sql->queryf("UPDATE web_users SET msgchecksum=MD5('%s') WHERE id='%s' AND callsign='%s' AND guest='N'",
              _sql->Escape(key).c_str(),
              _sql->Escape(id).c_str(),
              _sql->Escape(source).c_str());

  // Is this user already verified?
  if (_sql->queryf("SELECT id FROM web_users WHERE id='%s' AND verified='Y' AND callsign='%s' AND guest='N' LIMIT 1",
                 id.c_str(), _sql->Escape(source).c_str())) {
    if (_sql->num_rows() > 0) {
      ret = "Already verified.";
      return true;
    } // if
  } // if

  if (_sql->queryf("UPDATE web_users SET verified_ts='%d', email_verified='Y', verified='Y' WHERE id='%s' AND verified='N' AND callsign='%s' AND verify='%s' AND verify_ts > '%d' AND guest='N'",
                  time(NULL),
                  _sql->Escape(id).c_str(),
                  _sql->Escape(source).c_str(),
                  _sql->Escape(key).c_str(),
                  time(NULL))) {
    if (_sql->num_affected_rows() > 0)
      ret = "Verification successful!";
    else
      ret = "Verification failed, check key and try again.";
  } // if
  else
    ret = "Unknown error occurred.";

  return true;
} // DBI::tryVerify

/**
 * DBI::createMessage
 *
 * Try and create a message from daemon.
 *
 */
const bool DBI::createMessage(const string &source,
                             const string &target,
                             const string &message,
                             const bool local) {
  unsigned int id;

  // try and get last message ID for source.
  id = nextSendMessageID(OPENAPRS_CONFIG_CREATE);

  return createMessage(source, target, message, id, local);
} // DBI::createMessage

/**
 * DBI::createMessage
 *
 * Try and create message.
 *
 */
const bool DBI::createMessage(const string &source,
                             const string &target,
                             const string &message,
                             const unsigned int id,
                             const bool local) {
  aprs::Create c;
  int numAffectedRows = 0;
  string msgack;
  stringstream s;

  if (source.length() > 9 || source.length() < 1)
    return false;

  if (target.length() > 9 || target.length() < 1)
    return false;

  if (source == target)
    return false;

  if (message.length() > 512 || message.length() < 1)
    return false;

  if (!c.encodeMessageID(id, msgack))
    return false;

  // create our message
  s << message << "{" << msgack << "}";

  if (_sql->queryf("INSERT INTO create_message (source, target, message, msgnum, msgack, create_ts, local) VALUES ('%s', '%s', '%s', '%d', '%s', '%d', '%s')",
                  _sql->Escape(source).c_str(),
                  _sql->Escape(target).c_str(),
                  _sql->Escape(s.str()).c_str(),
                  id,
                  msgack.c_str(),
                  time(NULL),
                  (local) ? "Y" : "N")) {
    numAffectedRows = _sql->num_affected_rows();
  } // if
  else
    _logf("*** SQL: Could not insert message for %s", source.c_str());

  return (numAffectedRows > 0) ? true : false;
} // DBI::createMessage

/**************************
 ** User Session Members **
 **************************/

const bool DBI::isUserSession(const string &source, const time_t timeout) {
  int numRows;

  _sql->queryf("SELECT user_id FROM session_message WHERE callsign='%s' AND create_ts > '%d' LIMIT 1",
               _sql->Escape(source).c_str(),
               (time(NULL)-timeout));

  numRows = _sql->num_rows();

  return (numRows > 0) ? true : false;
} // DBI::isUserSession

  /**************************************************************************
   ** DBI::Create Class                                                     **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  DBI::Inject::Inject(DBI *dbi, PutMySQL *sql, mysqlpp::Connection *sqlpp) : _dbi(dbi), _sql(sql), _sqlpp(sqlpp) {
    // build queries
    //_query["getCallsignId"] << "SELECT id FROM callsign WHERE source='%0q:source' LIMIT 1",
  } // DBI::Inject::Inject

  DBI::Inject::~Inject() {
  } // DBI::Inject::~Inject

  const bool DBI::Inject::getCallsignId(const string &source, string &id) {
    int numRows = 0;

    // try and get stations last message id
    //_sql->queryf("SELECT id FROM callsign WHERE source='%s' LIMIT 1",
    //            _sql->Escape(source).c_str());

    try {
      mysqlpp::Query query = _sqlpp->query("SELECT id FROM callsign WHERE source=%0q:source LIMIT 1");
      query.parse();

      mysqlpp::StoreQueryResult res = query.store(source);

      for(size_t i=0; i < res.num_rows();  i++)
        id = res[i][0].c_str();

      numRows = res.num_rows();
    } // try
    catch(const mysqlpp::BadQuery &e) {
      _dbi->_logf("*** MySQL++ Error{getCallsignId}: #%d %s", e.errnum(), e.what());
    } // catch
    catch(const mysqlpp::Exception &e) {
      _dbi->_logf("*** MySQL++ Error{getCallsignId}: %s", e.what());
    } // catch

    return (numRows > 0) ? true : false;
  } // DBI::Inject::getCallsignId

  const bool DBI::Inject::insertCallsign(const string &source, string &id) {
    mysqlpp::SimpleResult res;
    int numRows = 0;
    stringstream s;

    s.str("");
    id = "";

    try {
      mysqlpp::Query query = _sqlpp->query();
      query << "INSERT IGNORE INTO callsign (source, create_ts) VALUES (UPPER(%0q:source), %1:timestamp)";
      query.parse();
      res = query.execute(source, time(NULL));
      numRows = res.rows();
    } // try
    catch(mysqlpp::BadQuery e) {
      _dbi->_logf("*** MySQL++ Error{insertCallsign}: #%d %s", e.errnum(), e.what());
      return false;
    } // catch
    catch(mysqlpp::Exception e) {
      _dbi->_logf("*** MySQL++ Error{insertCallsign}: %s", e.what());
      return false;
    } // catch

    if (numRows) {
      if (res.insert_id() == 0)
        return false;
      s << res.insert_id();
      id = s.str();
    } // if

    return (numRows > 0) ? true : false;
  } // DBI::Inject::insertCallsign

  const bool DBI::Inject::getPathId(const string &hash, string &id) {
    int numRows = 0;

    try {
      mysqlpp::Query query = _sqlpp->query("SELECT id FROM path WHERE hash=%0q:hash LIMIT 1");
      query.parse();

      mysqlpp::StoreQueryResult res = query.store(hash);

      for(size_t i=0; i < res.num_rows();  i++)
        id = res[i][0].c_str();

      numRows = res.num_rows();
    } // try
    catch(const mysqlpp::BadQuery &e) {
      _dbi->_logf("*** MySQL++ Error{getPathId}: #%d %s", e.errnum(), e.what());
    } // catch
    catch(const mysqlpp::Exception &e) {
      _dbi->_logf("*** MySQL++ Error{getPathId}: %s", e.what());
    } // catch

    return (numRows > 0) ? true : false;
  } // DBI::Inject::getPathId

  const bool DBI::Inject::insertPath(const string &hash, const string &body, string &id) {
    mysqlpp::SimpleResult res;
    int numRows = 0;
    stringstream s;

    s.str("");
    id = "";

    try {
      mysqlpp::Query query = _sqlpp->query();
      query << "INSERT IGNORE INTO path (hash, body, create_ts) VALUES (%0q:hash, %1q:body, UNIX_TIMESTAMP())";
      query.parse();
      res = query.execute(hash, body);
      numRows = res.rows();
    } // try
    catch(mysqlpp::BadQuery e) {
      _dbi->_logf("*** MySQL++ Error{insertPath}: #%d %s", e.errnum(), e.what());
      return false;
    } // catch
    catch(mysqlpp::Exception e) {
      _dbi->_logf("*** MySQL++ Error{insertPath}: %s", e.what());
      return false;
    } // catch

    if (numRows) {
      if (res.insert_id() == 0)
        return false;
      s << res.insert_id();
      id = s.str();
    } // if

    return (numRows > 0) ? true : false;
  } // DBI::Inject::insertPath

  const bool DBI::Inject::getStatusId(const string &hash, string &id) {
    int numRows = 0;

    try {
      mysqlpp::Query query = _sqlpp->query("SELECT id FROM statuses WHERE hash=%0q:hash LIMIT 1");
      query.parse();

      mysqlpp::StoreQueryResult res = query.store(hash);

      for(size_t i=0; i < res.num_rows();  i++)
        id = res[i][0].c_str();

      numRows = res.num_rows();
    } // try
    catch(const mysqlpp::BadQuery &e) {
      _dbi->_logf("*** MySQL++ Error{getStatusId}: #%d %s", e.errnum(), e.what());
    } // catch
    catch(const mysqlpp::Exception &e) {
      _dbi->_logf("*** MySQL++ Error{getStatusId}: %s", e.what());
    } // catch

    return (numRows > 0) ? true : false;
  } // DBI::Inject::getPathId

  const bool DBI::Inject::insertStatus(const string &hash, const string &body, string &id) {
    mysqlpp::SimpleResult res;
    int numRows = 0;
    stringstream s;

    s.str("");
    id = "";

    try {
      mysqlpp::Query query = _sqlpp->query();
      query << "INSERT IGNORE INTO statuses (hash, body) VALUES (%0q:hash, %1q:body)";
      query.parse();
      res = query.execute(hash, body);
      numRows = res.rows();
    } // try
    catch(mysqlpp::BadQuery e) {
      _dbi->_logf("*** MySQL++ Error{insertStatus}: #%d %s", e.errnum(), e.what());
      return false;
    } // catch
    catch(mysqlpp::Exception e) {
      _dbi->_logf("*** MySQL++ Error{insertStatus}: %s", e.what());
      return false;
    } // catch

    if (numRows) {
      if (res.insert_id() == 0)
        return false;
      s << res.insert_id();
      id = s.str();
    } // if

    return (numRows > 0) ? true : false;
  } // DBI::Inject::insertStaus

  const bool DBI::Inject::insertPacket(const string &callsignId, string &id) {
    bool isOK;
    int numRows = 0;
    stringstream s;

    s.str("");
    id = "";

    // try and get stations last message id
    isOK = _sql->queryf("INSERT IGNORE INTO packet (callsign_id, create_ts) VALUES ('%s', '%d')",
                        _sql->Escape(callsignId).c_str(),
                        time(NULL));

    if (!isOK)
      return false;

    numRows = _sql->num_affected_rows();

    if (numRows) {
      if (_sql->insert_id() == 0)
        return false;

      s << _sql->insert_id();
      id = s.str();
    } // if

    return (numRows > 0) ? true : false;
  } // DBI::Inject::insertPacket

  /**************************************************************************
   ** DBI::Create Class                                                     **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  DBI::Create::Create(DBI *dbi, PutMySQL *sql) : _dbi(dbi), _sql(sql) {
  } // DBI::Create::Create

  DBI::Create::~Create() {
  } // DBI::Create::~Create

  const unsigned int DBI::Create::disableObjectBeacon(const string &name) {
    bool isOk;
    unsigned int numAffectedRows = 0;
    stringstream s;

    s.str();
    s << "UPDATE "
      <<   "create_object "
      << "SET "
      <<   "beacon = '0' "
      << "WHERE "
      <<   "name = '" << _sql->Escape(name) << "'";

    isOk = _sql->query(s.str());

    if (isOk)
      numAffectedRows = _sql->num_affected_rows();

    return numAffectedRows;
  } // DBI::Create::disableObjectBeacon

  const bool DBI::Create::Object(const string &name,
                                const string &source,
                                const string &destination,
                                const double latitude,
                                const double longitude,
                                const string &symbol_table,
                                const string &symbol_code,
                                const double speed,
                                const int course,
                                const double altitude,
                                const bool compress,
                                const int ambiguity,
                                const string &comment,
                                const time_t beacon,
                                const time_t expire_ts,
                                const bool local) {
    time_t create_ts = time(NULL);
    unsigned int numRows = 0;

    if (_sql->queryf("REPLACE INTO create_object (name, source, destination, latitude, longitude, course, speed, altitude, status, symbol_table, symbol_code, type, beacon, expire_ts, local, create_ts) VALUES ('%s', '%s', '%s', '%f', '%f', '%d', '%f', '%f', '%s', '%s', '%s', 'O', '%d', '%d', '%s', '%d')",
                _sql->Escape(name).c_str(),
                _sql->Escape(source).c_str(),
                _sql->Escape(destination).c_str(),
                latitude,
                longitude,
                course,
                speed,
                altitude,
                _sql->Escape(comment).c_str(),
                _sql->Escape(symbol_table).c_str(),
                _sql->Escape(symbol_code).c_str(),
                beacon,
                expire_ts,
                (local) ? "Y" : "N",
                create_ts)) {

      numRows = _sql->num_affected_rows();
    } // if

    return (numRows > 0) ? true : false;
  } // DBI::Create::Object

  const bool DBI::Create::Position(const string &source,
                                  const string &destination,
                                  const double latitude,
                                  const double longitude,
                                  const string &symbol_table,
                                  const string &symbol_code,
                                  const double speed,
                                  const int course,
                                  const double altitude,
                                  const bool compress,
                                  const int ambiguity,
                                  const string &comment,
                                  const bool local) {
    time_t create_ts = time(NULL);
    unsigned int numRows = 0;

    if (_sql->queryf("REPLACE INTO create_position (source, destination, latitude, longitude, course, speed, altitude, status, symbol_table, symbol_code, local, create_ts) VALUES ('%s', '%s', '%f', '%f', '%d', '%f', '%f', '%s', '%s', '%s', '%s', '%d')",
                _sql->Escape(source).c_str(),
                _sql->Escape(destination).c_str(),
                latitude,
                longitude,
                course,
                speed,
                altitude,
                _sql->Escape(comment).c_str(),
                _sql->Escape(symbol_table).c_str(),
                _sql->Escape(symbol_code).c_str(),
                (local) ? "Y" : "N",
                create_ts)) {

      numRows = _sql->num_affected_rows();
    } // if

    return (numRows > 0) ? true : false;
  } // DBI::Create::Position

  const unsigned int DBI::Create::getPendingMessages(setMapType &setMap) {
    stringstream s;
    unsigned int numRows;

    s << "SELECT id, source, target, message, local FROM create_message WHERE broadcast_ts = '0' AND error='N' ORDER BY create_ts ASC";

    numRows = _dbi->fetch(s.str(), setMap);

    return numRows;
  } // DBI::Create::getPendingMessages

  const unsigned int DBI::Create::setMessageSent(const int id, const string &decayId, const time_t timestamp) {
    bool isOK;
    stringstream s;
    unsigned int numRows = 0;

    isOK = _sql->queryf("UPDATE create_message SET broadcast_ts='%d', decay_id='%s' WHERE id='%d'",
                      timestamp,
                      _sql->Escape(decayId).c_str(),
                      id);

    if (isOK)
      numRows = _sql->num_affected_rows();

    return numRows;
  } // DBI::Create::setMessageSent

  const unsigned int DBI::Create::setMessageError(const int id) {
    bool isOK;
    stringstream s;
    unsigned int numRows = 0;

    isOK = _sql->queryf("UPDATE create_message SET error='Y' WHERE id = '%d'", id);

    if (isOK)
      numRows = _sql->num_affected_rows();

    return numRows;
  } // DBI::Create::setMessageError

  const unsigned int DBI::Create::setPositionSent(const int id, const time_t timestamp) {
    bool isOK;
    stringstream s;
    unsigned int numRows = 0;

    isOK = _sql->queryf("UPDATE create_position SET broadcast_ts = '%d' WHERE id = '%d'",
                      timestamp,
                      id);

    if (isOK)
      numRows = _sql->num_affected_rows();

    return numRows;
  } // DBI::Create::setPositionSent

  const unsigned int DBI::Create::setPositionError(const int id) {
    bool isOK;
    stringstream s;
    unsigned int numRows = 0;

    isOK = _sql->queryf("UPDATE create_position SET error='Y' WHERE id = '%d'", id);

    if (isOK)
      numRows = _sql->num_affected_rows();

    return numRows;
  } // DBI::Create::setPositionError

  const unsigned int DBI::Create::getPendingPositions(setMapType &setMap) {
    stringstream s;
    unsigned int numReturned;

    s << "SELECT "
      <<   "id, "
      <<   "broadcast_ts, "
      <<   "source, "
      <<   "latitude, "
      <<   "longitude, "
      <<   "symbol_table, "
      <<   "symbol_code, "
      <<   "local, "
      <<   "speed, "
      <<   "course, "
      <<   "altitude, "
      <<   "status "
      << "FROM create_position "
      << "WHERE broadcast_ts = '0' AND error='N' "
      << "ORDER BY create_ts ASC";

    numReturned = _dbi->fetch(s.str(), setMap);

    return numReturned;
  } // DBI::Create::getPendingPositions

  const unsigned int DBI::Create::getPendingObjects(const time_t timestamp, setMapType &setMap) {
    stringstream s;
    unsigned int numReturned;

    s << "SELECT "
      <<   "id, "
      <<   "broadcast_ts, "
      <<   "expire_ts, "
      <<   "beacon, "
      <<   "name, "
      <<   "source, "
      <<   "latitude, "
      <<   "longitude, "
      <<   "symbol_table, "
      <<   "symbol_code, "
      <<   "decay_id, "
      <<   "local, "
      <<   "speed, "
      <<   "course, "
      <<   "altitude, "
      <<   "status, "
      <<   "`kill` "
      << "FROM create_object "
      << "WHERE "
      <<   "(broadcast_ts = '0' || beacon > '0') "
      <<   "AND error='N' "
      <<   "AND ((expire_ts = '0' AND beacon = '0') "
      <<   "OR expire_ts > '" << timestamp << "') "
      << "ORDER BY create_ts ASC";

    numReturned = _dbi->fetch(s.str(), setMap);

    return numReturned;
  } // DBI::Create::getPendingObjects

  const unsigned int DBI::Create::setObjectSent(const int id, const string &decayId, const time_t timestamp) {
    bool isOK;
    stringstream s;
    unsigned int numRows = 0;

    isOK = _sql->queryf("UPDATE create_object SET broadcast_ts='%d', decay_id='%s' WHERE id='%d'",
                      timestamp,
                      _sql->Escape(decayId).c_str(),
                      id);

    if (isOK)
      numRows = _sql->num_affected_rows();

    return numRows;
  } // DBI::Create::setObjectSent

  const unsigned int DBI::Create::setObjectError(const int id) {
    bool isOK;
    stringstream s;
    unsigned int numRows = 0;

    isOK = _sql->queryf("UPDATE create_object SET error='Y' WHERE id = '%d'", id);

    if (isOK)
      numRows = _sql->num_affected_rows();

    return numRows;
  } // DBI::Create::setObjectError

  /**************************************************************************
   ** DBI::APRSMail Class                                                   **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  DBI::APRSMail::APRSMail(DBI *dbi, PutMySQL *sql) : _dbi(dbi), _sql(sql) {
  } // DBI::APRSMail::APRSMail

  DBI::APRSMail::~APRSMail() {
  } // DBI::APRSMail::~APRSMail

  const unsigned int DBI::APRSMail::writeError(const int id, const char *writeFormat, ...) {
    bool isOk;
    char writeBuffer[MAXBUF + 1] = {0};
    va_list va;

    va_start(va, writeFormat);
    vsnprintf(writeBuffer, sizeof(writeBuffer), writeFormat, va);
    va_end(va);

    isOk = _sql->queryf("UPDATE aprsmail_mail SET errmsg='%s' WHERE id='%d'",
                      _sql->Escape(writeBuffer).c_str(),
                      id);

    return strlen(writeBuffer);
  } // DBI::APRSMail::writeError

  const unsigned int DBI::APRSMail::getPending(setMapType &setMap) {
    stringstream s;
    unsigned int numRows;

    s << "SELECT "
      << "  am.id, "
      << "  am.mail_from, "
      << "  am.mail_to, "
      << "  am.callsign, "
      << "  am.sender, "
      << "  am.mail_subject, "
      << "  am.mail_content, "
      << "  am.user_id, "
      << "  wu.callsign AS user_callsign "
      << "FROM "
      << "  aprsmail_mail am "
      << "LEFT JOIN web_users wu ON wu.id = am.user_id "
      << "WHERE "
      << "  am.read_ts = '0' "
      << "ORDER BY am.create_ts ASC";

    numRows = _dbi->fetch(s.str(), setMap);

    return numRows;
  } // DBI::APRSMail::getPending

  const unsigned int DBI::APRSMail::setRead(const int id, const time_t timestamp) {
    bool isOK;
    stringstream s;
    unsigned int numRows = 0;

    isOK = _sql->queryf("UPDATE aprsmail_mail SET read_ts='%d' WHERE id='%d'", timestamp, id);

    if (isOK)
      numRows = _sql->num_affected_rows();

    return numRows;
  } // DBI::APRSMail::setRead

  /**************************************************************************
   ** DBI::Message Class                                                    **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  DBI::Message::Message(DBI *dbi, PutMySQL *sql) : _dbi(dbi), _sql(sql) {
  } // DBI::Message::Message

  DBI::Message::~Message() {
  } // DBI::Message::~Message

  const unsigned int DBI::Message::LastByCallsign(const string &callsign,
                                                 const time_t start_ts,
                                                 const time_t end_ts,
                                                 const unsigned int limit,
                                                 setMapType &setMap) {
    Set st;
    bool isOk;
    string field, value, wildcard;
    unsigned int numRows = 0;
    unsigned int i,j;

    // initialize variables
    setMap.clear();
    st.resultMap.clear();
    wildcard = _dbi->Wildcard(callsign);

    isOk = _sql->queryf("SELECT COUNT(*) AS count FROM last_message lm LEFT JOIN callsign c ON c.id = lm.callsign_id WHERE c.source LIKE '%s' AND lm.create_ts >= '%d' AND lm.create_ts <= '%d'",
                      _sql->Escape(wildcard).c_str(),
                      start_ts,
                      end_ts);

    if (isOk)
      while((*_sql)++)
        numRows = atoi((*_sql)["count"]);

    isOk = _sql->queryf("SELECT lm.packet_id, c.source, lm.addressee, lm.text, lm.id, lm.create_ts FROM last_message lm LEFT JOIN callsign c ON c.id = lm.callsign_id WHERE c.source LIKE '%s' AND lm.create_ts >= '%d' AND lm.create_ts <= '%d' ORDER BY lm.create_ts DESC LIMIT %d",
                      _sql->Escape(wildcard).c_str(),
                      start_ts,
                      end_ts,
                      limit);

    if (isOk) {
      for(j=0;(*_sql)++;j++) {
        st.resultMap.clear();

        for(i=0; i < _sql->num_fields(); i++) {
          field = _sql->getFieldName(i);

          if ((*_sql)[field.c_str()] == NULL)
            continue;

          if (strlen((*_sql)[field.c_str()]) == 1)
            value = string((*_sql)[field.c_str()]);
          else
            value = _sql->Unescape((*_sql)[field.c_str()]);

          st.resultMap.insert(pair<string, string>(field, value));
        } // for

        setMap.insert(pair<unsigned int, Set>(j, st));
      } // while
    } // if

    return numRows;
  } // DBI::Message::LastByCallsign

  /**************************************************************************
   ** DBI::Uls Class                                                        **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  DBI::Uls::Uls(DBI *dbi, PutMySQL *sql) : _dbi(dbi), _sql(sql) {
  } // DBI::Uls::Uls

  DBI::Uls::~Uls() {
  } // DBI::Uls::~Uls

  const unsigned int DBI::Uls::FindByCallsign(const string &callsign,
                                                 setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM uls_hd WHERE callsign = '%s' LIMIT 1",
                          _sql->Escape(callsign).c_str());

    snprintf(sq, bufsize, "SELECT uls_hd.callsign, uls_hd.grant_date, uls_hd.expired_date, uls_hd.effective_date, uls_hd.last_action_date, uls_en.entity_name, uls_en.last_name, uls_en.first_name, uls_en.middle_initial, uls_en.suffix, uls_en.street_address, uls_en.city, uls_en.state, uls_en.zip_code, uls_am.class, uls_am.previous_callsign FROM uls_hd LEFT JOIN uls_am ON uls_am.usi = uls_hd.usi LEFT JOIN uls_en ON uls_en.usi = uls_hd.usi WHERE uls_hd.callsign='%s' AND uls_hd.license_status = 'A' LIMIT 1",
                          _sql->Escape(callsign).c_str());

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Uls::FindByCallsign

  /**************************************************************************
   ** DBI::Signup Class                                                    **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  DBI::Signup::Signup(DBI *dbi, PutMySQL *sql) : _dbi(dbi), _sql(sql) {
  } // DBI::Signup::Signup

  DBI::Signup::~Signup() {
  } // DBI::Signup::~Signup

  const bool DBI::Signup::isUniqueCall(const string &callsign) {
    bool isOk;
    unsigned int numRows = 0;

    isOk = _sql->queryf("SELECT callsign FROM web_users WHERE callsign='%s' LIMIT 1",
                      _sql->Escape(callsign).c_str());

    if (isOk)
      numRows = _sql->num_rows();

    return (numRows > 0) ? false : true;
  } // DBI::Signup::isUniqueCall

  const bool DBI::Signup::isKey(const string &client, const string &hash, const string &seed, bool &autoverify) {
    bool isOk;
    unsigned int numRows = 0;

    autoverify = false;

    isOk = _sql->queryf("SELECT hash, autoverify FROM web_dcc_access WHERE client='%s' AND MD5(CONCAT('%s', hash))='%s' AND active='Y' LIMIT 1",
                        _sql->Escape(client).c_str(),
                        _sql->Escape(seed).c_str(),
                        _sql->Escape(hash).c_str());

    if (isOk) {
      numRows = _sql->num_rows();

      while((*_sql)++) {
        if (!cmp((*_sql)["autoverify"], "Y"))
          autoverify = true;
      } // while
    } // if

    return (numRows > 0) ? true : false;
  } // DBI::Signup::isKey

  const bool DBI::Signup::isBanned(const string &uid) {
    bool isOk;
    unsigned int numRows = 0;

    isOk = _sql->queryf("SELECT id FROM web_dcc_bans WHERE uid='%s' AND active='Y' LIMIT 1",
                      _sql->Escape(uid).c_str());

    if (isOk)
      numRows = _sql->num_rows();

    return (numRows > 0) ? true : false;
  } // DBI::Signup::isBanned

  const bool DBI::Signup::isUniqueEmail(const string &email) {
    bool isOk;
    unsigned int numRows = 0;

    isOk = _sql->queryf("SELECT email FROM web_users WHERE email='%s' LIMIT 1",
                      _sql->Escape(email).c_str());

    if (isOk)
      numRows = _sql->num_rows();

    return (numRows > 0) ? false : true;
  } // DBI::Signup::isUniqueEmail

  const bool DBI::Signup::isUserById(const string &userId) {
    bool isOk;
    unsigned int numRows = 0;

    isOk = _sql->queryf("SELECT email FROM web_users WHERE id='%s' LIMIT 1",
                      _sql->Escape(userId).c_str());

    if (isOk)
      numRows = _sql->num_rows();

    return (numRows > 0) ? true : false;
  } // DBI::Signup::isUserById

  const bool DBI::Signup::isIEUpgrade(const string &email, const string &passhash) {
    bool isOk;
    unsigned int numAffectedRows = 0;
    time_t create_ts = time(NULL);

    isOk = _sql->queryf("UPDATE web_users SET verified='Y', ie='Y', verified_ts='%d' WHERE email='%s' AND md5=MD5('%s')",
                      create_ts,
                      _sql->Escape(email).c_str(),
                      _sql->Escape(passhash).c_str());

    if (isOk)
      numAffectedRows = _sql->num_affected_rows();

    return (numAffectedRows > 0) ? true : false;
  } // DBI::Signup::isIEUpgrade

  const bool DBI::Signup::setIE(const string &userId, const bool setMe) {
    bool isOk;
    unsigned int numAffectedRows = 0;
    time_t create_ts = time(NULL);

    isOk = _sql->queryf("UPDATE web_users SET active='%s', ie='%s', verified='%s', verified_ts='%d' WHERE id='%s'",
                      (setMe ? "Y" : "N"),
                      (setMe ? "Y" : "N"),
                      (setMe ? "Y" : "N"),
                      create_ts,
                      _sql->Escape(userId).c_str());

    if (isOk)
      numAffectedRows = _sql->num_affected_rows();

    return (numAffectedRows > 0) ? true : false;
  } // DBI::Signup::setIE

  const bool DBI::Signup::setActivated(const string &userId, const bool setMe) {
    bool isOk;
    unsigned int numAffectedRows = 0;

    isOk = _sql->queryf("UPDATE web_users SET active='%s' WHERE id='%s'",
                        (setMe ? "Y" : "N"),
                        _sql->Escape(userId).c_str());

    if (isOk)
      numAffectedRows = _sql->num_affected_rows();

    return (numAffectedRows > 0) ? true : false;
  } // DBI::Signup::setActivated

  const bool DBI::Signup::setVerified(const string &userId, const bool setMe) {
    bool isOk;
    unsigned int numAffectedRows = 0;
    time_t create_ts = time(NULL);

    isOk = _sql->queryf("UPDATE web_users SET active='%s', verified='%s', verified_ts='%d' WHERE id='%s'",
                      (setMe ? "Y" : "N"),
                      (setMe ? "Y" : "N"),
                      create_ts,
                      _sql->Escape(userId).c_str());

    if (isOk)
      numAffectedRows = _sql->num_affected_rows();

    return (numAffectedRows > 0) ? true : false;
  } // DBI::Signup::setVerified

  const bool DBI::Signup::deleteUser(const string &userId) {
    bool isOk;
    unsigned int numAffectedRows = 0;

    isOk = _sql->queryf("DELETE FROM web_users_options WHERE user_id='%s' LIMIT 1",
                      _sql->Escape(userId).c_str());

    isOk = _sql->queryf("DELETE FROM web_users WHERE id='%s' LIMIT 1",
                      _sql->Escape(userId).c_str());

    if (isOk)
      numAffectedRows = _sql->num_affected_rows();

    return (numAffectedRows > 0) ? true : false;
  } // DBI::Signup::deleteUser

  const bool DBI::Signup::isActivated(const string &email) {
    bool isOk;
    unsigned int numRows = 0;

    isOk = _sql->queryf("SELECT email FROM web_users WHERE email='%s' AND active='Y' LIMIT 1",
                      _sql->Escape(email).c_str());

    if (isOk)
      numRows = _sql->num_rows();

    return (numRows > 0) ? true : false;
  } // DBI::Signup::isActivated

  const unsigned int DBI::Signup::FindUser(const string &callsign,
                                          setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    // initialize variables
    wildcard = _dbi->Wildcard(callsign);

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM web_users WHERE (callsign LIKE '%s') OR (email LIKE '%s') LIMIT 1",
                          _sql->Escape(wildcard).c_str(),
                          _sql->Escape(wildcard).c_str());

    snprintf(sq, bufsize, "SELECT id, callsign, CONCAT(UPPER(first), ' ', UPPER(last), ' <', email, '>') AS name, IF(create_ts=0, 'never', FROM_UNIXTIME(create_ts, '%%m-%%d-%%Y %%h:%%i:%%s %%p')) AS create_ts, IF(login_ts=0, 'never', FROM_UNIXTIME(login_ts, '%%m-%%d-%%Y %%h:%%i:%%s %%p')) AS login_ts, IF(verified_ts=0, 'never', FROM_UNIXTIME(verified_ts, '%%m-%%d-%%Y %%h:%%i:%%s %%p')) AS verified_ts, active, verified, ie, admin FROM web_users WHERE (callsign LIKE '%s') OR (email LIKE '%s') LIMIT 1",
                          _sql->Escape(wildcard).c_str(),
                          _sql->Escape(wildcard).c_str());

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::signup::FindUser

/**
 * DBI::SignupCreate
 *
 * Try and create message.
 *
 */
const bool DBI::Signup::Create(const string &firstname, 
                              const string &lastname, 
                              const string &email,
                              const string &password,
                              const string &activate,
                              const string &callsign,
                              const unsigned int active_until,
                              const bool verify) {
    time_t create_ts = time(NULL);
    unsigned int numRows = 0;

    if (_sql->queryf("INSERT INTO web_users (first, last, email, callsign, md5, activate, email_activate, ie, verified, verified_ts, email_verified, active_until, create_ts) VALUES (UPPER('%s'), UPPER('%s'), '%s', UPPER('%s'), MD5('%s'), '%s', 'Y', '%s', '%s', '%d', 'N', '%d', '%d')",
                _sql->Escape(firstname).c_str(),
                _sql->Escape(lastname).c_str(),
                _sql->Escape(email).c_str(),
                _sql->Escape(callsign).c_str(),
                _sql->Escape(password).c_str(),
                _sql->Escape(activate).c_str(),
                (verify ? "Y" : "N"),
                (verify ? "Y" : "N"),
                (verify ? create_ts : 0),
                (time(NULL) + active_until),
                create_ts)) {

      numRows = _sql->num_affected_rows();
    } // if

    return (numRows > 0) ? true : false;
} // DBI::signup::Create

/**
 * DBI::SignupCreate
 *
 * Try and create message.
 *
 */
const bool DBI::Signup::RegisterForNotifications(const string &user_id,
                                                const string &callsign, 
                                                const string &uid, 
                                                const string &device_token) {
    time_t create_ts = time(NULL);
    unsigned int numRows = 0;

    if (_sql->queryf("INSERT INTO apns_register (user_id, callsign, uid, device_token, check_ts, update_ts, create_ts) VALUES ('%s', '%s', '%s', '%s', '%d', '%d', '%d') ON DUPLICATE KEY UPDATE callsign='%s', update_ts='%d'",
                _sql->Escape(user_id).c_str(),
                _sql->Escape(callsign).c_str(),
                _sql->Escape(uid).c_str(),
                _sql->Escape(device_token).c_str(),
                create_ts,
                create_ts,
                create_ts,
                _sql->Escape(callsign).c_str(),
                create_ts)) {

      numRows = _sql->num_affected_rows();
    } // if

    return (numRows > 0) ? true : false;
} // DBI::signup::RegisterForNotifications

/**
 * DBI::ResendActivation
 *
 * Try and create message.
 *
 */
const bool DBI::Signup::ResendActivation(const string &email) {
    unsigned int numRows = 0;

    if (_sql->queryf("UPDATE web_users SET email_activate='Y' WHERE email='%s'",
                     _sql->Escape(email).c_str())) {

      numRows = _sql->num_affected_rows();
    } // if

    return (numRows > 0) ? true : false;
} // DBI::signup::ResendActivation

/**
 * DBI::PasswordReset
 *
 * Try and create message.
 *
 */
const bool DBI::Signup::PasswordReset(const string &email) {
    unsigned int numRows = 0;

    if (_sql->queryf("UPDATE web_users SET email_password='Y' WHERE email='%s'",
                     _sql->Escape(email).c_str())) {

      numRows = _sql->num_affected_rows();
    } // if

    return (numRows > 0) ? true : false;
} // DBI::signup::PasswordReset

  /**************************************************************************
   ** DBI::Position Class                                                   **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  DBI::Position::Position(DBI *dbi, PutMySQL *sql) : _dbi(dbi), _sql(sql) {
  } // DBI::Position::Position

  DBI::Position::~Position() {
  } // DBI::Position::~Position

  const unsigned int DBI::Position::deleteObject(const string &name) {
    bool isOk;
    unsigned int numAffectedRows = 0;
    stringstream s;

    s.str();
    s << "DELETE FROM "
      <<   "lastposition "
      << "WHERE "
      <<   "name = '" << _sql->Escape(name) << "' "
      << "AND "
      << "type = 'O'";

    isOk = _sql->query(s.str());

    if (isOk)
      numAffectedRows = _sql->num_affected_rows();

    return numAffectedRows;
  } // DBI::Position::deleteObject

  const unsigned int DBI::Position::LastByCallsign(const string &callsign,
                                                  const time_t start_ts,
                                                  const time_t end_ts,
                                                  const unsigned int limit,
                                                  setMapType &setMap) {
    Set st;
    bool isOk;
    string field, value, wildcard;
    unsigned int numRows = 0;
    unsigned int i,j;
    stringstream s;

    // initialize variables
    setMap.clear();
    st.resultMap.clear();
    wildcard = _dbi->Wildcard(callsign);

    isOk = _sql->queryf("SELECT COUNT(*) AS count FROM lastposition lp LEFT JOIN callsign c ON c.id = lp.callsign_id WHERE ((c.source LIKE '%s' AND lp.type='P') || (lp.name LIKE '%s' AND lp.type = 'O')) AND lp.create_ts >= '%d' AND lp.create_ts <= '%d'",
                        _sql->Escape(wildcard).c_str(),
                        _sql->Escape(wildcard).c_str(),
                        start_ts,
                        end_ts);

    if (isOk)
      while((*_sql)++)
        numRows = atoi((*_sql)["count"]);

    //isOk = _sql->queryf("SELECT packet_id, source, name, destination, latitude, longitude, course, speed, altitude, status, symbol_table, symbol_code, type, weather, create_ts FROM lastposition WHERE ((source LIKE '%s' AND type='P') OR (name LIKE '%s' AND type = 'O')) AND create_ts >= '%d' AND create_ts <= '%d' ORDER BY create_ts DESC LIMIT %d",
    s << "SELECT "
      <<   "lp.packet_id, "
      <<   "c.source, "
      <<   "lp.name, "
      <<   "lp.destination, "
      <<   "lp.latitude, "
      <<   "lp.longitude, "
      <<   "lp.course, "
      <<   "lp.speed, "
      <<   "lp.altitude, "
      <<   "s.body AS status, "
      <<   "lp.symbol_table, "
      <<   "lp.symbol_code, "
      <<   "lp.type, "
      <<   "lp.weather, "
      <<   "lp.create_ts, "
      <<   "ic.image AS icon "
      << "FROM "
      <<   "lastposition lp "
      <<   "LEFT JOIN callsign c ON c.id = lp.callsign_id "
      <<   "LEFT JOIN statuses s ON s.id = lp.status_id "
      <<   "LEFT JOIN aprs_icons ic ON (ic.symbol_table = IF(lp.symbol_table = '\\\\' OR lp.symbol_table = '/',lp.symbol_table,'\\\\') AND BINARY ic.icon = lp.symbol_code) "
      << "WHERE "
      <<   "((c.source LIKE '" << _sql->Escape(wildcard).c_str() << "' AND lp.type='P') "
      <<   "OR (lp.name LIKE '" << _sql->Escape(wildcard).c_str() << "' AND lp.type = 'O')) "
      <<   "AND lp.create_ts >= '" << start_ts << "' "
      <<   "AND lp.create_ts <= '" << end_ts << "' "
      << "ORDER BY lp.create_ts DESC "
      << "LIMIT " << limit;

    isOk = _sql->query(s.str());

    if (isOk) {
      for(j=0;(*_sql)++;j++) {
        st.resultMap.clear();

        for(i=0; i < _sql->num_fields(); i++) {
          field = _sql->getFieldName(i);

          if ((*_sql)[field.c_str()] == NULL)
            continue;

          if (strlen((*_sql)[field.c_str()]) == 1)
            value = string((*_sql)[field.c_str()]);
          else
            value = _sql->Unescape((*_sql)[field.c_str()]);

          st.resultMap.insert(pair<string, string>(field, value));
        } // for

        setMap.insert(pair<unsigned int, Set>(j, st));
      } // while
    } // if

    return numRows;
  } // DBI::Position::LastByCallsign

  const unsigned int DBI::Position::HistoryByCallsign(const string &callsign,
                                                     const time_t start_ts,
                                                     const time_t end_ts,
                                                     const unsigned int limit,
                                                     setMapType &setMap) {
    Set st;
    bool isOk;
    string field, value, wildcard;
    unsigned int numRows = 0;
    unsigned int i,j;

    // initialize variables
    setMap.clear();
    st.resultMap.clear();
    wildcard = _dbi->Wildcard(callsign);

    isOk = _sql->queryf("SELECT COUNT(*) AS count FROM position p LEFT JOIN callsign c ON c.id = p.callsign_id WHERE c.source LIKE '%s' AND lp.create_ts >= '%d' AND lp.create_ts <= '%d'",
                      _sql->Escape(wildcard).c_str(),
                      start_ts,
                      end_ts);

    if (isOk)
      while((*_sql)++)
        numRows = atoi((*_sql)["count"]);

    isOk = _sql->queryf("SELECT c.source, p.latitude, p.longitude, p.course, p.speed, p.altitude, p.create_ts FROM position p LEFT JOIN callsign c ON c.id = p.callsign_id WHERE c.source LIKE '%s' AND p.create_ts >= '%d' AND p.create_ts <= '%d' ORDER BY p.create_ts DESC LIMIT %d",
                      _sql->Escape(wildcard).c_str(),
                      start_ts,
                      end_ts,
                      limit);

    if (isOk) {
      for(j=0;(*_sql)++;j++) {
        st.resultMap.clear();

        for(i=0; i < _sql->num_fields(); i++) {
          field = _sql->getFieldName(i);

          if ((*_sql)[field.c_str()] == NULL)
            continue;

          if (strlen((*_sql)[field.c_str()]) == 1)
            value = string((*_sql)[field.c_str()]);
          else
            value = _sql->Unescape((*_sql)[field.c_str()]);

          st.resultMap.insert(pair<string, string>(field, value));
        } // for

        setMap.insert(pair<unsigned int, Set>(j, st));
      } // while
    } // if

    return numRows;
  } // DBI::Position::HistoryByCallsign

  const unsigned int DBI::Position::LastByLatLong(const double lat1,
                                                 const double lon1,
                                                 const double lat2,
                                                 const double lon2,
                                                 const time_t start_ts,
                                                 const time_t end_ts,
                                                 const unsigned int limit,
                                                 setMapType &setMap) {
    char *sc, *sq;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM lastposition lp LEFT JOIN callsign c ON c.id = lp.callsign_id WHERE  (lp.latitude <= FORMAT(%f, 6) AND lp.longitude >= FORMAT(%f, 6) AND lp.latitude >= FORMAT(%f, 6) AND lp.longitude <= FORMAT(%f, 6)) AND lp.create_ts >= '%ld' AND lp.create_ts <= '%ld'",
                      lat1,
                      lon1,
                      lat2,
                      lon2,
                      start_ts,
                      end_ts);

    snprintf(sq, bufsize, "SELECT lp.packet_id, c.source, lp.name, lp.destination, lp.latitude, lp.longitude, lp.course, lp.speed, lp.altitude, s.body AS status, lp.symbol_table, lp.symbol_code, lp.type, lp.weather, lp.create_ts, ic.image AS icon FROM lastposition lp LEFT JOIN callsign c ON c.id = lp.callsign_id LEFT JOIN statuses s ON s.id = lp.status_id LEFT JOIN aprs_icons ic ON (ic.symbol_table = IF(lp.symbol_table = '\\\\' OR lp.symbol_table = '/',lp.symbol_table,'\\\\') AND BINARY ic.icon = lp.symbol_code) WHERE (lp.latitude <= FORMAT(%f, 6) AND lp.longitude >= FORMAT(%f, 6) AND lp.latitude >= FORMAT(%f, 6) AND lp.longitude <= FORMAT(%f, 6)) AND lp.create_ts >= '%ld' AND lp.create_ts <= '%ld' ORDER BY lp.create_ts DESC LIMIT %d",
                      lat1,
                      lon1,
                      lat2,
                      lon2,
                      start_ts,
                      end_ts,
                      limit);

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Position::LastByLatLong

  /**************************************************************************
   ** DBI::Weather Class                                                    **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  DBI::Weather::Weather(DBI *dbi, PutMySQL *sql) : _dbi(dbi), _sql(sql) {
  } // DBI::Weather::Weather

  DBI::Weather::Weather::~Weather() {
  } // DBI::Weather::~Weather

  const unsigned int DBI::Weather::LastByCallsign(const string &callsign,
                                                 const time_t start_ts,
                                                 const time_t end_ts,
                                                 const unsigned int limit,
                                                 setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    // initialize variables
    wildcard = _dbi->Wildcard(callsign);

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM lastweather lw LEFT JOIN callsign c ON c.id = lw.callsign_id WHERE (c.source LIKE '%s') AND lw.create_ts >= '%ld' AND lw.create_ts <= '%ld'",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts);

    snprintf(sq, bufsize, "SELECT c.source, lw.latitude, lw.longitude, lw.wind_direction, lw.wind_speed, lw.wind_sustained, lw.temperature, lw.rain_hour, lw.rain_calendar_day, lw.rain_24hour_day, lw.humidity, lw.barometer, lw.luminosity, lw.create_ts FROM lastweather lw LEFT JOIN callsign c ON c.id = lw.callsign_id WHERE (c.source LIKE '%s') AND lw.create_ts >= '%ld' AND lw.create_ts <= '%ld' ORDER BY lw.create_ts DESC LIMIT %u",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts,
                          limit);

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Weather::LastByCallsign

  const unsigned int DBI::Weather::HistoryByCallsign(const string &callsign,
                                                    const time_t start_ts,
                                                    const time_t end_ts,
                                                    const unsigned int limit,
                                                    setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    // initialize variables
    wildcard = _dbi->Wildcard(callsign);

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM weather w LEFT JOIN callsign c ON c.id = w.callsign_id WHERE (c.source LIKE '%s') AND w.create_ts >= '%ld' AND w.create_ts <= '%ld'",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts);

    snprintf(sq, bufsize, "SELECT c.source, w.wind_direction, w.wind_speed, w.wind_sustained, w.temperature, w.rain_hour, w.rain_calendar_day, w.rain_24hour_day, w.humidity, w.barometer, w.luminosity FROM weather w LEFT JOIN callsign c ON c.id = w.callsign_id WHERE (c.source LIKE '%s') AND w.create_ts >= '%ld' AND w.create_ts <= '%ld' ORDER BY w.create_ts DESC LIMIT %u",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts,
                          limit);

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Weather::HistoryByCallsign

  const unsigned int DBI::Weather::LastByLatLong(const double lat1,
                                                const double lon1,
                                                const double lat2,
                                                const double lon2,
                                                const time_t start_ts,
                                                const time_t end_ts,
                                                const unsigned int limit,
                                                setMapType &setMap) {
    char *sc, *sq;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM lastweather lw WHERE (lw.latitude <= FORMAT(%f, 6) AND lw.longitude >= FORMAT(%f, 6) AND lw.latitude >= FORMAT(%f, 6) AND lw.longitude <= FORMAT(%f, 6)) AND lw.create_ts >= '%ld' AND lw.create_ts <= '%ld'",
                          lat1,
                          lon1,
                          lat2,
                          lon2,
                          start_ts,
                          end_ts);

    snprintf(sq, bufsize, "SELECT c.source, lw.latitude, lw.longitude, lw.wind_direction, lw.wind_speed, lw.wind_sustained, lw.temperature, lw.rain_hour, lw.rain_calendar_day, lw.rain_24hour_day, lw.humidity, lw.barometer, lw.luminosity FROM lastweather lw LEFT JOIN callsign c ON c.id = lw.callsign_id WHERE (lw.latitude <= FORMAT(%f, 6) AND lw.longitude >= FORMAT(%f, 6) AND lw.latitude >= FORMAT(%f, 6) AND lw.longitude <= FORMAT(%f, 6)) AND lw.create_ts >= '%ld' AND lw.create_ts <= '%ld' ORDER BY lw.create_ts DESC LIMIT %u",
                          lat1,
                          lon1,
                          lat2,
                          lon2,
                          start_ts,
                          end_ts,
                          limit);

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Weather::LastByLatLong

  /**************************************************************************
   ** DBI::Telemetry Class                                                  **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  DBI::Telemetry::Telemetry(DBI *dbi, PutMySQL *sql) : _dbi(dbi), _sql(sql) {
  } // DBI::Telemetry::Telemetry

  DBI::Telemetry::Telemetry::~Telemetry() {
  } // DBI::Telemetry::~Telemetry

  const unsigned int DBI::Telemetry::LabelsByCallsign(const string &callsign,
                                                     const time_t start_ts,
                                                     const time_t end_ts,
                                                     const unsigned int limit,
                                                     setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    // initialize variables
    wildcard = _dbi->Wildcard(callsign);

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM telemetry_parm tp LEFT JOIN callsign c ON c.id = tp.callsign_id WHERE (c.source LIKE '%s') AND tp.create_ts >= '%ld' AND tp.create_ts <= '%ld'",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts);

    snprintf(sq, bufsize, "SELECT c.source, tp.a_0, tp.a_1, tp.a_2, tp.a_3, tp.a_4, tp.d_0, tp.d_1, tp.d_2, tp.d_3, tp.d_5, tp.d_6, tp.d_7, tp.create_ts FROM telemetry_parm tp LEFT JOIN callsign c ON c.id = tp.callsign_id WHERE (c.source LIKE '%s') AND tp.create_ts >= '%ld' AND tp.create_ts <= '%ld' ORDER BY tp.create_ts DESC LIMIT %u",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts,
                          limit);

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Telemtry::LabelsByCallsign

  const unsigned int DBI::Telemetry::UnitsByCallsign(const string &callsign,
                                                     const time_t start_ts,
                                                     const time_t end_ts,
                                                     const unsigned int limit,
                                                     setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    // initialize variables
    wildcard = _dbi->Wildcard(callsign);

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM telemetry_unit tu LEFT JOIN callsign c ON c.id = tu.callsign_id WHERE (c.source LIKE '%s') AND tu.create_ts >= '%ld' AND tu.create_ts <= '%ld'",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts);

    snprintf(sq, bufsize, "SELECT c.source, tu.a_0, tu.a_1, tu.a_2, tu.a_3, tu.a_4, tu.d_0, tu.d_1, tu.d_2, tu.d_3, tu.d_5, tu.d_6, tu.d_7, tu.create_ts FROM telemetry_unit tu LEFT JOIN callsign c ON c.id = tu.callsign_id WHERE (c.source LIKE '%s') AND tu.create_ts >= '%ld' AND tu.create_ts <= '%ld' ORDER BY tu.create_ts DESC LIMIT %u",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts,
                          limit);

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Telemtry::UnitsByCallsign

  const unsigned int DBI::Telemetry::BitsByCallsign(const string &callsign,
                                                   const time_t start_ts,
                                                   const time_t end_ts,
                                                   const unsigned int limit,
                                                   setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    // initialize variables
    wildcard = _dbi->Wildcard(callsign);

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM telemetry_bits tb LEFT JOIN callsign c ON c.id = tb.callsign_id WHERE (c.source LIKE '%s') AND tb.create_ts >= '%ld' AND tb.create_ts <= '%ld'",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts);

    snprintf(sq, bufsize, "SELECT c.source, tb.bitsense, tb.project_title, tb.create_ts FROM telemetry_bits tb LEFT JOIN callsign c ON c.id = tb.callsign_id WHERE (c.source LIKE '%s') AND tb.create_ts >= '%ld' AND tb.create_ts <= '%ld' ORDER BY tb.create_ts DESC LIMIT %u",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts,
                          limit);

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Telemtry::BitsByCallsign

  const unsigned int DBI::Telemetry::EqnsByCallsign(const string &callsign,
                                                   const time_t start_ts,
                                                   const time_t end_ts,
                                                   const unsigned int limit,
                                                   setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    // initialize variables
    wildcard = _dbi->Wildcard(callsign);

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM telemetry_eqns te LEFT JOIN callsign c ON c.id = te.callsign_id WHERE (c.source LIKE '%s') AND te.create_ts >= '%ld' AND te.create_ts <= '%ld'",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts);

    snprintf(sq, bufsize, "SELECT c.source, te.a_0 AS a0, te.b_0 AS b0, te.c_0 AS c0, te.a_1 AS a1, te.b_1 AS b1, te.c_1 AS c1, te.a_2 AS a2, te.b_2 AS b2, te.c_2 AS c2, te.a_3 AS a3, te.b_3 AS b3, te.c_3 AS c3, te.a_4 AS a4, te.b_4 AS b4, te.c_4 AS c4, te.create_ts FROM telemetry_eqns te LEFT JOIN callsign c ON c.id = te.callsign_id WHERE (c.source LIKE '%s') AND te.create_ts >= '%ld' AND te.create_ts <= '%ld' ORDER BY te.create_ts DESC LIMIT %u",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts,
                          limit);

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Telemtry::EqnsByCallsign

  const unsigned int DBI::Telemetry::LastByCallsign(const string &callsign,
                                                   const time_t start_ts,
                                                   const time_t end_ts,
                                                   const unsigned int limit,
                                                   setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    // initialize variables
    wildcard = _dbi->Wildcard(callsign);

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM last_telemetry lt LEFT JOIN callsign c ON c.id = lt.callsign_id WHERE (c.source LIKE '%s') AND lt.create_ts >= '%ld' AND lt.create_ts <= '%ld'",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts);

    snprintf(sq, bufsize, "SELECT lt.sequence, c.source, lt.analog_0, lt.analog_1, lt.analog_2, lt.analog_3, lt.analog_4, lt.digital, lt.create_ts FROM last_telemetry lt LEFT JOIN callsign c ON c.id = lt.callsign_id WHERE (c.source LIKE '%s') AND lt.create_ts >= '%ld' AND lt.create_ts <= '%ld' ORDER BY lt.create_ts DESC LIMIT %u",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts,
                          limit);

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Telemtry::LastByCallsign

  const unsigned int DBI::Telemetry::HistoryByCallsign(const string &callsign,
                                                      const time_t start_ts,
                                                      const time_t end_ts,
                                                      const unsigned int limit,
                                                      setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    // initialize variables
    wildcard = _dbi->Wildcard(callsign);

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM telemetry t LEFT JOIN callsign c ON c.id = t.callsign_id WHERE (c.source LIKE '%s') AND t.create_ts >= '%ld' AND t.create_ts <= '%ld'",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts);

    snprintf(sq, bufsize, "SELECT t.sequence, c.source, t.analog_0, t.analog_1, t.analog_2, t.analog_3, t.analog_4, t.digital, t.create_ts FROM telemetry t LEFT JOIN callsign c ON c.id = t.callsign_id WHERE (c.source LIKE '%s') AND t.create_ts >= '%ld' AND t.create_ts <= '%ld' ORDER BY t.create_ts DESC LIMIT %u",
                          _sql->Escape(wildcard).c_str(),
                          start_ts,
                          end_ts,
                          limit);

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Telemetry::HistoryByCallsign

  /**************************************************************************
   ** DBI::Extra Class                                                      **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  DBI::Extra::Extra(DBI *dbi, PutMySQL *sql) : _dbi(dbi), _sql(sql) {
  } // DBI::Extra::Extra

  DBI::Extra::Extra::~Extra() {
  } // DBI::Extra::~Extra

  const unsigned int DBI::Extra::FrequencyByCallsign(const string &callsign,
                                                    const time_t start_ts,
                                                    const time_t end_ts,
                                                    const unsigned int limit,
                                                    setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    unsigned int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    // initialize variables
    wildcard = _dbi->Wildcard(callsign);

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM last_frequency lf LEFT JOIN callsign c ON c.id = lf.callsign_id WHERE (c.source LIKE '%s' || lf.name LIKE '%s') AND lf.create_ts >= '%ld' AND lf.create_ts <= '%ld'",
                      _sql->Escape(wildcard).c_str(),
                      _sql->Escape(wildcard).c_str(),
                      start_ts,
                      end_ts);

    snprintf(sq, bufsize, "SELECT c.source, lf.name, lf.frequency, lf.range, lf.range_east, lf.tone, lf.afrs_type, lf.receive, lf.alternate, lf.type, lf.create_ts FROM last_frequency lf LEFT JOIN callsign c ON c.id = lf.callsign_id WHERE (c.source LIKE '%s' || lf.name LIKE '%s') AND lf.create_ts >= '%ld' AND lf.create_ts <= '%ld' ORDER BY lf.create_ts DESC LIMIT %d",
                      _sql->Escape(wildcard).c_str(),
                      _sql->Escape(wildcard).c_str(),
                      start_ts,
                      end_ts,
                      limit);

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Extra::FrequencyByCallsign

  /**************************************************************************
   ** DBI::Email Class                                                      **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  DBI::Email::Email(DBI *dbi, PutMySQL *sql) : _dbi(dbi), _sql(sql) {
  } // DBI::Email::Email

  DBI::Email::Email::~Email() {
  } // DBI::Email::~Email

  const bool DBI::Email::add(const string &callsign,
                            const string &alias,
                            const string &email) {
    bool isOk;
    int numRows = 0;
    time_t now = time(NULL);

    isOk = _sql->queryf("INSERT INTO email_shortcuts (callsign, alias, email, create_ts) VALUES ('%s', '%s', '%s', '%ld')",
                      _sql->Escape(callsign).c_str(),
                      _sql->Escape(alias).c_str(),
                      _sql->Escape(email).c_str(),
                      now);

    if (isOk)
      numRows = _sql->num_affected_rows();

    return (numRows > 0) ? true : false;
  } // DBI::Extra::add

  const bool DBI::Email::cache(const string &callsign,
                              const string &message) {
    bool isOk;
    int numRows = 0;
    time_t now = time(NULL);
    md5wrapper md5;
    string hash = md5.getHashFromString(message.c_str());

    isOk = _sql->queryf("INSERT INTO web_email_cache (callsign, hash, create_ts) VALUES ('%s', '%s', '%ld')",
                      _sql->Escape(callsign).c_str(),
                      _sql->Escape(hash).c_str(),
                      now);

    if (isOk)
      numRows = _sql->num_affected_rows();

    return (numRows > 0) ? true : false;
  } // DBI::Extra::cache

  const bool DBI::Email::find(const string &callsign,
                             const string &alias,
                             string &email) {
    bool isOk;
    unsigned int numRows = 0;

    isOk = _sql->queryf("SELECT email FROM email_shortcuts WHERE callsign = '%s' AND alias = '%s' LIMIT 1",
                      _sql->Escape(callsign).c_str(),
                      _sql->Escape(alias).c_str(),
                      _sql->Escape(email).c_str());

    if (isOk) {
      numRows = _sql->num_rows();
      while((*_sql)++)
        email = (*_sql)["email"];
    } // if

    return (numRows > 0) ? true : false;
  } // DBI::Extra::find

  const bool DBI::Email::remove(const string &callsign,
                               const string &alias) {
    bool isOk;
    unsigned int numRows = 0;

    isOk = _sql->queryf("DELETE FROM email_shortcuts WHERE callsign='%s' AND alias='%s'",
                      _sql->Escape(callsign).c_str(),
                      _sql->Escape(alias).c_str());

    if (isOk)
      numRows = _sql->num_affected_rows();

    return (numRows > 0) ? true : false;
  } // DBI::Extra::remove

  const bool DBI::Email::ignore(const string &callsign) {
    bool isOk;
    unsigned int numRows = 0;

    isOk = _sql->queryf("SELECT callsign FROM web_email_ignore WHERE callsign='%s'",
                      _sql->Escape(callsign).c_str());

    if (isOk)
      numRows = _sql->num_rows();

    return (numRows > 0) ? true : false;
  } // DBI::Extra::ignore

  const bool DBI::Email::cached(const string &callsign, const string &message) {
    bool isOk;
    unsigned int numRows = 0;
    md5wrapper md5;
    string hash = md5.getHashFromString(message.c_str());
    time_t old_t = time(NULL) - 86400;

    isOk = _sql->queryf("SELECT callsign FROM web_email_cache WHERE callsign='%s' AND hash='%s' AND create_ts > '%ld'",
                      _sql->Escape(callsign).c_str(),
                      _sql->Escape(hash).c_str(),
                      old_t);

    if (isOk)
      numRows = _sql->num_rows();

    return (numRows > 0) ? true : false;
  } // DBI::Extra::cached

  const int DBI::Email::list(const string &callsign,
                            setMapType &setMap) {
    char *sc, *sq;
    string wildcard;
    unsigned int bufsize = PUTMYSQL_MAXBUF;
    int numReturned;

    sc = new char[bufsize+1];
    sq = new char[bufsize+1];

    // initialize variables
    wildcard = _dbi->Wildcard(callsign);

    snprintf(sc, bufsize, "SELECT COUNT(*) AS count FROM email_shortcuts WHERE callsign='%s'",
                          _sql->Escape(wildcard).c_str());

    snprintf(sq, bufsize, "SELECT alias, email FROM email_shortcuts WHERE callsign='%s' ORDER BY alias, email",
                          _sql->Escape(wildcard).c_str());

    numReturned = _dbi->fetchRowsAndCount(sc, sq, setMap);

    delete [] sc;
    delete [] sq;

    return numReturned;
  } // DBI::Email::list
} // namespace openaprs

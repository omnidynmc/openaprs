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

#include "APRS.h"
#include "MGMT_DBI.h"
#include "Server.h"

#include "SSQLS.h"

#include "config.h"

namespace mgmt {
  using std::string;
  using std::list;
  using std::ofstream;
  using std::ostream;
  using std::stringstream;
  using std::ios;
  using std::endl;
  using std::cout;
  using aprs::StringTool;

/**************************************************************************
 ** DB Class                                                             **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

  MGMT_DBI::MGMT_DBI() : DBI() {
    _query["user"] = "\
SELECT \
  id AS ID, \
  callsign AS Callsign, \
  CONCAT(first, ' ', last, ' <', email, '>') AS Name, \
  FROM_UNIXTIME(create_ts) AS Created, \
  FROM_UNIXTIME(login_ts) AS `Last Login`, \
  FROM_UNIXTIME(verify_ts) AS `Verified On`, \
  active AS Active, \
  verified AS Verified, \
  email_password AS `Pass Reset`, \
  ie AS iE, \
  admin AS Admin \
FROM \
  web_users \
WHERE \
  %0:column=%1q:target \
LIMIT 1";

    _query["verify.user"] = "\
UPDATE \
  web_users \
SET \
  verified='Y', \
  email_verified='Y', \
  verify_ts=UNIX_TIMESTAMP() \
WHERE \
  %0:column=%1q:target";

    _query["activate.user"] = "\
UPDATE \
  web_users \
SET \
  active='Y' \
WHERE \
  %0:column=%1q:target";

  } // MGMT_DBI::MGMT_DBI

  MGMT_DBI::~MGMT_DBI() {
  } // MGMT_DBI::~MGMT_DBI

  const DBI::resultSizeType MGMT_DBI::getUserByCallsign(const string &callsign, DBI::resultType &res) {
    DBI::queryType query = _sqlpp->query(_query["user"]);
    query.parse();

    try {
      res = query.store("callsign", callsign);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.num_rows();
  } // MGMT_DBI::getUserByCallsign

  const DBI::resultSizeType MGMT_DBI::getUserById(const string &id, DBI::resultType &res) {
    DBI::queryType query = _sqlpp->query(_query["user"]);
    query.parse();

    try {
      res = query.store("id", id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.num_rows();
  } // MGMT_DBI::getUserById

  const DBI::resultSizeType MGMT_DBI::getUserByEmail(const string &email, DBI::resultType &res) {
    DBI::queryType query = _sqlpp->query(_query["user"]);
    query.parse();

    try {
      res = query.store("email", email);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.num_rows();
  } // MGMT_DBI::getUserByEmail

  const DBI::resultSizeType MGMT_DBI::verifyUserByCallsign(const string &callsign) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["verify.user"]);
    query.parse();

    try {
      res = query.execute("callsign", callsign);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // MGMT_DBI::verifyUserByCallsign

  const DBI::resultSizeType MGMT_DBI::verifyUserByEmail(const string &email) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["verify.user"]);
    query.parse();

    try {
      res = query.execute("email", email);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // MGMT_DBI::verifyUserByEmail

  const DBI::resultSizeType MGMT_DBI::verifyUserById(const string &id) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["verify.user"]);
    query.parse();

    try {
      res = query.execute("id", id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // MGMT_DBI::verifyUserById

  const DBI::resultSizeType MGMT_DBI::activateUserByCallsign(const string &callsign) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["activate.user"]);
    query.parse();

    try {
      res = query.execute("callsign", callsign);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // MGMT_DBI::activateUserByCallsign

  const DBI::resultSizeType MGMT_DBI::activateUserByEmail(const string &email) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["activate.user"]);
    query.parse();

    try {
      res = query.execute("email", email);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // MGMT_DBI::activateUserByEmail

  const DBI::resultSizeType MGMT_DBI::activateUserById(const string &id) {
    mysqlpp::SimpleResult res;
    DBI::queryType query = _sqlpp->query(_query["activate.user"]);
    query.parse();

    try {
      res = query.execute("id", id);
    } // try
    catch(mysqlpp::BadQuery e) {
      _consolef("*** SQL Error: #%d %s", e.errnum(), e.what());
      return 0;
    } // catch

    return res.rows();
  } // MGMT_DBI::activateUserById
} // namespace mgmt

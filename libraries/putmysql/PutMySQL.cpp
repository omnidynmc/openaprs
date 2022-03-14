/*
 ##########################################################################
 ## Dynamic Networking Solutions                                         ##
 ##########################################################################
 ## HAL9000, Internet Relay Chat Bot - src/PutMySQL.cpp                  ##
 ## Copyright (C) 1999 Gregory A. Carter                                 ##
 ##                    Dynamic Networking Solutions                      ##
 ##                                                                      ##
 ## This program is free software; you can redistribute it and/or modify ##
 ## it under the terms of the GNU General Public License as published by ##
 ## the Free Software Foundation; either version 1, or (at your option)  ##
 ## any later version.                                                   ##
 ##                                                                      ##
 ## This program is distributed in the hope that it will be useful,      ##
 ## but WITHOUT ANY WARRANTY; without even the implied warranty of       ##
 ## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        ##
 ## GNU General Public License for more details.                         ##
 ##                                                                      ##
 ## You should have received a copy of the GNU General Public License    ##
 ## along with this program; if not, write to the Free Software          ##
 ## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            ##
 ##########################################################################
 */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "PutMySQL.h"

#include "config.h"
#include "mysql.h"

namespace putmysql {

using std::string;

PutMySQL::PutMySQL(const string &host, const string &user, const string &pass, const string &db) {
  const char *timeout = "10";

  // init our result stats
  putmysql_current_row = 0;
  putmysql_insert_id = 0;
  putmysql_num_rows = 0;
  putmysql_num_fields = 0;
  putmysql_num_affected_rows = 0;
  putmysql_free_result = false;
  putmysql_result = NULL;

  //mysql_library_init(&putmysql_mysql);
  mysql_init(&putmysql_mysql);
  mysql_options(&putmysql_mysql, MYSQL_OPT_CONNECT_TIMEOUT, timeout);

  putmysql_mysql.reconnect = 1;

  putmysql_is_ready = this->login(host.c_str(), user.c_str(), pass.c_str(), db.c_str());
} // PutMySQL::PutMySQL

PutMySQL::~PutMySQL() {
  if (putmysql_free_result == true)
    mysql_free_result(putmysql_result);

  mysql_close(&putmysql_mysql);
} // PutMySQL::~PutMySQL

void PutMySQL::end() {
  mysql_library_end();
} // PutMySQL::end

bool PutMySQL::login(const char *host, const char *user, const char *pass, const char *db) {
  // sanity checking
  if (host == NULL || user == NULL || pass == NULL || db == NULL)
    return false;

  // set alarm to prevent long timeouts
  alarm(10);
  // mysql_options(&putmysql_mysql, MYSQL_OPT_CONNECT_TIMEOUT, (const char *) "10");
  if (!mysql_real_connect(&putmysql_mysql, host, user, pass, db, 0, NULL, 0)) {
    // mysql_store_result() should have returned data
    putmysql_error_number = mysql_errno(&putmysql_mysql);
    putmysql_error_message = mysql_error(&putmysql_mysql);

    fprintf(stderr, "*** PutMySQL::login>  #%d %s\n", mysql_errno(&putmysql_mysql), mysql_error(&putmysql_mysql));
    alarm(0);
    return false;
  }
  alarm(0);

  // If there was no error return that we authed successfully
  return true;
}

bool PutMySQL::ping() {
  if (mysql_ping(&putmysql_mysql) != 0) {
    // mysql_store_result() should have returned data
    putmysql_error_number = mysql_errno(&putmysql_mysql);
    putmysql_error_message = mysql_error(&putmysql_mysql);
    fprintf(stderr, "*** PutMySQL::ping> #%d %s\n", mysql_errno(&putmysql_mysql), mysql_error(&putmysql_mysql));
    return false;
  } // if

  // If there was no error return that we authed successfully
  return true;
}

bool PutMySQL::queryf(const char *fmt, ...) {
  char q[PUTMYSQL_MAXBUF + 1];
  int query_size;
  va_list va;

  va_start(va, fmt);
  query_size = vsnprintf(q, sizeof(q), fmt, va);
  va_end(va);

  return query(q, query_size);
} // PutMySQL::queryf

const bool PutMySQL::query(const string &sqls) {
  return query(sqls.c_str(), sqls.length());
}

const bool PutMySQL::query(const char *q, const int query_size) {
  char *qu;
  int ret;


  // reset the query, results and counter
  reset();

  qu = new char[query_size+1];
  strncpy(qu, q, query_size+1);

  ret = mysql_real_query(&putmysql_mysql, qu, query_size);

  delete [] qu;

  if (ret != 0) {
    // The query failed for some reason.
    putmysql_error_number = mysql_errno(&putmysql_mysql);
    putmysql_error_message = mysql_error(&putmysql_mysql);

    if (ret == -1 && mysql_errno(&putmysql_mysql) != 1062) {
      // mysql_store_result() should have returned data
      fprintf(stderr, "*** PutMySQL::query> #%d %s\n", mysql_errno(&putmysql_mysql), mysql_error(&putmysql_mysql));
      fprintf(stderr, "*** PutMySQL::query> %s\n", q);
    } // if

    return false;
  } // if

  putmysql_result = mysql_store_result(&putmysql_mysql);
  if (putmysql_result) {
    // there are rows stored
    putmysql_fields = mysql_fetch_fields(putmysql_result);
    putmysql_num_fields = mysql_num_fields(putmysql_result);

    // retrieve results and call mysql_free_result()
    putmysql_num_rows = mysql_num_rows(putmysql_result);
    putmysql_free_result = true;

    return true;
  } // if
  else {
    // no result were returned, should there have been?
    putmysql_num_fields = mysql_field_count(&putmysql_mysql);

    if (putmysql_num_fields == 0) {
      // query didn't return data, probably wasn't a SELECT statement
      putmysql_num_affected_rows = mysql_affected_rows(&putmysql_mysql);
      putmysql_insert_id = mysql_insert_id(&putmysql_mysql);

      return true;
    } // else
    else {
      // mysql_store_result() should have returned data
      putmysql_error_number = mysql_errno(&putmysql_mysql);
      putmysql_error_message = mysql_error(&putmysql_mysql);
      fprintf(stderr, "PutMySQL::query>  #%d %s\n", mysql_errno(&putmysql_mysql), mysql_error(&putmysql_mysql));
      fprintf(stderr, "PutMySQL::query>  %s\n", q);

      return false;
    } // else
  } // else

  return false;
}

void PutMySQL::reset() {
  // was there a result?
  if (putmysql_result != NULL) {
    mysql_free_result(putmysql_result);
    putmysql_result = NULL;
  } // if

  // init our result stats
  putmysql_error_number = 0;
  putmysql_error_message = "";
  putmysql_current_row = 0;
  putmysql_insert_id = 0;
  putmysql_num_rows = 0;
  putmysql_num_fields = 0;
  putmysql_num_affected_rows = 0;
  putmysql_free_result = false;

  return;
} // PutMySQL::reset

char *PutMySQL::escape(const char *escape_me) {
  char *escaped_line = NULL;

  if (escape_me == NULL)
    return NULL;

  escaped_line = new char[(strlen(escape_me) * 2) + 1];
  mysql_escape_string(escaped_line, escape_me, strlen(escape_me));

  return escaped_line;
} // PutMySQL::escape

const string PutMySQL::Escape(const string &escapeMe) {
  string escapedLine;
  char *escaped_line = NULL;

  if (escapeMe.length() < 1)
    return "";

  escaped_line = new char[(escapeMe.length() * 2) + 1];
  mysql_escape_string(escaped_line, escapeMe.c_str(), escapeMe.length());

  escapedLine = escaped_line;

  delete [] escaped_line;

  return escapedLine;
} // PutMySQL::Escape

string &PutMySQL::unescape(string &unescapeMe) {
  string::size_type pos;		// string position

  pos = unescapeMe.find("\\");
  while(pos != string::npos) {
    unescapeMe.erase(pos, 1);
    pos = unescapeMe.find("\\");
  } // while

  return unescapeMe;
} // PutMySQL::unescape

const string PutMySQL::Unescape(const string &parseMe) {
  bool isEscaped = false;
  string ret = "";
  unsigned int pos;

  for(pos=0; pos < parseMe.length(); pos++) {
    if (!isEscaped && parseMe[pos] == '\\')
      isEscaped = true;
    else {
      isEscaped = false;
      ret += parseMe[pos];
    } // else
  } // for

  return ret; 
} // PutMySQL::Unescape

const string PutMySQL::Unescape(const char *parseMe) {
  string stringMe = parseMe;

  return Unescape(stringMe); 
} // PutMySQL::Unescape

bool PutMySQL::fetch() {
  if (putmysql_free_result == true) {
    putmysql_row = mysql_fetch_row(putmysql_result);

    if (putmysql_row == NULL)
      return false;

    return true;
  } // if

  return false;
} // PutMySQL::fetch

bool PutMySQL::operator++(int x) {
  if (putmysql_free_result == true) {
    if (putmysql_current_row >= putmysql_num_rows)
      return false;

    putmysql_row = mysql_fetch_row(putmysql_result);

    putmysql_current_row++;

    if (putmysql_row == NULL)
      return false;

    return true;
  } // if

  return false;
} // PutMySQL::operator++

const char *PutMySQL::operator[](const char *name) const {
  unsigned int i;

  if (putmysql_free_result == true) {
    for(i = 0; i < putmysql_num_fields; i++) {
      if (!strcasecmp(name, putmysql_fields[i].name))
        return putmysql_row[i];
    } // for

    // prevent coring and memory leak by returning a constant
    return PUTMYSQL_ERR_NOFIELD;
  } // if

  // prevent coring and memory leak by returning a constant
  return PUTMYSQL_ERR_NOFIELD;
} // PutMySQL::operator[]

const string PutMySQL::getFieldName(const unsigned int myNumber) const {
  if (putmysql_free_result == true) {
    if (myNumber < putmysql_num_fields)
      return putmysql_fields[myNumber].name;

    // prevent coring and memory leak by returning a constant
    return PUTMYSQL_ERR_NOFIELD;
  } // if

  // prevent coring and memory leak by returning a constant
  return PUTMYSQL_ERR_NOFIELD;
} // PutMySQL::getFieldName

const int PutMySQL::getFieldMaxLength(const unsigned int myNumber) const {
  if (putmysql_free_result == true) {
    if (myNumber < putmysql_num_fields)
      return putmysql_fields[myNumber].max_length;

    // prevent coring and memory leak by returning a constant
    return 0;
  } // if

  // prevent coring and memory leak by returning a constant
  return 0;
} // PutMySQL::getFieldMaxLength

}

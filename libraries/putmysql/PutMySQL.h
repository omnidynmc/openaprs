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
 $Id: PutMySQL.h,v 1.2 2005/12/13 16:08:01 omni Exp $
 **************************************************************************/

#ifndef __PUTMYSQL_H
#define __PUTMYSQL_H

#include <string>

#include "mysql.h"

namespace putmysql {

using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

//#define PUTMYSQL_MAXBUF		16777216
#define PUTMYSQL_MAXBUF		1048576

#define CREATE_PUTMYSQL(mysql_variable, sql_login) \
try { \
  mysql_variable = new PutMySQL(sql_login); \
  mysql_variable->queryf("%s", "SET NAMES 'utf8'"); \
} \
catch(bad_alloc xa) { \
  assert(false); \
}

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

using namespace std;

class PutMySQL {
  public:
  PutMySQL(const string &, const string &, const string &, const string &);
  ~PutMySQL();

  bool is_ready() { return putmysql_is_ready; }
  bool ping();
  bool login(const char *, const char *, const char *, const char *);
  bool queryf(const char *, ...);
  const bool query(const string &);
  const bool query(const char *, const int);
  bool fetch();

  const string getFieldName(const unsigned int) const;
  const int getFieldMaxLength(const unsigned int) const;

  const string getErrorMessage() const
    { return putmysql_error_message; }
  const unsigned int getErrorNumber() const
    { return putmysql_error_number; }

  unsigned int current_row() { return putmysql_current_row; }
  unsigned int insert_id() { return putmysql_insert_id; }
  unsigned int num_rows() { return putmysql_num_rows; }
  unsigned int num_fields() { return putmysql_num_fields; }
  unsigned int num_affected_rows() { return putmysql_num_affected_rows; }

  bool operator++(int);

  char *escape(const char *);
  const string Escape(const string &);
  const char *operator[](const char *) const;

  string &unescape(string &);
  const string Unescape(const string &);
  const string Unescape(const char *);

  void reset();
  static void end();

  private:
  bool putmysql_free_result;			// do we need to free results?
  bool putmysql_is_ready;			// are we connected?
  const char *host;				// mysql hostname
  const char *username;				// mysql username
  const char *pass;				// mysql password
  const char *db;				// mysql database
  string putmysql_error_message;		// error message
  string myUnescaped;				// unescaped string
  unsigned int putmysql_current_row;		// current row number
  unsigned int putmysql_insert_id;		// id of last inserted record
  unsigned int putmysql_error_number;		// error number
  unsigned int putmysql_num_rows;		// number of rows returned
  unsigned int putmysql_num_fields;		// number of fields
  unsigned int putmysql_num_affected_rows;	// number of rows affected

  // Define mysql support vars
  MYSQL putmysql_mysql;
  MYSQL_RES *putmysql_result;
  MYSQL_FIELD *putmysql_fields;
  MYSQL_ROW putmysql_row;

  protected:
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

// Exit codes

#define PUTMYSQL_EX_USAGE 1
#define PUTMYSQL_EX_MYSQLERR 2
#define PUTMYSQL_EX_CONSCHECK 3

// index into 'show fields from table'

#define PUTMYSQL_SHOW_FIELDNAME  0
#define PUTMYSQL_SHOW_TYPE       1
#define PUTMYSQL_SHOW_NULL       2
#define PUTMYSQL_SHOW_DEFAULT    4
#define PUTMYSQL_SHOW_EXTRA      5

#define PUTMYSQL_QUERYLEN		9216

#define PUTMYSQL_ERR_NOFIELD	"Error: field not found."
#define PUTMYSQL_ERR_NOERR	"Error: no error message given"

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

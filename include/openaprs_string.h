/**************************************************************************
 ** Dynamic Networking Solutions                                         **
 **************************************************************************
 ** OpenAPRS, Internet APRS MySQL Injector                               **
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
 $Id: openaprs_string.h,v 1.1 2005/11/21 18:16:03 omni Exp $
 **************************************************************************/

#ifndef __OPENAPRS_STRING_H
#define __OPENAPRS_STRING_H

#include <list>
#include <map>
#include <string>

#include "regex.h"

namespace openaprs {

using std::string;
using std::list;
using std::map;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class regexMatch { 
  public:
    string matchString;			// matched string
    regoff_t matchBegin;		// begining of match
    regoff_t matchEnd;			// end of match
    regoff_t matchLength;		// length of match

  private:  
  protected:
}; // regexReturn

struct openaprs_duration {
  long int days;
  long int days_total;
  long int hours;
  long int hours_total;
  long int minutes;
  long int minutes_total;
  long int seconds;
  long int seconds_total;
};

struct timeframe { 
  long int hours;
  long int minutes;
  long int seconds;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

int cmp(const char *s1, const char *s2);
char *mask(char *hostmask, const char *host, const int hostsize);
int stpcrlf(char *string);
bool isnum(const char *ch);
string &string_to_lower(string &);


const string dec2oct(const unsigned int);
const string f2c(const double);
const string openaprs_string_graph(const int, const int, const int);
const bool openaprs_string_acceptable(const char *, const string &);
const bool openaprs_string_duration(const string, unsigned int &);
const bool openaprs_string_valid_hostmask(const string, const bool);
const bool openaprs_string_valid_username(const string);
const bool openaprs_timeval_duration(const time_t, struct openaprs_duration &);
const bool openaprs_timeval_difference(const struct timeval &, const struct timeval &, struct timeval &);
const bool openaprs_string_regex(const string &regexRegex, const string &regexString, map<int, regexMatch> &);
const bool ereg(const string &regexRegex, const string &regexString, map<int, regexMatch> &);
const string &openaprs_string_int(string &, int);
const string &openaprs_string_trim(string &);
const unsigned int openaprs_string_pad(string &, const unsigned int);
const unsigned int openaprs_string_strip_crlf(string &);
const unsigned int openaprs_string_to_lower(string &);
const unsigned int openaprs_string_replace(const string &, const string &, string &);
const unsigned int openaprs_string_wordwrap(const string &, unsigned int, list<string> &);
const unsigned int openaprs_string_create_list(const list<string> &, string &, const string &);
const char *openaprs_date(time_t);
const string strtoupper(const string &);
const string strtolower(const string &);

}
#endif

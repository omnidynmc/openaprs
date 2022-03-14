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
 $Id: APRS.h,v 1.1 2005/11/21 18:16:02 omni Exp $
 **************************************************************************/

#ifndef __LIBAPRS_STRINGTOOL_H
#define __LIBAPRS_STRINGTOOL_H

#include <string>
#include <map>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <regex.h>

namespace aprs {

  using std::string;
  using std::map;
  using std::list;
  using std::pair;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class StringTool {
    public:
      StringTool();				// constructor
      virtual ~StringTool();			// destructor

      class regexMatch {
        public:
          string matchString;                 // matched string
          regoff_t matchBegin;                // begining of match
          regoff_t matchEnd;                  // end of match
          regoff_t matchLength;               // length of match
      }; // regexMatch

      /**********************
       ** Type Definitions **
       **********************/
      typedef map<int, regexMatch> regexMatchListType;
      typedef map<string, string> stringMapType;
      typedef list<string> matchListType;

      /***************
       ** Members **
       ***************/

      static const string char2hex(const char);
      static const string toLower(const string &);
      static const string toUpper(const string &);
      static const string far2cel(const double);
      static const bool ereg(const string &, const string &, regexMatchListType &);
      static const string trim(const string &);
      static const unsigned int replace(const string &, const string &, string &);
      static const string intToString(const int);
      static const size_t pad(string &, const unsigned int);
      static const size_t pad(string &, const string &, const unsigned int);
      static const unsigned int stripcrlf(string &);
      static const bool match(const char *, const char *);
      static const string float2string(const double, const int);
      static const string int2string(const int);
      static const string uint2string(const unsigned int);
      static const bool acceptable(const char *, const string &);
      static const string stringf(const size_t, const char *, ...);
      const int compare(const char *, const char *);

      const bool addString(const string &, const string &);
      const bool replaceString(const string &, const string &);
      const bool removeString(const string &);
      const bool isString(const string &);
      const string getString(const string &);
      const unsigned int matchStrings(const string &, matchListType &);
      inline stringMapType::iterator stringBegin() { return _stringList.begin(); }
      inline stringMapType::iterator stringEnd() { return _stringList.end(); }

      void showStringTree(const string, unsigned int, string, list<string> &);

      // iterators
      const unsigned int stringSize() { return _stringList.size(); }
      const unsigned int clearStrings() {
        unsigned int numStrings = stringSize();

        _stringList.clear();

        return numStrings;
      } // clearStrings

    protected:
    private:
      stringMapType _stringList;		// String list.
  };

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

#define STRINGTOOL_DEBUG_STRINGS(x, y, z) \
list<string> y; \
x->showStringTree(z, 1, "", y);

} // namespace aprs
#endif

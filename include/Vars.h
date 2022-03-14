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
 **************************************************************************/

#ifndef __VARS_H
#define __VARS_H

#include <map>
#include <string>
#include <list>
#include <ctime>

#include "noCaseCompare.h"

namespace openaprs {

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


class Vars {
  public:
    Vars();
    Vars(const string &);
    Vars(const Vars &);
    ~Vars();

    typedef map<string, string, noCaseCompare> varMapType;

    enum filterEnumType { NOTFOUND, YES, NO };

    const unsigned int Parse(const string &);
    const bool exists(const string &name) { return isName(name); }
    const bool isName(const string &);
    const bool add(const string &, const string &);
    const bool remove(const string &);
    const unsigned int compile(string &, const string &);
    const filterEnumType isFiltered(const string &, const string &);
    const string operator[](const string &);
    Vars &operator=(const string &);
    const string operator[](const unsigned i);
    const string getFieldAtIndex(const unsigned i);
    const string Escape(const string &);
    const string Unescape(const string &);
    const string Urlencode(const string &);
    const string Urldecode(const string &);
    const int oct2dec(int);
    const string char2hex(const char);
    char hex2char(const string &);
    const varMapType::size_type size() {
      return _varMap.size();
    } // size

  private:
    varMapType _varMap;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

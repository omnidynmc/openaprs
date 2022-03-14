/**************************************************************************
 ** OpenFrame Library                                                    **
 **************************************************************************
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

#ifndef __LIBOPENFRAME_URL_H
#define __LIBOPENFRAME_URL_H

#include <map>
#include <string>
#include <list>
#include <ctime>

#include "noCaseCompare.h"

namespace openframe {

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


class Url {
  public:
    Url();
    Url(const char, const char);
    Url(const string &);
    Url(const string &, const char, const char);
    ~Url();

    static const char DEFAULT_DELIMITER;
    static const char DEFAULT_FIELD_DELIMITER;
    static const bool DEFAULT_FIELD_UPPERCASE;
    static const bool DEFAULT_ENCODE_SPACES;

    typedef map<string, string, noCaseCompare> varMapType;
    typedef varMapType varMap_t;
    typedef varMap_t::iterator varMap_itr;
    typedef varMap_t::const_iterator varMap_citr;
    typedef varMap_t::size_type varMap_st;

    enum filterEnumType { NOTFOUND, YES, NO };

    const unsigned int Parse(const string &);
    const bool exists(const string &name) { return isName(name); }
    const bool isName(const string &);
    const bool add(const string &, const string &);
    const bool remove(const string &);
    const string compile();
    const unsigned int compile(string &, const string &);
    const filterEnumType isFiltered(const string &, const string &);
    const string operator[](const string &);
    Url &operator=(const string &);
    const string operator[](const unsigned i);
    const string getFieldAtIndex(const unsigned i);
    const string Escape(const string &);
    const string Unescape(const string &);
    static const string Urlencode(const string &);
    static const string Urldecode(const string &);
    static const string char2url(const char);
    static const int oct2dec(int);
    static const string char2hex(const char);
    static char hex2char(const string &);
    inline const bool uppercase() const { return _uppercase; }
    void uppercase(const bool uppercase) { _uppercase = uppercase; }
    inline const bool spaces() const { return _spaces; }
    void spaces(const bool spaces) { _spaces = spaces; }
    void print();
    void clear() { varMap.clear(); }
    const varMapType::size_type size() {
      return varMap.size();
    } // size
    varMap_itr begin() { return varMap.begin(); }
    varMap_itr end() { return varMap.end(); }

  private:
    bool _uppercase;
    bool _spaces;
    char _delimiter;
    char _fieldDelimiter;
    varMapType varMap;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

}
#endif

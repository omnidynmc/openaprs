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

#ifndef __LIBOPENFRAME_SERIALIZE_H
#define __LIBOPENFRAME_SERIALIZE_H

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


class Serialize {
  public:
    Serialize();
    Serialize(const char);
    Serialize(const string &);
    Serialize(const string &, const char);
    ~Serialize();

    typedef vector<string> varType;
    typedef varType::size_type varSizeType;

    static const char DEFAULT_DELIMITER;

    enum filterEnumType { NOTFOUND, YES, NO };

    const varSizeType Parse(const string &);
    const bool add(const string &);
    const bool add(const bool);
    const bool add(const int);
    const bool add(const unsigned int);
    const bool add(const double);
    const bool remove(const varSizeType);
    const varSizeType compile(string &);
    const string compile();
    Serialize &operator=(const string &);
    const string operator[](const varSizeType i);
    const string getFieldAtIndex(const varSizeType i);
    const string Escape(const string &);
    const string Unescape(const string &);
    const string Urlencode(const string &);
    const string Urldecode(const string &);
    const int oct2dec(int);
    const string char2hex(const char);
    char hex2char(const string &);
    const varSizeType size() { return _var.size(); }
    void clear() { _var.clear(); }

  private:
    char _delimiter;
    varType _var;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openframe
#endif

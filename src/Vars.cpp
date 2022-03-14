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
 $Id: Vars.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
 **************************************************************************/

#include <list>
#include <fstream>
#include <queue>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <iostream>
#include <string>
#include <exception>
#include <ctime>
#include <sstream>

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <math.h>

#include "Vars.h"

#include "openframe/StringToken.h"

#include "openaprs_string.h"

namespace openaprs {
  using openframe::StringToken;
  using namespace std;

  /**************************************************************************
   ** Vars Class                                                           **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  Vars::Vars() {
    _varMap.clear();
  } // Vars::Vars

  Vars::Vars(const string &varMe) {
    _varMap.clear();

    Parse(varMe);
  } // Vars::Vars

  Vars::~Vars() {
  } // Vars::~Vars

  Vars::Vars(const Vars &n) : _varMap(n._varMap) {
  } // Vars::Vars

  Vars &Vars::operator=(const string &varMe) {
    _varMap.clear();

    Parse(varMe);

    return *this;
  } // Vars::operator=

  const unsigned int Vars::Parse(const string &varMe) {
    bool isEscaped;
    string name, field, parseMe;
    size_t pos;

    // initialize variables
    parseMe = varMe;

    // format is field:message|field:message|...
    while(parseMe.length()) {
      // store field name
      name = "";
      for(pos=0; pos < parseMe.length() && parseMe[pos] != ':'; pos++)
        name += parseMe[pos];

      parseMe.erase(0, pos+1);

      //cout << "name(" << name << ")" << endl;
      //cout << "parseMe(" << parseMe << ")" << endl;

      // try and get field data
      field = "";
      for(pos=0; pos < parseMe.length(); pos++) {
        if (!isEscaped && parseMe[pos] == '|')
          break;

        if (!isEscaped && parseMe[pos] == '\\')
          isEscaped = true;
        else {
          isEscaped = false;
          field += parseMe[pos];
        } //else
      } // for

      parseMe.erase(0, pos+1);
      //cout << "field(" << field << ")" << endl;
      if (name.length() > 0 && field.length() > 0)
        _varMap.insert(pair<string, string>(name, Urldecode(field)));
    } // while

    return _varMap.size();
  } // Vars::Parse

  const bool Vars::add(const string &name, const string &field) {

    if (name.length() < 1 || field.length() < 1)
      return false;

    if (isName(name))
      return false;

    _varMap.insert(pair<string, string>(name, field));

    return true;
  } // Vars::add

  const bool Vars::isName(const string &findMe) {
    StringToken st;
    unsigned int i;

    st.setDelimiter(',');
    st = findMe;

    if (!st.size())
      return false;

    for(i=0; i < st.size(); i++) {
      if (_varMap.find(st[i]) == _varMap.end())
        return false;
    } // for

    return true;
  } // Vars::isName

  const Vars::filterEnumType Vars::isFiltered(const string &findMe, const string &filter) {
    varMapType::iterator ptr;
    StringToken f;
    map<int, regexMatch> regexList;
    string rest;
    unsigned int i;

    if ((ptr = _varMap.find(findMe)) == _varMap.end())
      return NOTFOUND;

//cout << "FOUND FILTER(" << filter << ")" << endl;

    f.setDelimiter(',');
    f = filter;

    if (f.size() < 1)
      return NO;

    for(i=0; i < f.size(); i++) {
      if (f[i].length() < 2)
        continue;

      rest = f[i].substr(1, f[i].length()-1);

      switch(f[i][0]) {
        case '@':
          if (!openaprs_string_regex(rest,
                                     ptr->second,
                                     regexList))
            return YES;
          break;
        case '=':
          if (rest != ptr->second)
            return YES;
          break;
        case '>':
//cout << ptr->second.c_str() << " > " << atof(rest.c_str()) << endl;
          if (atof(ptr->second.c_str()) < atof(rest.c_str()))
            return YES;
          break;
        case '<':
          if (atof(ptr->second.c_str()) > atof(rest.c_str()))
            return YES;
          break;
      } // switch
    } // for

    return NO;
  } // Vars::isFiltered

  const string Vars::operator[](const string &findMe) {
    varMapType::iterator ptr;

    if ((ptr = _varMap.find(findMe)) != _varMap.end())
      return ptr->second;

    return string("");
  } // Vars::operator[]

  const string Vars::operator[](const unsigned int w) {
    varMapType::iterator ptr;
    unsigned int i=0;

    for(ptr=_varMap.begin(); ptr != _varMap.end(); ptr++) {
      if (i == w)
        return ptr->first;

      i++;
    } // for

    return string("");
  } // Vars::operator[]

  const string Vars::getFieldAtIndex(const unsigned int w) {
    varMapType::iterator ptr;
    unsigned int i=0;

    for(ptr=_varMap.begin(); ptr != _varMap.end(); ptr++) {
      if (i == w)
        return ptr->second;

      i++;
    } // for

    return string("");
  } // Vars::getFieldAtIndex

  const unsigned int Vars::compile(string &ret, const string &fields) {
    StringToken st;
    varMapType::iterator ptr;
    bool wasMatched;
    stringstream s;
    unsigned int i;

    ret = "";
    st.setDelimiter(',');
    st = fields;

    s.str("");
    for(ptr = _varMap.begin(); ptr != _varMap.end(); ptr++) {
      if (st.size() > 0) {
        wasMatched = false;
        for(i=0; i < st.size() && !wasMatched; i++) {
          if (strtoupper(st[i]) == strtoupper(ptr->first))
            wasMatched = true;
        } // for

        if (wasMatched == false)
          continue;
      } // if

      if (s.str().length() > 0)
        s << "|";

      s << strtoupper(ptr->first) << ":" << Escape(ptr->second);
    } // for

    ret = s.str();

    return ret.length();
  } // compile

  const bool Vars::remove(const string &findMe) {
    varMapType::iterator ptr;

    if ((ptr = _varMap.find(findMe)) == _varMap.end())
      return false;

    _varMap.erase(findMe);

    return true;
  } // Vars::remove

  const string Vars::Escape(const string &parseMe) {
    string ret;
    size_t pos;

    ret = "";

    for(pos=0; pos < parseMe.length(); pos++) {
      if (parseMe[pos] == '|' || parseMe[pos] == '\\')
        ret += string("\\") + parseMe[pos];
      else
        ret += parseMe[pos];
    } // for

    return Urlencode(ret);
  } // Vars::Escape

  const string Vars::Urlencode(const string &parseMe) {
    stringstream s;
    size_t pos;

    s.str("");

    for(pos=0; pos < parseMe.length(); pos++) {
      // if this character is not normal ascii, encode it
      if ((int) parseMe[pos] < 32 || (int) parseMe[pos] > 126)
        s << string("%") << char2hex(parseMe[pos]);
      else if (parseMe[pos] == '%')
        s << string("%%");
      else
        s << parseMe[pos];
    } // for

    return s.str();
  } // Vars::Urlencode

  const string Vars::Urldecode(const string &parseMe) {
    bool isEscaped = false;
    string ret = "";
    stringstream alnum;
    size_t pos;

    alnum.str("");

    if (parseMe.length() == 1) {
      ret = parseMe;
      return ret;
    } // if

    for(pos=0; pos < parseMe.length(); pos++) {
      if (!isEscaped && parseMe[pos] == '%') {
        isEscaped = true;
      } // if
      else if (isEscaped && alnum.str().length() == 0 && parseMe[pos] == '%') {
        isEscaped = false;
        alnum.str("");
        ret += "%";
        continue;
      } // else if
      else if (isEscaped && alnum.str().length() < 2) {
        alnum << parseMe[pos];
      } // else if
      else {
        isEscaped = false;
        if (alnum.str().length()) {
          ret += (char) hex2char(alnum.str());
          alnum.str("");
          pos--;
          continue;
        } // if

        ret += parseMe[pos];
      } // else
    } // for

    if (alnum.str().length()) {
      ret += hex2char(alnum.str());
      alnum.str("");
    } // if

    return ret;
  } // Vars::Urldecode

  char Vars::hex2char(const string &hexNumber) {
    char *stop;
    return strtol(hexNumber.c_str(), &stop, 16);
  } // Vars::hex2char
	
  const int Vars::oct2dec(int oct) {
    int n,r,s=0,i;
    n=oct;
    for(i=0;n!=0;i++) {
      r=n%10;
      s=s+r*(int)pow(8,i);
      n=n/10;
    } // for

    return s;
  } // oct2dec

  const string Vars::Unescape(const string &parseMe) {
    bool isEscaped = false;
    string ret = "";
    size_t pos;

    if (parseMe.length() == 1) {
      ret = parseMe;
      return ret;
    } // if

    for(pos=0; pos < parseMe.length(); pos++) {
      if (!isEscaped && parseMe[pos] == '\\')
        isEscaped = true;
      else {
        isEscaped = false;
        ret += parseMe[pos];
      } // else
    } // for

    return ret;
  } // Vars::Unescape

  const string Vars::char2hex(const char dec) {
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=65-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=65-10;

    string r;
    r.append(&dig1, 1);
    r.append(&dig2, 1);
    return r;
  } // char2hex
} // namespace openaprs

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

#include <openframe/config.h>

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

#include "openframe/StringToken.h"
#include "openframe/StringTool.h"
#include "openframe/Url.h"

namespace openframe {

  /**************************************************************************
   ** Url Class                                                           **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const char Url::DEFAULT_DELIMITER 		= '&';
  const char Url::DEFAULT_FIELD_DELIMITER	= '=';
  const bool Url::DEFAULT_FIELD_UPPERCASE	= false;
  const bool Url::DEFAULT_ENCODE_SPACES	= false;

  Url::Url() : _delimiter(DEFAULT_DELIMITER), _fieldDelimiter(DEFAULT_FIELD_DELIMITER) {
    _uppercase = DEFAULT_FIELD_UPPERCASE;
    _spaces = DEFAULT_ENCODE_SPACES;
    varMap.clear();
  } // Url::Url

  Url::Url(const char delimiter, const char fieldDelimiter) : _delimiter(delimiter), _fieldDelimiter(fieldDelimiter) {
    _uppercase = DEFAULT_FIELD_UPPERCASE;
    _spaces = DEFAULT_ENCODE_SPACES;
    varMap.clear();
  } // Url::Url

  Url::Url(const std::string &varMe) : _delimiter(DEFAULT_DELIMITER), _fieldDelimiter(DEFAULT_FIELD_DELIMITER) {
    _uppercase = DEFAULT_FIELD_UPPERCASE;
    _spaces = DEFAULT_ENCODE_SPACES;
    varMap.clear();

    Parse(varMe);
  } // Url::Url

  Url::Url(const std::string &varMe, const char delimiter, const char fieldDelimiter) : _delimiter(delimiter), _fieldDelimiter(fieldDelimiter) {
    _uppercase = DEFAULT_FIELD_UPPERCASE;
    _spaces = DEFAULT_ENCODE_SPACES;
    varMap.clear();

    Parse(varMe);
  } // Url::Url

  Url::~Url() {
  } // Url::~Url

  Url &Url::operator=(const std::string &varMe) {
    varMap.clear();

    Parse(varMe);

    return *this;
  } // Url::operator=

  const unsigned int Url::Parse(const std::string &varMe) {
    bool isEscaped;
    std::string name, field, parseMe;
    size_t pos;

    varMap.clear();

    // initialize variables
    parseMe = varMe;

    // format is field:message|field:message|...
    while(parseMe.length()) {
      // store field name
      name = "";
      for(pos=0; pos < parseMe.length() && parseMe[pos] != _fieldDelimiter; pos++)
        name += parseMe[pos];

      parseMe.erase(0, pos+1);

      //cout << "name(" << name << ")" << endl;
      //cout << "parseMe(" << parseMe << ")" << endl;

      // try and get field data
      field = "";
      for(pos=0; pos < parseMe.length(); pos++) {
        if (!isEscaped && parseMe[pos] == _delimiter)
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
        varMap.insert(pair<string, string>(name, Urldecode(field)));
    } // while

    return varMap.size();
  } // Url::Parse

  const bool Url::add(const std::string &name, const std::string &field) {

    if (name.length() < 1 || field.length() < 1)
      return false;

    if (isName(name))
      return false;

    varMap.insert(pair<string, string>(name, field));

    return true;
  } // Url::add

  const bool Url::isName(const std::string &findMe) {
    StringToken st;
    unsigned int i;

    st.setDelimiter(',');
    st = findMe;

    if (!st.size())
      return false;

    for(i=0; i < st.size(); i++) {
      if (varMap.find(st[i]) == varMap.end())
        return false;
    } // for

    return true;
  } // Url::isName

  const Url::filterEnumType Url::isFiltered(const std::string &findMe, const std::string &filter) {
    varMapType::iterator ptr;
    StringToken f;
    StringTool::regexMatchListType regexList;
    std::string rest;
    unsigned int i;

    if ((ptr = varMap.find(findMe)) == varMap.end())
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
          if (!StringTool::ereg(rest,
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
  } // Url::isFiltered

  const std::string Url::operator[](const std::string &findMe) {
    varMapType::iterator ptr;

    if ((ptr = varMap.find(findMe)) != varMap.end())
      return ptr->second;

    return string("");
  } // Url::operator[]

  const std::string Url::operator[](const unsigned int w) {
    varMapType::iterator ptr;
    unsigned int i=0;

    for(ptr=varMap.begin(); ptr != varMap.end(); ptr++) {
      if (i == w)
        return ptr->first;

      i++;
    } // for

    return string("");
  } // Url::operator[]

  const std::string Url::getFieldAtIndex(const unsigned int w) {
    varMapType::iterator ptr;
    unsigned int i=0;

    for(ptr=varMap.begin(); ptr != varMap.end(); ptr++) {
      if (i == w)
        return ptr->second;

      i++;
    } // for

    return string("");
  } // Url::getFieldAtIndex

  const std::string Url::compile() {
    std::string ret = "";

    if (compile(ret, ""))
      return ret;

    return string("");
  } // Url::compile

  const unsigned int Url::compile(string &ret, const std::string &fields) {
    StringToken st;
    varMapType::iterator ptr;
    bool wasMatched;
    std::stringstream s;
    unsigned int i;

    ret = "";
    st.setDelimiter(',');
    st = fields;

    s.str("");
    for(ptr = varMap.begin(); ptr != varMap.end(); ptr++) {
      if (st.size() > 0) {
        wasMatched = false;
        for(i=0; i < st.size() && !wasMatched; i++) {
          if (StringTool::toUpper(st[i]) == StringTool::toUpper(ptr->first))
            wasMatched = true;
        } // for

        if (wasMatched == false)
          continue;
      } // if

      if (s.str().length() > 0)
        s << string(1, _delimiter);

      s << (_uppercase ? StringTool::toUpper(ptr->first) : ptr->first)
        << string(1, _fieldDelimiter) << Urlencode(ptr->second);
    } // for

    ret = s.str();

    return ret.length();
  } // compile

  const bool Url::remove(const std::string &findMe) {
    varMapType::iterator ptr;

    if ((ptr = varMap.find(findMe)) == varMap.end())
      return false;

    varMap.erase(findMe);

    return true;
  } // Url::remove

  const std::string Url::Escape(const std::string &parseMe) {
    std::string ret;
    size_t pos;

    ret = "";

    for(pos=0; pos < parseMe.length(); pos++) {
      if (parseMe[pos] == _delimiter || parseMe[pos] == '\\')
        ret += string("\\") + parseMe[pos];
      else
        ret += parseMe[pos];
    } // for

    return Urlencode(ret);
  } // Url::Escape

  const std::string Url::Urlencode(const std::string &parseMe) {
    std::stringstream s;
    size_t pos;

    s.str("");

    for(pos=0; pos < parseMe.length(); pos++)
      s << Url::char2url(parseMe[pos]);

    return s.str();
  } // Url::Urlencode

  const std::string Url::char2url(const char c) {
    // +'s as spaces
    if (c == ' ')
      return "+";

    // only exception
    if (c == '-' || c == '_' || c == '.' || c == '~')
      return string(1, c);

    std::stringstream s;
    int n = (int) c;
    if ((n >= 48 && n <= 57)
         || (n >=65 && n <= 90)
         || (n >=97 && n <= 122))
      s << c;
    else
      s << string("%") << char2hex(c);

    return s.str();
  } // Url::char2url

  const std::string Url::Urldecode(const std::string &parseMe) {
    bool isEscaped = false;
    std::string ret = "";
    std::stringstream alnum;
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

        if (parseMe[pos] == '+')
          ret += " ";
        else
          ret += parseMe[pos];
      } // else
    } // for

    if (alnum.str().length()) {
      ret += hex2char(alnum.str());
      alnum.str("");
    } // if

    return ret;
  } // Url::Urldecode

  char Url::hex2char(const std::string &hexNumber) {
    char *stop;
    return strtol(hexNumber.c_str(), &stop, 16);
  } // Url::hex2char

  const int Url::oct2dec(int oct) {
    int n,r,s=0,i;
    n=oct;
    for(i=0;n!=0;i++) {
      r=n%10;
      s=s+r*(int)pow(8,i);
      n=n/10;
    } // for

    return s;
  } // oct2dec

  const std::string Url::Unescape(const std::string &parseMe) {
    bool isEscaped = false;
    std::string ret = "";
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
  } // Url::Unescape

  const std::string Url::char2hex(const char dec) {
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=65-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=65-10;

    std::string r;
    r.append(&dig1, 1);
    r.append(&dig2, 1);
    return r;
  } // char2hex

  void Url::print() {
    varMapType::iterator ptr;
    for(ptr = varMap.begin(); ptr != varMap.end(); ptr++)
      std::cout << ptr->first << " = " << ptr->second << std::endl;
  } // Url::print
} // namespace openframe

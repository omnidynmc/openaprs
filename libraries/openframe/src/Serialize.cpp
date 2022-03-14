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

#include <openframe/StringToken.h>
#include <openframe/Serialize.h>
#include <openframe/stringify.h>

namespace openframe {
  /**************************************************************************
   ** Serialize Class                                                      **
   **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  const char Serialize::DEFAULT_DELIMITER		= '|';

  Serialize::Serialize() : _delimiter(DEFAULT_DELIMITER) {
    _var.clear();
  } // Serialize::Serialize

  Serialize::Serialize(const char delimiter) : _delimiter(delimiter) {
    _var.clear();
  } // Serialize::Serialize

  Serialize::Serialize(const std::string &varMe) : _delimiter(DEFAULT_DELIMITER) {
    _var.clear();

    Parse(varMe);
  } // Serialize::Serialize

  Serialize::Serialize(const std::string &varMe, const char delimiter) : _delimiter(delimiter) {
    _var.clear();

    Parse(varMe);
  } // Serialize::Serialize

  Serialize::~Serialize() {
  } // Serialize::~Serialize

  Serialize &Serialize::operator=(const std::string &varMe) {
    _var.clear();

    Parse(varMe);

    return *this;
  } // Serialize::operator=

  const Serialize::varSizeType Serialize::Parse(const std::string &varMe) {
    bool isEscaped;
    std::string field, parseMe;
    size_t pos;

    // initialize variables
    parseMe = varMe;

    // format is field:message|field:message|...
    while(parseMe.length()) {
      field = "";
      for(pos=0; pos < parseMe.length(); pos++) {
        if (!isEscaped && parseMe[pos] == _delimiter)
          break;

        if (!isEscaped && parseMe[pos] == '\\')
          isEscaped = true;
        else if (isEscaped && parseMe[pos] == '0')
          isEscaped = false;
        else {
          isEscaped = false;
          field += parseMe[pos];
        } //else
      } // for

      parseMe.erase(0, pos+1);
      //cout << "field(" << field << ")" << endl;
//      if (field.length() > 0)
        _var.push_back( Urldecode(field) );
    } // while

    return _var.size();
  } // Serialize::Parse

  const bool Serialize::add(const std::string &field) {
//    if (field.length() < 1)
//      return false;

    _var.push_back( field );

    return true;
  } // Serialize::add


  const bool Serialize::add(const bool field) {
    _var.push_back( stringify<bool>(field) );

    return true;
  } // Serialize::add

  const bool Serialize::add(const int field) {
    _var.push_back( stringify<int>(field) );

    return true;
  } // Serialize::add

  const bool Serialize::add(const unsigned int field) {
    _var.push_back( stringify<unsigned int>(field) );

    return true;
  } // Serialize::add

  const bool Serialize::add(const double field) {
    _var.push_back( stringify<double>(field) );

    return true;
  } // Serialize::add

  const std::string Serialize::operator[](const varSizeType w) {
    return getFieldAtIndex(w);
  } // Serialize::operator[]

  const std::string Serialize::getFieldAtIndex(const varSizeType w) {
    if (w > _var.size()-1)
      return string("");

    return _var[w];
  } // Serialize::getFieldAtIndex

  const Serialize::varSizeType Serialize::compile(string &ret) {
    std::stringstream s;

    for(varSizeType i=0; i < _var.size(); i++) {
      if (i) s << "|";
      s << Escape(_var[i]);
    } // for

    ret = s.str();

    return ret.length();
  } // compile

  const std::string Serialize::compile() {
    std::stringstream s;

    for(varSizeType i=0; i < _var.size(); i++) {
      if (i) s << "|";
      s << Escape(_var[i]);
    } // for

    return s.str();
  } // compile

  const bool Serialize::remove(const size_t i) {
    if (i > _var.size()-1)
      return false;

    _var.erase(_var.begin()+i);

    return true;
  } // Serialize::remove

  const std::string Serialize::Escape(const std::string &parseMe) {
    std::string ret;
    size_t pos;

    if (parseMe.length() < 1)
      return "\\0";

    ret = "";

    for(pos=0; pos < parseMe.length(); pos++) {
      if (parseMe[pos] == _delimiter || parseMe[pos] == '\\')
        ret += string("\\") + parseMe[pos];
      else
        ret += parseMe[pos];
    } // for

    return Urlencode(ret);
  } // Serialize::Escape

  const std::string Serialize::Urlencode(const std::string &parseMe) {
    std::stringstream s;
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
  } // Serialize::Urlencode

  const std::string Serialize::Urldecode(const std::string &parseMe) {
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

        ret += parseMe[pos];
      } // else
    } // for

    if (alnum.str().length()) {
      ret += hex2char(alnum.str());
      alnum.str("");
    } // if

    return ret;
  } // Serialize::Urldecode

  char Serialize::hex2char(const std::string &hexNumber) {
    char *stop;
    return strtol(hexNumber.c_str(), &stop, 16);
  } // Serialize::hex2char

  const int Serialize::oct2dec(int oct) {
    int n,r,s=0,i;
    n=oct;
    for(i=0;n!=0;i++) {
      r=n%10;
      s=s+r*(int)pow(8,i);
      n=n/10;
    } // for

    return s;
  } // oct2dec

  const std::string Serialize::Unescape(const std::string &parseMe) {
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
  } // Serialize::Unescape

  const std::string Serialize::char2hex(const char dec) {
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
} // namespace openframe

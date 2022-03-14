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
 $Id: StringToken.cpp,v 1.1 2005/11/21 18:16:04 omni Exp $
 **************************************************************************/

#include <vector>
#include <new>
#include <cassert>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "StringToken.h"

namespace aprs {

using std::string;
using std::vector;

/**************************************************************************
 ** StringToken Class                                                    **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

StringToken::StringToken() {
  // set default
  myDelimiter = STRINGTOKEN_DELIMITER;
} // StringToken::StringToken

StringToken::StringToken(const string &parseMe) {
  // set the string we want to parse
  myString = parseMe;
  myDelimiter = STRINGTOKEN_DELIMITER;

  tokenize();
} // StringToken::StringToken

StringToken::StringToken(const string &parseMe, char splitMe) {
  // set the string we want to parse
  myString = parseMe;
  myDelimiter = splitMe;

  tokenize();
} // StringToken::StringToken

StringToken::~StringToken() {
} // StringToken::~StringToken

/**********************
 ** Operator Members **
 **********************/

StringToken &StringToken::operator=(const string &parseMe) {
  // set the string we want to parse
  myString = parseMe;

  tokenize();

  return *this;
} // StringToken::operator=

const string &StringToken::operator[](const size_type myID) const {
  assert(myID <= size());

  return stringList[myID];
} // StringToken::opterator=

/*******************
 ** Token Members **
 *******************/

const string &StringToken::getToken(const size_type myID) const {
  assert(myID <= size());

  return stringList[myID];
} // StringToken::getToken

void StringToken::add(const string &addMe) {
  stringList.push_back(addMe);

  return;
} // StringToken::add

void StringToken::clear() {
  stringList.clear();
  return;
} // StringToken::clear

void StringToken::tokenize() {
  string::size_type pos;		// position in string

  clear();

  while(!myString.empty()) {
    pos = myString.find(myDelimiter);

    if (pos != string::npos) {
      // add as a token then erase
      add(myString.substr(0, pos));
      myString.erase(0, pos + 1);

      // try again next time
      continue;
    } // if

    // we hit the end of the string
    add(myString.substr(0, myString.length()));
    myString.erase(0, myString.length());
  } // while

  return;
} // StringToken::tokenize

const string StringToken::join(const string &delimiter) {
  string ret = "";
  int i;

  if (!size())
    return ret;

  for(i=0; i < size(); i++) {
    if (i > 0)
      ret.append(delimiter);

    ret.append(getToken(i));
  } // for

  return ret;
} // StringToken::join

/*******************
 ** Range Members **
 *******************/

const string &StringToken::getRange(const size_type x, const size_type y) {
  size_type i;			// counter

  assert(x <= size());

  myString = "";

  for(i = x; i < y; i++) {
    if (i > x)
      myString.append(&myDelimiter, 1);

    myString.append(getToken(i));
  } // for

  return myString;
} // StringToken::getRange

const string &StringToken::getTrailing(const size_type x) {
  return getRange(x, size());
} // StringToken::getTrailing

}

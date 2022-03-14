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

#include <vector>
#include <new>
#include <cassert>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "openframe/StringToken.h"

namespace openframe {

using std::string;
using std::vector;

/**************************************************************************
 ** StringToken Class                                                    **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

StringToken::StringToken() : _wantQuote(false) {
  // set default
  myDelimiter = STRINGTOKEN_DELIMITER;
} // StringToken::StringToken

StringToken::StringToken(const string &parseMe) : _wantQuote(false) {
  // set the string we want to parse
  myString = parseMe;
  myDelimiter = STRINGTOKEN_DELIMITER;

  tokenize();
} // StringToken::StringToken

StringToken::StringToken(const string &parseMe, char splitMe) : _wantQuote(false) {
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
  bool enQuote = false;
  string buf = "";
  clear();

  string::size_type lpos = 0;
  for(string::size_type pos=0; pos <  myString.size(); pos++) {
    // if we're in a quote speed till end quote
    if (_wantQuote) {
      for(; enQuote && pos < myString.size(); pos++) {
        if (myString[pos] == _quote)
          enQuote = false;
      } // for

      if (myString[pos] == _quote) {
        enQuote = true;
        continue;
      } // if
    } // if

    char c = myString[pos];
    if (c == myDelimiter) {
      size_t len = pos-lpos;
      if (len) {
        string tok = myString.substr(lpos, len);
        add(tok);
      } // if
      lpos = pos+1;
    } // if

  } // for

  if (lpos < myString.size()) {
    size_t len = myString.size() - lpos;
    if (len) {
      string tok = myString.substr(lpos, len);
      add(tok);
    } // if
  } // if

} // StringToken::tokenize

/*
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
*/

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

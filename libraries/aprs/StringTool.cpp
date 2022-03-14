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
 $Id: APRS.cpp,v 1.2 2005/12/13 21:07:03 omni Exp $
 **************************************************************************/

#include <list>
#include <new>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>
#include <regex.h>
#include <string.h>

#include "StringTool.h"
#include "StringToken.h"

namespace aprs {

using namespace std;

/**************************************************************************
 ** StringTool Class                                                     **
 **************************************************************************/

/******************************
 ** Constructor / Destructor **
 ******************************/

  StringTool::StringTool() {
    // set defaults
  } // StringTool::StringTool

  StringTool::~StringTool() {
    // cleanup
    clearStrings();
  } // StringTool::~StringTool

  /***************
   ** Variables **
   ***************/

  const string StringTool::char2hex(const char dec) {
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);

    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=97-10;

    string r;
    r.append(&dig1, 1);
    r.append(&dig2, 1);

    return r;
  } // StringTool::char2hex

  const string StringTool::toLower(const string &lowerMe) {
    string ret;
    size_t pos;

    for(pos=0; pos < lowerMe.length(); pos++) {
      if (lowerMe[pos] >= 65 && lowerMe[pos] <= 90)
        ret += lowerMe[pos]+32;
      else
        ret += lowerMe[pos];
    } // for

    return ret;
  } // StringTool::toLower

  const string StringTool::toUpper(const string &upMe) {
    string ret;
    size_t pos;

    for(pos=0; pos < upMe.length(); pos++) {
      if (upMe[pos] >= 97 && upMe[pos] <= 122)
        ret += upMe[pos]-32;
      else
        ret += upMe[pos];
    } // for

    return ret;
  } // StringTool::toUpper

  const unsigned int StringTool::stripcrlf(string &stripMe) {
    string tempBuf;               // temp buffer
    unsigned int i;               // counter
    unsigned int j;               // counter

    // initialize variables
    tempBuf = "";
    j = 0;

    for(i = 0; i < stripMe.length(); i++) {
      if (stripMe[i] == '\r' || stripMe[i] == '\n') {
        j++;
        continue;
      } // if

      tempBuf += stripMe[i];
    } // for
    stripMe = tempBuf;

    return j;
  } // StringTool::stripcrlf


  const bool StringTool::ereg(const string &regexRegex, const string &regexString, map<int, regexMatch> &regexList) {
    regexMatch aMatch;		// regex match
    char errbuf[255];		// error buffer
    regex_t preg;			// preg
    regmatch_t *pmatch = NULL;	// pmatch
    size_t nmatch;		// nmatch
    int errcode;			// error code
    unsigned int i;		// counter

    // compile regex
    if ((errcode = regcomp(&preg, regexRegex.c_str(), REG_EXTENDED)) != 0) {
      // try and snag the text for the error
      regerror(errcode, &preg, errbuf, 255);

      //aServer->writeLog(OPENAPRS_LOG_DEBUG, "regcomp()> #%d %s", errcode, errbuf);
      // debug(HERE, "regcomp(): errno(%d) errmsg(%s)\n", errcode, errbuf);

      return false;
    } // if

    // initialize variables
    nmatch = preg.re_nsub + 1;
    pmatch = new regmatch_t[nmatch];
    regexList.clear();

    if (regexec(&preg, regexString.c_str(), nmatch, pmatch, 0) != 0) {
      // try and snag the text for the error
      regerror(errcode, &preg, errbuf, 255);

      //if (errcode != 0)
      //aServer->writeLog(OPENAPRS_LOG_DEBUG, "regexec()> #%d %s", errcode, errbuf);
      //  debug(HERE, "regcomp(): errno(%d) errmsg(%s)\n", errcode, errbuf);

      // cleanup
      delete [] pmatch;
      regfree(&preg);

      return false;
    } // if

    // save the results
    for(i = 0; i < nmatch && pmatch[i].rm_so != -1 && pmatch[i].rm_eo != -1; i++) {
      // cout << i << endl;
      // cout << pmatch[i].rm_so << " " << pmatch[i].rm_eo << endl;
      if (pmatch[i].rm_so == -1 || pmatch[i].rm_eo == -1)
        continue;

      aMatch.matchString = regexString.substr(pmatch[i].rm_so, (pmatch[i].rm_eo - pmatch[i].rm_so));
      aMatch.matchBegin = pmatch[i].rm_so;
      aMatch.matchEnd = pmatch[i].rm_eo;
      aMatch.matchLength = (pmatch[i].rm_eo - pmatch[i].rm_so);

      // cout << aMatch.matchString << endl;

      regexList[i] = aMatch;
      // debug(HERE, "openaprs_string_regex(%s): Number: %d, Start: %d, End: %d, Match: %s\n", regexRegex.c_str(), nmatch, (long) aMatch.matchBegin, (long) aMatch.matchEnd, aMatch.matchString.c_str());
    } // for

    // cleanup
    delete [] pmatch;
    regfree(&preg);

    return true;
  } // ereg

  const string StringTool::trim(const string &trimMe) {
    string ret;
    string tempBuf;             // temp buffer
    unsigned int i;             // counter

    for(i = 0; i < trimMe.length(); i++)
      if (trimMe[i] != ' ')
        break;

    ret = trimMe.substr(i, trimMe.length());

    for(i = ret.length(); i > 0; i--)
      if (ret[(i - 1)] != ' ')
        break;

    ret = ret.substr(0, i);

    return ret;
  } // StringTool::trim

  const unsigned int StringTool::replace(const string &replaceMe, const string &replaceWith, string &replaceLine) {
    unsigned int i;		// counter
    size_t pos;		// position in string

    // initialize variables
    i = 0;
    pos = 0;

    for(pos = replaceLine.find(replaceMe, pos); pos != string::npos; pos = replaceLine.find(replaceMe, pos)) {
      i++;
      replaceLine.replace(pos, replaceMe.length(), replaceWith);
      pos += replaceWith.length();
    } // for

    return i;
  } // StringTool::replace

  const string StringTool::intToString(const int num) {
    stringstream s;

    s.str("");
    s << num;
    return s.str();
  } // StringTool::intToString

  const size_t StringTool::pad(string &padMe, const unsigned int padLength) {
    unsigned int i;			// counter

    for(i = padMe.length(); i < padLength; i++)
      padMe += " ";

    return padMe.length();
  } // StringTool::pad

  const size_t StringTool::pad(string &padMe, const string &with, const unsigned int padLength) {
    unsigned int i;			// counter

    for(i = padMe.length(); i < padLength; i++)
      padMe += with;

    return padMe.length();
  } // StringTool::pad

  const string StringTool::far2cel(const double convertMe) {
    char tempBuf[7];
    double temperature;

    temperature = ((convertMe - 32) / 9) * 5;

    snprintf(tempBuf, 7, "%0.2f", temperature);

    return tempBuf;
  } // far2cel

  const bool StringTool::addString(const string &name, const string &value) {
    stringMapType::iterator ptr;                  // pointer to a string list

    ptr = _stringList.find(name);
    if (ptr != _stringList.end()) {
      ptr->second = value;
      return true;
    } // if

    _stringList.insert( make_pair(name, value) );

    return true;
  } // StringTool::addString


  const bool StringTool::replaceString(const string &stringName, const string &addMe) {
    removeString(stringName);
    addString(stringName, addMe);

    return true;
  } // StringTool::replaceString

  const bool StringTool::removeString(const string &stringName) {
    stringMapType::iterator ptr;                  // pointer to a string list
    if ((ptr = _stringList.find(stringName)) == _stringList.end())
      return false;

    _stringList.erase(ptr);

    return true;
  } // StringTool::removeString

  const string StringTool::getString(const string &stringName) {
    stringMapType::iterator ptr;

    if ((ptr = _stringList.find(stringName)) == _stringList.end())
      return "";

    return ptr->second;
  } // StringTool::getString

  const bool StringTool::isString(const string &stringName) {
    stringMapType::iterator ptr;

    if ((ptr = _stringList.find(stringName)) == _stringList.end())
      return false;

    return true;
  } // StringTool::isString

  void StringTool::showStringTree(const string treeMember, unsigned int treeLevel, 
                              string treeSpacer, list<string> &treeList) {
    StringToken treeToken;                        // Tokenize a stringName.
    StringToken treeMemberToken;                  // Tokenise a treeMember name.
    stringMapType::iterator treePtr;              // iterator to a string map
    int numMatches;                               // number of matches
    int maxMatches;                               // max matches
    string stringName;                            // Name of the string.
    string stringValue;                           // Value of the string.
    string lastName;                              // Last name matched.
    stringstream s;                               // stringstream

    // initialize variables
    treeToken.setDelimiter('.');
    treeMemberToken.setDelimiter('.');
    treeMemberToken = treeMember;

    if (treeLevel == 1) {
      s << treeSpacer << treeMember << " (0)";
      treeList.push_back(s.str());
      // cout << endl;
    } // if
    // cout << "treeMember: " << treeMember << endl;

    // count matches
    maxMatches = 0;
    treeLevel = treeMemberToken.size();

    for(treePtr = _stringList.begin(); treePtr != _stringList.end(); treePtr++) {
      // initialize variables
      treeToken = stringName = string("root.") + treePtr->first;
      if ((treeLevel + 1) > treeToken.size() ||
          treeLevel > treeMemberToken.size())
        continue;

      // cout << "[" << stringName << "] " << treeToken.getRange(0, (treeLevel + 1)) << " != " << lastName << endl;
      if (treeToken.getRange(0, treeLevel)  == treeMemberToken.getRange(0, treeLevel) &&
          treeToken.getRange(0, (treeLevel + 1)) != lastName) {
        // cout << "[" << stringName << "] " << treeToken.getRange(0, treeLevel) << " == " << treeMember << endl;
        maxMatches++;
        lastName = treeToken.getRange(0, (treeLevel + 1));
      } // if
    } // for

    // initialize variables
    numMatches = 0;
    lastName = "";

    for(treePtr = _stringList.begin(); treePtr != _stringList.end(); treePtr++) {
      // initialize variables
      treeToken = stringName = string("root.") + treePtr->first;
      stringValue = treePtr->second;

      if ((treeLevel + 1) > treeToken.size() ||
          treeLevel > treeMemberToken.size())
        continue;

      if (treeToken.getRange(0, treeLevel) == treeMemberToken.getRange(0, treeLevel) &&
          treeToken.getRange(0, (treeLevel + 1)) != lastName) {
        numMatches++;
        treeSpacer += (numMatches == maxMatches) ? " " : " |";
        s.str("");
        s << treeSpacer << ((numMatches == maxMatches) ? "`" : "") << "-"
          << treeToken[treeLevel];
        if ((treeLevel + 2) > treeToken.size())
          s << " = \"" << stringValue << "\"";
        treeList.push_back(s.str());
        treeSpacer += (numMatches == maxMatches) ? " " : "";
        showStringTree(treeToken.getRange(0, treeLevel), ++treeLevel, treeSpacer, treeList);
        treeSpacer.replace((treeSpacer.length() - 2), 2, "");
        treeLevel--;
        lastName = treeToken.getRange(0, (treeLevel + 1));
      } // if
    }  // for

    return;
  } // StringTool::showStringTree

  const unsigned int StringTool::matchStrings(const string &matchName, matchListType &matchList) {
    stringMapType::iterator ptr;          // Iterator to an Option map.
    unsigned int size;

    // initialize variables
    matchList.clear();

    for(ptr = _stringList.begin(); ptr != _stringList.end(); ptr++) {
      if (match(matchName.c_str(), ptr->first.c_str()))
        matchList.push_back(ptr->first);
    } // for

    size = matchList.size();

    return size;
  } // StringTool::matchStrings

  const bool StringTool::match(const char *check, const char *orig) {
    while (*check == '*' || tolower(*check) == tolower(*orig) || *check == '?') {
      if (*check == '*') {
        if (*++check) {
          while (*orig)
            if (match(check, orig++))
              return true;

          return false;
        } // if
        else {
          return true;
        } // else
      } // if
      else if (!*check)
        return true;
      else if (!*orig)
        return false;
      else {
        ++check;
        ++orig;
      } // if
    } // while

    return false;
  } // StringTool::match

  const string StringTool::float2string(const double f, const int precision) {
    stringstream s;

    s.str("");
    s << setprecision(precision) << f;

    return s.str();
  } // StringTool::float2string

  const string StringTool::int2string(const int i) {
    stringstream s;

    s.str("");
    s << i;

    return s.str();
  } // StringTool::int2string

  const string StringTool::uint2string(const unsigned int i) {
    stringstream s;

    s.str("");
    s << i;

    return s.str();
  } // StringTool::uint2string

  const bool StringTool::acceptable(const char *acceptChars, const string &acceptMe) {
    unsigned int i;                       // counter
    unsigned int j;                       // counter

    // make sure the search is acceptable
    for(i = 0; i < acceptMe.length(); i++) {
      for(j = 0; j < strlen(acceptChars); j++) {
        if (*(acceptChars + j) == acceptMe[i])
          break;
      } // for

      // we didn't find it as an acceptible char
      if (j == strlen(acceptChars))
        return false;
    } // for

    return true;
  } // StringTool::acceptable

  const string StringTool::stringf(const size_t maxsize, const char *fmt, ...) {
    char *buf;
    string ret = "";
    va_list va;
    int written;

    buf = new char[maxsize + 1];

    va_start(va, fmt);
    written = vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    if (written)
      ret.append(buf, written);

    delete buf;

    return ret;
  } // StringTool::stringf

  const int StringTool::compare(const char *s1, const char *s2) {
    return (strcasecmp(s1, s2));
  } // StringTool::compare

} // namespace aprs

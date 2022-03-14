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
#include <vector>

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
#include <errno.h>

// #include <ossp/uuid.h>

#include <openframe/StringTool.h>
#include <openframe/StringToken.h>

namespace openframe {

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

  const std::string StringTool::char2hex(const char dec, const bool upper) {
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);

    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=(upper ? 65 : 97)-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=(upper ? 65 : 97)-10;

    std::string r;
    r.append(&dig1, 1);
    r.append(&dig2, 1);

    return r;
  } // StringTool::char2hex

  const std::string StringTool::toLower(const std::string &lowerMe) {
    std::string ret;
    size_t pos;

    for(pos=0; pos < lowerMe.length(); pos++) {
      if (lowerMe[pos] >= 65 && lowerMe[pos] <= 90)
        ret += lowerMe[pos]+32;
      else
        ret += lowerMe[pos];
    } // for

    return ret;
  } // StringTool::toLower

  const std::string StringTool::toUpper(const std::string &upMe) {
    std::string ret;
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
    std::string tempBuf;               // temp buffer
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

  const bool StringTool::ereg(const std::string &regexRegex, const std::string &regexString, map<int, regexMatch> &regexList) {
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
    } // for

    // cleanup
    delete [] pmatch;
    regfree(&preg);

    return true;
  } // ereg

  const std::string StringTool::trim(const std::string &trimMe) {
    std::string ret;
    std::string tempBuf;             // temp buffer
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

  const unsigned int StringTool::replace(const std::string &replaceMe, const std::string &replaceWith, std::string &replaceLine) {
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

  const std::string StringTool::intToString(const int num) {
    std::stringstream s;

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

  const std::string StringTool::ipad(const std::string &padMe, const std::string &with, const unsigned int padLength) {
    std::string ret = padMe;

    for(unsigned int i = ret.length(); i < padLength; i++)
      ret += with;

    return ret;
  } // StringTool::ipad

  const std::string StringTool::ppad(const std::string &padMe, const std::string &with, const unsigned int padLength) {
    std::string ret = padMe;

    for(unsigned int i = ret.length(); i < padLength; i++)
      ret = with + ret;

    return ret;
  } // StringTool::ppad

  const size_t StringTool::pad(string &padMe, const std::string &with, const unsigned int padLength) {
    unsigned int i;			// counter

    for(i = padMe.length(); i < padLength; i++)
      padMe += with;

    return padMe.length();
  } // StringTool::pad

  const std::string StringTool::far2cel(const double convertMe) {
    char tempBuf[7];
    double temperature;

    temperature = ((convertMe - 32) / 9) * 5;

    snprintf(tempBuf, 7, "%0.2f", temperature);

    return tempBuf;
  } // far2cel

  const bool StringTool::addString(const std::string &name, const std::string &value) {
    stringMapType::iterator ptr;                  // pointer to a std::string list

    ptr = _stringList.find(name);
    if (ptr != _stringList.end()) {
      ptr->second = value;
      return true;
    } // if

    _stringList.insert( make_pair(name, value) );

    return true;
  } // StringTool::addString


  const bool StringTool::replaceString(const std::string &stringName, const std::string &addMe) {
    removeString(stringName);
    addString(stringName, addMe);

    return true;
  } // StringTool::replaceString

  const bool StringTool::removeString(const std::string &stringName) {
    stringMapType::iterator ptr;                  // pointer to a std::string list
    if ((ptr = _stringList.find(stringName)) == _stringList.end())
      return false;

    _stringList.erase(ptr);

    return true;
  } // StringTool::removeString

  const std::string StringTool::getString(const std::string &stringName) {
    stringMapType::iterator ptr;

    if ((ptr = _stringList.find(stringName)) == _stringList.end())
      return "";

    return ptr->second;
  } // StringTool::getString

  const bool StringTool::isString(const std::string &stringName) {
    stringMapType::iterator ptr;

    if ((ptr = _stringList.find(stringName)) == _stringList.end())
      return false;

    return true;
  } // StringTool::isString

  void StringTool::showStringTree(const std::string treeMember, unsigned int treeLevel, 
                              std::string treeSpacer, list<string> &treeList) {
    StringToken treeToken;                        // Tokenize a stringName.
    StringToken treeMemberToken;                  // Tokenise a treeMember name.
    stringMapType::iterator treePtr;              // iterator to a std::string map
    int numMatches;                               // number of matches
    int maxMatches;                               // max matches
    std::string stringName;                            // Name of the string.
    std::string stringValue;                           // Value of the string.
    std::string lastName;                              // Last name matched.
    std::stringstream s;                               // stringstream

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

  const unsigned int StringTool::matchStrings(const std::string &matchName, matchListType &matchList) {
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

  const std::string StringTool::float2string(const double f, const int precision) {
    std::stringstream s;

    s.str("");
    s << std::setprecision(precision) << f;

    return s.str();
  } // StringTool::float2string

  const std::string StringTool::int2string(const int i) {
    std::stringstream s;

    s.str("");
    s << i;

    return s.str();
  } // StringTool::int2string

  const std::string StringTool::uint2string(const unsigned int i) {
    std::stringstream s;

    s.str("");
    s << i;

    return s.str();
  } // StringTool::uint2string

  const bool StringTool::acceptable(const char *acceptChars, const std::string &acceptMe) {
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

  const std::string StringTool::stringf(const size_t maxsize, const char *fmt, ...) {
    char *buf;
    std::string ret = "";
    va_list va;
    int written;

    buf = new char[maxsize + 1];

    va_start(va, fmt);
    written = vsnprintf(buf, sizeof(buf), fmt, va);
    va_end(va);

    if (written)
      ret.append(buf, written);

    delete [] buf;

    return ret;
  } // StringTool::stringf

  const int StringTool::compare(const char *s1, const char *s2) {
    return (strcasecmp(s1, s2));
  } // StringTool::compare

  const std::string StringTool::getFileContents(const std::string &filename) {
    std::ifstream in;
    std::string s = "";

    in.open(filename.c_str(), std::ifstream::in);

    if (!in)
      throw StringTool_Exception("cannot open message file "+filename+"; "+strerror(errno));

    while(in.good()) {
      char c = in.get();

      if (in.good())
        s.append(1, c);
    } // while

    in.close();

    return s;
  } // StringTool::getFileContents

  const std::string StringTool::randstr(const size_t min_len, size_t max_len) {
    std::stringstream ret;

    if (min_len < 1)
      throw StringTool_Exception("min length must be greater than 1");

    if (max_len == 0)
      max_len = min_len;

    if (min_len > max_len)
      throw StringTool_Exception("min length must be less than or equal to max length");

    size_t len;
    if (min_len == max_len)
      len = min_len;
    else
      len = (rand() % (max_len - min_len)) + min_len;

    for(size_t i=0; i < len; i++) {
      char c = (char) (rand() % 94) + 32;
      ret << c;
    } // for

    return ret.str();
  } // StringTool::randstr

  const std::string StringTool::randnum(const size_t min_len, size_t max_len) {
    std::stringstream ret;

    if (min_len < 1)
      throw StringTool_Exception("min lenght must be greater than 1");

    if (max_len == 0)
      max_len = min_len;

    if (min_len > max_len)
      throw StringTool_Exception("min length must be less than or equal to max length");

    size_t len;
    if (min_len == max_len)
      len = min_len;
    else
      len = (rand() % (max_len - min_len)) + min_len;

    for(size_t i=0; i < len; i++) {
      char c = (char) (rand() % 9) + 48;
      ret << c;
    } // for

    return ret.str();
  } // StringTool::randnum

  const std::string StringTool::date() {
    std::stringstream ret;

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);

    ret << (tm->tm_year+1900)
        << "-" << std::setfill('0') << std::setw(2) << tm->tm_mon+1
        << "-" << std::setfill('0') << std::setw(2) << tm->tm_mday
        << " " << std::setfill('0') << std::setw(2) << tm->tm_hour
        << ":" << std::setfill('0') << std::setw(2) << tm->tm_min
        << ":" << std::setfill('0') << std::setw(2) << tm->tm_sec;
    return ret.str();
  } // StringTool::date

  const std::string StringTool::safe(const std::string &buf) {
    std::stringstream s;

    s.str("");
    for(size_t i=0; i < buf.length(); i++) {
      if (((int) buf[i] < 32 || (int) buf[i] > 126) && (int) buf[i] != 10)
        s << "\\x" << StringTool::char2hex(buf[i]);
      else
        s << buf[i];
    } // for

    return s.str();
  } // StringTool::safe

  const std::string StringTool::hexdump(const std::string &buf, const std::string &prefix) {
    std::stringstream out;
    std::stringstream lin;
    std::stringstream asc;

    lin.str("");

    for(size_t i=0; i < buf.length(); i++) {
      if (i) {
        if ((i % 16) == 0) {
          out << prefix
              << lin.str()
              << "  "
              << asc.str()
              << std::endl;
          asc.str("");
          lin.str("");
        } // if
        else if ((i % 8) == 0)
          lin << " ";

      } // if

      lin << StringTool::char2hex(buf[i], true) << " ";

      if (((int) buf[i] < 32 || (int) buf[i] > 126))
        asc << ".";
      else
        asc << buf[i];
    } // for

    if (lin.str().length()) {
      std::string pad = StringTool::ipad(lin.str(), " ", 16*3+1);
      out << prefix
          << pad
          << "  "
          << asc.str()
          << std::endl;
      asc.str("");
    } // if

    return out.str();
  } // StringTool::hexdump

  const bool StringTool::is_number(const std::string &str) {
    for(string::size_type i=0; i < str.length(); i++) {
      char c = str[i];
      if (c < 48 || c > 57) return false;
    } // for

    return true;
  } // StringTool::is_number

  const std::string StringTool::strtoi(const std::string &in) {
    if (in.length() % 2 != 0) throw StNotHexString_Exception();

    std::stringstream buf;
    for(string::size_type i=0; i < in.length(); i+=2) {
      std::stringstream s(in.substr(i, 2));
      unsigned short byte;
      s.flags( std::ios_base::hex );
      s >> byte;
      buf.write((char *) &byte, 1);
    } // for

    return buf.str();
  } // StringTool::strtoi

  const std::string StringTool::binary(const std::string &str) {
    std::string ret;
    for(string::size_type pos = 0; pos < str.length(); pos++) {
      ret += StringTool::binary((int) str[pos]) + string(" ");
    } // for

    return  ret;
  } // StringTool::binary

  const std::string StringTool::escape(const std::string &parseMe) {
    std::string ret;
    size_t pos;

    ret = "";

    for(pos=0; pos < parseMe.length(); pos++) {
      if (parseMe[pos] == '"' || parseMe[pos] == '\\')
        ret += string("\\") + parseMe[pos];
      else if (parseMe[pos] == '\n')
        ret += string("\\") + "n";
      else if (parseMe[pos] == '\r')
        ret += string("\\") + "r";
      else
        ret += parseMe[pos];
    } // for

    return ret;
  } // StringTool::Escape

  const char *StringTool::binary(int v) {
    static char binstr[9];
    int i ;

    binstr[8] = '\0' ;
    for (i=0; i<8; i++) {
      binstr[7-i] = v & 1 ? '1' : '0' ;
      v = v / 2 ;
    } // for

    return binstr;
  } // StringTool::binary

  void StringTool::convert8to7(unsigned char* pIn,unsigned char* pOut) {
    int nShift=7;
    *pOut=*pIn++<<1;
    while(--nShift) {
      (*pOut++)|=(*pIn>>nShift);
      *pOut=*pIn++<<(8-nShift);
    } // while
    *pOut|=*pIn;
  } // StringTool::convert8to7

  void StringTool::convert7to8(unsigned char* pIn,unsigned char* pOut) {
    int nShift=7;
    *pOut++=*pIn>>1;
    while(nShift--) {
      *pOut=(*pIn++<<nShift)&0x7f;
      (*pOut++)|=(*pIn>>(8-nShift));
    } // while
  } // StringTool::convert7to8

  const std::string StringTool::join(const std::string &str, std::list<string> j) {
    std::stringstream ret;

    list<string>::size_type i=0;
    for(list<string>::const_iterator itr=j.begin(); itr != j.end(); itr++, i++) {
      if (i) ret << str;
      ret << *itr;
    } // for

    return ret.str();
  } // StringTool::join

  const std::string StringTool::join(const std::string &str, std::vector<string> j) {
    std::stringstream ret;

    vector<string>::size_type i=0;
    for(vector<string>::const_iterator itr=j.begin(); itr != j.end(); itr++, i++) {
      if (i) ret << str;
      ret << *itr;
    } // for

    return ret.str();
  } // StringTool::join
} // namespace openframe

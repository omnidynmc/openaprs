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

#include <cstdlib>
#include <cstring>
#include <list>
#include <string>
#include <sstream>
#include <iostream>

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "openframe/LineBuffer.h"
#include "openframe/StringToken.h"
#include "openframe/StringTool.h"
#include "openframe/ConfController.h"

namespace openframe {

/**************************************************************************
 ** ConfController Class                                                  **
 **************************************************************************/

  /******************************
   ** Constructor / Destructor **
   ******************************/

  // ### Public Members ###
  ConfController::ConfController() : begin_(NULL), end_(NULL), current_(NULL) {
    lastToken_ = tokenNone;
  } // ConfController::ConfController

  ConfController::ConfController(const std::string &filename) : begin_(NULL), end_(NULL), current_(NULL) {
    lastToken_ = tokenNone;
    load(filename);
  } // ConfController::ConfController

  ConfController::~ConfController() {
  } // ConfController::~ConfController

  const unsigned int ConfController::load(const std::string &filename) {
    std::string buf;

    try {
      buf = StringTool::getFileContents(filename);
    } // try
    catch(StringTool_Exception ex) {
      throw ConfController_Exception(ex.message());
    } // catch

    parse_(buf);
    return buf.length();
  } // ConfController::load

  void ConfController::parse_(const std::string &str) {
    if (str.length() < 1)
      throw ConfController_Exception("empty config");

    conf_ = str;
    current_ = begin_ = (char *) conf_.c_str();
    end_ = begin_ + conf_.length();

    Token token(tokenObjectBegin, begin_, end_);

    readObject_(token);
  } // ConfController::parse_

  const bool ConfController::readValue_(Token &token) {
    bool isOK = true;

    switch ( token.type() ) {
      case tokenObjectBegin:
        lastToken_ = tokenNone; // reset; this is the only time we must do this
        isOK = readObject_(token);
        break;
      case tokenObjectEnd:
      case tokenTerminator:
        exitObject_(true);
        break;
      case tokenArrayBegin:
        isOK = readArray_();
        break;
      case tokenNumber:
        isOK = decodeNumber_(token);
        break;
      case tokenString:
        isOK = decodeString_(token);
        break;
      case tokenField:
        if (lastToken_ == tokenField)
          error_("field invalid characters after it, expected a value, object or array", token);

        isOK = decodeField_(token);
        if (isOK)
          enterObject_(currentField_);
        break;
      case tokenTrue:
        //currentValue() = true;
        _push_bool(node_(), true, false);
        break;
      case tokenFalse:
        //currentValue() = false;
        _push_bool(node_(), false, false);
        break;
      case tokenNull:
//        currentValue() = Value();
      case tokenComment:
        isOK = readComment_();
        break;
      default:
//        throw ConfController_Exception("syntax error: value, object or array expected");
        error_("syntax error: value, object or array expected", token);
        break;
    }

    lastToken_ = token.type();

    return isOK;
  } // ConfController::readValue_

  const bool ConfController::readArray_() {
    skipSpaces_();
    Token token;
    while( readToken_(token) ) {
       if (token.type() == tokenArraySeparator)
         continue;

       if (token.type() == tokenArrayEnd)
         break;

       bool okType = (token.type() == tokenString || token.type() == tokenTrue || token.type() == tokenFalse || token.type() == tokenNumber);
       if (!okType)
         //throw ConfController_Exception("only string, bool and number are supported in arrays");
         error_("only string, bool and number are supported in arrays", token);

       readValue_(token);

//cout << "readArray(" << *current_ << ")" << endl;

    } // while

    return true;
  } // ConfController::readArray_

  const bool ConfController::readField_() {
    char c = 0;
    while(current_ != end_) {
      c = getNextChar_();
      if (c == ' ')
        break;
//cout << "readField(" << c << ")" << endl;
    } // while

    return (c == ' ');
  } // ConfController::readField_

  const bool ConfController::decodeField_(Token &token) {
    std::string decoded;
    if ( !decodeField_(token, decoded) )
      return false;

     currentField_ = decoded;
//cout << print_() << "decodeField(" << decoded << ")" << endl;
    return true;
  } // ConfController::decodeField_

  const bool ConfController::decodeField_(Token &token, std::string &ret) {
    locationType current = token.start();	// skip '"'
    locationType end = token.end();		// do not include '"'

    std::string field = "";
    while(current != end) {
      char c = *current;
      if (c == ' ')
        break;

//cout << print_() << " decodeField(" << c << ")" << endl;

      if (!(c >= '0' && c <= '9')
          && !(c >= 'a' && c <= 'z')
          && !(c >= 'A' && c <= 'Z')
          && !in_(c, "-_."))
        //throw ConfController_Exception("field name has invalid characters");
        error_("field name has invalid characters", token);

      field.append(1, *current);
      ++current;
    } // while

//cout << print_() << " field(" << field << ")" << endl;

    ret = field;

    return (field.length() > 0);
  } // ConfController::decodeField_

  const bool ConfController::decodeString_(Token &token) {
    std::string decoded;
    if ( !decodeString_( token, decoded ) )
      return false;

     currentValue_ = decoded;
//cout << print_() << " value " << decoded << endl;

    _push_string(node_(), decoded, false);
    return true;
  } // ConfController::decodeString_

  const bool ConfController::decodeString_(Token &token, std::string &decoded) {
    decoded.reserve( token.end() - token.start() - 2 );
    locationType current = token.start() + 1; // skip '"'
    locationType end = token.end() - 1;      // do not include '"'
    while (current != end) {
       char c = *current++;

       if ( c == '"' )
         break;
       else if ( c == '\\' ) {
         if (current == end)
//           throw ConfController_Exception("empty escape sequence in string");
           error_("empty escape sequence in string", token);

          char escape = *current++;
          switch (escape) {
            case '"': decoded += '"'; break;
            case '/': decoded += '/'; break;
            case '\\': decoded += '\\'; break;
            case 'b': decoded += '\b'; break;
            case 'f': decoded += '\f'; break;
            case 'n': decoded += '\n'; break;
            case 'r': decoded += '\r'; break;
            case 't': decoded += '\t'; break;
          default:
//            throw ConfController_Exception("bad escape sequence in string");
            error_("bad escape sequence in string", token);
          } // switch
       } // else if
       else
         decoded.append(1, c);
    } // while

    return true;
  } // ConfController::decodeString_

  const bool ConfController::readToken_(Token &token) {
    skipSpaces_();
    token.start(current_);
    char c = getNextChar_();
    bool isOK = true;

//cout << print_() << " readToken(" << c << ")" << endl;

    switch(c) {
      case '{':
        token.type(tokenObjectBegin);
        break;
      case '}':
        token.type(tokenObjectEnd);
        break;
      case '[':
        token.type(tokenArrayBegin);
        break;
      case ']':
        token.type(tokenArrayEnd);
        break;
      case '"':
        token.type(tokenString);
        isOK = readString_();
        break;
      case '#':
        token.type(tokenComment);
        break;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '-':
        token.type(tokenNumber);
        readNumber_();
        break;
      case 't':
        token.type(tokenTrue);
        isOK = match_("rue", 3);
        break;
      case 'f':
        token.type(tokenFalse);
        isOK = match_("alse", 4);
        break;
      case 'n':
        token.type(tokenNull);
        isOK = match_( "ull", 3 );
        break;
      case ',':
        token.type(tokenArraySeparator);
        break;
      case ';':
        token.type(tokenTerminator);
        break;
      case 0:
        token.type(tokenEndOfStream);
        break;
      default:
        isOK = false;
        break;
    } // switch

    if (!isOK) {
      isOK = readField_();
      token.type( (isOK ? tokenField : tokenError) );
    } // if

    token.end(current_);
    return isOK;
  } // ConfController::readToken_

  char ConfController::getNextChar_() {
     if ( current_ == end_ )
        return 0;
     return *current_++;
  } // ConfController::getNextChar

  void ConfController::skipCommentTokens_(Token &token) {
    do {
      readToken_(token);
    } // do
    while( token.type() == tokenComment );
  } // ConfController::skipCommentTokens

  void ConfController::skipSpaces_() {
    while(current_ != end_) {
      char c = *current_;
      if ( c == ' '  ||  c == '\t'  ||  c == '\r'  ||  c == '\n')
        ++current_;
      else
        break;
    } // while
  } // ConfController::_skipSpaces

  void ConfController::skipComments_() {
    skipSpaces_();
    if (*current_ != '#')
      return;

    while(current_ != end_) {
      char c = getNextChar_();
      if (c == '\n')
        break;
    } // while
  } // ConfController::skipComments_

  const bool ConfController::readComment_() {
//cout << "readComment(";
    while(current_ != end_) {
      char c = getNextChar_();
      if (c == '\r' || c == '\n')
        break;
//cout << c;
    } // while
//cout << ")" << endl;
    return true;
  } // ConfController::readComment_

  const bool ConfController::readString_() {
    char c = 0;
    while (current_ != end_) {
      c = getNextChar_();
      if ( c == '\\' )
         getNextChar_();
      else if ( c == '"' )
         break;
    } // while

    return (c == '"');
  } // ConfController::readString_

  const bool ConfController::in_(const char c, const char *a) {
    for(char *ptr = (char *) a; *ptr != '\0'; ptr++) {
      if (*ptr == c)
        return true;
    } // while

    return false;
  } // ConfController::in_

  const bool ConfController::decodeNumber_(Token &token) {
    bool isDouble = false;

    locationType current = token.start();
    locationType end = token.end();
//    bool isNegative = *current == '-';

    for(locationType inspect = current; inspect != end; inspect++)
      isDouble = isDouble || in_(*inspect, ".");

    if (isDouble)
      return decodeDouble_(token);

    std::string number = "";
    while(current != end) {
      number.append(1, *current);
      ++current;
    } // while

    if (number.length() < 1)
//      throw ConfController_Exception("not a number");
      error_("not a number", token);

    currentValue_ = number;
    _push_int(node_(), atoi(currentValue_.c_str()), false);

    return true;
  } // ConfController::decodeNumber_

  const bool ConfController::decodeDouble_(Token &token) {
    locationType current = token.start();
    locationType end = token.end();
    //bool isNegative = *current == '-';
    std::string number = "";
    while(current != end) {
      number.append(1, *current);
      ++current;
    } // while

    if (number.length() < 1)
      error_("not a number", token);

    currentValue_ = number;
    _push_double(node_(), atof(currentValue_.c_str()), false);

    return true;
  } // ConfController::decodeDouble_

  void ConfController::readNumber_() {
    while (current_ != end_) {
      if (!(*current_ >= '0'  &&  *current_ <= '9')  &&
           !in_(*current_, ".eE+-") )
        break;
      ++current_;
    } // while
  } // ConfController::readNumber_

  const bool ConfController::readObject_(Token &tokenStart) {
    Token token;

//cout << numCalled << "READ OBJECT CALLED" << endl;

    while( readToken_(token) ) {
//cout << token.type() << " == " << tokenField << endl;

      if (token.type() == tokenEndOfStream)
        break;

      bool isOK = readValue_(token);

      if (!isOK || token.type() == tokenObjectEnd)
        // this object is done, bail out
        break;

    } // while

//cout << numCalled << "READ OBJECT QUIT" << endl;

   return true;
  } // ConfController::readObject_

  void ConfController::enterObject_(const std::string &name) {
    nodes_.push_back(name);
//    cout << "entering " << print_() << endl;
  } // ConfController::enterObject

  const bool ConfController::exitObject_(const bool ok) {
    if (nodes_.size() == 0)
      throw ConfController_Exception("unbalanced object terminator");

//    cout << "exiting " << print_() << endl;
    nodes_.pop_back();
    return ok;
  } // ConfController::exitObject

  const bool ConfController::match_(const char *pattern, int patternLength) {
    if (end_ - current_ < patternLength)
       return false;

    int index = patternLength;
    while (index--)
      if (current_[index] != pattern[index])
        return false;

    current_ += patternLength;

    return true;
  } // ConfController::match_


  const std::string ConfController::print_() {
    nodeType::iterator ptr;
    std::stringstream s;

    for(ptr = nodes_.begin(); ptr != nodes_.end(); ptr++) {
      if (s.str().length())
        s << ".";
      s << (*ptr);
    } // for

    s << " = ";

    return s.str();
  } // ConfController::print_

  void ConfController::error_(const std::string &message, Token &token) {
    locationType current = token.start();
    locationType end = token.end();
    std::stringstream err;

    size_t lineNum = 1;
    locationType linepos = begin_;
    while(linepos != current) {
      if (*linepos == '\n')
        lineNum++;

      ++linepos;
    } // while

    err << message << " at line #" << lineNum << " near; ";

    while(current != end) {
      if (*current != '\r' && *current != '\n')
        err << *current;
      ++current;
    } // while

    throw ConfController_Exception(err.str());
  } // ConfController::error

  const std::string ConfController::node_() {
    nodeType::iterator ptr;
    std::stringstream s;

    for(ptr = nodes_.begin(); ptr != nodes_.end(); ptr++) {
      if (s.str().length())
        s << ".";
      s << (*ptr);
    } // for

    return s.str();
  } // ConfController::node_
} // namespace opentest

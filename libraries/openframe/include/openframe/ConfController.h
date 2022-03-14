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

#ifndef LIBOPENFRAME_CONFCONTROLLER_H
#define LIBOPENFRAME_CONFCONTROLLER_H

#include <string>
#include <list>
#include <map>
#include <cassert>
#include <vector>

#include <time.h>
#include <sys/time.h>

#include "OFLock.h"
#include "OpenFrame_Abstract.h"
#include "VarController.h"
#include "noCaseCompare.h"
#include "stringify.h"

namespace openframe {
/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class ConfController_Exception : public OpenFrame_Exception {
  public:
    ConfController_Exception(const std::string message) throw() : OpenFrame_Exception(message) {
    } // ConfController_Exception

  private:
}; // class ConfController_Exception

class ConfController : public VarController {
  public:
    ConfController();
    ConfController(const std::string &);
    virtual ~ConfController();

    // ### Typedefs ###

    // ### Members ###
    virtual const unsigned int load(const std::string &);

  protected:

  private:
    // ### Type Definitions ###
    typedef char * locationType;
    typedef std::list<string> nodeType;
    typedef nodeType::size_type nodeSizeType;

    enum tokenTypeEnum {
      tokenEndOfStream = 0,
      tokenObjectBegin,
      tokenObjectEnd,
      tokenArrayBegin,
      tokenArrayEnd,
      tokenString,
      tokenNumber,
      tokenTrue,
      tokenFalse,
      tokenNull,
      tokenArraySeparator,
      tokenTerminator,
      tokenComment,
      tokenField,
      tokenNone,
      tokenError
    } _tokenType;

    class Token {
      public:
        Token() { }
        Token(tokenTypeEnum type, locationType start, locationType end) : type_(type), start_(start), end_(end) { }

        void type(tokenTypeEnum type) { type_ = type; }
        const tokenTypeEnum type() const { return type_; }
        void start(locationType start) { start_ = start; }
        const locationType start() const { return start_; }
        void end(locationType end) { end_ = end; }
        const locationType end() const { return end_; }

      private:
        tokenTypeEnum type_;
        locationType start_;
        locationType end_;
    };

    // ### Members ###
    const bool in_(const char c, const char *a);
    void parse_(const std::string &);
    char getNextChar_();
    const bool readToken_(Token &);
    void skipSpaces_();
    void skipComments_();
    const bool readString_();
    void readNumber_();
    const bool readValue_(Token &);
    const bool match_(const char *, int);
    void skipCommentTokens_(Token &);
    const bool readComment_();
    const bool readArray_();
    const bool readField_();
    const bool readField_(Token &token);
    const bool decodeNumber_(Token &);
    const bool decodeDouble_(Token &);
    const bool decodeField_(Token &);
    const bool decodeField_(Token &, std::string &);
    const bool decodeString_(Token &);
    const bool decodeString_(Token &, std::string &);
    const bool readObject_(Token &);
    const std::string print_();
    const std::string node_();

    const bool exitObject_(const bool);
    void enterObject_(const std::string &);
    void error_(const std::string &, Token &);

    // ### Variables ###
    tokenTypeEnum lastToken_;	// last token found
    std::string conf_;		// conf to parse
    locationType begin_;	// begining of conf
    locationType end_;		// end of conf
    locationType current_;	// current point in conf
    std::string currentField_;
    std::string currentValue_;
    nodeType nodes_;

};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openframe
#endif

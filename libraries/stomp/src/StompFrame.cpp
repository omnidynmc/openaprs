#include "config.h"

#include <set>
#include <string>
#include <cassert>
#include <map>
#include <new>
#include <iostream>

#include <openframe/openframe.h>

#include "StompFrame.h"
#include "Stomp_Exception.h"

namespace stomp {
  using std::stringstream;
  using std::string;
  using std::cout;
  using std::endl;

/**************************************************************************
 ** StompFrame Class                                                     **
 **************************************************************************/
  StompFrame::StompFrame(const string &command, const string &body) :
    Refcount(), _requires_resp(true), _command(command), _body(body), _created( time(NULL) ), _updated( time(NULL) ) {
    if (is_command("CONNECT"))
      _type = commandConnect;
    else if (is_command("STOMP"))
      _type = commandStomp;
    else if (is_command("CONNECTED"))
      _type = commandConnected;
    else if (is_command("SEND"))
      _type = commandSend;
    else if (is_command("SUBSCRIBE"))
      _type = commandSubscribe;
    else if (is_command("UNSUBSCRIBE"))
      _type = commandUnsubscribe;
    else if (is_command("BEGIN"))
      _type = commandBegin;
    else if (is_command("COMMIT"))
      _type = commandCommit;
    else if (is_command("ACK"))
      _type = commandAck;
    else if (is_command("DISCONNECT"))
      _type = commandDisconnect;
    else if (is_command("MESSAGE"))
      _type = commandMessage;
    else if (is_command("RECEIPT"))
      _type = commandReceipt;
    else if (is_command("ERROR"))
      _type = commandError;
    else
      _type = commandUnknown;
  } // StompFrame::StompFrame

  StompFrame::~StompFrame() {
    // free all of our headers
    for(stompHeader_itr itr = begin(); itr != end(); itr++)
      delete itr->second;

    clear();
  } // StompFrame::~StompFrame

  StompFrame &StompFrame::add_header(const string &name, const string &value) {
    if (!is_header(name))
      insert( make_pair(name, new StompHeader(name, value)) );
    return dynamic_cast<StompFrame &>(*this);
  } // StompFrame::add_header

  StompFrame &StompFrame::replace_header(const string &name, const string &value) {
    if (is_header(name)) remove_header(name);
    insert( make_pair(name, new StompHeader(name, value)) );
    return dynamic_cast<StompFrame &>(*this);
  } // StompFrame::replace_header

  StompFrame &StompFrame::remove_header(const string &name) {
      stompHeader_itr itr = find(name);
      if (itr == end()) throw StompNoSuchHeader_Exception(name);
      delete itr->second;
      erase(itr);
      return dynamic_cast<StompFrame &>(*this);
  } // StompFrame::remove_header

  StompFrame &StompFrame::copy_headers_from(const StompFrame *frame) {
    for(stompHeader_citr citr = frame->begin(); citr != frame->end(); citr++) {
      StompHeader *header = citr->second;
      replace_header(header->name(), header->value());
    } // for
    return dynamic_cast<StompFrame &>(*this);
  } // StompFrame::copy_headers_from

  const std::string StompFrame::get_header(const string &name) {
    stompHeader_itr itr = find(name);
    if (itr == end()) throw StompNoSuchHeader_Exception(name);

    return itr->second->value();
  } // StompFrame::get_header

  const std::string StompFrame::get_header(const std::string &name, const std::string &def) {
    stompHeader_itr itr = find(name);
    if (itr == end()) return def;

    return itr->second->value();
  } // StompFrame::get_header

  const bool StompFrame::get_header(const string &name, StompHeader *&header) {
    stompHeader_itr itr = find(name);
    if (itr == end()) return false;
    header = itr->second;
    return true;
  } // StompFrame::get_header

  const bool StompFrame::is_header(const string &name) {
    StompHeader *header = NULL;
    return get_header(name, header);
  } // StompFrame::is_header

  const string StompFrame::compile() {
    stringstream out;
    replace_header("Content-Length", stringify<size_t>(_body.length() ) );
    out.str("");
    out << _command
        << endl
        << headers()
        << endl
        << _body
        << '\0' << endl;
    return out.str();
  } // StompFrame::compile

  const string StompFrame::toString() const {
    stringstream out;
    out << "StompFrame "
        << "command=" << _command
        << ",body=" << _body;
//        << headersToString();

     return out.str();
  } // StompFrame::toString

  const string StompFrame::headersToString() const {
    stompHeader_t::const_iterator itr;
    stringstream out;

    for(itr = begin(); itr != end(); itr++)
      out << itr->second->toString() << endl;

    return out.str();
  } // StompFrame::headersToString

  // Private
  const stompHeaderSize_t StompFrame::headers(ByteData &buf) {
    while(buf.nextLength() != 0) {
      string line;
      try {
        line = buf.nextLine();
      } // try
      catch(std::out_of_range ex) {
        throw StompInvalidHeader_Exception();
      } // catch

      if (line.length() < 1)
        break;

      StringToken st;
      st.setDelimiter(':');
      st = line;

      if (st.size() < 2)
        throw StompInvalidHeader_Exception();

      string name = st[0];
      string value = st.trail(1);

      StompHeader *header = new StompHeader(name, value);
      insert( std::make_pair(name, header) );
    } // while

    return size();
  } // StompFrame::headers

  const string StompFrame::headers() const {
    stompHeader_t::const_iterator itr;
    stringstream ret;

    for(itr = begin(); itr != end(); itr++) {
      StompHeader *header = itr->second;
      ret << header->compile() << endl;
    } // for
    return ret.str();
  } // StompFrame::headers

 std::ostream &operator<<(std::ostream &ss, const StompFrame *frame) {
    ss << frame->toString();
    return ss;
  } // operator<<
} // namespace stomp

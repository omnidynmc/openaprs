#ifndef __MODULE_STOMPFRAME_H
#define __MODULE_STOMPFRAME_H

#include <sstream>
#include <map>
#include <string>

#include <netdb.h>
#include <unistd.h>

#include <openframe/openframe.h>

#include "Stomp_Exception.h"

namespace stomp {
  using openframe::noCaseCompare;
  using openframe::Refcount;
  using openframe::StringTool;
  using openframe::OFLock;
  using openframe::stringify;
  using openframe::ByteData;
  using openframe::StringToken;
  using std::string;
  using std::endl;
  using std::stringstream;
  using std::map;
  using std::cout;
  using std::endl;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

class StompHeader {
  public:
    StompHeader(const string &name, const string &value) : _name(name), _value(value) { }
    StompHeader(ByteData buf) {
      if (buf.length() < 1 || buf.find(':') == string::npos)
        throw StompInvalidHeader_Exception();

      StringToken st;
      st.setDelimiter(':');

      st = buf;
      if (st.size() < 1)
        throw StompInvalidHeader_Exception();

      _name = st[0];
      _value = (st.size() > 1 ? StringTool::trim( st.trail(1) ) : "");
    } // StompHeader
    virtual ~StompHeader() { }

    const string name() const { return _name; }
    const string value() const { return _value; }

    virtual const string toString() const {
      stringstream out;
      out << "Header "
          << "name=" << _name
          << ",value=" << StringTool::safe(_value);
      return out.str();
    } // toString
    virtual const string compile() const {
      stringstream out;
      out << _name << ": " << _value;
      return out.str();
    } // compile

  protected:
    string _name;
    string _value;
  private:
}; // class StompHeader

typedef map<string, StompHeader *, noCaseCompare> stompHeader_t;
typedef stompHeader_t::iterator stompHeader_itr;
typedef stompHeader_t::const_iterator stompHeader_citr;
typedef stompHeader_t::size_type stompHeaderSize_t;

class StompFrame : public Refcount, public stompHeader_t {
  public:
    enum commandEnum {
      commandConnect		= 0,
      commandStomp		= 1,
      commandConnected		= 2,
      commandSend		= 3,
      commandSubscribe		= 4,
      commandUnsubscribe	= 5,
      commandBegin		= 6,
      commandCommit		= 7,
      commandAck		= 8,
      commandDisconnect		= 9,
      commandMessage		= 10,
      commandReceipt		= 11,
      commandError		= 12,
      commandUnknown		= 999
    };

    StompFrame(const string &command, const string &body="") : Refcount(), _requires_resp(true), _command(command), _body(body), _created( time(NULL) ), _updated( time(NULL) ) {
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
    } // StompFrame
    virtual ~StompFrame() {
      // free all of our headers
      for(stompHeader_itr itr = begin(); itr != end(); itr++)
        delete itr->second;

      clear();
    } // StompFrame

    // ### Type Definitions ###

    // ### Public Members ###
    inline const string command() const { return _command; }
    inline const string body() const { return _body; }
    inline const bool is_command(const string &command) const { return (StringTool::toUpper(_command) == command); }
    inline const bool is_command(const commandEnum type) const { return (type == _type); }
    inline const commandEnum type() const { return _type; }
    inline const bool requires_resp() const { return _requires_resp; }
    inline const time_t created() const { return _created; }
    inline const time_t updated() const { return _updated; }
    inline const time_t updated_since() const { return ( time(NULL) - _updated ); }

    StompFrame &add_header(const string &name, const string &value) {
      if (!is_header(name))
        insert( make_pair(name, new StompHeader(name, value)) );
      return dynamic_cast<StompFrame &>(*this);
    } // add_header
    StompFrame &replace_header(const string &name, const string &value) {
      if (is_header(name)) remove_header(name);
      insert( make_pair(name, new StompHeader(name, value)) );
      return dynamic_cast<StompFrame &>(*this);
    } // add_header
    StompFrame &remove_header(const string &name) {
      stompHeader_itr itr;
      itr = find(name);
      if (itr == end()) throw StompNoSuchHeader_Exception(name);
      delete itr->second;
      return dynamic_cast<StompFrame &>(*this);
    } // remove_header
    const string get_header(const string &name) {
      stompHeader_itr itr;

      itr = find(name);
      if (itr == end())
        throw StompNoSuchHeader_Exception(name);

      return itr->second->value();
    } // get_header
    const bool get_header(const string &name, StompHeader *&header) {
      stompHeader_itr itr;

      itr = find(name);
      if (itr == end())
        return false;

      header = itr->second;

      return true;
    } // get_header
    const bool is_header(const string &name) {
      StompHeader *header = NULL;
      return get_header(name, header);
    } // // is_header

    virtual const size_t length() { return _body.length(); }
    virtual const string compile() {
      stringstream out;
      if (!is_header("Content-Length"))
        insert( std::make_pair("Content-Length", new StompHeader("Content-Length", stringify<size_t>(_body.length())) ) );

      out << _command
          << endl
          << headers()
          << endl
          << _body
          << '\0' << endl;
      return out.str();
    } // compile
    virtual const string toString() const {
      stringstream out;
      out << "StompFrame "
          << "command=" << _command
          << ",body=" << _body
          << endl
          << headersToString();

      return out.str();
    } // toString
    const string headersToString() const {
      stompHeader_t::const_iterator itr;
      stringstream out;

      for(itr = begin(); itr != end(); itr++)
        out << itr->second->toString() << endl;

      return out.str();
    } // headersToString

  protected:
    const stompHeaderSize_t headers(ByteData &buf) {
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
    } // headers

    const string headers() const {
      stompHeader_t::const_iterator itr;
      stringstream ret;

      for(itr = begin(); itr != end(); itr++) {
        StompHeader *header = itr->second;
        ret << header->compile() << endl;
      } // for
      return ret.str();
    } // headers

    // ### Protected Variables ###
    bool _requires_resp;

  private:
    // ### Private Variables ###
    string _command;
    string _body;
    time_t _created;
    time_t _updated;
    commandEnum _type;
};

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

extern "C" {
} // extern

}
#endif

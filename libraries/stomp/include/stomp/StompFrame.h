#ifndef __MODULE_STOMP_STOMPFRAME_H
#define __MODULE_STOMP_STOMPFRAME_H

#include <string>

#include <openframe/openframe.h>

#include "StompHeader.h"
#include "Stomp_Exception.h"

namespace stomp {
  using openframe::Refcount;
  using openframe::ByteData;
  using openframe::StringToken;
  using openframe::StringTool;
  using std::string;
  using std::map;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class StompFrame : public Refcount, public stompHeader_t {
    public:
      enum commandEnum {
        commandConnect		= 0,
        commandStomp		= 1,
        commandConnected	= 2,
        commandSend		= 3,
        commandSubscribe	= 4,
        commandUnsubscribe	= 5,
        commandBegin		= 6,
        commandCommit		= 7,
        commandAck		= 8,
        commandDisconnect	= 9,
        commandMessage		= 10,
        commandReceipt		= 11,
        commandError		= 12,
        commandUnknown		= 999
      };

      StompFrame(const string &command, const string &body="");
      virtual ~StompFrame();

      // ### Public Members ###
      inline const string command() const { return _command; }
      inline const string body() const { return _body; }
      inline const bool is_command(const string &command) const { return (StringTool::toUpper(_command) == command); }
      inline const bool is_command(const commandEnum type) const { return (type == _type); }
      inline const commandEnum type() const { return _type; }
      inline const bool requires_resp() const { return _requires_resp; }
      inline void requires_resp(const bool rr) { _requires_resp=rr; }
      inline const time_t created() const { return _created; }
      inline const time_t updated() const { return _updated; }
      inline const time_t updated_since() const { return ( time(NULL) - _updated ); }

      StompFrame &add_header(const string &name, const string &value);
      StompFrame &replace_header(const string &name, const string &value);
      StompFrame &copy_headers_from(const StompFrame *frame);
      StompFrame &remove_header(const string &name);
      const string get_header(const string &name);
      const std::string get_header(const std::string &name, const std::string &def);
      const bool get_header(const string &name, StompHeader *&header);
      const bool is_header(const string &name);

      virtual const size_t length() { return _body.length(); }
      virtual const string compile();
      virtual const string toString() const;
      const string headersToString() const;

    protected:
      const stompHeaderSize_t headers(ByteData &buf);
      const string headers() const;

      // ### Protected Variables ###
      bool _requires_resp;

    private:
      // ### Private Variables ###
      string _command;
      string _body;
      time_t _created;
      time_t _updated;
      commandEnum _type;

      stringstream _out;
  }; // class StompFrame

  std::ostream &operator<<(std::ostream &ss, const StompFrame *frame);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace stomp
#endif

#ifndef __MODULE_STOMP_STOMPMESSAGE_H
#define __MODULE_STOMP_STOMPMESSAGE_H

#include <string>

#include <openframe/openframe.h>

#include "StompFrame.h"

namespace stomp {
  using openframe::OpenFrame_Exception;
  using openframe::StringTool;
  using openframe::Refcount;
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class StompMessage : public StompFrame {
    public:
      StompMessage(const string &destination, const string &body, const time_t inactivity_timeout=0) :
        StompFrame("MESSAGE", body), _destination(destination), _body(body), _inactivity_timeout(inactivity_timeout), _created( time(NULL) ), _last_activity( time(NULL) ), _sent(0) {
        replace_header("destination", destination);
      } // StompMessage
      StompMessage(const string &destination, const string &transaction, const string &body, const time_t inactivity_timeout=0) : 
        StompFrame("MESSAGE", body), _destination(destination), _transaction(transaction), _body(body), _inactivity_timeout(inactivity_timeout), _created( time(NULL) ), _last_activity( time(NULL) ), _sent(0) {
        replace_header("destination", destination);
        replace_header("transaction", transaction);
      } // StompMessage
      virtual ~StompMessage() { }

      // ### Public Members ###
      inline const string destination() const { return _destination; }
      inline const string transaction() const { return _transaction; }
      inline const string body() const { return _body; }
      inline const string toString() const {
        stringstream out;
        out << "destination=" << _destination
            << ",transaction=" << _transaction
            << ",body=" << (_body.length() < 128 ? _body : _body.substr(0, 128)+"...");
        return out.str();
      } // toString

      inline const time_t inactivity_timeout() const { return _inactivity_timeout; }
      inline const time_t created() const { return _created; }
      inline const time_t last_activity() const { return _last_activity; }
      inline const time_t is_inactive() const { return ( _inactivity_timeout && _last_activity < (time(NULL) - _inactivity_timeout) ); }
      inline const time_t sent() const { return _sent; }
      inline const bool is_sent() const { return (_sent > 0); }
      inline void mark_sent() { _sent = time(NULL); }
      inline void mark_unsent() { _sent = 0; }

    protected:
    private:
      string _destination;
      string _transaction;
      string _body;
      time_t _inactivity_timeout;
      time_t _created;
      time_t _last_activity;
      time_t _sent;
  }; // StompMessage

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

  extern "C" {
  } // extern
} // namespace stomp
#endif

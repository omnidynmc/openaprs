#ifndef __MODULE_STOMP_STOMPMESSAGE_H
#define __MODULE_STOMP_STOMPMESSAGE_H

#include <string>
#include <list>

#include <openframe/openframe.h>

#include "StompFrame.h"

namespace stomp {
  using std::string;
  using std::list;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class StompMessage : public StompFrame {
    public:
      StompMessage(const string &destination, const string &body, const time_t inactivity_timeout=0);
      StompMessage(const string &destination, const string &transaction, const string &body, const time_t inactivity_timeout=0);
      virtual ~StompMessage();

      // ### Public Members ###
      inline const string destination() const { return _destination; }
      inline const string transaction() const { return _transaction; }
      inline const string id() const { return _id; }
      inline const bool is_id(const string &id) { return (id == _id); }
      inline const string body() const { return _body; }
      const string toString() const;

      inline time_t inactivity_timeout() const { return _inactivity_timeout; }
      inline time_t created() const { return _created; }
      inline time_t last_activity() const { return _last_activity; }
      inline bool is_inactive() const { return ( _inactivity_timeout && _last_activity < (time(NULL) - _inactivity_timeout) ); }
      inline time_t sent() const { return _sent; }
      inline bool is_sent() const { return (_sent > 0); }
      inline void mark_sent() { _sent = time(NULL); }
      inline void mark_unsent() { _sent = 0; }
      static const string create_uuid();

    protected:

    private:
      string _destination;
      string _transaction;
      string _id;
      string _body;
      time_t _inactivity_timeout;
      time_t _created;
      time_t _last_activity;
      time_t _sent;
  }; // StompMessage

  typedef list<StompMessage *> mesgList_t;
  typedef mesgList_t::iterator mesgList_itr;
  typedef mesgList_t::const_iterator mesgList_citr;
  typedef mesgList_t::size_type mesgListSize_t;

  std::ostream &operator<<(std::ostream &ss, const StompMessage *smesg);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace stomp
#endif

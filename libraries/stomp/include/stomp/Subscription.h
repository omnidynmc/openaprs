#ifndef __MODULE_STOMP_SUBSCRIPTION_H
#define __MODULE_STOMP_SUBSCRIPTION_H

#include <set>
#include <string>

#include <openframe/openframe.h>

#include "StompMessage.h"

namespace stomp {
  using std::set;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class StompPeer;
  class Subscription : public openframe::OpenFrame_Abstract, public openframe::Refcount {
    public:
      enum ackModeEnum {
        ackModeAuto		= 0,
        ackModeClient		= 1,
        ackModeClientIndv	= 2
      };

      typedef list<StompMessage *> queue_t;
      typedef queue_t::iterator queue_itr;
      typedef queue_t::const_iterator queue_citr;
      typedef queue_t::size_type queue_st;

      Subscription(StompPeer *peer, const string &id, const string &key, const ackModeEnum ack_mode=ackModeAuto);
      virtual ~Subscription();

      static const time_t kDefaultStatsInterval;

      inline Subscription &prefetch(const size_t prefetch) {
        _prefetch = prefetch;
        return *this;
      } // prefetch
      inline const size_t prefetch() const { return _prefetch; }
      inline const bool prefetch_ok() { return (_prefetch == 0 || _sentq.size() < _prefetch); }

      inline const string id() const { return _id; }
      inline const bool is_id(const string &id) const { return (id == _id); }
      inline const bool is_peer(StompPeer *peer) const { return (peer == _peer); }
      inline const string key() const { return _key; }
      inline const ackModeEnum ack_mode() const { return _ack_mode; }
      const bool match(const string &key) const;
      bool try_stats();

      void bind();
      void unbind();

      void enqueue(StompMessage *smesg);
      size_t dequeue(const string &);
      const bool dequeue_for_send(StompMessage *&smesg);
      void dequeue_all(mesgList_t &ret);

      const string toString() const;
      const string toStats() const;

      inline queue_st sendq() const { return _sendq.size(); }
      inline queue_st sentq() const { return _sentq.size(); }

    protected:
    private:
      StompPeer *_peer;
      string _id;
      string _key;
      ackModeEnum _ack_mode;
      queue_t _sendq;
      queue_t _sentq;
      size_t _prefetch;

      openframe::Stopwatch *_profile;

      // stats
      struct obj_stats_t {
        size_t num_enqueued;
        size_t num_dequeued;
        time_t report_interval;
        time_t last_stats_at;
        time_t created_at;
      } _stats; // obj_stats_t
      void init_stats(const time_t report_interval=0, const bool startup=false);
  }; // class Subscription

  std::ostream &operator<<(std::ostream &ss, const Subscription *sub);

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace stomp
#endif

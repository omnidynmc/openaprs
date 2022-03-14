#include "config.h"

#include <string>
#include <cassert>
#include <cstring>
#include <list>
#include <queue>
#include <map>
#include <new>
#include <iostream>
#include <sstream>

#include <openframe/openframe.h>

#include "StompMessage.h"
#include "Exchange.h"
#include "Subscription.h"
#include "StompPeer.h"

namespace stomp {
  using namespace openframe::loglevel;

/**************************************************************************
 ** Exchange Class                                                       **
 **************************************************************************/
  const time_t Exchange::kDefaultExpireInterval		= 5;
  const time_t Exchange::kDefaultDeferredInterval	= 2;
  const time_t Exchange::kDefaultStatsIntval		= 15;
  const time_t Exchange::kDefaultExchangeStatsInterval	= 10;
  const size_t Exchange::kDefaultByteLimit		= 3145728;

  Exchange::Exchange(const std::string &key)
           : _key(key),
             _expire_interval(kDefaultExpireInterval),
             _last_expire( time(NULL) ),
             _deferred_interval(kDefaultDeferredInterval),
             _last_dispatch(0),
             _byte_limit(kDefaultByteLimit) {
    _num_posts = 0;
    _num_bytes = 0;
    _last_stats = time(NULL);
    init_stats(kDefaultExchangeStatsInterval, true);
  } // Exchange::Exchange

  Exchange::~Exchange() {
    unbind_all();

    while( !_sendq.empty() ) {
      StompMessage *smesg = _sendq.front();
      smesg->release();
      _sendq.pop_front();
    } // while

    while( !_deferd.empty() ) {
      StompMessage *smesg = _deferd.front();
      smesg->release();
      _deferd.pop_front();
    } // while
  } // Exchange::~Exchange

  void Exchange::onDescribeStats() {
    describe_stat("num.posts", _key+"/num posts", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.dispatched", _key+"/num dispatched", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.deferred", _key+"/num deferred", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.inactive.expired", _key+"/num expired", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeSum);
    describe_stat("num.sendq", _key+"/num sendq", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeMean);
    describe_stat("num.bytes.sendq", _key+"/bytes sendq", openstats::graphTypeGauge, openstats::dataTypeInt, openstats::useTypeMean);
  } // Exchange::onDescribeStats

  void Exchange::onDestroyStats() {
    destroy_stat("*");
  } // Exchange::onDestroyStats

  void Exchange::init_stats(const time_t report_interval, const bool startup) {
    _stats.num_dispatched = 0;
    _stats.num_deferred = 0;
    _stats.num_posted = 0;
    _stats.last_stats_at = time(NULL);
    if (report_interval) _stats.report_interval = report_interval;
    if (startup) {
      _stats.num_sendq = 0;
      _stats.created_at = time(NULL);
    } // if
  } // Exchange::init_stats

  bool Exchange::try_stats() {
    if (_stats.last_stats_at > time(NULL) - _stats.report_interval) return false;

    if (_stats.num_posted) {
        LOG(LogInfo, << "Exchange posted " << _stats.num_posted
                     << " to " << this << std::endl);
    } // if

    if (_stats.num_dispatched) {
        LOG(LogInfo, << "Exchange dispatched " << _stats.num_dispatched
                     << " to " << this << std::endl);
    } // if

    if (_stats.num_deferred) {
      LOG(LogInfo, << "Exchange deferring " << _stats.num_deferred
                   << " msgs due to no matching subs; "
                   << this
                   << std::endl);
    } // if

    for(bind_itr itr = _binds.begin(); itr != _binds.end(); itr++) {
      Subscription *sub = *itr;
      sub->try_stats();
    } // for

    init_stats();
    return true;
  } // Exchange::try_stats

  bool Exchange::bind(Subscription *sub) {
    assert(sub != NULL); // bug
    bind_citr citr = _binds.find(sub);
    if (citr != _binds.end()) return false;

    sub->retain();
    sub->bind();
    _binds.insert(sub);
    LOG(LogInfo, << "Exchange bound " << sub << " to " << this << std::endl);
    return true;
  } // Exchange::bind

  void Exchange::recover_unsent(Subscription *sub) {
    // FIXME: this is ugly
    // dequeue unsent message
    mesgList_t ml;
    sub->dequeue_all(ml);
    LOG(LogInfo, << "Exchange recovering " << ml.size() << " messages; " << this << std::endl);
    while( !ml.empty() ) {
      StompMessage *smesg = ml.front();
      inc_bytes(smesg);
      LOG(LogDebug, << "Exchange recovering " << smesg << std::endl);
      _sendq.push_front( ml.front() );
      ++_stats.num_sendq;
      ml.pop_front();
    } // while
  } // Exchange::recover_unsent

  bool Exchange::unbind(Subscription *sub) {
    assert(sub != NULL); // bug

    bind_itr itr = _binds.find(sub);
    if (itr == _binds.end()) return false;
    LOG(LogInfo, << "Exchange unbound " << sub << " to " << this << std::endl);
    _binds.erase(sub);
    recover_unsent(sub);
    sub->unbind();
    sub->release();
    return true;
  } // Exchange::unbind

  Exchange::bind_st Exchange::unbind(const string &id) {
    bind_st num = 0;
    std::queue<Subscription *> r;

    for(bind_itr itr = _binds.begin(); itr != _binds.end(); itr++) {
      Subscription *sub = (*itr);
      bool remove = sub->is_id(id);
      if (!remove) continue;
      recover_unsent(sub);
      r.push(sub);
      num++;
    } // for

    while( !r.empty() ) {
      Subscription *sub = r.front();
      _binds.erase(sub);
      LOG(LogInfo, << "Exchange unbound " << sub << " to " << this << std::endl);
      sub->unbind();
      sub->release();
      r.pop();
    } // while

    return num;
  } // Exchange::unbind

  Exchange::bind_st Exchange::unbind(StompPeer *peer) {
    assert(peer != NULL);	// bug
    bind_st num = 0;
    std::queue<Subscription *> r;

    for(bind_itr itr = _binds.begin(); itr != _binds.end(); itr++) {
      Subscription *sub = (*itr);
      bool remove = sub->is_peer(peer);
      if (!remove) continue;
      LOG(LogInfo, << "Exchange unbound " << sub << " from " << this << std::endl);
      recover_unsent(sub);
      r.push(sub);
      num++;
    } // for

    while( !r.empty() ) {
      Subscription *sub = r.front();
      _binds.erase(sub);
      sub->unbind();
      sub->release();
      r.pop();
    } // while

    return num;
  } // Exchange::unbind

  Exchange::bind_st Exchange::unbind_all() {
    bind_st num = 0;
    for(bind_itr itr = _binds.begin(); itr != _binds.end(); itr++) {
      Subscription *sub = (*itr);
      recover_unsent(sub);
      sub->unbind();
      sub->release();
      num++;
    } // for

    _binds.clear();
    return num;
  } // Exchange::unbind_all

  Exchange::list_st Exchange::find_matches(const string &key, list_t &ret) {
    for(bind_itr itr = _binds.begin(); itr != _binds.end(); itr++) {
      Subscription *sub = (*itr);
      if (!sub->match(key) || !sub->prefetch_ok() ) continue;
      ret.push_back(sub);
    } // for

    return ret.size();;
  } // Exchange::find_matches

  const string Exchange::toString() const {
    std::stringstream out;
    out << "Exchange "
        << "key=" << _key
        << ",binds=" << _binds.size()
        << ",sendq=" << _sendq.size()
        << ",unackd=" << _unackd.size()
        << ",bytes=" << _num_bytes;
    return out.str();
  } // Exchange::toString

  size_t Exchange::expire_inactive(const size_t limit) {
    size_t num_expired = 0;
    size_t num_over_limit = 0;

    static time_t last_report = time(NULL);
    if (last_report < time(NULL) - 1) {
      datapoint("num.sendq", _stats.num_sendq);
      datapoint("num.bytes.sendq", _num_bytes);
      last_report = time(NULL);
    } // if

    bool is_ready =  is_next_expire() && !_sendq.empty();
    if (!is_ready) return 0;

    while( !_sendq.empty() ) {
      StompMessage *smesg = _sendq.front();
      bool is_inactive = smesg->is_inactive() || is_over_byte_limit();
      if (!is_inactive) break;

      if (!smesg->is_inactive()) num_over_limit++;
      expire(smesg);
      ++num_expired;
      _sendq.pop_front();
    } // while

    if (num_expired)
      LOG(LogWarn, << "Exchange expired "
                   << num_expired << " inactive "
                   << num_over_limit << " over byte limit ("
                   << _byte_limit << ") messages from "
                   << this << std::endl);

    _last_expire = time(NULL);
    _stats.num_sendq -= num_expired;
    datapoint("num.inactive.expired", num_expired);

    return num_expired;
  } // Exchange::expire_inactive

  void Exchange::post(StompMessage *smesg) {
    assert( smesg != NULL );	 // bug

//    bool drop = (_type == Exchange::exchangeTypeTopic && !_binds.size());
//    if (drop) return;

    smesg->retain();
    _sendq.push_back(smesg);

    datapoint("num.posts", 1);

    _num_posts++;
    _stats.num_posted++;
    _stats.num_sendq++;
    inc_bytes(smesg);
  } // Exchange::post

  size_t Exchange::inc_bytes(StompMessage *smesg) {
    _num_bytes += smesg->body().length();
    return _num_bytes;
  } // Exchange::inc_bytes

  size_t Exchange::dec_bytes(StompMessage *smesg) {
    if (smesg->body().length() > _num_bytes) assert(false); // bug
    _num_bytes -= smesg->body().length();
    return _num_bytes;
  } // Exchange::dec_bytes

  void Exchange::expire(StompMessage *smesg) {
    dispatched(smesg);
  } // Exchange::expire

  void Exchange::dispatched(StompMessage *smesg) {
    dec_bytes(smesg);
    smesg->release();
  } // Exchange::dispatched

  std::ostream &operator<<(std::ostream &ss, const Exchange *exch) {
    ss << exch->toString();
    return ss;
  } // operator<<
} // namespace stomp

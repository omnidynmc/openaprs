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

#ifndef __LIBOPENFRAME_THREADMESSAGE_H
#define __LIBOPENFRAME_THREADMESSAGE_H

#include <string>
#include <map>
#include <set>

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "openframe/OFLock.h"
#include "openframe/Refcount.h"
#include "openframe/VarController.h"

namespace openframe {
  using std::string;

/**************************************************************************
 ** General Defines                                                      **
 **************************************************************************/

/**************************************************************************
 ** Structures                                                           **
 **************************************************************************/

  class ThreadMessage : public Refcount {
    public:
      typedef unsigned int threadIdType;

      ThreadMessage() : Refcount(), _id(0) {
        var = new VarController();
      } // ThreadMessage
      ThreadMessage(const threadIdType id, const string &message="") : Refcount(), _id(id), _message(message) { var = new VarController(); }
      ThreadMessage(const string &action, const string &message) : Refcount(), _id(0), _action(action), _message(message) {
        var = new VarController();
      } // ThreadMessage
      ThreadMessage(const ThreadMessage &tm) : Refcount(), _id(tm._id), _action(tm._action), _message(tm._message) {
        var = new VarController(*(tm.var));
      } // ThreadMessage

      virtual ~ThreadMessage() { delete var; }

      const threadIdType id() const { return _id; }
      const string message() const { return _message; }
      const string action() const { return _action; }
      void message(const string &message) { _message = message; }

      VarController *var;

    private:
      threadIdType _id;		// thread id
      string _action;
      string _message;		// message
  };

  typedef list<ThreadMessage *> threadMessages_t;
  typedef map<pthread_t, threadMessages_t> threads_t;
  typedef threads_t::iterator threads_itr;
  typedef threads_t::size_type threadsSize_t;

  class ThreadMessenger : protected threads_t {
    public:
      ThreadMessenger() { }
      virtual ~ThreadMessenger() {
        clearThreadQs();
      } // ~ThreadMessenger

      // ### Members ###
      void clearThreadQs() {
        for(threads_itr itr = begin(); itr != end(); itr++) {
          threadMessages_t *tms = &itr->second;
          while( !tms->empty() ) {
            tms->front()->release();
            tms->pop_front();
          } // while
        } // for

        clear();
      } // clearThreadQs

      const bool createThreadQ(const pthread_t tid) {
        threads_itr ptr;
        _threads_l.Lock();
        ptr = find(tid);
        if (ptr != end()) {
          _threads_l.Unlock();
          return false;
        } // if

        threadMessages_t tms;
        insert( make_pair(tid, tms) );
        _threads_l.Unlock();
        return true;
      } // create

      const bool destroyThreadQ(const pthread_t tid) {
        threads_itr itr;
        _threads_l.Lock();
        itr = find(tid);
        if (itr == end()) {
          _threads_l.Unlock();
          return false;
        } // if

        threadMessages_t *tms = &itr->second;
        while( !tms->empty() ) {
          tms->front()->release();
          tms->pop_front();
        } // while

        erase(itr);
        _threads_l.Unlock();
        return true;
      } // destroy

      const bool queue(const pthread_t tid, ThreadMessage *tm) {
        threads_itr ptr;
        _threads_l.Lock();
        ptr = find(tid);
        if (ptr == end()) {
          _threads_l.Unlock();
          return false;
        } // if

        tm->retain();
        ptr->second.push_back(tm);
        _threads_l.Unlock();
        return true;
      } // queue

      const bool queue(ThreadMessage *tm) {
        threads_itr itr;

        _threads_l.Lock();
        for(itr = begin(); itr != end(); itr++) {
          tm->retain();
          itr->second.push_back(tm);
        } // for
        _threads_l.Unlock();
        return true;
      } // queue

      const threadMessages_t::size_type dequeue(const pthread_t tid, threadMessages_t &tms, const size_t limit=0) {
        threads_itr ptr;
        _threads_l.Lock();
        ptr = find(tid);
        if (ptr == end()) {
          _threads_l.Unlock();
          return 0;
        } // if

        for(size_t i=0; !ptr->second.empty(); i++) {
          if (i != 0 && i > limit)
            break;

          tms.push_back(ptr->second.front());
          ptr->second.pop_front();
        } // while

        _threads_l.Unlock();
        return true;
      } // dequeue

    protected:
    private:
      OFLock _threads_l;
  }; // class ThreadMessenger

/**************************************************************************
 ** Macro's                                                              **
 **************************************************************************/

/**************************************************************************
 ** Proto types                                                          **
 **************************************************************************/

} // namespace openframe
#endif
